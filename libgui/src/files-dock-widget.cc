/*

Copyright (C) 2011-2012 Jacob Dawid

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "resource-manager.h"
#include "files-dock-widget.h"

#include <QApplication>
#include <QFileInfo>
#include <QCompleter>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QSizePolicy>

files_dock_widget::files_dock_widget (QWidget *p)
  : octave_dock_widget (p)
{
  setObjectName ("FilesDockWidget");
  setWindowIcon (QIcon(":/actions/icons/logo.png"));
  setWindowTitle (tr ("File Browser"));
  setToolTip (tr ("Browse your files."));

  QWidget *container = new QWidget (this);

  setWidget (container);

  connect (this, SIGNAL (open_file (const QString&)),
           parent (), SLOT (open_file (const QString&)));

  connect (this, SIGNAL (displayed_directory_changed (const QString&)),
           parent (), SLOT (set_current_working_directory (const QString&)));

  connect (parent (), SIGNAL (settings_changed (const QSettings *)),
           this, SLOT (notice_settings (const QSettings *)));

  // Create a toolbar
  QToolBar *navigation_tool_bar = new QToolBar ("", container);
  navigation_tool_bar->setAllowedAreas (Qt::TopToolBarArea);
  navigation_tool_bar->setMovable (false);
  navigation_tool_bar->setIconSize (QSize (20, 20));

  _current_directory = new QComboBox (navigation_tool_bar);
  _current_directory->setToolTip (tr ("Enter the path or filename"));
  _current_directory->setEditable(true);
  _current_directory->setMaxCount(MaxMRUDirs);
  _current_directory->setInsertPolicy(QComboBox::NoInsert);
  _current_directory->setSizeAdjustPolicy (QComboBox::AdjustToMinimumContentsLengthWithIcon);
  QSizePolicy sizePol(QSizePolicy::Expanding, QSizePolicy::Preferred);
  _current_directory->setSizePolicy(sizePol);

  QAction *directory_up_action = new QAction (QIcon(":/actions/icons/up.png"),
                                              "", navigation_tool_bar);
  directory_up_action->setToolTip (tr ("Move up one directory"));

  _sync_browser_directory_action = new QAction (QIcon(":/actions/icons/reload.png"),
                                                "", navigation_tool_bar);
  _sync_browser_directory_action->setToolTip (tr ("Goto current octave directory"));
  _sync_browser_directory_action->setEnabled ("false");

  _sync_octave_directory_action = new QAction (QIcon(":/actions/icons/ok.png"),
                                               "", navigation_tool_bar);
  _sync_octave_directory_action->setToolTip (tr ("Set octave directroy to current browser directory"));
  _sync_octave_directory_action->setEnabled ("false");

  navigation_tool_bar->addWidget (_current_directory);
  navigation_tool_bar->addAction (directory_up_action);
  navigation_tool_bar->addAction (_sync_browser_directory_action);
  navigation_tool_bar->addAction (_sync_octave_directory_action);

  connect (directory_up_action, SIGNAL (triggered ()), this,
           SLOT (change_directory_up ()));
  connect (_sync_octave_directory_action, SIGNAL (triggered ()), this,
           SLOT (do_sync_octave_directory ()));
  connect (_sync_browser_directory_action, SIGNAL (triggered ()), this,
           SLOT (do_sync_browser_directory ()));

  // TODO: Add other buttons for creating directories

  // Create the QFileSystemModel starting in the home directory
  QString homePath = QDir::homePath ();

  _file_system_model = new QFileSystemModel (this);
  _file_system_model->setFilter (QDir::NoDotAndDotDot | QDir::AllEntries);
  QModelIndex rootPathIndex = _file_system_model->setRootPath (homePath);

  // Attach the model to the QTreeView and set the root index
  _file_tree_view = new QTreeView (container);
  _file_tree_view->setModel (_file_system_model);
  _file_tree_view->setRootIndex (rootPathIndex);
  _file_tree_view->setSortingEnabled (true);
  _file_tree_view->setAlternatingRowColors (true);
  _file_tree_view->setAnimated (true);
  _file_tree_view->setToolTip (tr ("Doubleclick a file to open it"));

  // get sort column and order as well as cloumn state (order and width)
  QSettings *settings = resource_manager::get_settings ();
  // FIXME -- what should happen if settings is 0?
  _file_tree_view->sortByColumn (
              settings->value ("filesdockwidget/sort_files_by_column",0).toInt (),
              static_cast<Qt::SortOrder>(settings->value ("filesdockwidget/sort_files_by_order",Qt::AscendingOrder).toUInt ())
  );
  _file_tree_view->header ()->restoreState (settings->value ("filesdockwidget/column_state").toByteArray ());
  
  QStringList mru_dirs = settings->value ("filesdockwidget/mru_dir_list").toStringList ();
  _current_directory->addItems(mru_dirs);

  _current_directory->setEditText(_file_system_model->fileInfo (rootPathIndex).
                              absoluteFilePath ());

  connect (_file_tree_view, SIGNAL (doubleClicked (const QModelIndex &)),
           this, SLOT (item_double_clicked (const QModelIndex &)));

  // Layout the widgets vertically with the toolbar on top
  QVBoxLayout *vbox_layout = new QVBoxLayout ();
  vbox_layout->setSpacing (0);
  vbox_layout->addWidget (navigation_tool_bar);
  vbox_layout->addWidget (_file_tree_view);
  vbox_layout->setMargin (1);

  container->setLayout (vbox_layout);

  // TODO: Add right-click contextual menus for copying, pasting, deleting files (and others)

  connect (_current_directory->lineEdit(), SIGNAL (returnPressed ()),
            this, SLOT (accept_directory_line_edit ()));

  connect (_current_directory, SIGNAL (activated (const QString &)),
           this, SLOT (set_current_directory (const QString &)));

  QCompleter *completer = new QCompleter (_file_system_model, this);
  _current_directory->setCompleter (completer);

  setFocusProxy (_current_directory);
  
  _sync_octave_dir = true;   // default, overwirtten with notice_settings ()
  _octave_dir = "";
}

files_dock_widget::~files_dock_widget ()
{
  QSettings *settings = resource_manager::get_settings ();
  int sort_column = _file_tree_view->header ()->sortIndicatorSection ();
  Qt::SortOrder sort_order = _file_tree_view->header ()->sortIndicatorOrder ();
  settings->setValue ("filesdockwidget/sort_files_by_column", sort_column);
  settings->setValue ("filesdockwidget/sort_files_by_order", sort_order);
  settings->setValue ("filesdockwidget/column_state", _file_tree_view->header ()->saveState ());

  QStringList dirs;
  for(int i=0; i< _current_directory->count(); i++)
  {
    dirs.append(_current_directory->itemText(i));
  }
  settings->setValue ("filesdockwidget/mru_dir_list", dirs);

  settings->sync ();
}

void
files_dock_widget::item_double_clicked (const QModelIndex& index)
{
  // Retrieve the file info associated with the model index.
  QFileInfo fileInfo = _file_system_model->fileInfo (index);
  set_current_directory (fileInfo.absoluteFilePath ());
}

void
files_dock_widget::set_current_directory (const QString& dir)
{
  display_directory (dir);
}

void
files_dock_widget::accept_directory_line_edit (void)
{
  display_directory (_current_directory->currentText ());
}

void
files_dock_widget::change_directory_up (void)
{
  QDir dir = QDir (_file_system_model->filePath (_file_tree_view->rootIndex ()));
  dir.cdUp ();
  display_directory (dir.absolutePath ());
}

void
files_dock_widget::do_sync_octave_directory (void)
{
  QDir dir = QDir (_file_system_model->filePath (_file_tree_view->rootIndex ()));
  emit displayed_directory_changed (dir.absolutePath ());
}

void
files_dock_widget::do_sync_browser_directory (void)
{
  display_directory (_octave_dir,false);  // false: no sync of octave dir
}

void
files_dock_widget::update_octave_directory (const QString& dir)
{
  _octave_dir = dir;
  if (_sync_octave_dir)
    display_directory (_octave_dir,false);  // false: no sync of octave dir
}

void
files_dock_widget::display_directory (const QString& dir, bool set_octave_dir)
{
  QFileInfo fileInfo (dir);
  if (fileInfo.exists ())
    {
      if (fileInfo.isDir ())
        {
          _file_tree_view->setRootIndex (_file_system_model->
                                         index (fileInfo.absoluteFilePath ()));
          _file_system_model->setRootPath (fileInfo.absoluteFilePath ());
          _file_system_model->sort (0, Qt::AscendingOrder);
          if (_sync_octave_dir && set_octave_dir)
            emit displayed_directory_changed (fileInfo.absoluteFilePath ());

          // see if its in the list, and if it is, remove it and then, put at top of the list
          int index = _current_directory->findText(fileInfo.absoluteFilePath ());
          if(index != -1)
          {
             _current_directory->removeItem(index);
          }
          _current_directory->insertItem(0, fileInfo.absoluteFilePath ());
          _current_directory->setCurrentIndex(0);
        }
      else
        {
          if (QFile::exists (fileInfo.absoluteFilePath ()))
            emit open_file (fileInfo.absoluteFilePath ());
        }
    }
}

void
files_dock_widget::notice_settings (const QSettings *settings)
{
  // Qsettings pointer is checked before emitting.

  // file names are always shown, other columns can be hidden by settings
  _file_tree_view->setColumnHidden (0, false);
  _file_tree_view->setColumnHidden (1, !settings->value ("filesdockwidget/showFileSize",false).toBool ());
  _file_tree_view->setColumnHidden (2, !settings->value ("filesdockwidget/showFileType",false).toBool ());
  _file_tree_view->setColumnHidden (3, !settings->value ("filesdockwidget/showLastModified",false).toBool ());
  _file_tree_view->setAlternatingRowColors (settings->value ("filesdockwidget/useAlternatingRowColors",true).toBool ());
  if (settings->value ("filesdockwidget/showHiddenFiles",false).toBool ())
    {
      // TODO: React on option for hidden files.
    }
  // enalbe the buttons to sync octave/browser dir only if this is not done by default
  _sync_octave_dir = settings->value ("filesdockwidget/sync_octave_directory",false).toBool ();
  _sync_octave_directory_action->setEnabled (!_sync_octave_dir);
  _sync_browser_directory_action->setEnabled (!_sync_octave_dir);

  if (_sync_octave_dir)
    display_directory (_octave_dir);  // sync browser to octave dir

}
