/*

Copyright (C) 2011 Michael Goffioul.

This file is part of QConsole.

Foobar is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QConsole is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QFont>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtDebug>
#include <QThread>
#include <QTimer>
#include <QToolTip>
#include <QCursor>
#include <QMessageBox>

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdarg.h>
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 
#include <windows.h>
#include <cstring>
#include <limits>

#include "QWinTerminalImpl.h"
#include "QTerminalColors.h"

// Uncomment to log activity to LOGFILENAME
// #define DEBUG_QCONSOLE
#define LOGFILENAME "QConsole.log"
// Uncomment to create hidden console window
#define HIDDEN_CONSOLE

#ifdef _MSC_VER
# pragma warning(disable : 4996)
#endif

//////////////////////////////////////////////////////////////////////////////

class QConsoleView : public QWidget
{
public:
  QConsoleView (QWinTerminalImpl* parent = 0) : QWidget (parent), q (parent) { }
  ~QConsoleView (void) { }

protected:
  void paintEvent (QPaintEvent* event) { q->viewPaintEvent (this, event); }
  void resizeEvent (QResizeEvent* event) { q->viewResizeEvent (this, event); }

private:
  QWinTerminalImpl* q;
};

//////////////////////////////////////////////////////////////////////////////

class QConsoleThread : public QThread
{
public:
  QConsoleThread (QWinTerminalImpl* console) : QThread (console), q (console) { }

protected:
  void run (void)
    { q->start (); }

private:
  QWinTerminalImpl* q;
};

//////////////////////////////////////////////////////////////////////////////

class QConsolePrivate
{
  friend class QWinTerminalImpl;

public:

  enum KeyboardCursorType
    {
      BlockCursor,
      UnderlineCursor,
      IBeamCursor
    };

  QConsolePrivate (QWinTerminalImpl* parent, const QString& cmd = QString ());
  ~QConsolePrivate (void);

  void updateConsoleSize (bool sync = false);
  void syncConsoleParameters (void);
  void grabConsoleBuffer (CHAR_INFO* buf = 0);
  void updateScrollBar (void);
  void setScrollValue (int value);
  void updateConsoleView (bool grab = true);
  void monitorConsole (void);
  void startCommand (void);
  void sendConsoleText (const QString& s);
  QRect cursorRect (void);

  void log (const char* fmt, ...);

  void closeStandardIO (int fd, DWORD stdHandleId, const char* name);
  void setupStandardIO (DWORD stdHandleId, int fd, const char* name,
                        const char* devName);

  QPoint posToCell (const QPoint& pt);
  QString getSelection (void);
  void updateSelection (void);
  void clearSelection (void);

  QColor backgroundColor (void) const;
  QColor foregroundColor (void) const;
  QColor selectionColor (void) const;
  QColor cursorColor (void) const;

  void setBackgroundColor (const QColor& color);
  void setForegroundColor (const QColor& color);
  void setSelectionColor (const QColor& color);
  void setCursorColor (const QColor& color);

private:
  QWinTerminalImpl* q;

private:
  QFont m_font;
  QString m_command;
  QConsoleColors m_colors;
  bool m_inWheelEvent;
  QString m_title;

  QSize m_charSize;
  QSize m_bufferSize;
  QRect m_consoleRect;
  QPoint m_cursorPos;
  bool m_cursorBlinking;
  bool m_hasBlinkingCursor;
  QTimer *m_blinkCursorTimer;
  KeyboardCursorType m_cursorType;

  QPoint m_beginSelection;
  QPoint m_endSelection;

  HANDLE m_stdOut;
  HWND m_consoleWindow;
  CHAR_INFO* m_buffer;
  CHAR_INFO* m_tmpBuffer;
  HANDLE m_process;

  QConsoleView* m_consoleView;
  QScrollBar* m_scrollBar;
  QTimer* m_consoleWatcher;
  QConsoleThread *m_consoleThread;

  // The delay in milliseconds between redrawing blinking text.
  static const int BLINK_DELAY = 500;
};

static void maybeSwapPoints (QPoint& begin, QPoint& end)
{
  if (end.y () < begin.y ()
      || (end.y () == begin.y () && end.x () < begin.x ()))
    qSwap (begin, end);
}

//////////////////////////////////////////////////////////////////////////////

QConsolePrivate::QConsolePrivate (QWinTerminalImpl* parent, const QString& cmd)
  : q (parent), m_command (cmd), m_hasBlinkingCursor (true),
    m_cursorType (BlockCursor), m_beginSelection (0, 0),
    m_endSelection (0, 0), m_process (NULL), m_inWheelEvent (false)
{
  log (NULL);

  // Possibly detach from any existing console
  log ("Detaching from existing console (if any)...\n");
  FreeConsole ();
  log ("Closing standard IO...\n");
  closeStandardIO (0, STD_INPUT_HANDLE, "STDIN");
  closeStandardIO (1, STD_OUTPUT_HANDLE, "STDOUT");
  closeStandardIO (2, STD_ERROR_HANDLE, "STDERR");

#ifdef HIDDEN_CONSOLE
  HWINSTA hOrigSta, hNewSta;

  // Create new (hidden) console
  hOrigSta = GetProcessWindowStation ();
  hNewSta = CreateWindowStation (NULL, 0, GENERIC_ALL, NULL);
  log ("Current Windows station: %p.\nNew Windows station: %p.\n", hOrigSta,
       hNewSta);
  if (! SetProcessWindowStation (hNewSta))
    log ("Failed to switch to new Windows station.\n");
#endif
  if (! AllocConsole ())
    log ("Failed to create new console.\n");
#ifdef HIDDEN_CONSOLE
  if (! SetProcessWindowStation (hOrigSta))
    log ("Failed to restore original Windows station.\n");
  if (! CloseWindowStation (hNewSta))
    log ("Failed to close new Windows station.\n");
#endif

  log ("New (hidden) console created.\n");

  setupStandardIO (STD_INPUT_HANDLE,  0, "STDIN",  "CONIN$");
  setupStandardIO (STD_OUTPUT_HANDLE, 1, "STDOUT", "CONOUT$");
  setupStandardIO (STD_ERROR_HANDLE,  2, "STDERR", "CONOUT$");

  log ("Standard input/output/error set up.\n");

  *stdin = *(fdopen (0, "rb"));
  *stdout = *(fdopen (1, "wb"));
  *stderr = *(fdopen (2, "wb"));

  log ("POSIX standard streams created.\n");

  setvbuf (stdin, NULL, _IONBF, 0);
  setvbuf (stdout, NULL, _IONBF, 0);
  setvbuf (stderr, NULL, _IONBF, 0);

  log ("POSIX standard stream buffers adjusted.\n");

  HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

  log ("Console allocated: hStdOut: %p\n", hStdOut);

  m_stdOut = hStdOut;
  m_consoleWindow = GetConsoleWindow ();

  // In case the console window hasn't been created hidden...
  ShowWindow (m_consoleWindow, SW_HIDE);

  CONSOLE_SCREEN_BUFFER_INFO sbi;

  GetConsoleScreenBufferInfo (hStdOut, &sbi);
  m_bufferSize = QSize (sbi.dwSize.X, qMax (sbi.dwSize.Y, (SHORT)500));
  m_consoleRect = QRect (sbi.srWindow.Left, sbi.srWindow.Top,
                         sbi.srWindow.Right - sbi.srWindow.Left + 1,
                         sbi.srWindow.Bottom - sbi.srWindow.Top + 1);
  m_cursorPos = QPoint (sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y);

  log ("Initial console parameters:\n");
  log ("  buffer size: %d x %d\n", m_bufferSize.width (),
       m_bufferSize.height ());
  log ("  window: (%d, %d) -> (%d, %d) [%d x %d]\n",
       m_consoleRect.left (), m_consoleRect.top (),
       m_consoleRect.right (), m_consoleRect.bottom (),
       m_consoleRect.width (), m_consoleRect.height ());

  wchar_t titleBuf[260];
  GetConsoleTitleW (titleBuf, sizeof (titleBuf));
  q->setWindowTitle (QString::fromWCharArray (titleBuf));

  m_font.setFamily ("Lucida Console");
  m_font.setPointSize (9);
  m_font.setStyleHint (QFont::TypeWriter);

  m_buffer = m_tmpBuffer = 0;

  m_consoleView = new QConsoleView (parent);
  m_scrollBar = new QScrollBar (Qt::Vertical, parent);

  QHBoxLayout* l = new QHBoxLayout (parent);
  l->setContentsMargins (0, 0, 0, 0);
  l->setSpacing (0);
  l->addWidget (m_consoleView, 1);
  l->addWidget (m_scrollBar, 0);

  SetConsoleTextAttribute (m_stdOut, 0xF0);

  setBackgroundColor (Qt::white);
  setForegroundColor (Qt::black);

  QPalette palette (backgroundColor ());

  m_consoleView->setPalette (palette);
  m_consoleView->setAutoFillBackground (true);

  m_consoleView->setFont (m_font);
  parent->setFocusPolicy (Qt::StrongFocus);
  parent->winId ();

  updateScrollBar ();

  m_consoleWatcher = new QTimer (parent);
  m_consoleWatcher->setInterval (10);
  m_consoleWatcher->setSingleShot (false);

  m_blinkCursorTimer = new QTimer (parent);
  QObject::connect (m_blinkCursorTimer, SIGNAL (timeout()),
                    q, SLOT (blinkCursorEvent ()));  

  QObject::connect (m_scrollBar, SIGNAL (valueChanged (int)),
                    q, SLOT (scrollValueChanged (int)));
  QObject::connect (m_consoleWatcher, SIGNAL (timeout (void)),
                    q, SLOT (monitorConsole (void)));

  m_consoleWatcher->start ();

  if (m_command.isEmpty ())
    m_consoleThread = 0;
  else
    {
      m_consoleThread = new QConsoleThread (q);
      QObject::connect (m_consoleThread, SIGNAL (finished (void)),
                        q, SIGNAL (terminated (void)));
      m_consoleThread->start ();
    }
}

//////////////////////////////////////////////////////////////////////////////

QConsolePrivate::~QConsolePrivate (void)
{
  if (m_consoleThread && m_consoleThread->isRunning () && m_process)
    {
      TerminateProcess (m_process, (UINT)-1);
      m_consoleThread->wait ();
    }
  if (m_buffer)
    delete [] m_buffer;
  if (m_tmpBuffer)
    delete [] m_tmpBuffer;
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::setupStandardIO (DWORD stdHandleId, int targetFd,
                                       const char* name, const char* devName)
{
  log ("Opening %s...\n", devName);

  int fd = open (devName, _O_RDWR | _O_BINARY);

  if (fd != -1)
    {
      if (fd != targetFd)
        {
          log ("Opened %s is not at target file descriptor %d, "
               "duplicating...\n", name, targetFd);
          if (dup2 (fd, targetFd) == -1)
            log ("Failed to duplicate file descriptor: errno=%d.\n", errno);
          if (close (fd) == -1)
            log ("Failed to close original file descriptor: errno=%d.\n",
                 errno);
        }
      else
        log ("%s opened and assigned to file descriptor %d.\n", devName, fd);
      if (! SetStdHandle (stdHandleId, (HANDLE) _get_osfhandle (targetFd)))
        log ("Failed to re-assign %s: error=%08x.\n", name, GetLastError ());
    }
  else
    log ("Failed to open %s: errno=%d.\n", devName, errno);
}

QPoint QConsolePrivate::posToCell (const QPoint& p)
{
  return QPoint (m_consoleRect.left () + p.x () / m_charSize.width (),
                 m_consoleRect.top () + p.y () / m_charSize.height ());
}

QString QConsolePrivate::getSelection (void)
{
  QString selection;

  QPoint begin = m_beginSelection;
  QPoint end = m_endSelection;

  maybeSwapPoints (begin, end);

  if (m_beginSelection != m_endSelection)
    {
      CHAR_INFO* buf;
      COORD bufSize, bufCoord;
      SMALL_RECT bufRect;
      int nr;

      nr = m_endSelection.y () - m_beginSelection.y () + 1;
      buf =  new CHAR_INFO[m_bufferSize.width () * nr];
      bufSize.X = m_bufferSize.width ();
      bufSize.Y = nr;
      bufCoord.X = 0;
      bufCoord.Y = 0;

      bufRect.Left = 0;
      bufRect.Right = m_bufferSize.width ();
      bufRect.Top = m_beginSelection.y ();
      bufRect.Bottom = m_endSelection.y ();

      if (ReadConsoleOutput (m_stdOut, buf, bufSize, bufCoord, &bufRect))
        {
          int start = m_beginSelection.x ();
          int end = (nr - 1) * m_bufferSize.width () + m_endSelection.x ();
          int lastNonSpace = -1;

          for (int i = start; i <= end; i++)
            {
              if (i && (i % m_bufferSize.width ()) == 0)
                {
                  if (lastNonSpace >= 0)
                    selection.truncate (lastNonSpace);
                  selection.append ('\n');
                  lastNonSpace = selection.length ();
                }

              QChar c (buf[i].Char.UnicodeChar);

              selection.append (c);
              if (! c.isSpace ())
                lastNonSpace = selection.length ();
            }

          if (lastNonSpace >= 0)
            selection.truncate (lastNonSpace);
        }
    }

  return selection;
}

void QConsolePrivate::updateSelection (void)
{
  QPoint begin = m_beginSelection;
  QPoint end = m_endSelection;

  maybeSwapPoints (begin, end);

  begin.rx () = 0;
  end.rx () = m_consoleRect.width ();

  m_consoleView->update ();
}

void QConsolePrivate::clearSelection (void)
{
  m_beginSelection = m_endSelection = QPoint ();

  m_consoleView->update ();
}

QColor QConsolePrivate::backgroundColor (void) const { return m_colors[15]; }
QColor QConsolePrivate::foregroundColor (void) const { return m_colors[0]; }
QColor QConsolePrivate::selectionColor (void) const { return m_colors[7]; }
QColor QConsolePrivate::cursorColor (void) const { return m_colors[8]; }

void QConsolePrivate::setBackgroundColor (const QColor& color)
{
  m_colors[15] = color;
}

void QConsolePrivate::setForegroundColor (const QColor& color)
{
  m_colors[0] = color;
}

void QConsolePrivate::setSelectionColor (const QColor& color)
{
  m_colors[7] = color;
}

void QConsolePrivate::setCursorColor (const QColor& color)
{
  m_colors[8] = color;
}

/////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::closeStandardIO (int fd, DWORD stdHandleId,
                                       const char* name)
{
  if (close (fd) == -1)
    log ("Failed to close file descriptor %d: errno=%d.\n", fd, errno);
  if (! CloseHandle (GetStdHandle (stdHandleId)))
    log ("Failed to close Win32 %s: error=%08x.\n", name, GetLastError ());
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::log (const char* fmt, ...)
{
#ifdef DEBUG_QCONSOLE
  if (fmt)
    {
      va_list l;
      FILE* flog = fopen (LOGFILENAME, "ab");

      va_start (l, fmt);
      vfprintf (flog, fmt, l);
      va_end (l);
      fclose (flog);
    }
  else
    {
      // Special case to re-initialize the log file
      FILE* flog = fopen (LOGFILENAME, "w");
      fclose (flog);
    }
#else
  Q_UNUSED (fmt);
#endif
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::updateConsoleSize (bool sync)
{
  QFontMetrics fm (m_font);
  QSize winSize = m_consoleView->size ();

  m_charSize.rwidth () = fm.averageCharWidth ();
  m_charSize.rheight () = fm.lineSpacing ();

  m_consoleRect.setWidth (winSize.width () / fm.averageCharWidth ());
  m_consoleRect.setHeight (winSize.height () / fm.lineSpacing ());

  m_bufferSize.rwidth () = m_consoleRect.width ();
  m_bufferSize.rheight () = qMax (m_bufferSize.height (),
                                  m_consoleRect.height ());

  m_consoleRect.moveLeft (0);
  if (m_consoleRect.bottom () >= m_bufferSize.height ())
    m_consoleRect.moveTop (m_bufferSize.height () - m_consoleRect.height ());

  log ("Console resized:\n");
  log ("  widget size: %d x %d\n", winSize.width (), winSize.height ());
  log ("  buffer size: %d x %d\n", m_bufferSize.width (),
       m_bufferSize.height ());
  log ("  window: (%d, %d) -> (%d, %d) [%d x %d]\n",
       m_consoleRect.left (), m_consoleRect.top (),
       m_consoleRect.right (), m_consoleRect.bottom (),
       m_consoleRect.width (), m_consoleRect.height ());

  if (sync)
    syncConsoleParameters ();

  updateScrollBar ();
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::syncConsoleParameters (void)
{
  CONSOLE_SCREEN_BUFFER_INFO sbi;
  HANDLE hStdOut = m_stdOut;

  GetConsoleScreenBufferInfo (hStdOut, &sbi);

  COORD bs;
  SMALL_RECT sr;

  bs.X = sbi.dwSize.X;
  bs.Y = m_bufferSize.height ();
  sr.Left   = sbi.srWindow.Left;
  sr.Right  = sbi.srWindow.Right;
  sr.Top    = m_consoleRect.top ();
  sr.Bottom = m_consoleRect.bottom ();

  if (bs.Y > sbi.dwSize.Y)
    {
      SetConsoleScreenBufferSize (hStdOut, bs);
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
    }
  else
    {
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
      SetConsoleScreenBufferSize (hStdOut, bs);
    }

  bs.X = m_bufferSize.width ();
  sr.Left  = m_consoleRect.left ();
  sr.Right = m_consoleRect.right ();

  if (bs.X > sbi.dwSize.X)
    {
      SetConsoleScreenBufferSize (hStdOut, bs);
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
    }
  else
    {
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
      SetConsoleScreenBufferSize (hStdOut, bs);
    }

  log ("Sync'ing console parameters:\n");
  log ("  buffer size: %d x %d\n", bs.X, bs.Y);
  log ("  window: (%d, %d) -> (%d, %d)\n",
       sr.Left, sr.Top, sr.Right, sr.Bottom);

  if (m_buffer)
    delete [] m_buffer;
  if (m_tmpBuffer)
    delete [] m_tmpBuffer;

  int bufSize = m_consoleRect.width () * m_consoleRect.height ();

  m_buffer = new CHAR_INFO[bufSize];
  m_tmpBuffer = new CHAR_INFO[bufSize];
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::grabConsoleBuffer (CHAR_INFO* buf)
{
  COORD bs, bc;
  SMALL_RECT r;

  bs.X = m_consoleRect.width ();
  bs.Y = m_consoleRect.height ();
  bc.X = 0;
  bc.Y = 0;

  r.Left   = m_consoleRect.left ();
  r.Top    = m_consoleRect.top ();
  r.Right  = m_consoleRect.right ();
  r.Bottom = m_consoleRect.bottom ();

  if (! ReadConsoleOutput (m_stdOut, (buf ? buf : m_buffer), bs, bc, &r))
    qCritical ("cannot read console output");
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::updateScrollBar (void)
{
  m_scrollBar->setMinimum (0);
  if (m_bufferSize.height () > m_consoleRect.height ())
    m_scrollBar->setMaximum (m_bufferSize.height () - m_consoleRect.height ());
  else
    m_scrollBar->setMaximum (0);
  m_scrollBar->setSingleStep (1);
  m_scrollBar->setPageStep (m_consoleRect.height ());
  m_scrollBar->setValue (m_consoleRect.top ());

  log ("Scrollbar parameters updated: %d/%d/%d/%d\n",
       m_scrollBar->minimum (), m_scrollBar->maximum (),
       m_scrollBar->singleStep (), m_scrollBar->pageStep ());
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::setScrollValue (int value)
{
  if (value == m_consoleRect.top ())
    return;

  SMALL_RECT r;
  HANDLE hStdOut = m_stdOut;

  if (value + m_consoleRect.height () > m_bufferSize.height ())
    value = m_bufferSize.height () - m_consoleRect.height ();

  r.Left = m_consoleRect.left ();
  r.Top = value;
  r.Right = m_consoleRect.right ();
  r.Bottom = value + m_consoleRect.height () - 1;

  log ("Scrolling window: (%d, %d) -> (%d, %d) [%d x %d]\n",
       r.Left, r.Top, r.Right, r.Bottom, 
       r.Right - r.Left + 1, r.Bottom - r.Top + 1);

  if (SetConsoleWindowInfo (hStdOut, TRUE, &r))
    {
      m_consoleRect.moveTop (value);
      updateConsoleView ();
    }
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::updateConsoleView (bool grab)
{
  if (grab)
    grabConsoleBuffer ();
  m_consoleView->update ();
  m_consoleWatcher->start ();
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::monitorConsole (void)
{
  CONSOLE_SCREEN_BUFFER_INFO sbi;
  HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

  static wchar_t titleBuf[260];

  GetConsoleTitleW (titleBuf, sizeof (titleBuf));
  QString title = QString::fromWCharArray (titleBuf);

  if (title != m_title)
    {
      q->setWindowTitle (title);
      emit q->titleChanged (title);
    }

  if (GetConsoleScreenBufferInfo (hStdOut, &sbi))
    {
      if (m_bufferSize.width () != sbi.dwSize.X
          || m_bufferSize.height () != sbi.dwSize.Y)
        {
          // Buffer size changed
          m_bufferSize.rwidth () = sbi.dwSize.X;
          m_bufferSize.rheight () = sbi.dwSize.Y;
          updateScrollBar ();
        }

      if (m_cursorPos.x () != sbi.dwCursorPosition.X
          || m_cursorPos.y () != sbi.dwCursorPosition.Y)
        {
          // Cursor position changed
          m_consoleView->update
            ((m_cursorPos.x () - sbi.srWindow.Left) * m_charSize.width (),
             (m_cursorPos.y () - sbi.srWindow.Top) * m_charSize.height (),
             m_charSize.width (), m_charSize.height ());
          m_cursorPos.rx () = sbi.dwCursorPosition.X;
          m_cursorPos.ry () = sbi.dwCursorPosition.Y;
          m_consoleView->update
            ((m_cursorPos.x () - sbi.srWindow.Left) * m_charSize.width (),
             (m_cursorPos.y () - sbi.srWindow.Top) * m_charSize.height (),
             m_charSize.width (), m_charSize.height ());
        }

      if (m_consoleRect.left () != sbi.srWindow.Left
          || m_consoleRect.right () != sbi.srWindow.Right
          || m_consoleRect.top () != sbi.srWindow.Top
          || m_consoleRect.bottom () != sbi.srWindow.Bottom)
        {
          // Console window changed
          m_consoleRect = QRect (sbi.srWindow.Left, sbi.srWindow.Top,
                                 sbi.srWindow.Right - sbi.srWindow.Left + 1,
                                 sbi.srWindow.Bottom - sbi.srWindow.Top + 1);
          updateScrollBar ();
          updateConsoleView ();
          return;
        }

      if (m_tmpBuffer && m_buffer)
        {
          grabConsoleBuffer (m_tmpBuffer);
          if (memcmp (m_tmpBuffer, m_buffer,
                      sizeof (CHAR_INFO) * m_consoleRect.width () *
                      m_consoleRect.height ()))
            {
              // FIXME: compute the area to update based on the
              // difference between the 2 buffers.
              qSwap (m_buffer, m_tmpBuffer);
              updateConsoleView (false);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::startCommand (void)
{
  QString cmd = m_command;

  if (cmd.isEmpty ())
    cmd = qgetenv ("COMSPEC").constData ();

  if (! cmd.isEmpty ())
    {
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory (&si, sizeof (si));
      si.cb = sizeof (si);
      ZeroMemory (&pi, sizeof (pi));

      if (CreateProcessW (NULL,
                          (LPWSTR)cmd.unicode (),
                          NULL,
                          NULL,
                          TRUE,
                          0,
                          NULL,
                          NULL,
                          &si,
                          &pi))
        {
          CloseHandle (pi.hThread);
          m_process = pi.hProcess;
          WaitForSingleObject (m_process, INFINITE);
          CloseHandle (m_process);
          m_process = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void QConsolePrivate::sendConsoleText (const QString& s)
{
  // Send the string in chunks of 512 characters. Each character is
  // translated into an equivalent keypress event.

#define TEXT_CHUNK_SIZE 512

  int len = s.length ();
  INPUT_RECORD events[TEXT_CHUNK_SIZE];
  DWORD nEvents = 0, written;
  HANDLE hStdIn = GetStdHandle (STD_INPUT_HANDLE);

  ZeroMemory (events, sizeof (events));

  for (int i = 0; i < len; i++)
    {
      QChar c = s.at (i);

      if (c == L'\r' || c == L'\n')
        {
          if (c == L'\r' && i < (len - 1) && s.at (i+1) == L'\n')
            i++;
          if (nEvents)
            {
              WriteConsoleInput (hStdIn, events, nEvents, &written);
              nEvents = 0;
              ZeroMemory (events, sizeof (events));
            }
          PostMessage (m_consoleWindow, WM_KEYDOWN, VK_RETURN, 0x001C0001);
          PostMessage (m_consoleWindow, WM_KEYDOWN, VK_RETURN, 0xC01C0001);
        }
      else
        {
          events[nEvents].EventType                        = KEY_EVENT;
          events[nEvents].Event.KeyEvent.bKeyDown          = TRUE;
          events[nEvents].Event.KeyEvent.wRepeatCount      = 1;
          events[nEvents].Event.KeyEvent.wVirtualKeyCode   =
            LOBYTE (VkKeyScan (c.unicode ()));
          events[nEvents].Event.KeyEvent.wVirtualScanCode  = 0;
          events[nEvents].Event.KeyEvent.uChar.UnicodeChar = c.unicode ();
          events[nEvents].Event.KeyEvent.dwControlKeyState = 0;
          nEvents++;
        }

      if (nEvents == TEXT_CHUNK_SIZE
          || (nEvents > 0 && i == (len - 1)))
        {
          WriteConsoleInput (hStdIn, events, nEvents, &written);
          nEvents = 0;
          ZeroMemory (events, sizeof (events));
        }
    }
}

QRect
QConsolePrivate::cursorRect (void)
{
  int cw = m_charSize.width ();
  int ch = m_charSize.height ();

  return QRect ((m_cursorPos.x () - m_consoleRect.x ()) * cw,
                (m_cursorPos.y () - m_consoleRect.y ()) * ch,
                cw, ch);
}

//////////////////////////////////////////////////////////////////////////////

QWinTerminalImpl::QWinTerminalImpl (QWidget* parent)
    : QTerminalInterface (parent), d (new QConsolePrivate (this))
{
}

//////////////////////////////////////////////////////////////////////////////

QWinTerminalImpl::QWinTerminalImpl (const QString& cmd, QWidget* parent)
    : QTerminalInterface (parent), d (new QConsolePrivate (this, cmd))
{
}

//////////////////////////////////////////////////////////////////////////////

QWinTerminalImpl::~QWinTerminalImpl (void)
{
  delete d;
}

void QWinTerminalImpl::mouseMoveEvent (QMouseEvent *event)
{
  d->m_endSelection = d->posToCell (event->pos ());

  updateSelection ();
}

void QWinTerminalImpl::mousePressEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
    d->m_beginSelection = d->posToCell (event->pos ());
}

void QWinTerminalImpl::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
    {
      d->m_endSelection = d->posToCell (event->pos ());

      updateSelection ();
    }
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::viewResizeEvent (QConsoleView*, QResizeEvent*)
{
  d->updateConsoleSize (true);
  d->grabConsoleBuffer ();
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::viewPaintEvent (QConsoleView* w, QPaintEvent* event)
{
  QPainter p (w);
  int cw = d->m_charSize.width (), ch = d->m_charSize.height ();
  int ascent, stride, cx1, cy1, cx2, cy2, x, y;
  WORD attr = 0;
  QString s;
  bool hasChar = false;

  QRect updateRect = event->rect ();

  cx1 = updateRect.left () / cw;
  cy1 = updateRect.top () / ch;
  cx2 = qMin (d->m_consoleRect.width () - 1, updateRect.right () / cw);
  cy2 = qMin (d->m_consoleRect.height () - 1, updateRect.bottom () / ch);

  if (cx1 > d->m_consoleRect.width () - 1
      || cy1 > d->m_consoleRect.height () - 1)
    return;

  p.setFont (d->m_font);
  p.setPen (d->foregroundColor ());

  ascent = p.fontMetrics ().ascent ();
  stride = d->m_consoleRect.width ();

  s.reserve (cx2 - cx1 + 1);
  y = ascent + cy1 * ch;;

  QPoint begin = d->m_beginSelection;
  QPoint end = d->m_endSelection;

  bool haveSelection = (begin != end);

  if (haveSelection)
    maybeSwapPoints (begin, end);

  if (haveSelection)
    d->log ("cy1: %d, cy2: %d, begin.y: %d, end.y: %d\n",
            cy1, cy2, begin.y (), end.y ());

  int scrollOffset = d->m_consoleRect.top ();

  begin.ry () -= scrollOffset;
  end.ry () -= scrollOffset;

  for (int j = cy1; j <= cy2; j++, y += ch)
    {
      // Reset string buffer and starting X coordinate
      s.clear ();
      hasChar = false;
      x = cx1 * cw;

      int charsThisLine = 0;

      for (int i = cx1; i <= cx2; i++)
        {
          CHAR_INFO* ci = &(d->m_buffer[stride*j+i]);

          if ((ci->Attributes & 0x00ff) != attr)
            {
              // Character attributes changed
              if (! s.isEmpty ())
                {
                  // String buffer not empty -> draw it
                  if (hasChar || (attr & 0x00f0))
                    {
                      if (attr & 0x00f0)
                        p.fillRect (x, y-ascent, s.length () * cw, ch,
                                    p.brush ());
                      p.drawText (x, y, s);
                    }
                  int len = s.length ();
                  charsThisLine += len;
                  x += (len * cw);
                  s.clear ();
                  hasChar = false;
                }
              // Update current pen and store current attributes
              // FIXME: what about background?
              attr = (ci->Attributes & 0x00ff);
              p.setPen (d->m_colors[attr & 0x000f]);
              p.setBrush (d->m_colors[(attr >> 4) & 0x000f]);
            }

          // Append current character to the string buffer
          s.append (ci->Char.UnicodeChar);
          if (ci->Char.UnicodeChar != L' ')
            hasChar = true;
        }

      if (! s.isEmpty () && (hasChar || (attr & 0x00f0)))
        {
          int len = s.length ();
          charsThisLine += len;

          // Line end reached, but string buffer not empty -> draw it
          // No need to update s or x, they will be reset on the next
          // for-loop iteration

          if (attr & 0x00f0)
            p.fillRect (x, y-ascent, len * cw, ch, p.brush ());

          p.drawText (x, y, s);
        }

      if (haveSelection && j >= begin.y () && j <= end.y ())
        {
          int selectionBegin = j == begin.y () ? begin.x (): 0;

          int len = ((j == end.y () && end.x () < charsThisLine)
                     ? end.x () - selectionBegin + 1
                     : stride - selectionBegin);

          QColor color = d->selectionColor ();

          p.save ();

          p.setCompositionMode (QPainter::RasterOp_SourceXorDestination);

          p.fillRect (selectionBegin * cw, y-ascent, len * cw, ch, color);

          p.restore ();
        }
    }

  if (! d->m_cursorBlinking)
    {
      QColor color = d->cursorColor ();
      QRect cursorRect = d->cursorRect ();

      p.setPen (d->foregroundColor ());

      if (d->m_cursorType == QConsolePrivate::BlockCursor)
        {
          if (hasFocus ())
            {
              p.setCompositionMode (QPainter::RasterOp_SourceXorDestination);

              p.fillRect (cursorRect, color);
            }
          else
            {
              // draw the cursor outline, adjusting the area so that
              // it is draw entirely inside 'rect'
 
              int penWidth = qMax (1, p.pen().width());
 
              p.setBrush (Qt::NoBrush);

              p.drawRect (cursorRect.adjusted (penWidth/2, penWidth/2,
                                               - penWidth/2 - penWidth%2,
                                               - penWidth/2 - penWidth%2));
            }
        }
      else if (d->m_cursorType == QConsolePrivate::UnderlineCursor)
        {
          p.drawLine (cursorRect.left (), cursorRect.bottom (),
                      cursorRect.right (), cursorRect.bottom ());
        }
      else if (d->m_cursorType == QConsolePrivate::IBeamCursor)
        {
          p.drawLine (cursorRect.left (), cursorRect.top (),
                      cursorRect.left (), cursorRect.bottom ());
        }
    }
}

void QWinTerminalImpl::blinkCursorEvent (void)
{
  if (d->m_hasBlinkingCursor)
    d->m_cursorBlinking = ! d->m_cursorBlinking;
  else
    d->m_cursorBlinking = false;

  d->m_consoleView->update (d->cursorRect ());
}

void QWinTerminalImpl::setBlinkingCursor (bool blink)
{
  d->m_hasBlinkingCursor = blink;

  setBlinkingCursorState (blink);
}

void QWinTerminalImpl::setBlinkingCursorState (bool blink)
{
  if (blink && ! d->m_blinkCursorTimer->isActive ())
    d->m_blinkCursorTimer->start (d->BLINK_DELAY);

  if (! blink && d->m_blinkCursorTimer->isActive ())
    {
      d->m_blinkCursorTimer->stop ();

      if (d->m_cursorBlinking)
        blinkCursorEvent ();
    }
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::wheelEvent (QWheelEvent* event)
{
  if (! d->m_inWheelEvent)
    {
      // Forward to the scrollbar (avoid recursion)
      d->m_inWheelEvent = true;
      QApplication::sendEvent (d->m_scrollBar, event);
      d->m_inWheelEvent = false;
    }
}

//////////////////////////////////////////////////////////////////////////////

bool QWinTerminalImpl::winEvent (MSG* msg, long* result)
{
  switch (msg->message)
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
    //case WM_CHAR:
      // Forward Win32 message to the console window
      PostMessage (d->m_consoleWindow,
                   msg->message,
                   msg->wParam,
                   msg->lParam);
      result = 0;
      return true;
    default:
      return false;
    }
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::scrollValueChanged (int value)
{
  d->setScrollValue (value);
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::monitorConsole (void)
{
  d->monitorConsole ();
}

void QWinTerminalImpl::updateSelection (void)
{
  d->updateSelection ();
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::focusInEvent (QFocusEvent* event)
{
  setBlinkingCursorState (true);

  QWidget::focusInEvent (event);
}

void QWinTerminalImpl::focusOutEvent (QFocusEvent* event)
{
  // Force the cursor to be redrawn.
  d->m_cursorBlinking = true;

  setBlinkingCursorState (false);

  QWidget::focusOutEvent (event);
}

void QWinTerminalImpl::keyPressEvent (QKeyEvent* event)
{
  if (d->m_hasBlinkingCursor)
    {
      d->m_blinkCursorTimer->start (d->BLINK_DELAY);

      if (d->m_cursorBlinking)
        blinkCursorEvent ();
    }

  QWidget::keyPressEvent (event);
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::start (void)
{
  d->startCommand ();
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::sendText (const QString& s)
{
  d->sendConsoleText (s);
}

void QWinTerminalImpl::setCursorType (CursorType type, bool blinking)
{
  switch (type)
    {
    case UnderlineCursor:
      d->m_cursorType = QConsolePrivate::UnderlineCursor;
      break;

    case BlockCursor:
      d->m_cursorType = QConsolePrivate::BlockCursor;
      break;

    case IBeamCursor:
      d->m_cursorType = QConsolePrivate::IBeamCursor;
      break;
    }

  setBlinkingCursor (blinking);
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::setTerminalFont (const QFont& f)
{
  d->m_font = f;
  d->m_consoleView->setFont (f);
  d->updateConsoleSize (true);
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::setSize (int columns, int lines)
{
  Q_UNUSED (columns);
  Q_UNUSED (lines);
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::copyClipboard (void)
{
  QClipboard *clipboard = QApplication::clipboard ();

  clipboard->setText (d->getSelection ());

  d->clearSelection ();
}

//////////////////////////////////////////////////////////////////////////////

void QWinTerminalImpl::pasteClipboard (void)
{
  QString text = QApplication::clipboard()->text (QClipboard::Clipboard);

  if (! text.isEmpty ())
    sendText (text);
}
