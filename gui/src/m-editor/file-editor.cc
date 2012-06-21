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

#include "file-editor.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QTextStream>

file_editor::file_editor (QTerminal *terminal, main_window *m)
  : file_editor_interface(terminal, m)
{
  construct ();

  _terminal = terminal;
  _main_window = m;
  setVisible (false);
}

file_editor::~file_editor ()
{
}

lexer_octave_gui *
file_editor::lexer ()
{
  return _lexer;
}

QTerminal *
file_editor::terminal ()
{
  return _terminal;
}

main_window *
file_editor::get_main_window ()
{
  return _main_window;
}

QMenu *
file_editor::debug_menu ()
{
  return _debug_menu;
}

void
file_editor::request_new_file ()
{
  file_editor_tab *fileEditorTab = new file_editor_tab (this);
  if (fileEditorTab)
    {
      add_file_editor_tab (fileEditorTab);
      fileEditorTab->new_file ();
    }
}

void
file_editor::request_open_file ()
{
  file_editor_tab *fileEditorTab = new file_editor_tab (this);
  if (fileEditorTab)
    {
      add_file_editor_tab (fileEditorTab);
      if (!fileEditorTab->open_file ())
        {
          // If no file was loaded, remove the tab again.
          _tab_widget->removeTab (_tab_widget->indexOf (fileEditorTab));
        }
    }
}

void
file_editor::request_open_file (QString fileName)
{
  if (!isVisible ())
    {
      show ();
    }

  file_editor_tab *fileEditorTab = new file_editor_tab (this);
  if (fileEditorTab)
    {
      add_file_editor_tab (fileEditorTab);
      fileEditorTab->load_file (fileName);
    }
}

void
file_editor::request_undo ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->undo ();
}

void
file_editor::request_redo ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->redo ();
}

void
file_editor::request_copy ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->copy ();
}

void
file_editor::request_cut ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->cut ();
}

void
file_editor::request_paste ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->paste ();
}

void
file_editor::request_save_file ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->save_file ();
}

void
file_editor::request_save_file_as ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->save_file_as ();
}

void
file_editor::request_run_file ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->run_file ();
}

void
file_editor::request_toggle_bookmark ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->toggle_bookmark ();
}

void
file_editor::request_next_bookmark ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->next_bookmark ();
}

void
file_editor::request_previous_bookmark ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->previous_bookmark ();
}

void
file_editor::request_remove_bookmark ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->remove_bookmark ();
}

void
file_editor::request_comment_selected_text ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->comment_selected_text ();
}

void
file_editor::request_uncomment_selected_text ()
{
  file_editor_tab *activeFileEditorTab = active_editor_tab ();
  if (activeFileEditorTab)
    activeFileEditorTab->uncomment_selected_text ();
}

void
file_editor::handle_file_name_changed (QString fileName)
{
  QObject *senderObject = sender ();
  file_editor_tab *fileEditorTab = dynamic_cast<file_editor_tab*> (senderObject);
  if (fileEditorTab)
    {
      for(int i = 0; i < _tab_widget->count (); i++)
        {
          if (_tab_widget->widget (i) == fileEditorTab)
            {
              _tab_widget->setTabText (i, fileName);
            }
        }
    }
}

void
file_editor::handle_tab_close_request (int index)
{
  file_editor_tab *fileEditorTab = dynamic_cast <file_editor_tab*> (_tab_widget->widget (index));
  if (fileEditorTab)
    if (fileEditorTab->close ())
      {
        _tab_widget->removeTab (index);
        delete fileEditorTab;
      }
}

void
file_editor::handle_tab_close_request ()
{
  file_editor_tab *fileEditorTab = dynamic_cast <file_editor_tab*> (sender ());
  if (fileEditorTab)
    if (fileEditorTab->close ())
      {
        _tab_widget->removeTab (_tab_widget->indexOf (fileEditorTab));
        delete fileEditorTab;
      }
}

void
file_editor::active_tab_changed (int index)
{
  Q_UNUSED (index);
  handle_editor_state_changed ();
}

void
file_editor::handle_editor_state_changed ()
{
  file_editor_tab *f = active_editor_tab ();
  if (f)
    {
      bool copy_available = f->copy_available ();
      _copy_action->setEnabled (copy_available);
      _cut_action->setEnabled (copy_available);
    }
}

