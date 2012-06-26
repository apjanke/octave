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

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QStyle>
#include <QToolBar>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>

#include "main-window.h"
#include "file-editor.h"
#include "settings-dialog.h"

main_window::main_window (QWidget * parent)
  : QMainWindow (parent), octave_event_observer ()
{
  // We have to set up all our windows, before we finally launch octave.
  construct ();
  octave_link::instance ()->launch_octave();
}

main_window::~main_window ()
{
}

void
main_window::event_accepted (octave_event *e)
{
  delete e;
}

void
main_window::event_reject (octave_event *e)
{
  delete e;
}

void
main_window::new_file ()
{
  _file_editor->request_new_file ();
}

void
main_window::open_file ()
{
  _file_editor->request_open_file ();
}

void
main_window::report_status_message (QString statusMessage)
{
  _status_bar->showMessage (statusMessage, 1000);
}

void
main_window::handle_save_workspace_request ()
{
  QString selectedFile =
      QFileDialog::getSaveFileName (this, tr ("Save Workspace"),
                                    resource_manager::instance ()->get_home_path ());
  octave_link::instance ()
      ->post_event (new octave_save_workspace_event (*this,
        selectedFile.toStdString()));
}

void
main_window::handle_load_workspace_request ()
{
  QString selectedFile =
      QFileDialog::getOpenFileName (this, tr ("Load Workspace"),
                                    resource_manager::instance ()->get_home_path ());
  if (!selectedFile.isEmpty ())
    {
      octave_link::instance ()
          ->post_event (new octave_load_workspace_event (*this,
            selectedFile.toStdString()));
    }
}

void
main_window::handle_clear_workspace_request ()
{
  octave_link::instance ()
      ->post_event (new octave_clear_workspace_event (*this));
}

void
main_window::handle_command_double_clicked (QString command)
{
  _terminal->sendText(command);
  _terminal->setFocus ();
}

void
main_window::open_bug_tracker_page ()
{
  QDesktopServices::openUrl (QUrl ("http://savannah.gnu.org/bugs/?group=octave"));
}

void
main_window::open_agora_page ()
{
  QDesktopServices::openUrl (QUrl ("http://agora.panocha.org.mx/"));
}

void
main_window::open_octave_forge_page ()
{
  QDesktopServices::openUrl (QUrl ("http://octave.sourceforge.net/"));
}

void
main_window::process_settings_dialog_request ()
{
  settings_dialog *settingsDialog = new settings_dialog (this);
  settingsDialog->exec ();
  delete settingsDialog;
  emit settings_changed ();
  resource_manager::instance ()->update_network_settings ();
  notice_settings();
}

void
main_window::notice_settings ()
{
  // Set terminal font:
  QSettings *settings = resource_manager::instance ()->get_settings ();
  QFont font = QFont();
  font.setFamily(settings->value("terminal/fontName").toString());
  font.setPointSize(settings->value("terminal/fontSize").toInt ());
  _terminal->setTerminalFont(font);
}

void
main_window::prepare_for_quit ()
{
  write_settings ();
}

void
main_window::reset_windows ()
{
  // TODO: Implement.
}

void
main_window::current_working_directory_has_changed (QString directory)
{
  if (_current_directory_combo_box->count () > 31)
    {
      _current_directory_combo_box->removeItem (0);
    }
  _current_directory_combo_box->addItem (directory);
  int index = _current_directory_combo_box->findText (directory);
  _current_directory_combo_box->setCurrentIndex (index);

  _files_dock_widget->set_current_directory (directory);
}

void
main_window::change_current_working_directory ()
{
  QString selectedDirectory =
      QFileDialog::getExistingDirectory(this, tr ("Set working direcotry"));

  if (!selectedDirectory.isEmpty ())
    {
      octave_link::instance ()
          ->post_event (new octave_change_directory_event (*this,
                        selectedDirectory.toStdString ()));
    }
}

void
main_window::set_current_working_directory (QString directory)
{
  octave_link::instance ()
      ->post_event (new octave_change_directory_event (*this,
                    directory.toStdString ()));
}

void
main_window::current_working_directory_up ()
{
  octave_link::instance ()
      ->post_event (new octave_change_directory_event (*this, ".."));

}

