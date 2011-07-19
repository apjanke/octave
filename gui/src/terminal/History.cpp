/*
    This file is part of Konsole, an X terminal.
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "History.h"

// System
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>


// Reasonable line size
#define LINE_SIZE	1024

/*
   An arbitrary long scroll.

   One can modify the scroll only by adding either cells
   or newlines, but access it randomly.

   The model is that of an arbitrary wide typewriter scroll
   in that the scroll is a serie of lines and each line is
   a serie of cells with no overwriting permitted.

   The implementation provides arbitrary length and numbers
   of cells and line/column indexed read access to the scroll
   at constant costs.

KDE4: Can we use QTemporaryFile here, instead of KTempFile?

FIXME: some complain about the history buffer comsuming the
       memory of their machines. This problem is critical
       since the history does not behave gracefully in cases
       where the memory is used up completely.

       I put in a workaround that should handle it problem
       now gracefully. I'm not satisfied with the solution.

FIXME: Terminating the history is not properly indicated
       in the menu. We should throw a signal.

FIXME: There is noticeable decrease in speed, also. Perhaps,
       there whole feature needs to be revisited therefore.
       Disadvantage of a more elaborated, say block-oriented
       scheme with wrap around would be it's complexity.
*/

//FIXME: tempory replacement for tmpfile
//       this is here one for debugging purpose.

//#define tmpfile xTmpFile

// History File ///////////////////////////////////////////

/*
  A Row(X) data type which allows adding elements to the end.
*/

HistoryFile::HistoryFile ():ion (-1), length (0), fileMap (0)
{
  if (tmpFile.open ())
    {
      tmpFile.setAutoRemove (true);
      ion = tmpFile.handle ();
    }
}

HistoryFile::~HistoryFile ()
{
  if (fileMap)
    unmap ();
}

//TODO:  Mapping the entire file in will cause problems if the history file becomes exceedingly large,
//(ie. larger than available memory).  HistoryFile::map() should only map in sections of the file at a time,
//to avoid this.
void
HistoryFile::map ()
{
  assert (fileMap == 0);

  fileMap = (char *) mmap (0, length, PROT_READ, MAP_PRIVATE, ion, 0);

  //if mmap'ing fails, fall back to the read-lseek combination
  if (fileMap == MAP_FAILED)
    {
      readWriteBalance = 0;
      fileMap = 0;
      qDebug () << ": mmap'ing history failed.  errno = " << errno;
    }
}

void
HistoryFile::unmap ()
{
  int result = munmap (fileMap, length);
  assert (result == 0);

  fileMap = 0;
}

bool
HistoryFile::isMapped ()
{
  return (fileMap != 0);
}

void
HistoryFile::add (const unsigned char *bytes, int len)
{
  if (fileMap)
    unmap ();

  readWriteBalance++;

  int rc = 0;

  rc = lseek (ion, length, SEEK_SET);
  if (rc < 0)
    {
      perror ("HistoryFile::add.seek");
      return;
    }
  rc = write (ion, bytes, len);
  if (rc < 0)
    {
      perror ("HistoryFile::add.write");
      return;
    }
  length += rc;
}

void
HistoryFile::get (unsigned char *bytes, int len, int loc)
{
  //count number of get() calls vs. number of add() calls.  
  //If there are many more get() calls compared with add() 
  //calls (decided by using MAP_THRESHOLD) then mmap the log
  //file to improve performance.
  readWriteBalance--;
  if (!fileMap && readWriteBalance < MAP_THRESHOLD)
    map ();

  if (fileMap)
    {
      for (int i = 0; i < len; i++)
	bytes[i] = fileMap[loc + i];
    }
  else
    {
      int rc = 0;

      if (loc < 0 || len < 0 || loc + len > length)
	fprintf (stderr, "getHist(...,%d,%d): invalid args.\n", len, loc);
      rc = lseek (ion, loc, SEEK_SET);
      if (rc < 0)
	{
	  perror ("HistoryFile::get.seek");
	  return;
	}
      rc = read (ion, bytes, len);
      if (rc < 0)
	{
	  perror ("HistoryFile::get.read");
	  return;
	}
    }
}

int
HistoryFile::len ()
{
  return length;
}


// History Scroll abstract base class //////////////////////////////////////


HistoryScroll::HistoryScroll (HistoryType * t):m_histType (t)
{
}

HistoryScroll::~HistoryScroll ()
{
  delete m_histType;
}

