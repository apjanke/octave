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

#ifndef FILEEDITORMDISUBWINDOW_H
#define FILEEDITORMDISUBWINDOW_H

#include "main-window.h"
#include "file-editor-interface.h"
#include "file-editor-tab.h"

#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QTabWidget>
#include <Qsci/qsciapis.h>
// Not available in the Debian repos yet!
// #include <Qsci/qscilexeroctave.h>
#include "lexer-octave-gui.h"

const char UNNAMED_FILE[]     = "<unnamed>";
const char SAVE_FILE_FILTER[] = "Octave Files (*.m);;All Files (*.*)";
enum MARKER
  {
    MARKER_BOOKMARK,
    MARKER_BREAKPOINT
  };

class file_editor : public file_editor_interface
{
Q_OBJECT

public:
  file_editor (QTerminal *terminal, main_window *mainWindow);
  ~file_editor ();
  void loadFile (QString fileName);

  lexer_octave_gui *lexer ();
  QTerminal *       terminal ();
  main_window *     mainWindow ();

public slots:
  void request_new_file ();
  void request_open_file ();
  void request_open_file (QString fileName);

  void handle_entered_debug_mode ();
  void handle_quit_debug_mode ();

  void request_undo ();
  void request_redo ();
  void request_copy ();
  void request_cut ();
  void request_paste ();
  void request_save_file ();
  void request_save_file_as ();
  void request_run_file ();
  void request_toggle_bookmark ();
  void request_next_bookmark ();
  void request_previous_bookmark ();
  void request_remove_bookmark ();
  void request_comment_selected_text ();
  void request_uncomment_selected_text ();

  void handle_file_name_changed (QString fileName);
  void handle_tab_close_request (int index);
  void handle_tab_close_request ();
  void active_tab_changed (int index);
  void handle_editor_state_changed ();

private:
  void construct ();
  void add_file_editor_tab(file_editor_tab *f);
  file_editor_tab *active_editor_tab();

  QMenuBar *        _menu_bar;
  QToolBar *        _tool_bar;
  QMenu *           _debug_menu;
  QAction*          _copy_action;
  QAction*          _cut_action;
  QTabWidget *      _tab_widget;
  int               _marker_bookmark;
  lexer_octave_gui *_lexer;
  QsciAPIs *        _lexer_api;
};

#endif // FILEEDITORMDISUBWINDOW_H
