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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QtGui/QMainWindow>
#include <QThread>
#include <QTabWidget>
#include <QMdiArea>
#include <QStatusBar>
#include <QToolBar>
#include <QQueue>
#include <QMdiSubWindow>
#include <QCloseEvent>
#include <QToolButton>
#include <QComboBox>

// Editor includes
#include "file-editor-interface.h"

// QTerminal includes
#include "QTerminal.h"

// Own includes
#include "resource-manager.h"
#include "octave-link.h"
#include "workspace-view.h"
#include "history-dockwidget.h"
#include "files-dockwidget.h"
#include "terminal-dockwidget.h"
#include "octave-qt-event-listener.h"
#include "octave-event-observer.h"

/**
  * \class MainWindow
  *
  * Represents the main window.
  */
class main_window
    : public QMainWindow, public octave_event_observer
{
Q_OBJECT public:
  main_window (QWidget * parent = 0);
  ~main_window ();

  void event_accepted (octave_event *e);
  void event_reject (octave_event *e);

  QTerminal *get_terminal_view () { return _terminal; }
  history_dock_widget *get_history_dock_widget () { return _history_dock_widget; }
  files_dock_widget *get_files_dock_widget () { return _files_dock_widget; }
  bool is_closing () { return _closing; }

signals:
  void settings_changed ();

public slots:
  void report_status_message (QString statusMessage);
  void handle_save_workspace_request ();
  void handle_load_workspace_request ();
  void handle_clear_workspace_request ();
  void handle_command_double_clicked (QString command);
  void new_file ();
  void open_file ();
  void open_bug_tracker_page ();
  void open_agora_page ();
  void open_octave_forge_page ();
  void process_settings_dialog_request ();
  void show_about_octave ();
  void notice_settings ();
  void prepare_for_quit ();
  void reset_windows ();
  void update_current_working_directory (QString directory);
  void change_current_working_directory ();
  void change_current_working_directory (QString directory);
  void current_working_directory_up ();

  void handle_entered_debug_mode ();
  void handle_quit_debug_mode ();

protected:
  void closeEvent (QCloseEvent * closeEvent);
  void read_settings ();
  void write_settings ();

private:
  void construct ();
  void establish_octave_link ();

  QTerminal *               _terminal;
  file_editor_interface *   _file_editor;

  // Dock widgets.
  workspace_view *          _workspace_view;
  history_dock_widget *     _history_dock_widget;
  files_dock_widget *       _files_dock_widget;
  terminal_dock_widget *    _terminal_dock_widget;

  // Toolbars.
  QStatusBar *              _status_bar;

  QComboBox *               _current_directory_combo_box;
  QToolButton *             _current_directory_tool_button;
  QToolButton *             _current_directory_up_tool_button;

  octave_qt_event_listener *_octave_qt_event_listener;

  // Flag for closing whole application
  bool                      _closing;
};

#endif // MAINWINDOW_H