bool
HistoryScroll::hasScroll ()
{
  return true;
}

// History Scroll File //////////////////////////////////////

/* 
   The history scroll makes a Row(Row(Cell)) from
   two history buffers. The index buffer contains
   start of line positions which refere to the cells
   buffer.

   Note that index[0] addresses the second line
   (line #1), while the first line (line #0) starts
   at 0 in cells.
*/

HistoryScrollFile::HistoryScrollFile (const QString & logFileName):HistoryScroll (new HistoryTypeFile (logFileName)),
m_logFileName
(logFileName)
{
}

HistoryScrollFile::~HistoryScrollFile ()
{
}

int
HistoryScrollFile::getLines ()
{
  return index.len () / sizeof (int);
}

int
HistoryScrollFile::getLineLen (int lineno)
{
  return (startOfLine (lineno + 1) -
	  startOfLine (lineno)) / sizeof (Character);
}

bool
HistoryScrollFile::isWrappedLine (int lineno)
{
  if (lineno >= 0 && lineno <= getLines ())
    {
      unsigned char flag;
      lineflags.get ((unsigned char *) &flag, sizeof (unsigned char),
		     (lineno) * sizeof (unsigned char));
      return flag;
    }
  return false;
}

int
HistoryScrollFile::startOfLine (int lineno)
{
  if (lineno <= 0)
    return 0;
  if (lineno <= getLines ())
    {

      if (!index.isMapped ())
	index.map ();

      int res;
      index.get ((unsigned char *) &res, sizeof (int),
		 (lineno - 1) * sizeof (int));
      return res;
    }
  return cells.len ();
}

void
HistoryScrollFile::getCells (int lineno, int colno, int count,
			     Character res[])
{
  cells.get ((unsigned char *) res, count * sizeof (Character),
	     startOfLine (lineno) + colno * sizeof (Character));
}

void
HistoryScrollFile::addCells (const Character text[], int count)
{
  cells.add ((unsigned char *) text, count * sizeof (Character));
}

void
HistoryScrollFile::addLine (bool previousWrapped)
{
  if (index.isMapped ())
    index.unmap ();

  int locn = cells.len ();
  index.add ((unsigned char *) &locn, sizeof (int));
  unsigned char flags = previousWrapped ? 0x01 : 0x00;
  lineflags.add ((unsigned char *) &flags, sizeof (unsigned char));
}


// History Scroll Buffer //////////////////////////////////////
HistoryScrollBuffer::HistoryScrollBuffer (unsigned int maxLineCount):
HistoryScroll (new HistoryTypeBuffer (maxLineCount)),
_historyBuffer (),
_maxLineCount (0),
_usedLines (0),
_head (0)
{
  setMaxNbLines (maxLineCount);
}

HistoryScrollBuffer::~HistoryScrollBuffer ()
{
  delete[]_historyBuffer;
}

void
HistoryScrollBuffer::addCellsVector (const QVector < Character > &cells)
{
  _head++;
  if (_usedLines < _maxLineCount)
    _usedLines++;

  if (_head >= _maxLineCount)
    {
      _head = 0;
    }

  _historyBuffer[bufferIndex (_usedLines - 1)] = cells;
  _wrappedLine[bufferIndex (_usedLines - 1)] = false;
}

void
HistoryScrollBuffer::addCells (const Character a[], int count)
{
  HistoryLine newLine (count);
  qCopy (a, a + count, newLine.begin ());

  addCellsVector (newLine);
}

void
HistoryScrollBuffer::addLine (bool previousWrapped)
{
  _wrappedLine[bufferIndex (_usedLines - 1)] = previousWrapped;
}

int
HistoryScrollBuffer::getLines ()
{
  return _usedLines;
}

int
HistoryScrollBuffer::getLineLen (int lineNumber)
{
  Q_ASSERT (lineNumber >= 0 && lineNumber < _maxLineCount);

  if (lineNumber < _usedLines)
    {
      return _historyBuffer[bufferIndex (lineNumber)].size ();
    }
  else
    {
      return 0;
    }
}

bool
HistoryScrollBuffer::isWrappedLine (int lineNumber)
{
  Q_ASSERT (lineNumber >= 0 && lineNumber < _maxLineCount);

  if (lineNumber < _usedLines)
    {
      //kDebug() << "Line" << lineNumber << "wrapped is" << _wrappedLine[bufferIndex(lineNumber)];
      return _wrappedLine[bufferIndex (lineNumber)];
    }
  else
    return false;
}