void
main_window::handle_entered_debug_mode ()
{
  setWindowTitle ("Octave (Debugging)");
  _debug_continue->setEnabled (true);
  _debug_step_into->setEnabled (true);
  _debug_step_over->setEnabled (true);
  _debug_step_out->setEnabled (true);
  _debug_quit->setEnabled (true);
}

void
main_window::handle_quit_debug_mode ()
{
  setWindowTitle ("Octave");
  _debug_continue->setEnabled (false);
  _debug_step_into->setEnabled (false);
  _debug_step_over->setEnabled (false);
  _debug_step_out->setEnabled (false);
  _debug_quit->setEnabled (false);
}

void
main_window::debug_continue ()
{
  octave_link::instance ()
      ->post_event (new octave_debug_continue_event (*this));
}

void
main_window::debug_step_into ()
{
  octave_link::instance ()
      ->post_event (new octave_debug_step_into_event (*this));
}

void
main_window::debug_step_over ()
{
  octave_link::instance ()
      ->post_event (new octave_debug_step_over_event (*this));
}

void
main_window::debug_step_out ()
{
  octave_link::instance ()
      ->post_event (new octave_debug_step_out_event (*this));
}

void
main_window::debug_quit ()
{
  octave_link::instance ()
      ->post_event (new octave_debug_quit_event (*this));
}

void
main_window::update_performance_information ()
{
  octave_link::performance_information p
      = octave_link::instance ()->get_performance_information ();

  int generate_events_msec
    = (p.generate_events_stop - p.generate_events_start) * 1000 / CLOCKS_PER_SEC;

  int process_events_msec
    = (p.process_events_stop - p.process_events_start) * 1000 / CLOCKS_PER_SEC;

  _status_bar->showMessage
      (QString ("CPU time of the GUI in octave thread - generating rx events: ~%1 ms - processing tx events: ~%2 ms (%3 events)")
       .arg (generate_events_msec)
       .arg (process_events_msec)
       .arg (p.event_queue_size));
}

void
main_window::show_about_octave ()
{
  QString message =
      "GNU Octave\n"
      "Copyright (C) 2009 John W. Eaton and others.\n"
      "This is free software; see the source code for copying conditions."
      "There is ABSOLUTELY NO WARRANTY; not even for MERCHANTABILITY or"
      "FITNESS FOR A PARTICULAR PURPOSE.  For details, type `warranty'.\n"
      "\n"
      "Octave was configured for \"x86_64-pc-linux-gnu\".\n"
      "\n"
      "Additional information about Octave is available at http://www.octave.org.\n"
      "\n"
      "Please contribute if you find this software useful."
      "For more information, visit http://www.octave.org/help-wanted.html\n"
      "\n"
      "Report bugs to <bug@octave.org> (but first, please read"
      "http://www.octave.org/bugs.html to learn how to write a helpful report).\n"
      "\n"
      "For information about changes from previous versions, type `news'.\n";

  QMessageBox::about (this, tr ("About Octave"), message);
}

void
main_window::closeEvent (QCloseEvent * closeEvent)
{
  closeEvent->ignore ();
  octave_link::instance ()->post_event (new octave_exit_event (*this));
 }

void
main_window::read_settings ()
{
  QSettings *settings = resource_manager::instance ()->get_settings ();
  restoreGeometry (settings->value ("MainWindow/geometry").toByteArray ());
  restoreState (settings->value ("MainWindow/windowState").toByteArray ());
  emit settings_changed ();
}

void
main_window::write_settings ()
{
  QSettings *settings = resource_manager::instance ()->get_settings ();
  settings->setValue ("MainWindow/geometry", saveGeometry ());
  settings->setValue ("MainWindow/windowState", saveState ());
  settings->sync ();
}

