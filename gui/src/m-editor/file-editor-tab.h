/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 Jacob Dawid (jacob.dawid@googlemail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILEEDITORTAB_H
#define FILEEDITORTAB_H

#include <Qsci/qsciscintilla.h>
#include <QWidget>
#include <QCloseEvent>
#include <QFileSystemWatcher>

class file_editor;
class file_editor_tab : public QWidget
{
  Q_OBJECT
public:
  file_editor_tab (file_editor *fileEditor);
  bool copy_available ();

public slots:
  void update_window_title(bool modified);
  void handle_copy_available(bool enableCopy);
  void handle_margin_clicked (int line, int margin, Qt::KeyboardModifiers state);
  void comment_selected_text ();
  void uncomment_selected_text ();
  void remove_bookmark ();
  void toggle_bookmark ();
  void next_bookmark ();
  void previous_bookmark ();
  void cut ();
  void copy ();
  void paste ();
  void undo ();
  void redo ();

  void set_modified (bool modified = true);

  bool open_file ();
  void load_file (QString fileName);
  void new_file ();
  bool save_file ();
  bool save_file(QString saveFileName);
  bool save_file_as();
  void run_file ();

  void file_has_changed (QString fileName);

signals:
  void file_name_changed (QString fileName);
  void editor_state_changed ();
  void close_request ();

protected:
  void closeEvent (QCloseEvent *event);
  void set_file_name (QString fileName);

private:
  void update_tracked_file ();
  int check_file_modified (QString msg, int cancelButton);
  void do_comment_selected_text (bool comment);

  file_editor *         _file_editor;
  QsciScintilla *       _edit_area;

  QString               _file_name;
  QString               _file_name_short;

  bool                  _long_title;
  bool                  _copy_available;

  QFileSystemWatcher    _file_system_watcher;
};

#endif // FILEEDITORTAB_H
