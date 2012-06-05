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

#include "resource-manager.h"
#include "files-dockwidget.h"

#include <QApplication>
#include <QFileInfo>
#include <QCompleter>
#include <QSettings>
#include <QProcess>
#include <QDebug>

files_dock_widget::files_dock_widget (QWidget *parent)
  : QDockWidget (parent)
{
  setObjectName ("FilesDockWidget");
  setWindowTitle (tr ("Current Directory"));
  setWidget (new QWidget (this));

  // Create a toolbar
  _navigation_tool_bar = new QToolBar ("", widget ());
  _navigation_tool_bar->setAllowedAreas (Qt::TopToolBarArea);
  _navigation_tool_bar->setMovable (false);
  _navigation_tool_bar->setIconSize (QSize (20, 20));

  // Add a button to the toolbar with the QT standard icon for up-directory
  _directory_icon = QIcon(":/actions/icons/up.png");
  _directory_up_action = new QAction (_directory_icon, "", _navigation_tool_bar);
  _directory_up_action->setStatusTip (tr ("Move up one directory."));

  _current_directory = new QLineEdit (_navigation_tool_bar);
  _current_directory->setStatusTip (tr ("Enter the path or filename."));

  _navigation_tool_bar->addAction (_directory_up_action);
  _navigation_tool_bar->addWidget (_current_directory);
  connect (_directory_up_action, SIGNAL (triggered ()), this,
           SLOT (do_up_directory ()));

  // TODO: Add other buttons for creating directories

  // Create the QFileSystemModel starting in the home directory
  QString
    homePath = QDir::homePath ();
  // TODO: This should occur after Octave has been initialized and the startup directory of Octave is established

  _file_system_model = new QFileSystemModel (this);
  _file_system_model->setFilter (QDir::NoDotAndDotDot | QDir::AllEntries);
  QModelIndex
    rootPathIndex = _file_system_model->setRootPath (homePath);

  // Attach the model to the QTreeView and set the root index
  _file_tree_view = new QTreeView (widget ());
  _file_tree_view->setModel (_file_system_model);
  _file_tree_view->setRootIndex (rootPathIndex);
  _file_tree_view->setSortingEnabled (true);
  _file_tree_view->setAlternatingRowColors (true);
  _file_tree_view->setAnimated (true);
  _file_tree_view->setColumnHidden (1, true);
  _file_tree_view->setColumnHidden (2, true);
  _file_tree_view->setColumnHidden (3, true);
  _file_tree_view->setStatusTip (tr ("Doubleclick a file to open it."));

  set_current_directory (_file_system_model->fileInfo (rootPathIndex).
                       absoluteFilePath ());

  connect (_file_tree_view, SIGNAL (doubleClicked (const QModelIndex &)), this,
           SLOT (item_double_clicked (const QModelIndex &)));

  // Layout the widgets vertically with the toolbar on top
  QVBoxLayout *
    layout = new QVBoxLayout ();
  layout->setSpacing (0);
  layout->addWidget (_navigation_tool_bar);
  layout->addWidget (_file_tree_view);
  layout->setMargin (1);
  widget ()->setLayout (layout);
  // TODO: Add right-click contextual menus for copying, pasting, deleting files (and others)

  connect (_current_directory, SIGNAL (returnPressed ()), this,
           SLOT (current_directory_entered ()));
  QCompleter *
    completer = new QCompleter (_file_system_model, this);
  _current_directory->setCompleter (completer);

  connect (this, SIGNAL (visibilityChanged(bool)), this, SLOT(handle_visibility_changed(bool)));
}

void
files_dock_widget::item_double_clicked (const QModelIndex & index)
{
  // Retrieve the file info associated with the model index.
  QFileInfo fileInfo = _file_system_model->fileInfo (index);

  // If it is a directory, cd into it.
  if (fileInfo.isDir ())
    {
      _file_system_model->setRootPath (fileInfo.absolutePath ());
      _file_tree_view->setRootIndex (index);
      set_current_directory (_file_system_model->fileInfo (index).
                           absoluteFilePath ());
    }
  // Otherwise attempt to open it.
  else
    {
      // Check if the user wants to use a custom file editor.
      QSettings *settings = resource_manager::instance ()->get_settings ();
      if (settings->value ("useCustomFileEditor").toBool ())
        {
          QString editor = settings->value ("customFileEditor").toString ();
          QStringList arguments;
          arguments << fileInfo.filePath ();
          QProcess::startDetached (editor, arguments);
        }
      else
        {
          emit open_file (fileInfo.filePath ());
        }
    }
}

void
files_dock_widget::set_current_directory (QString currentDirectory)
{
  _current_directory->setText (currentDirectory);
}

void
files_dock_widget::do_up_directory (void)
{
  QDir dir =
    QDir (_file_system_model->filePath (_file_tree_view->rootIndex ()));
  dir.cdUp ();
  _file_system_model->setRootPath (dir.absolutePath ());
  _file_tree_view->setRootIndex (_file_system_model->
                                index (dir.absolutePath ()));
  set_current_directory (dir.absolutePath ());
}

void
files_dock_widget::current_directory_entered ()
{
  QFileInfo fileInfo (_current_directory->text ());
  if (fileInfo.isDir ())
    {
      _file_tree_view->setRootIndex (_file_system_model->
                                    index (fileInfo.absolutePath ()));
      _file_system_model->setRootPath (fileInfo.absolutePath ());
      set_current_directory (fileInfo.absoluteFilePath ());
    }
  else
    {
      if (QFile::exists (fileInfo.absoluteFilePath ()))
        emit open_file (fileInfo.absoluteFilePath ());
    }
}

void
files_dock_widget::notice_settings ()
{
  QSettings *settings = resource_manager::instance ()->get_settings ();
  _file_tree_view->setColumnHidden (0, !settings->value ("showFilenames").toBool ());
  _file_tree_view->setColumnHidden (1, !settings->value ("showFileSize").toBool ());
  _file_tree_view->setColumnHidden (2, !settings->value ("showFileType").toBool ());
  _file_tree_view->setColumnHidden (3, !settings->value ("showLastModified").toBool ());
  _file_tree_view->setAlternatingRowColors (settings->value ("useAlternatingRowColors").toBool ());
  //if (settings.value ("showHiddenFiles").toBool ())
  // TODO: React on option for hidden files.
}

void
files_dock_widget::handle_visibility_changed (bool visible)
{
  if (visible)
    emit active_changed (true);
}

void
files_dock_widget::closeEvent (QCloseEvent *event)
{
  emit active_changed (false);
  QDockWidget::closeEvent (event);
}
