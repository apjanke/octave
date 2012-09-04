/* Copyright (C) 2009 P.L. Lucas
 * Copyright (C) 2012 Jacob Dawid <jacob.dawid@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "parser.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QRegExp>
#include <QProcess>
#include <QBuffer>

parser::parser(QObject *parent)
  : QObject(parent)
{
  _compressors_map.insert ("bz2",  "bzip2 -dc \"%1\"");
  _compressors_map.insert ("gz",   "gzip -dc \"%1\"");
  _compressors_map.insert ("lzma", "lzma -dc \"%1\"");
  _compressors_map.insert ("xz",   "xz -dc \"%1\"");
  _compressors_map.insert ("Z",    "gunzip -c \"%1\"");
}

void
parser::set_info_path (QString infoPath)
{
  this->_info_path = infoPath;

  _info_files.clear ();

  QFileInfo info (infoPath);

  QString path = info.absolutePath ();
  QString fileName = info.fileName ();

  QDir infoDir (path);
  QStringList filter;
  filter.append (fileName + "*");

  _info_files = infoDir.entryInfoList (filter, QDir::Files);
  parse_info_map ();
}

QString
parser::get_info_path ()
{
  return _info_path;
}

QIODevice *
parser::open_file (QFileInfo & file_info)
{
  QIODevice *iodevice=NULL;
  if ( _compressors_map.contains(file_info.suffix ()))
    {
      QProcess gzip;
      gzip.start (_compressors_map.value (file_info.suffix ()).arg (file_info.absoluteFilePath ()));

      if (!gzip.waitForFinished ())
        return NULL;

      QByteArray result = gzip.readAll ();

      QBuffer *io = new QBuffer (this);
      io->setData (result);

      if (!io->open (QIODevice::ReadOnly | QIODevice::Text))
        return NULL;

      iodevice = io;
    }
  else
    {
      QFile *io = new QFile (file_info.absoluteFilePath ());
      if (!io->open (QIODevice::ReadOnly | QIODevice::Text))
        return NULL;
      iodevice = io;
    }

  return iodevice;
}

int
parser::is_ref (QString node)
{
  if (_ref_map.contains (node))
    {
      node_position ref = _ref_map [node];

      return ref.pos-_node_map [ref._node_name].pos;
    }
  return -1;
}

QString
parser::search_node (QString node)
{
  QFileInfo file_info;
  QString ref;

  if (_ref_map.contains (node))
    {
      ref = node;
      node = _ref_map [ref]._node_name;
    }

  if (_node_map.contains (node))
    {
      int pos = _node_map [node].pos;
      int realPos;

      real_position (pos, file_info, realPos);

      QIODevice *io = open_file (file_info);
      if (io == NULL)
        {
          return QString ();
        }

      seek (io, realPos);

      QString text = get_next_node (io);
      if (!text.isEmpty())
        {
          return text;
        }

      io->close ();
      delete io;
    }

  return QString ();
}

QString
parser::search_node (QString node, QIODevice *io)
{
  while (!io->atEnd ())
    {
      QString text = get_next_node (io);
      if(node == get_node_name (text))
        {
          return text;
        }
    }

  return QString ();
}

QString
parser::get_next_node (QIODevice *io)
{
  QString text;
  while (!io->atEnd ())
    {
      QByteArray line = io->readLine ();
      if (line.at(0) == 31)
        {
          break;
        }
      else
        {
          text.append (line);
        }
    }
  return text;
}

static QString
get_first_line (QString text)
{
  int n = text.indexOf ("\n");

  if (n < 0)
    {
      return QString ();
    }

  QString first_line = text.left (n);
  return first_line;
}

static QString
parser_node (QString text, QString node_name)
{
  QString firstLine = get_first_line (text);
  QStringList nodes = firstLine.split (",");
  for (int i = 0;i < nodes.size (); i++)
    {
      QString node = nodes.at (i).trimmed ();

      if (node.startsWith (node_name))
        {
          return node.remove (0, node_name.size ()).trimmed ();
        }
    }
  return QString ();
}

QString
parser::get_node_name (QString text)
{
  return parser_node (text, "Node:");
}

QString
parser::get_node_up (QString text)
{
  return parser_node (text, "Up:");
}

QString
parser::get_node_next (QString text)
{
  return parser_node (text, "Next:");
}

QString
parser::get_node_prev (QString text)
{
  return parser_node (text, "Prev:");
}

static void
replace_links (QString &text)
{
  QRegExp re ("(\\*[N|n]ote|\n\\*)([ |\n]+)([^:]+):([^:\\.,]*)([:,\\.])");
  int i = 0, f;

  while ( (i = re.indexIn (text,i)) != -1)
    {
      QString type     = re.cap (1);
      QString note     = re.cap (3);
      QString url_link = re.cap (4);
      QString link     = re.cap (4);

      if (url_link.isEmpty ())
        {
          url_link = note;
        }

      url_link = url_link.trimmed ();
      url_link.replace ("\n"," ");
      url_link.replace (QRegExp ("  +")," ");
      url_link.replace ("<b>","");
      url_link.replace ("</b>","");
      url_link = QUrl::toPercentEncoding (url_link, "", "'");

      QString href;
      if (type=="\n*")
        {
          href="\n<img src=':/actions/icons/bookmark.png'/>";
        }
      else
        {
          href="<img src=':/actions/icons/bookmark.png'/>";
        }
      href += re.cap (2) + "<a href='" + url_link + "'>" + note + ":" + link + re.cap (5) + "</a>";
      f = re.matchedLength ();
      text.replace (i,f,href);
      i += href.size ();
    }
}

static void
replace_colons (QString &text)
{
  QRegExp re ("`([^']+)'");
  int i = 0, f;
  while ( (i = re.indexIn (text, i)) != -1)
    {
      QString t = re.cap (1);
      QString bold = "<b>`" + t + "</b>'";

      f = re.matchedLength ();
      text.replace (i,f,bold);
      i += bold.size ();
    }
}

static void
info_to_html (QString &text)
{
  text.replace ("&", "&amp;");
  text.replace ("<", "&lt;");
  text.replace (">", "&gt;");

  text.replace ("\n* Menu:", "\n<b>Menu:</b>");
  text.replace ("*See also:*", "<b>See also:</b>");
  replace_colons (text);
  replace_links (text);
}

QString
parser::node_text_to_html (QString text, int anchorPos, QString anchor)
{
  QString nodeName = get_node_name (text);
  QString nodeUp   = get_node_up (text);
  QString nodeNext = get_node_next (text);
  QString nodePrev = get_node_prev (text);

  if (anchorPos > -1)
    {
      QString text1 = text.left (anchorPos);
      QString text2 = text.mid (anchorPos);

      int n = text1.indexOf ("\n");
      text1.remove (0, n);

      info_to_html (text1);
      info_to_html (text2);

      text = text1 + "<a name='" + anchor + "' /><img src=':/actions/icons/stop.png'>" + text2;
    }
  else
    {
      int n = text.indexOf ("\n");
      text.remove (0, n);
      info_to_html (text);
    }

  QString navigationLinks = QString (
        "<img src=':/actions/icons/arrow_right.png'/> <b>Section:</b> %1<br>"
        "<b>Previous Section:</b> <a href='%2'>%3</a><br>"
        "<b>Next Section:</b> <a href='%4'>%5</a><br>"
        "<b>Up:</b> <a href='%6'>%7</a><br>\n"
        )
      .arg (nodeName)
      .arg (QString (QUrl::toPercentEncoding (nodePrev, "", "'")))
      .arg (nodePrev)
      .arg (QString (QUrl::toPercentEncoding (nodeNext, "", "'")))
      .arg (nodeNext)
      .arg (QString (QUrl::toPercentEncoding (nodeUp, "", "'")))
      .arg (nodeUp);


  text.prepend ("<hr>\n<pre>");
  text.append ("</pre>\n<hr><hr>\n");
  text.prepend (navigationLinks);
  text.append (navigationLinks);
  text.prepend ("<html><body>\n");
  text.append ("</body></html>\n");
  return text;
}

void
parser::parse_info_map ()
{
  QRegExp re ("(Node|Ref): ([^\\0177]+)\\0177(\\d+)\n");
  QRegExp re_files ("([^:]+): (\\d+)\n");
  int foundCount = 0;

  for(int i = 0; i < _info_files.size (); i++)
    {
      QFileInfo fileInfo = _info_files.at (i);

      QIODevice *io = open_file (fileInfo);
      if (io == NULL)
        {
          continue;
        }

      QString nodeText;
      while (! (nodeText=get_next_node (io)).isEmpty () && foundCount < 2)
        {
          QString first_line = get_first_line (nodeText);
          if (first_line.startsWith ("Tag") )
            {
              foundCount++;
              int pos = 0;
              QString last_node;

              while ((pos = re.indexIn (nodeText, pos)) != -1) {
                  QString type = re.cap (1);
                  QString node = re.cap (2);
                  int index = re.cap (3).toInt ();

                  if (type == "Node")
                    {
                      node_map_item item;
                      item.pos = index;
                      _node_map [node] = item;
                      last_node = node;
                    }
                  else if (type == "Ref")
                    {
                      node_position item;
                      item._node_name = last_node;
                      item.pos = index;
                      _ref_map [node] = item;
                    }
                  pos += re.matchedLength ();
                }
              break;
            }
          else if (first_line.startsWith ("Indirect:"))
            {
              foundCount++;
              int pos = 0;

              while ( (pos = re_files.indexIn (nodeText, pos)) != -1) {
                  QString fileCap = re_files.cap (1).trimmed ();
                  int index = re_files.cap (2).toInt ();

                  info_file_item item;
                  for (int j = 0;j < _info_files.size (); j++)
                    {
                      QFileInfo info = _info_files.at (j);
                      if (info.fileName ().startsWith (fileCap))
                        {
                          item.file_info = info;
                          break;
                        }
                    }
                  item.real_size = index;
                  _info_file_real_size_list.append (item);
                  pos += re_files.matchedLength ();
                }

            }
        }
      io->close ();
      delete io;
    }
}

void
parser::real_position (int pos, QFileInfo & file_info, int & real_pos)
{
  int header = -1, sum = 0;
  for (int i = 0; i < _info_file_real_size_list.size (); i++)
    {
      info_file_item item = _info_file_real_size_list.at (i);
      if (header == -1)
        {
          file_info = item.file_info;
          header = item.real_size;
        }

      if (pos < item.real_size)
        {
          break;
        }

      file_info = item.file_info;
      sum = item.real_size;
    }
  real_pos = pos - sum + header + 2;
}

void
parser::seek (QIODevice *io, int pos)
{
  char ch;
  while (!io->atEnd () && pos > 0)
    {
      io->getChar (&ch);
      pos--;
    }
}

static void
replace (QString &text, QRegExp re, QString after)
{
  int pos = 0;

  while ( (pos = re.indexIn (text, pos)) != -1)
    {
      QString cap = text.mid (pos,re.matchedLength ());
      QString a (after);
      a = a.arg (cap);
      text.remove (pos, re.matchedLength ());
      text.insert (pos, a);
      pos += a.size ();
    }
}

QString
parser::global_search (QString text, int max_founds)
{
  QString results;
  QStringList words = text.split (" ",QString::SkipEmptyParts);

  QString re_program ("(" + words.at (0));
  for (int i = 1; i < words.size (); i++)
    {
      re_program += "|" + words.at (i);
    }
  re_program += ")";

  QRegExp re (re_program, Qt::CaseInsensitive);

  results.append ("<html><body>\n<h1>Search results</h1>\n<b>Results for:</b> ");
  results.append (text);
  results.append ("<br>\n");

  for (int i = 0; i < _info_files.size (); i++)
    {
      QFileInfo file_info = _info_files.at (i);
      QIODevice *io = open_file (file_info);
      if (io == NULL)
        {
          continue;
        }

      QString node_text;
      while ( !(node_text = get_next_node (io)).isEmpty ())
        {
          QString firstLine = get_first_line (node_text);
          QString node = get_node_name (node_text);
          if (node.isEmpty ())
            {
              continue;
            }

          int n = node_text.indexOf ("\n");
          node_text.remove (0, n);

          int pos = 0;
          int founds = 0;

          for (; founds < words.size () && node_text.indexOf (words.at (founds)) >= 0; founds++)
            { }

          if (founds<words.size ())
            {
              continue;
            }
          founds = 0;

          while ( (pos = re.indexIn (node_text, pos)) != -1 && founds < max_founds)
            {
              int line_start, line_end;
              line_start = node_text.lastIndexOf ("\n", pos);
              line_end = node_text.indexOf ("\n", pos);
              QString line = node_text.mid (line_start, line_end - line_start).trimmed ();

              if (founds == 0)
                {
                  results.append(
                        "<br>\n<img src=':/actions/icons/bookmark.png'> <a href='"
                        + QString(QUrl::toPercentEncoding(node,"","'")) +
                        "'>");
                  results.append (node);
                  results.append ("</a><br>\n");
                }

              replace (line, re, "<i>%1</i>");
              results.append (line);
              results.append ("<br>\n");

              founds++;

              pos += re.matchedLength ();
            }
        }
      io->close ();
      delete io;
    }

  results.append ("</body></html>");
  return results;
}
