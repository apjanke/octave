/*

Copyright (C) 2013-2017 John Donoghue

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QTableView>
#include <QFileDialog>
#include <QStatusBar>
#include <QIcon>
#include <QFileInfo>
#include <QTimer>
#include <QDirIterator>
#include <QTextStream>
#include <QGroupBox>

#include "find-files-dialog.h"
#include "find-files-model.h"
#include "resource-manager.h"

find_files_dialog::find_files_dialog (QWidget *p)
  : QDialog (p)
{
  setWindowTitle (tr ("Find Files"));
  setWindowIcon (resource_manager::icon ("edit-find"));

  m_dir_iterator = nullptr;

  m_timer = new QTimer (this);
  connect (m_timer, SIGNAL (timeout (void)),
           this, SLOT (look_for_files (void)));

  QSettings *settings = resource_manager::get_settings ();

  QLabel *file_name_label = new QLabel (tr ("Named:"));
  m_file_name_edit = new QLineEdit;
  m_file_name_edit->setToolTip (tr ("Enter the filename search expression"));

  m_file_name_edit->setText (settings->value ("findfiles/file_name",
                                              "*").toString ());
  file_name_label->setBuddy (m_file_name_edit);

  QLabel *start_dir_label = new QLabel (tr ("Start in:"));

  m_start_dir_edit = new QLineEdit;
  m_start_dir_edit->setText (settings->value ("findfiles/start_dir",
                                              QDir::currentPath ()).toString ());
  m_start_dir_edit->setToolTip (tr ("Enter the start directory"));
  start_dir_label->setBuddy (m_start_dir_edit);

  m_browse_button = new QPushButton (tr ("Browse..."));
  m_browse_button->setToolTip (tr ("Browse for start directory"));
  connect (m_browse_button, SIGNAL (clicked (void)),
           this, SLOT (browse_folders (void)));

  m_recurse_dirs_check = new QCheckBox (tr ("Search subdirectories"));
  m_recurse_dirs_check->setChecked (settings->value ("findfiles/recurse_dirs",
                                                     false).toBool ());
  m_recurse_dirs_check->setToolTip (tr ("Search recursively through directories for matching files"));

  m_include_dirs_check = new QCheckBox (tr ("Include directory names"));
  m_include_dirs_check->setChecked (settings->value ("findfiles/include_dirs",
                                                     false).toBool ());
  m_include_dirs_check->setToolTip (tr ("Include matching directories in search results"));

  m_name_case_check = new QCheckBox (tr ("Name case insensitive"));
  m_name_case_check->setChecked (settings->value ("findfiles/name_case",
                                                  false).toBool ());
  m_name_case_check->setToolTip (tr ("Set matching name is case insensitive"));

  m_contains_text_check = new QCheckBox (tr ("Contains text:"));
  m_contains_text_check->setToolTip (tr ("Enter the file content search expression"));
  m_contains_text_check->setChecked (settings->value ("findfiles/check_text",
                                                      false).toBool ());

  m_contains_text_edit = new QLineEdit ();
  m_contains_text_edit->setToolTip (tr ("Text to match"));
  m_contains_text_edit->setText (settings->value ("findfiles/contains_text",
                                                  "").toString ());

  m_content_case_check = new QCheckBox (tr ("Text case insensitive"));
  m_content_case_check->setChecked (settings->value ("findfiles/content_case",
                                                     false).toBool ());
  m_content_case_check->setToolTip (tr ("Set text content is case insensitive"));

  find_files_model *model = new find_files_model (this);

  m_file_list = new QTableView;
  m_file_list->setWordWrap (false);
  m_file_list->setModel (model);
  m_file_list->setShowGrid (false);
  m_file_list->setSelectionBehavior (QAbstractItemView::SelectRows);
  m_file_list->setSelectionMode (QAbstractItemView::SingleSelection);
  m_file_list->setAlternatingRowColors (true);
  m_file_list->setToolTip (tr ("Search results"));
  m_file_list->setSortingEnabled (true);
  m_file_list->horizontalHeader ()->restoreState (settings->value ("findfiles/column_state").toByteArray ());
  m_file_list->horizontalHeader ()->setSortIndicatorShown (true);
#if defined (HAVE_QT4)
  m_file_list->horizontalHeader ()->setClickable (true);
#else
  m_file_list->horizontalHeader ()->setSectionsClickable (true);
#endif
  m_file_list->horizontalHeader ()->setStretchLastSection (true);
  m_file_list->sortByColumn (settings->value ("findfiles/sort_files_by_column",0).toInt (),
                             static_cast<Qt::SortOrder>
                             (settings->value ("findfiles/sort_files_by_order",
                                               Qt::AscendingOrder).toUInt ()));

  connect (m_file_list, SIGNAL (doubleClicked (const QModelIndex&)),
           this, SLOT (item_double_clicked (const QModelIndex &)));

  m_status_bar = new QStatusBar;
  m_status_bar->showMessage (tr ("Idle."));

  m_find_button = new QPushButton (tr ("Find"));
  m_find_button->setToolTip (tr ("Start search for matching files"));
  connect (m_find_button, SIGNAL (clicked (void)),
           this, SLOT (start_find (void)));

  m_stop_button = new QPushButton (tr ("Stop"));
  m_stop_button->setToolTip (tr ("Stop searching"));
  m_stop_button->setEnabled (false);
  connect (m_stop_button, SIGNAL (clicked (void)),
           this, SLOT (stop_find (void)));

  // layout everything
  QDialogButtonBox *button_box = new QDialogButtonBox (Qt::Vertical);
  button_box->addButton (m_find_button, QDialogButtonBox::ActionRole);
  button_box->addButton (m_stop_button, QDialogButtonBox::ActionRole);

  // add dialog close button
  m_close_button = button_box->addButton (QDialogButtonBox::Close);
  connect (button_box, SIGNAL (rejected (void)), this, SLOT (close (void)));

  // name options
  QGroupBox *name_group = new QGroupBox (tr ("Filename/location"));
  QGridLayout *name_layout = new QGridLayout;
  name_group->setLayout (name_layout);

  name_layout->addWidget (file_name_label,1,1, 1,1);
  name_layout->addWidget (m_file_name_edit,1,2, 1,-1);

  name_layout->addWidget (start_dir_label,2,1);
  name_layout->addWidget (m_start_dir_edit,2,2,1,3);
  name_layout->addWidget (m_browse_button,2,5);
  name_layout->setColumnStretch (2,1);

  name_layout->addWidget (m_recurse_dirs_check,3,1);
  name_layout->addWidget (m_include_dirs_check,3,2);
  name_layout->addWidget (m_name_case_check,3,3);

  // content options
  QGroupBox *content_group = new QGroupBox (tr ("File contents"));
  QGridLayout *content_layout = new QGridLayout;
  content_group->setLayout (content_layout);
  content_layout->addWidget (m_contains_text_check,4,1);
  content_layout->addWidget (m_contains_text_edit,4,2,1,3);
  content_layout->setColumnStretch (2,1);
  content_layout->addWidget (m_content_case_check,5,1);

  QGridLayout *main_layout = new QGridLayout;
  main_layout->setSizeConstraint (QLayout::SetFixedSize);
  main_layout->addWidget (name_group, 0, 0);
  main_layout->addWidget (content_group, 1, 0);
  main_layout->addWidget (button_box, 0, 1,3,1);
  main_layout->addWidget (m_file_list,2,0);
  main_layout->setRowStretch (2,1);
  main_layout->addWidget (m_status_bar,3,0,1,-1);

  setLayout (main_layout);

  connect (this, SIGNAL (finished (int)), this, SLOT (handle_done (int)));
}

find_files_dialog::~find_files_dialog (void)
{
  delete m_dir_iterator;
}

void
find_files_dialog::save_settings (void)
{
  QSettings *settings = resource_manager::get_settings ();

  if (! settings)
    return;

  int sort_column = m_file_list->horizontalHeader ()->sortIndicatorSection ();
  Qt::SortOrder sort_order
    = m_file_list->horizontalHeader ()->sortIndicatorOrder ();
  settings->setValue ("findfiles/sort_files_by_column", sort_column);
  settings->setValue ("findfiles/sort_files_by_order", sort_order);
  settings->setValue ("findfiles/column_state",
                      m_file_list->horizontalHeader ()->saveState ());

  settings->setValue ("findfiles/file_name", m_file_name_edit->text ());

  settings->setValue ("findfiles/start_dir", m_start_dir_edit->text ());

  settings->setValue ("findfiles/recurse_dirs", m_recurse_dirs_check->text ());
  settings->setValue ("findfiles/include_dirs", m_include_dirs_check->text ());
  settings->setValue ("findfiles/name_case", m_name_case_check->text ());

  settings->setValue ("findfiles/contains_text", m_contains_text_edit->text ());
  settings->setValue ("findfiles/check_text",
                      m_contains_text_check->isChecked ());
  settings->setValue ("findfiles/content_case",
                      m_content_case_check->isChecked ());

  settings->sync ();
}

void find_files_dialog::set_search_dir (const QString& dir)
{
  stop_find ();
  m_start_dir_edit->setText (dir);
}

void
find_files_dialog::start_find (void)
{
  stop_find ();

  find_files_model *m = static_cast<find_files_model *> (m_file_list->model ());
  m->clear ();

  QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
  if (m_recurse_dirs_check->isChecked ())
    flags |= QDirIterator::Subdirectories;

  QDir::Filters filters = QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files;
  if (! m_name_case_check->isChecked ())
    filters |= QDir::CaseSensitive;

  QStringList nameFilters;
  nameFilters.append (m_file_name_edit->text ());

  if (m_dir_iterator)
    delete m_dir_iterator;

  m_dir_iterator = new QDirIterator (m_start_dir_edit->text (), nameFilters,
                                     filters, flags);

  // enable/disable widgets
  m_find_button->setEnabled (false);
  m_stop_button->setEnabled (true);
  m_close_button->setEnabled (false);
  m_browse_button->setEnabled (false);
  m_start_dir_edit->setEnabled (false);
  m_file_name_edit->setEnabled (false);
  m_recurse_dirs_check->setEnabled (false);
  m_include_dirs_check->setEnabled (false);
  m_name_case_check->setEnabled (false);
  m_contains_text_check->setEnabled (false);
  m_content_case_check->setEnabled (false);
  m_contains_text_edit->setEnabled (false);

  m_status_bar->showMessage (tr ("Searching..."));
  m_timer->start (0);
}

void
find_files_dialog::stop_find (void)
{
  m_timer->stop ();

  m_find_button->setEnabled (true);
  m_stop_button->setEnabled (false);
  m_close_button->setEnabled (true);
  m_browse_button->setEnabled (true);
  m_start_dir_edit->setEnabled (true);
  m_file_name_edit->setEnabled (true);
  m_recurse_dirs_check->setEnabled (true);
  m_include_dirs_check->setEnabled (true);
  m_name_case_check->setEnabled (true);
  m_contains_text_check->setEnabled (true);
  m_content_case_check->setEnabled (true);
  m_contains_text_edit->setEnabled (true);

  find_files_model *m = static_cast<find_files_model *> (m_file_list->model ());
  QString res_str = QString (tr ("%1 match (es)")).arg (m->rowCount ());

  m_status_bar->showMessage (res_str);
}

void
find_files_dialog::browse_folders (void)
{
  QString dir =
    QFileDialog::getExistingDirectory (this, tr ("Set search directory"),
                                       m_start_dir_edit->text ());

  if (! dir.isEmpty ())
    m_start_dir_edit->setText (dir);
}

void
find_files_dialog::item_double_clicked (const QModelIndex& idx)
{
  find_files_model *m = static_cast<find_files_model *> (m_file_list->model ());

  QFileInfo info = m->fileInfo (idx);

  if (idx.column () == 1)
    {
      // clicked in directory part
      emit dir_selected (info.absolutePath ());
    }
  else
    {
      // clicked in filename part
      if (info.isDir ())
        emit dir_selected (info.absoluteFilePath ());
      else
        emit file_selected (info.absoluteFilePath ());
    }
}

void
find_files_dialog::look_for_files (void)
{
  if (m_dir_iterator && m_dir_iterator->hasNext ())
    {
      QFileInfo info (m_dir_iterator->next ());

      find_files_model *m
        = static_cast<find_files_model *> (m_file_list->model ());

      if (is_match (info))
        m->addFile (info);
    }
  else
    {
      stop_find ();
    }
}

void
find_files_dialog::handle_done (int)
{
  // make sure we stopped processing
  stop_find ();
}

bool
find_files_dialog::is_match (const QFileInfo& info)
{
  bool match = true;
  if (info.isDir ())
    {
      if (! m_include_dirs_check->isChecked ()) match = false;
      if (m_contains_text_check->isChecked ()) match = false;
    }
  else
    {
      // a file
      if (m_contains_text_check->isChecked ())
        {
          match = false;

          QFile file (info.absoluteFilePath ());
          if (file.open (QIODevice::ReadOnly))
            {
              QTextStream stream (&file);

              QString line;
              QString match_str = m_contains_text_edit->text ();

              Qt::CaseSensitivity cs = m_content_case_check->isChecked () ?
                Qt::CaseInsensitive : Qt::CaseSensitive;

              do
                {
                  line = stream.readLine ();
                  match = line.contains (match_str, cs);
                }
              while (! line.isNull () && match == false);
            }

        }
    }

  return match;
}