void
HistoryScrollBuffer::getCells (int lineNumber, int startColumn, int count,
			       Character * buffer)
{
  if (count == 0)
    return;

  Q_ASSERT (lineNumber < _maxLineCount);

  if (lineNumber >= _usedLines)
    {
      memset (buffer, 0, count * sizeof (Character));
      return;
    }

  const HistoryLine & line = _historyBuffer[bufferIndex (lineNumber)];

  //kDebug() << "startCol " << startColumn;
  //kDebug() << "line.size() " << line.size();
  //kDebug() << "count " << count;

  Q_ASSERT (startColumn <= line.size () - count);

  memcpy (buffer, line.constData () + startColumn,
	  count * sizeof (Character));
}

void
HistoryScrollBuffer::setMaxNbLines (unsigned int lineCount)
{
  HistoryLine *oldBuffer = _historyBuffer;
  HistoryLine *newBuffer = new HistoryLine[lineCount];

  for (int i = 0; i < qMin (_usedLines, (int) lineCount); i++)
    {
      newBuffer[i] = oldBuffer[bufferIndex (i)];
    }

  _usedLines = qMin (_usedLines, (int) lineCount);
  _maxLineCount = lineCount;
  _head = (_usedLines == _maxLineCount) ? 0 : _usedLines - 1;

  _historyBuffer = newBuffer;
  delete[]oldBuffer;

  _wrappedLine.resize (lineCount);
}

int
HistoryScrollBuffer::bufferIndex (int lineNumber)
{
  Q_ASSERT (lineNumber >= 0);
  Q_ASSERT (lineNumber < _maxLineCount);
  Q_ASSERT ((_usedLines == _maxLineCount) || lineNumber <= _head);

  if (_usedLines == _maxLineCount)
    {
      return (_head + lineNumber + 1) % _maxLineCount;
    }
  else
    {
      return lineNumber;
    }
}


// History Scroll None //////////////////////////////////////

HistoryScrollNone::HistoryScrollNone ():HistoryScroll (new HistoryTypeNone ())
{
}

HistoryScrollNone::~HistoryScrollNone ()
{
}

bool
HistoryScrollNone::hasScroll ()
{
  return false;
}

int
HistoryScrollNone::getLines ()
{
  return 0;
}

int
HistoryScrollNone::getLineLen (int)
{
  return 0;
}

bool
HistoryScrollNone::isWrappedLine (int /*lineno */ )
{
  return false;
}

void
HistoryScrollNone::getCells (int, int, int, Character[])
{
}

void
HistoryScrollNone::addCells (const Character[], int)
{
}

void
HistoryScrollNone::addLine (bool)
{
}

// History Scroll BlockArray //////////////////////////////////////

HistoryScrollBlockArray::HistoryScrollBlockArray (size_t size):HistoryScroll (new
	       HistoryTypeBlockArray
	       (size))
{
  //m_blockArray.setHistorySize (size);	// nb. of lines.
}

HistoryScrollBlockArray::~HistoryScrollBlockArray ()
{
}

int
HistoryScrollBlockArray::getLines ()
{
  return m_lineLengths.count ();
}

int
HistoryScrollBlockArray::getLineLen (int lineno)
{
  if (m_lineLengths.contains (lineno))
    return m_lineLengths[lineno];
  else
    return 0;
}

bool
HistoryScrollBlockArray::isWrappedLine (int /*lineno */ )
{
  return false;
}

void
HistoryScrollBlockArray::getCells (int lineno, int colno,
				   int count, Character res[])
{
  /*
  if (!count)
    return;

  const Block *b = m_blockArray.at (lineno);

  if (!b)
    {
      memset (res, 0, count * sizeof (Character));	// still better than random data
      return;
    }

  assert (((colno + count) * sizeof (Character)) < ENTRIES);
  memcpy (res, b->data + (colno * sizeof (Character)),
	  count * sizeof (Character));
          */
}

void
HistoryScrollBlockArray::addCells (const Character a[], int count)
{
  /*
  Block *b = m_blockArray.lastBlock ();

  if (!b)
    return;

  // put cells in block's data
  assert ((count * sizeof (Character)) < ENTRIES);

  memset (b->data, 0, ENTRIES);

  memcpy (b->data, a, count * sizeof (Character));
  b->size = count * sizeof (Character);

  size_t res = m_blockArray.newBlock ();
  assert (res > 0);
  Q_UNUSED (res);

  m_lineLengths.insert (m_blockArray.getCurrent (), count);
  */
}