void
main_window::construct ()
{
  _closing = false;   // flag for editor files when closed
  setWindowIcon (resource_manager::instance ()->get_icon (resource_manager::octave));

  // Setup dockable widgets and the status bar.
  _workspace_view           = new workspace_view (this);
  _workspace_view->setStatusTip (tr ("View the variables in the active workspace."));
  _history_dock_widget      = new history_dock_widget (this);
  _history_dock_widget->setStatusTip (tr ("Browse and search the command history."));
  _files_dock_widget        = new files_dock_widget (this);
  _files_dock_widget->setStatusTip (tr ("Browse your files."));
  _status_bar               = new QStatusBar (this);

  _current_directory_combo_box = new QComboBox (this);
  _current_directory_combo_box->setFixedWidth (300);
  _current_directory_combo_box->setEditable (true);
  _current_directory_combo_box->setInsertPolicy (QComboBox::InsertAtTop);
  _current_directory_combo_box->setMaxVisibleItems (14);

  _current_directory_tool_button = new QToolButton (this);
  _current_directory_tool_button->setIcon (QIcon(":/actions/icons/search.png"));

  _current_directory_up_tool_button = new QToolButton (this);
  _current_directory_up_tool_button->setIcon (QIcon(":/actions/icons/up.png"));

  // Octave Terminal subwindow.
  _terminal = new QTerminal (this);
  _terminal->setObjectName ("OctaveTerminal");
  _terminal_dock_widget = new terminal_dock_widget (_terminal, this);

  QWidget *dummyWidget = new QWidget ();
  dummyWidget->setObjectName ("CentralDummyWidget");
  dummyWidget->resize (10, 10);
  dummyWidget->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
  dummyWidget->hide ();
  setCentralWidget (dummyWidget);

  _file_editor = new file_editor (_terminal, this);

  QMenu *file_menu = menuBar ()->addMenu (tr ("&File"));
  QAction *new_file_action
      = file_menu->addAction (QIcon(":/actions/icons/filenew.png"), tr ("New File"));

  QAction *open_file_action
      = file_menu->addAction (QIcon(":/actions/icons/fileopen.png"), tr ("Open File"));

  QAction *settings_action
      = file_menu->addAction (QIcon(":/actions/icons/configure.png"), tr ("Settings"));
  file_menu->addSeparator ();
  QAction *exit_action = file_menu->addAction (tr ("Exit"));

  QMenu *edit_menu = menuBar ()->addMenu (tr ("&Edit"));
  QAction *cut_action
      = edit_menu->addAction (QIcon(":/actions/icons/editcut.png"), tr ("Cut"));
  cut_action->setShortcut (QKeySequence::Cut);

  QAction *copy_action
      = edit_menu->addAction (QIcon(":/actions/icons/editcopy.png"), tr ("Copy"));
  copy_action->setShortcut (Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_C);

  QAction *paste_action
      = edit_menu->addAction (QIcon(":/actions/icons/editpaste.png"), tr ("Paste"));
  paste_action->setShortcut (Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_V);

  QAction *undo_action
      = edit_menu->addAction (QIcon(":/actions/icons/undo.png"), tr ("Undo"));
  undo_action->setShortcut (QKeySequence::Undo);

  QAction *redo_action
      = edit_menu->addAction (QIcon(":/actions/icons/redo.png"), tr ("Redo"));
  redo_action->setShortcut (QKeySequence::Redo);

  _debug_menu = menuBar ()->addMenu (tr ("De&bug"));

  _debug_continue = _debug_menu->addAction (tr ("Continue"));
  _debug_continue->setEnabled (false);
  _file_editor->debug_menu ()->addAction (_debug_continue);
  _debug_continue->setShortcut (Qt::Key_F5);

  _debug_step_into = _debug_menu->addAction (tr ("Step into"));
  _debug_step_into->setEnabled (false);
  _file_editor->debug_menu ()->addAction (_debug_step_into);
  _debug_step_into->setShortcut (Qt::Key_F9);

  _debug_step_over = _debug_menu->addAction (tr ("Next"));
  _debug_step_over->setEnabled (false);
  _file_editor->debug_menu ()->addAction (_debug_step_over);
  _debug_step_over->setShortcut (Qt::Key_F10);

  _debug_step_out = _debug_menu->addAction (tr ("Step out"));
  _debug_step_out->setEnabled (false);
  _file_editor->debug_menu ()->addAction (_debug_step_over);
  _debug_step_out->setShortcut (Qt::Key_F11);

  _debug_menu->addSeparator ();

  _debug_quit = _debug_menu->addAction (tr ("Quit"));
  _debug_quit->setEnabled (false);
  _file_editor->debug_menu ()->addAction (_debug_quit);
  _debug_quit->setShortcut (Qt::Key_Escape);

  //QMenu *parallelMenu = menuBar ()->addMenu (tr ("&Parallel"));

  QMenu *   desktop_menu = menuBar ()->addMenu (tr ("&Desktop"));
  QAction * load_workspace_action       = desktop_menu->addAction (tr ("Load workspace"));
  QAction * save_workspace_action       = desktop_menu->addAction (tr ("Save workspace"));
  QAction * clear_workspace_action      = desktop_menu->addAction (tr ("Clear workspace"));

  // Window menu
  QMenu *   window_menu = menuBar ()->addMenu (tr ("&Window"));
  QAction * show_command_window_action  = window_menu->addAction (tr ("Command Window"));
            show_command_window_action->setCheckable (true);
  QAction * show_workspace_action       = window_menu->addAction (tr ("Workspace"));
            show_workspace_action->setCheckable (true);
  QAction * show_history_action         = window_menu->addAction (tr ("Command History"));
            show_history_action->setCheckable (true);
  QAction * show_file_browser_action    = window_menu->addAction (tr ("Current Directory"));
            show_file_browser_action->setCheckable (true);
  QAction * show_editor_action          = window_menu->addAction (tr ("Editor"));
            show_editor_action->setCheckable (true);
  window_menu->addSeparator ();
  QAction * reset_windows_action        = window_menu->addAction (tr ("Reset Windows"));

  // Help menu
  QMenu *   help_menu = menuBar ()->addMenu (tr ("&Help"));
  QAction * report_bug_action           = help_menu->addAction (tr ("Report Bug"));
  QAction * agora_action                = help_menu->addAction (tr ("Visit Agora"));
  QAction * octave_forge_action         = help_menu->addAction (tr ("Visit Octave Forge"));
  help_menu->addSeparator ();
  QAction * about_octave_action         = help_menu->addAction (tr ("About Octave"));

  // Toolbars
  QToolBar *main_tool_bar = addToolBar ("Main");
            main_tool_bar->addAction (new_file_action);
            main_tool_bar->addAction (open_file_action);
            main_tool_bar->addSeparator ();
            main_tool_bar->addAction (cut_action);
            main_tool_bar->addAction (copy_action);
            main_tool_bar->addAction (paste_action);
            main_tool_bar->addAction (undo_action);
            main_tool_bar->addAction (redo_action);
            main_tool_bar->addSeparator ();
            main_tool_bar->addWidget (new QLabel (tr ("Current Directory:")));
            main_tool_bar->addWidget (_current_directory_combo_box);
            main_tool_bar->addWidget (_current_directory_tool_button);
            main_tool_bar->addWidget (_current_directory_up_tool_button);

  connect (qApp,                        SIGNAL (aboutToQuit ()),
           this,                        SLOT   (prepare_for_quit ()));
  connect (settings_action,             SIGNAL (triggered ()),
           this,                        SLOT   (process_settings_dialog_request ()));
  connect (exit_action,                 SIGNAL (triggered ()),
           this,                        SLOT   (close ()));
  connect (new_file_action,             SIGNAL (triggered ()),
           this,                        SLOT   (new_file ()));
  connect (open_file_action,            SIGNAL (triggered ()),
           this,                        SLOT   (open_file ()));
  connect (report_bug_action,           SIGNAL (triggered ()),
           this,                        SLOT   (open_bug_tracker_page ()));
  connect (agora_action,                SIGNAL (triggered ()),
           this,                        SLOT   (open_agora_page ()));
  connect (octave_forge_action,         SIGNAL (triggered ()),
           this,                        SLOT   (open_octave_forge_page ()));
  connect (about_octave_action,         SIGNAL (triggered ()),
           this,                        SLOT   (show_about_octave ()));
  connect (show_command_window_action,  SIGNAL (toggled (bool)),
           _terminal_dock_widget,       SLOT   (setVisible (bool)));
  connect (_terminal_dock_widget,       SIGNAL (active_changed (bool)),
           show_command_window_action,  SLOT   (setChecked (bool)));
  connect (show_workspace_action,       SIGNAL (toggled (bool)),
           _workspace_view,             SLOT   (setVisible (bool)));
  connect (_workspace_view,             SIGNAL (active_changed (bool)),
           show_workspace_action,       SLOT   (setChecked (bool)));
  connect (show_history_action,         SIGNAL (toggled (bool)),
           _history_dock_widget,        SLOT   (setVisible (bool)));
  connect (_history_dock_widget,        SIGNAL (active_changed (bool)),
           show_history_action,         SLOT   (setChecked (bool)));
  connect (show_file_browser_action,    SIGNAL (toggled (bool)),
           _files_dock_widget,          SLOT   (setVisible (bool)));
  connect (_files_dock_widget,          SIGNAL (active_changed (bool)),
           show_file_browser_action,    SLOT   (setChecked (bool)));
  connect (show_editor_action,          SIGNAL (toggled (bool)),
           _file_editor,                SLOT   (setVisible (bool)));
  connect (_file_editor,                SIGNAL (active_changed (bool)),
           show_editor_action,          SLOT   (setChecked (bool)));
  connect (reset_windows_action,        SIGNAL (triggered ()),
           this,                        SLOT   (reset_windows ()));
  connect (this,                        SIGNAL (settings_changed ()),
           _files_dock_widget,          SLOT   (notice_settings ()));
  connect (this,                        SIGNAL (settings_changed ()),
           this,                        SLOT   (notice_settings ()));
  connect (_files_dock_widget,          SIGNAL (open_file (QString)),
           _file_editor,                SLOT   (request_open_file (QString)));
  connect (_files_dock_widget,          SIGNAL (displayed_directory_changed(QString)),
           this,                        SLOT   (set_current_working_directory(QString)));
  connect (_history_dock_widget,        SIGNAL (information (QString)),
           this,                        SLOT   (report_status_message (QString)));
  connect (_history_dock_widget,        SIGNAL (command_double_clicked (QString)),
           this,                        SLOT   (handle_command_double_clicked (QString)));
  connect (save_workspace_action,       SIGNAL (triggered ()),
           this,                        SLOT   (handle_save_workspace_request ()));
  connect (load_workspace_action,       SIGNAL (triggered ()),
           this,                        SLOT   (handle_load_workspace_request ()));
  connect (clear_workspace_action,      SIGNAL (triggered ()),
           this,                        SLOT   (handle_clear_workspace_request ()));
  connect (_current_directory_tool_button, SIGNAL (clicked ()),
           this,                        SLOT   (change_current_working_directory ()));
  connect (_current_directory_up_tool_button, SIGNAL (clicked ()),
           this,                        SLOT   (current_working_directory_up()));
  connect (copy_action,                 SIGNAL (triggered()),
           _terminal,                   SLOT   (copyClipboard ()));
  connect (paste_action,                SIGNAL (triggered()),
           _terminal,                   SLOT   (pasteClipboard ()));
  connect (_current_directory_combo_box, SIGNAL (activated (QString)),
           this,                        SLOT (set_current_working_directory (QString)));
  connect (_debug_continue,             SIGNAL (triggered ()),
           this,                        SLOT (debug_continue ()));
  connect (_debug_step_into,            SIGNAL (triggered ()),
           this,                        SLOT (debug_step_into ()));
  connect (_debug_step_over,            SIGNAL (triggered ()),
           this,                        SLOT (debug_step_over ()));
  connect (_debug_step_out,             SIGNAL (triggered ()),
           this,                        SLOT (debug_step_out ()));
  connect (_debug_quit,                 SIGNAL (triggered ()),
           this,                        SLOT (debug_quit ()));

  setWindowTitle ("Octave");
  setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
  addDockWidget (Qt::LeftDockWidgetArea, _workspace_view);
  addDockWidget (Qt::LeftDockWidgetArea, _history_dock_widget);
  addDockWidget (Qt::RightDockWidgetArea, _files_dock_widget);
  addDockWidget (Qt::RightDockWidgetArea, _file_editor);
  addDockWidget (Qt::BottomDockWidgetArea, _terminal_dock_widget);
  setStatusBar (_status_bar);
  read_settings ();

  _octave_qt_event_listener = new octave_qt_event_listener ();
  octave_link::instance ()->register_event_listener (_octave_qt_event_listener);

  connect (_octave_qt_event_listener,
           SIGNAL (current_directory_has_changed_signal (QString)),
           this,
           SLOT (current_working_directory_has_changed (QString)));

  connect (_octave_qt_event_listener,
           SIGNAL (entered_debug_mode_signal ()),
           this,
           SLOT(handle_entered_debug_mode ()));

  connect (_octave_qt_event_listener,
           SIGNAL (quit_debug_mode_signal ()),
           this,
           SLOT (handle_quit_debug_mode ()));

  _update_performance_information_timer.setInterval (500);
  _update_performance_information_timer.setSingleShot (false);
  connect (&_update_performance_information_timer, SIGNAL (timeout ()),
           this, SLOT (update_performance_information ()));
  _update_performance_information_timer.start ();
}