void
file_editor::construct ()
{
  QWidget *widget = new QWidget (this);
  QSettings *settings = resource_manager::instance ()->get_settings ();
  QStyle *style = QApplication::style ();

  _menu_bar = new QMenuBar (widget);
  _tool_bar = new QToolBar (widget);
  _tab_widget = new QTabWidget (widget);
  _tab_widget->setTabsClosable (true);

  // Theme icons with QStyle icons as fallback
  QAction *new_action = new QAction (QIcon(":/actions/icons/filenew.png"),
        tr("&New File"), _tool_bar);

  QAction *open_action = new QAction (QIcon(":/actions/icons/fileopen.png"),
        tr("&Open File"), _tool_bar);

  QAction *save_action = new QAction (QIcon(":/actions/icons/filesave.png"),
        tr("&Save File"), _tool_bar);

  QAction *save_as_action = new QAction (QIcon(":/actions/icons/filesaveas.png"),
        tr("Save File &As"), _tool_bar);

  QAction *undo_action = new QAction (QIcon(":/actions/icons/undo.png"),
        tr("&Undo"), _tool_bar);

  QAction *redo_action = new QAction (QIcon(":/actions/icons/redo.png"),
        tr("&Redo"), _tool_bar);

  _copy_action = new QAction (QIcon::fromTheme ("edit-copy"), tr ("&Copy"), _tool_bar);
  _cut_action  = new QAction (QIcon::fromTheme ("edit-cut"), tr ("Cu&t"), _tool_bar);

  QAction *paste_action
      = new QAction (QIcon (":/actions/icons/editpaste.png"),
                     tr("Paste"), _tool_bar);
  QAction *next_bookmark_action       = new QAction (tr ("&Next Bookmark"),_tool_bar);
  QAction *previous_bookmark_action   = new QAction (tr ("Pre&vious Bookmark"),_tool_bar);
  QAction *toggle_bookmark_action     = new QAction (tr ("Toggle &Bookmark"),_tool_bar);
  QAction *remove_bookmark_action     = new QAction (tr ("&Remove All Bookmarks"),_tool_bar);
  QAction *comment_selection_action   = new QAction (tr ("&Comment Selected Text"),_tool_bar);
  QAction *uncomment_selection_action = new QAction (tr ("&Uncomment Selected Text"),_tool_bar);

  QAction *run_action = new QAction (QIcon(":/actions/icons/artsbuilderexecute.png"),
        tr("&Run File"), _tool_bar);

  // some actions are disabled from the beginning
  _copy_action->setEnabled(false);
  _cut_action->setEnabled(false);

  // short cuts
  new_action->setShortcut                       (QKeySequence::New);
  new_action->setShortcutContext                (Qt::WindowShortcut);
  open_action->setShortcut                      (QKeySequence::Open);
  open_action->setShortcutContext               (Qt::WindowShortcut);
  save_action->setShortcut                      (QKeySequence::Save);
  save_action->setShortcutContext               (Qt::WindowShortcut);
  save_as_action->setShortcut                   (QKeySequence::SaveAs);
  save_as_action->setShortcutContext            (Qt::WindowShortcut);
  undo_action->setShortcut                      (QKeySequence::Undo);
  undo_action->setShortcutContext               (Qt::WindowShortcut);
  redo_action->setShortcut                      (QKeySequence::Redo);
  redo_action->setShortcutContext               (Qt::WindowShortcut);
  _copy_action->setShortcut                     (QKeySequence::Copy);
  _copy_action->setShortcutContext              (Qt::WindowShortcut);
  _cut_action->setShortcut                      (QKeySequence::Cut);
  _cut_action->setShortcutContext               (Qt::WindowShortcut);
  paste_action->setShortcut                     (QKeySequence::Paste);
  paste_action->setShortcutContext              (Qt::WindowShortcut);
  run_action->setShortcut                       (Qt::SHIFT + Qt::Key_R);
  run_action->setShortcutContext                (Qt::WindowShortcut);
  next_bookmark_action->setShortcut             (Qt::Key_F2);
  next_bookmark_action->setShortcutContext      (Qt::WindowShortcut);
  previous_bookmark_action->setShortcut         (Qt::SHIFT + Qt::Key_F2);
  previous_bookmark_action->setShortcutContext  (Qt::WindowShortcut);
  toggle_bookmark_action->setShortcut           (Qt::Key_F7);
  toggle_bookmark_action->setShortcutContext    (Qt::WindowShortcut);
  comment_selection_action->setShortcut         (Qt::CTRL + Qt::Key_R);
  comment_selection_action->setShortcutContext  (Qt::WindowShortcut);
  uncomment_selection_action->setShortcut       (Qt::CTRL + Qt::Key_T);
  uncomment_selection_action->setShortcutContext(Qt::WindowShortcut);

  // toolbar
  _tool_bar->addAction (new_action);
  _tool_bar->addAction (open_action);
  _tool_bar->addAction (save_action);
  _tool_bar->addAction (save_as_action);
  _tool_bar->addSeparator ();
  _tool_bar->addAction (undo_action);
  _tool_bar->addAction (redo_action);
  _tool_bar->addAction (_copy_action);
  _tool_bar->addAction (_cut_action);
  _tool_bar->addAction (paste_action);
  _tool_bar->addSeparator ();
  _tool_bar->addAction (run_action);

  // menu bar
  QMenu *fileMenu = new QMenu (tr ("&File"), _menu_bar);
  fileMenu->addAction (new_action);
  fileMenu->addAction (open_action);
  fileMenu->addAction (save_action);
  fileMenu->addAction (save_as_action);
  fileMenu->addSeparator ();
  _menu_bar->addMenu (fileMenu);

  QMenu *editMenu = new QMenu (tr ("&Edit"), _menu_bar);
  editMenu->addAction (undo_action);
  editMenu->addAction (redo_action);
  editMenu->addSeparator ();
  editMenu->addAction (_copy_action);
  editMenu->addAction (_cut_action);
  editMenu->addAction (paste_action);
  editMenu->addSeparator ();
  editMenu->addAction (comment_selection_action);
  editMenu->addAction (uncomment_selection_action);
  editMenu->addSeparator ();
  editMenu->addAction (toggle_bookmark_action);
  editMenu->addAction (next_bookmark_action);
  editMenu->addAction (previous_bookmark_action);
  editMenu->addAction (remove_bookmark_action);
  _menu_bar->addMenu (editMenu);

  _debug_menu = new QMenu (tr ("&Debug"), _menu_bar);
  _menu_bar->addMenu (_debug_menu);

  QMenu *runMenu = new QMenu (tr ("&Run"), _menu_bar);
  runMenu->addAction (run_action);
  _menu_bar->addMenu (runMenu);

  QVBoxLayout *layout = new QVBoxLayout ();
  layout->addWidget (_menu_bar);
  layout->addWidget (_tool_bar);
  layout->addWidget (_tab_widget);
  layout->setMargin (0);
  widget->setLayout (layout);
  setWidget (widget);

  connect (new_action,               SIGNAL (triggered ()), this, SLOT (request_new_file ()));
  connect (open_action,              SIGNAL (triggered ()), this, SLOT (request_open_file ()));
  connect (undo_action,              SIGNAL (triggered ()), this, SLOT (request_undo ()));
  connect (redo_action,              SIGNAL (triggered ()), this, SLOT (request_redo ()));
  connect (_copy_action,            SIGNAL (triggered ()), this, SLOT (request_copy ()));
  connect (_cut_action,             SIGNAL (triggered ()), this, SLOT (request_cut ()));
  connect (paste_action,             SIGNAL (triggered ()), this, SLOT (request_paste ()));
  connect (save_action,              SIGNAL (triggered ()), this, SLOT (request_save_file ()));
  connect (save_as_action,            SIGNAL (triggered ()), this, SLOT (request_save_file_as ()));
  connect (run_action,               SIGNAL (triggered ()), this, SLOT (request_run_file ()));
  connect (toggle_bookmark_action,    SIGNAL (triggered ()), this, SLOT (request_toggle_bookmark ()));
  connect (next_bookmark_action,      SIGNAL (triggered ()), this, SLOT (request_next_bookmark ()));
  connect (previous_bookmark_action,      SIGNAL (triggered ()), this, SLOT (request_previous_bookmark ()));
  connect (remove_bookmark_action,    SIGNAL (triggered ()), this, SLOT (request_remove_bookmark ()));
  connect (comment_selection_action,   SIGNAL (triggered ()), this, SLOT (request_comment_selected_text ()));
  connect (uncomment_selection_action, SIGNAL (triggered ()), this, SLOT (request_uncomment_selected_text ()));
  connect (_tab_widget, SIGNAL (tabCloseRequested (int)), this, SLOT (handle_tab_close_request (int)));
  connect (_tab_widget, SIGNAL (currentChanged(int)), this, SLOT (active_tab_changed (int)));

  // this has to be done only once, not for each editor
  _lexer = new lexer_octave_gui ();

  // Editor font (default or from settings)
  _lexer->setDefaultFont (QFont (
                             settings->value ("editor/fontName","Courier").toString (),
                             settings->value ("editor/fontSize",10).toInt ()));

  // TODO: Autoindent not working as it should
  _lexer->setAutoIndentStyle (QsciScintilla::AiMaintain ||
                               QsciScintilla::AiOpening  ||
                               QsciScintilla::AiClosing);

  // The API info that is used for auto completion
  // TODO: Where to store a file with API info (raw or prepared?)?
  // TODO: Also provide infos on octave-forge functions?
  // TODO: Also provide infos on function parameters?
  // By now, use the keywords-list from syntax highlighting
  _lexer_api = new QsciAPIs (_lexer);

  QString keyword;
  QStringList keywordList;
  keyword = _lexer->keywords (1);  // get whole string with all keywords
  keywordList = keyword.split (QRegExp ("\\s+"));  // split into single strings
  int i;
  for (i = 0; i < keywordList.size (); i++)
    {
      _lexer_api->add (keywordList.at (i));  // add single strings to the API
    }
  _lexer_api->prepare ();           // prepare API info ... this make take some time
  resize (500, 400);
  setWindowIcon (QIcon::fromTheme ("accessories-text-editor", style->standardIcon (QStyle::SP_FileIcon)));
  setWindowTitle ("Octave Editor");
}

void
file_editor::add_file_editor_tab (file_editor_tab *f)
{
  _tab_widget->addTab (f, "");
  connect (f, SIGNAL (file_name_changed(QString)),
           this, SLOT(handle_file_name_changed(QString)));
  connect (f, SIGNAL (editor_state_changed ()),
           this, SLOT (handle_editor_state_changed ()));
  connect (f, SIGNAL (close_request ()),
           this, SLOT (handle_tab_close_request ()));
  _tab_widget->setCurrentWidget (f);
}

file_editor_tab *
file_editor::active_editor_tab ()
{
  return dynamic_cast<file_editor_tab*> (_tab_widget->currentWidget ());
}