void
HistoryScrollBlockArray::addLine (bool)
{
}

//////////////////////////////////////////////////////////////////////
// History Types
//////////////////////////////////////////////////////////////////////

HistoryType::HistoryType ()
{
}

HistoryType::~HistoryType ()
{
}

//////////////////////////////

HistoryTypeNone::HistoryTypeNone ()
{
}

bool
HistoryTypeNone::isEnabled () const
{
  return false;
}

HistoryScroll *
HistoryTypeNone::scroll (HistoryScroll * old) const
{
  delete old;
  return new HistoryScrollNone ();
}

int
HistoryTypeNone::maximumLineCount () const
{
  return 0;
}

//////////////////////////////

HistoryTypeBlockArray::HistoryTypeBlockArray (size_t size):m_size (size)
{
}

bool
HistoryTypeBlockArray::isEnabled () const
{
  return true;
}

int
HistoryTypeBlockArray::maximumLineCount () const
{
  return m_size;
}

HistoryScroll *
HistoryTypeBlockArray::scroll (HistoryScroll * old) const
{
  delete old;
  return new HistoryScrollBlockArray (m_size);
}


//////////////////////////////

HistoryTypeBuffer::HistoryTypeBuffer (unsigned int nbLines):m_nbLines (nbLines)
{
}

bool
HistoryTypeBuffer::isEnabled () const
{
  return true;
}

int
HistoryTypeBuffer::maximumLineCount () const
{
  return m_nbLines;
}

HistoryScroll *
HistoryTypeBuffer::scroll (HistoryScroll * old) const
{
  if (old)
    {
      HistoryScrollBuffer *oldBuffer =
	dynamic_cast < HistoryScrollBuffer * >(old);
      if (oldBuffer)
	{
	  oldBuffer->setMaxNbLines (m_nbLines);
	  return oldBuffer;
	}

      HistoryScroll *newScroll = new HistoryScrollBuffer (m_nbLines);
      int lines = old->getLines ();
      int startLine = 0;
      if (lines > (int) m_nbLines)
	startLine = lines - m_nbLines;

      Character line[LINE_SIZE];
      for (int i = startLine; i < lines; i++)
	{
	  int size = old->getLineLen (i);
	  if (size > LINE_SIZE)
	    {
	      Character *tmp_line = new Character[size];
	      old->getCells (i, 0, size, tmp_line);
	      newScroll->addCells (tmp_line, size);
	      newScroll->addLine (old->isWrappedLine (i));
	      delete[]tmp_line;
	    }
	  else
	    {
	      old->getCells (i, 0, size, line);
	      newScroll->addCells (line, size);
	      newScroll->addLine (old->isWrappedLine (i));
	    }
	}
      delete old;
      return newScroll;
    }
  return new HistoryScrollBuffer (m_nbLines);
}

//////////////////////////////

HistoryTypeFile::HistoryTypeFile (const QString & fileName):m_fileName
  (fileName)
{
}

bool
HistoryTypeFile::isEnabled () const
{
  return true;
}

const QString &
HistoryTypeFile::getFileName () const
{
  return m_fileName;
}

HistoryScroll *
HistoryTypeFile::scroll (HistoryScroll * old) const
{
  if (dynamic_cast < HistoryFile * >(old))
    return old;			// Unchanged.

  HistoryScroll *newScroll = new HistoryScrollFile (m_fileName);

  Character line[LINE_SIZE];
  int lines = (old != 0) ? old->getLines () : 0;
  for (int i = 0; i < lines; i++)
    {
      int size = old->getLineLen (i);
      if (size > LINE_SIZE)
	{
	  Character *tmp_line = new Character[size];
	  old->getCells (i, 0, size, tmp_line);
	  newScroll->addCells (tmp_line, size);
	  newScroll->addLine (old->isWrappedLine (i));
	  delete[]tmp_line;
	}
      else
	{
	  old->getCells (i, 0, size, line);
	  newScroll->addCells (line, size);
	  newScroll->addLine (old->isWrappedLine (i));
	}
    }

  delete old;
  return newScroll;
}

int
HistoryTypeFile::maximumLineCount () const
{
  return 0;
}
