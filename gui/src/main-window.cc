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

main_window::main_window (QWidget * parent):QMainWindow (parent)
{
  // We have to set up all our windows, before we finally launch octave.
  construct ();
  _octave_qt_event_listener = new octave_qt_event_listener ();
  octave_link::instance ()->register_event_listener (_octave_qt_event_listener);

  connect (_octave_qt_event_listener,
           SIGNAL (current_directory_changed(QString)),
           this,
           SLOT (update_current_working_directory(QString)));

  octave_link::instance ()->launch_octave();
}

main_window::~main_window ()
{
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
  _terminal->sendText (QString ("save \'%1\'\n").arg (selectedFile));
  _terminal->setFocus ();
}

void
main_window::handle_load_workspace_request ()
{
  QString selectedFile =
      QFileDialog::getOpenFileName (this, tr ("Load Workspace"),
                                    resource_manager::instance ()->get_home_path ());
  if (!selectedFile.isEmpty ())
    {
      _terminal->sendText (QString ("load \'%1\'\n").arg (selectedFile));
      _terminal->setFocus ();
    }
}

void
main_window::handle_clear_workspace_request ()
{
  _terminal->sendText ("clear\n");
  _terminal->setFocus ();
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
main_window::update_current_working_directory (QString directory)
{
  if (_current_directory_combo_box->count () > 31)
    {
      _current_directory_combo_box->removeItem (0);
    }
  _current_directory_combo_box->addItem (directory);
  int index = _current_directory_combo_box->findText (directory);
  _current_directory_combo_box->setCurrentIndex (index);
}

void
main_window::change_current_working_directory ()
{
  QString selectedDirectory =
      QFileDialog::getExistingDirectory(this, tr ("Set working direcotry"));

  if (!selectedDirectory.isEmpty ())
    {
      octave_link::instance ()
          ->request_working_directory_change (selectedDirectory.toStdString ());
    }
}

void
main_window::change_current_working_directory (QString directory)
{
  octave_link::instance ()->request_working_directory_change (directory.toStdString ());
}

void
main_window::current_working_directory_up ()
{
  _terminal->sendText ("cd ..\n");
  _terminal->setFocus ();
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
  octave_link::instance ()->request_octave_exit ();
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
  QStyle *style = QApplication::style ();
  // TODO: Check this.
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
  _current_directory_tool_button->setIcon (style->standardIcon (QStyle::SP_DirOpenIcon));

  _current_directory_up_tool_button = new QToolButton (this);
  _current_directory_up_tool_button->setIcon (style->standardIcon (QStyle::SP_FileDialogToParent));

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
    = file_menu->addAction (QIcon::fromTheme ("document-new",
      style->standardIcon (QStyle::SP_FileIcon)), tr ("New File"));

  QAction *open_file_action
      = file_menu->addAction (QIcon::fromTheme ("document-open",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Open File"));

  QAction *settings_action = file_menu->addAction (tr ("Settings"));
  file_menu->addSeparator ();
  QAction *exit_action = file_menu->addAction (tr ("Exit"));

  QMenu *edit_menu = menuBar ()->addMenu (tr ("&Edit"));
  QAction *cut_action
      = edit_menu->addAction (QIcon::fromTheme ("edit-cut",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Cut"));
  cut_action->setShortcut (QKeySequence::Cut);

  QAction *copy_action
      = edit_menu->addAction (QIcon::fromTheme ("edit-copy",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Copy"));
  copy_action->setShortcut (QKeySequence::Copy);

  QAction *paste_action
      = edit_menu->addAction (QIcon::fromTheme ("edit-paste",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Paste"));
  paste_action->setShortcut (QKeySequence::Paste);

  QAction *undo_action
      = edit_menu->addAction (QIcon::fromTheme ("edit-undo",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Undo"));
  undo_action->setShortcut (QKeySequence::Undo);

  QAction *redo_action
      = edit_menu->addAction (QIcon::fromTheme ("edit-redo",
        style->standardIcon (QStyle::SP_FileIcon)), tr ("Redo"));
  redo_action->setShortcut (QKeySequence::Redo);

  //QMenu *debugMenu = menuBar ()->addMenu (tr ("De&bug"));
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
//  connect (octave_link::instance (),    SIGNAL (working_directory_changed (QString)),
//           this,                        SLOT (update_current_working_directory (QString)));
  connect (_current_directory_combo_box, SIGNAL (activated (QString)),
           this,                        SLOT (change_current_working_directory (QString)));

  setWindowTitle ("Octave");
  setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
  addDockWidget (Qt::LeftDockWidgetArea, _workspace_view);
  addDockWidget (Qt::LeftDockWidgetArea, _history_dock_widget);
  addDockWidget (Qt::RightDockWidgetArea, _files_dock_widget);
  addDockWidget (Qt::RightDockWidgetArea, _file_editor);
  addDockWidget (Qt::BottomDockWidgetArea, _terminal_dock_widget);
  setStatusBar (_status_bar);
  read_settings ();
}

