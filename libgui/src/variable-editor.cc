/*

Copyright (C) 2013-2017 John W. Eaton
Copyright (C) 2015 Michael Barnes
Copyright (C) 2013 Rüdiger Sonderfeld

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <limits>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QScrollBar>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QTableView>
#include <QTextEdit>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "resource-manager.h"
#include "shortcut-manager.h"
#include "variable-editor.h"
#include "variable-editor-model.h"

static QString
idx_to_expr (int32_t from, int32_t to)
{
  return (from == to
          ? QString ("%1").arg (from + 1)
          : QString ("%1:%2").arg (from + 1).arg (to + 1));
}

QTableView *
var_editor_tab::get_edit_view (void) const
{
  return (m_edit_view_idx == m_widget_stack->currentIndex ()
          ? qobject_cast<QTableView *> (m_widget_stack->widget (m_edit_view_idx))
          : nullptr);
}

QTextEdit *
var_editor_tab::get_disp_view (void) const
{
  return (m_disp_view_idx == m_widget_stack->currentIndex ()
          ? qobject_cast<QTextEdit *> (m_widget_stack->widget (m_disp_view_idx))
          : nullptr);
}

void
var_editor_tab::set_edit_view (QTableView *edit_view)
{
  m_edit_view_idx = m_widget_stack->addWidget (edit_view);
}

void
var_editor_tab::set_disp_view (QTextEdit *disp_view)
{
  m_disp_view_idx = m_widget_stack->addWidget (disp_view);
}

bool
var_editor_tab::has_focus (void) const
{
  QTableView *edit_view = get_edit_view ();
  QTextEdit *disp_view = get_disp_view ();

  return ((disp_view && disp_view->hasFocus ())
          || (edit_view && edit_view->hasFocus ()));
}

void
var_editor_tab::keyPressEvent (QKeyEvent *event)
{
  QTableView *edit_view = get_edit_view ();

  if (edit_view)
    {
      int key = event->key ();

      if (key == Qt::Key_Right || key == Qt::Key_Tab)
        {
          QModelIndex idx = edit_view->currentIndex ();

          int curr_row = idx.row ();
          int next_col = idx.column () + 1;

          if (next_col == m_model->display_columns ())
            {
              m_model->maybe_resize_columns (next_col + 16);

              edit_view->setCurrentIndex (m_model->index (curr_row, next_col));
            }
        }
      else if (key == Qt::Key_Down || key == Qt::Key_PageDown)
        {
          QModelIndex idx = edit_view->currentIndex ();

          int next_row = idx.row () + 1;
          int curr_col = idx.column ();

          if (next_row == m_model->display_rows ())
            {
              m_model->maybe_resize_rows (next_row + 16);

              edit_view->setCurrentIndex (m_model->index (next_row, curr_col));
            }
        }
    }
}

void
var_editor_tab::set_editable (bool editable)
{
  int idx = (editable ? m_edit_view_idx : m_disp_view_idx);

  m_widget_stack->setCurrentIndex (idx);

  if (! editable)
    {
      QTextEdit *viewer = get_disp_view ();

      QVariant v_data = m_model->data ();

      QString str = v_data.toString ();

      viewer->setPlainText (str);
    }
}

void
var_editor_tab::handle_horizontal_scroll_action (int action)
{
  if (action == QAbstractSlider::SliderSingleStepAdd
      || action == QAbstractSlider::SliderPageStepAdd
      || action == QAbstractSlider::SliderToMaximum
      || action == QAbstractSlider::SliderMove)
    {
      QTableView *edit_view = get_edit_view ();

      if (edit_view)
        {
          QScrollBar *sb = edit_view->horizontalScrollBar ();

          if (sb && sb->value () == sb->maximum ())
            {
              int new_cols = m_model->display_columns () + 16;

              m_model->maybe_resize_columns (new_cols);
            }
        }
    }
}

void
var_editor_tab::handle_vertical_scroll_action (int action)
{
  if (action == QAbstractSlider::SliderSingleStepAdd
      || action == QAbstractSlider::SliderPageStepAdd
      || action == QAbstractSlider::SliderToMaximum
      || action == QAbstractSlider::SliderMove)
    {
      QTableView *edit_view = get_edit_view ();

      if (edit_view)
        {
          QScrollBar *sb = edit_view->verticalScrollBar ();

          if (sb && sb->value () == sb->maximum ())
            {
              int new_rows = m_model->display_rows () + 16;

              m_model->maybe_resize_rows (new_rows);
            }
        }
    }
}

// Functions for reimplemented tab widget.

var_editor_tab_widget::var_editor_tab_widget (QWidget *p)
  : QTabWidget (p)
{
  tab_bar *bar = new tab_bar (this);

  connect (bar, SIGNAL (close_current_tab_signal (bool)),
           p->parent (), SLOT (request_close_tab (bool)));

  this->setTabBar (bar);

  setTabsClosable (true);
  setMovable (true);
}

tab_bar *
var_editor_tab_widget::get_tab_bar (void) const
{
  return qobject_cast<tab_bar *> (tabBar ());
}

bool
var_editor_tab_widget::current_tab_has_focus (void) const
{
  var_editor_tab *tab
    = qobject_cast<var_editor_tab *> (widget (currentIndex ()));

  return tab->has_focus ();
}

QTableView *
var_editor_tab_widget::get_edit_view (void) const
{
  var_editor_tab *tab
    = qobject_cast<var_editor_tab *> (widget (currentIndex ()));

  return tab->get_edit_view ();
}

QTextEdit *
var_editor_tab_widget::get_disp_view (void) const
{
  var_editor_tab *tab
    = qobject_cast<var_editor_tab *> (widget (currentIndex ()));

  return tab->get_disp_view ();
}

// Variable editor.

variable_editor::variable_editor (QWidget *p)
  : octave_dock_widget (p),
    m_container (new QWidget (this)),
    m_tool_bar (new QToolBar ("", m_container)),
    m_tab_widget (new var_editor_tab_widget (m_container)),
    m_tab_bar (m_tab_widget->get_tab_bar ()),
    m_default_width (30),
    m_default_height (100),
    m_add_font_height (0),
    m_use_terminal_font (true),
    m_alternate_rows (true),
    m_stylesheet (""),
    m_font (),
    m_sel_font (),
    m_table_colors (),
    m_close_action (nullptr),
    m_close_others_action (nullptr),
    m_close_all_action (nullptr)
{
  setObjectName ("variable_editor");
  set_title (tr ("Variable Editor"));
  setStatusTip (tr ("Edit variables."));
  setWindowIcon (QIcon (":/actions/icons/logo.png"));

  construct_tool_bar ();

  // Colors.

  for (int i = 0; i < resource_manager::varedit_color_chars ().length (); i++)
    m_table_colors.append (QColor (Qt::white));

  // Tab Widget.

  connect (m_tab_widget, SIGNAL (tabCloseRequested (int)),
           this, SLOT (closeTab (int)));

  // Tab bar.

  m_close_action
    = add_action (m_tab_bar->get_context_menu (),
                  resource_manager::icon ("window-close",false),
                  tr ("&Close"),
                  SLOT (request_close_tab (bool)));

  m_close_others_action
    = add_action (m_tab_bar->get_context_menu (),
                  resource_manager::icon ("window-close",false),
                  tr ("Close &Other Tabs"),
                  SLOT (request_close_other_tabs (bool)));

  m_close_all_action
    = add_action (m_tab_bar->get_context_menu (),
                  resource_manager::icon ("window-close",false),
                  tr ("Close &All Tabs"),
                  SLOT (request_close_all_tabs (bool)));

  enable_actions ();

  // Layout the widgets vertically with the toolbar on top.

  QVBoxLayout *vbox_layout = new QVBoxLayout ();

  vbox_layout->setSpacing (0);
  vbox_layout->addWidget (m_tool_bar);
  vbox_layout->addWidget (m_tab_widget);
  vbox_layout->setMargin (1);

  m_container->setLayout (vbox_layout);

  setWidget (m_container);

  connect (this, SIGNAL (command_requested (const QString&)),
           p, SLOT (execute_command_in_terminal (const QString&)));
}

// Add an action to a menu or the widget itself.

QAction*
variable_editor::add_action (QMenu *menu, const QIcon& icon, const QString& text,
                             const char *member)
{
  QAction *a;

  if (menu)
    a = menu->addAction (icon, text, this, member);
  else
    {
      a = new QAction (this);
      connect (a, SIGNAL (triggered ()), this, member);
    }

  addAction (a);  // important for shortcut context
  a->setShortcutContext (Qt::WidgetWithChildrenShortcut);

  return a;
}

// Slot for the close tab action.

void
variable_editor::request_close_tab (bool)
{
  closeTab (m_tab_bar->currentIndex ());
}

// Slot for the close other tabs action.

void
variable_editor::request_close_other_tabs (bool)
{
  int current = m_tab_bar->currentIndex ();

  for (int index = m_tab_bar->count ()-1; index >= 0; index--)
    {
      if (current != index)
        closeTab (index);
    }
}

// Slot for closing all tabs.

void
variable_editor::request_close_all_tabs (bool)
{
  for (int index = m_tab_bar->count ()-1; index >= 0; index--)
    closeTab (index);
}

void
variable_editor::enable_actions (void)
{
  const int count = m_tab_widget->count ();

  m_tool_bar->setEnabled (count > 0);
  m_close_action->setEnabled (count > 0);
  m_close_all_action->setEnabled (count > 0);
  m_close_others_action->setEnabled (count > 1);
}

void
variable_editor::edit_variable (const QString& name, const octave_value& val)
{
  if (m_stylesheet.isEmpty ())
    {
      QSettings *settings = resource_manager::get_settings ();
      notice_settings (settings);
    }

  const int tab_count = m_tab_widget->count ();
  for (int i = 0; i < tab_count; ++i)
    {
      if (real_var_name (i) == name)
        {
          // Already open.

          m_tab_widget->setCurrentIndex (i);
          return;
        }
    }

  // Do not set parent.

  QStackedWidget *widget_stack = new QStackedWidget ();

  var_editor_tab *page = new var_editor_tab (widget_stack);

  QVBoxLayout *vbox = new QVBoxLayout (page);
  page->setLayout (vbox);

  QLabel *label = new QLabel (page);
  label->setTextFormat (Qt::PlainText);
  vbox->addWidget (label, 0, Qt::AlignTop);

  variable_editor_model *model = new variable_editor_model (name, val);

  QTableView *edit_view = make_edit_view (page, model);

  QTextEdit *disp_view = make_disp_view (page, model);

  page->set_model (model);

  page->set_edit_view (edit_view);
  page->set_disp_view (disp_view);

  vbox->addWidget (widget_stack);

  int tab_idx = m_tab_widget->addTab (page, name);

  m_tab_widget->setCurrentIndex (tab_idx);

  connect (model, SIGNAL (description_changed (const QString&)),
           label, SLOT (setText (const QString&)));

  connect (model, SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
           this, SLOT (callUpdate (const QModelIndex&, const QModelIndex&)));

  connect (this, SIGNAL (refresh_signal (void)),
           model, SLOT (update_data_cache (void)));

  connect (model, SIGNAL (set_editable_signal (bool)),
           page, SLOT (set_editable (bool)));

  enable_actions ();
}

QTableView *
variable_editor::make_edit_view (var_editor_tab *page,
                                  variable_editor_model *model)
{
  QTableView *table = new QTableView (page);

  model->setParent (table);

  table->setModel (model);
  table->setWordWrap (false);
  table->setContextMenuPolicy (Qt::CustomContextMenu);
  table->setSelectionMode (QAbstractItemView::ContiguousSelection);

  table->horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
  table->verticalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);

  connect (table->horizontalHeader (),
           SIGNAL (customContextMenuRequested (const QPoint&)),
           this, SLOT (columnmenu_requested (const QPoint&)));

  connect (table->verticalHeader (),
           SIGNAL (customContextMenuRequested (const QPoint&)),
           this, SLOT (rowmenu_requested (const QPoint&)));

  connect (table, SIGNAL (customContextMenuRequested (const QPoint&)),
           this, SLOT (contextmenu_requested (const QPoint&)));

  connect (table, SIGNAL (doubleClicked (const QModelIndex&)),
           this, SLOT (double_click (const QModelIndex&)));

  table->setHorizontalScrollMode (QAbstractItemView::ScrollPerPixel);
  table->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);

  connect (table->horizontalScrollBar (), SIGNAL (actionTriggered (int)),
           page, SLOT (handle_horizontal_scroll_action (int)));

  connect (table->verticalScrollBar (), SIGNAL (actionTriggered (int)),
           page, SLOT (handle_vertical_scroll_action (int)));

  table->setFont (m_font);
  table->setStyleSheet (m_stylesheet);
  table->setAlternatingRowColors (m_alternate_rows);
#if defined (HAVE_QT4)
  table->verticalHeader ()->setResizeMode (QHeaderView::Interactive);
#else
  table->verticalHeader ()->setSectionResizeMode (QHeaderView::Interactive);
#endif
  table->verticalHeader ()->setDefaultSectionSize (m_default_height
                                                   + m_add_font_height);

  int col_width = model->column_width ();

  if (col_width > 0)
    {
#if defined (HAVE_QT4)
      table->horizontalHeader ()->setResizeMode (QHeaderView::Interactive);
#else
      table->horizontalHeader ()->setSectionResizeMode (QHeaderView::Interactive);
#endif

      // col_width is in characters.  The font should be a fixed-width
      // font, so any character will do.  If not, you lose!

      QFontMetrics fm (m_font);
      int w = col_width * fm.width ('0');
      table->horizontalHeader ()->setDefaultSectionSize (w);
    }

  return table;
}

QTextEdit *
variable_editor::make_disp_view (var_editor_tab *page,
                                 variable_editor_model *model)
{
  QTextEdit *viewer = new QTextEdit (page);

  model->setParent (viewer);

  QVariant v_data = model->data ();

  QString str = v_data.toString ();

  viewer->setPlainText (str);

  return viewer;
}

void
variable_editor::refresh (void)
{
  emit refresh_signal ();
}

bool
variable_editor::has_focus (void)
{
  // FIXME: This only generates exceptions in certain circumstances.
  // Get a definitive list and eliminate the need to handle exceptions?

  if (m_tab_widget->currentIndex () == -1)
    return false;  // No tabs.

  try
    {
      return m_tab_widget->current_tab_has_focus ();
    }
  catch (...)
    {
      return false;
    }

  return false;
}

QList<QColor>
variable_editor::default_colors (void)
{
  QList<QColor> colorlist;

  colorlist << qApp->palette ().color (QPalette::WindowText);
  colorlist << qApp->palette ().color (QPalette::Base);
  colorlist << qApp->palette ().color (QPalette::HighlightedText);
  colorlist << qApp->palette ().color (QPalette::Highlight);
  colorlist << qApp->palette ().color (QPalette::AlternateBase);

  return colorlist;
}

QStringList
variable_editor::color_names (void)
{
  QStringList output;

  output << "Foreground";
  output << "Background";
  output << "Selected Foreground";
  output << "Selected Background";
  output << "Alternate Background";

  return output;
}

void
variable_editor::callUpdate (const QModelIndex&, const QModelIndex&)
{
  emit updated ();
}

void
variable_editor::notice_settings (const QSettings *settings)
{
  // FIXME: Why use object->tostring->toint?  Why not just 100?

  m_default_width = settings->value ("variable_editor/column_width",
                                     100).toInt ();

  m_default_height = settings->value ("variable_editor/row_height",
                                      10).toInt ();

  m_alternate_rows = settings->value ("variable_editor/alternate_rows",
                                      false).toBool ();

  QList<QColor> _default_colors = resource_manager::varedit_default_colors ();

  QString class_chars = resource_manager::varedit_color_chars ();

  m_use_terminal_font = settings->value ("variable_editor/use_terminal_font",
                                         true).toBool ();

  QString font_name;
  int font_size;

  if (m_use_terminal_font)
    {
      font_name = settings->value ("terminal/fontName", "Courier New").toString ();
      font_size = settings->value ("terminal/fontSize", 10).toInt ();
    }
  else
    {
      font_name = settings->value ("variable_editor/font_name",
                                   settings->value ("terminal/fontName",
                                                    "Courier New")).toString ();

      font_size = settings->value ("variable_editor/font_size", 10).toInt ();
    }

  m_font = QFont (font_name, font_size);

  QFontMetrics fm (m_font);

  m_add_font_height = fm.height ();

  for (int i = 0; i < class_chars.length (); i++)
    {
      QVariant default_var = _default_colors.at (i);

      QColor setting_color = settings->value ("variable_editor/color_"
                                              + class_chars.mid (i, 1),
                                              default_var).value<QColor> ();

      m_table_colors.replace (i, setting_color);
    }

  update_colors ();

  // Icon size in the toolbar.

  int icon_size_settings = settings->value ("toolbar_icon_size", 0).toInt ();
  QStyle *st = style ();
  int icon_size = st->pixelMetric (QStyle::PM_ToolBarIconSize);

  // FIXME: Magic numbers.  Use enum?

  if (icon_size_settings == 1)
    icon_size = st->pixelMetric (QStyle::PM_LargeIconSize);
  else if (icon_size_settings == -1)
    icon_size = st->pixelMetric (QStyle::PM_SmallIconSize);

  m_tool_bar->setIconSize (QSize (icon_size, icon_size));

  // Shortcuts.

  shortcut_manager::set_shortcut (m_close_action, "editor_file:close");
  shortcut_manager::set_shortcut (m_close_all_action, "editor_file:close_all");
  shortcut_manager::set_shortcut (m_close_others_action, "editor_file:close_other");
}

void
variable_editor::closeEvent (QCloseEvent *e)
{
  emit finished ();

  octave_dock_widget::closeEvent (e);
}

void
variable_editor::closeTab (int idx)
{
  if (idx < 0 || idx > m_tab_widget->count ())
    return;

  QWidget *wdgt = m_tab_widget->widget (idx);

  m_tab_widget->removeTab (idx);

  delete wdgt;

  enable_actions ();
}

void
variable_editor::contextmenu_requested (const QPoint& qpos)
{
  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QModelIndex index = view->indexAt (qpos);

  if (index.isValid ())
    {
      QMenu *menu = new QMenu (this);

      menu->addAction (resource_manager::icon ("edit-cut"),
                       tr ("Cut"), this, SLOT (cutClipboard ()));

      menu->addAction (resource_manager::icon ("edit-copy"),
                       tr ("Copy"), this, SLOT (copyClipboard ()));

      menu->addAction (resource_manager::icon ("edit-paste"),
                       tr ("Paste"), this, SLOT (pasteClipboard ()));

      // FIXME: Need different icon for paste table separate from paste?

      menu->addAction (resource_manager::icon ("edit-paste"),
                       tr ("Paste Table"), this,
                       SLOT (pasteTableClipboard ()));

      menu->addSeparator ();

      menu->addAction (resource_manager::icon ("edit-delete"),
                       tr ("Clear"), this, SLOT (clearContent ()));

      menu->addAction (resource_manager::icon ("document-new"),
                       tr ("Variable from Selection"), this,
                       SLOT (createVariable ()));

      // FIXME: addAction for sort?
      // FIXME: Add icon for transpose.

      menu->addAction (tr ("Transpose"), this, SLOT (transposeContent ()));

      QItemSelectionModel *sel = view->selectionModel ();

      QList<QModelIndex> indices = sel->selectedIndexes ();

      if (! indices.isEmpty ())
        {
          menu->addSeparator ();

          QSignalMapper *plot_mapper = new QSignalMapper (menu);

          plot_mapper->setMapping
            (menu->addAction ("plot", plot_mapper, SLOT (map ())),
             "figure (); plot (%1);");

          plot_mapper->setMapping
            (menu->addAction ("bar", plot_mapper, SLOT (map ())),
             "figure (); bar (%1);");

          plot_mapper->setMapping
            (menu->addAction ("stem", plot_mapper, SLOT (map ())),
             "figure (); stem (%1);");

          plot_mapper->setMapping
            (menu->addAction ("stairs", plot_mapper, SLOT (map ())),
             "figure (); stairs (%1);");

          plot_mapper->setMapping
            (menu->addAction ("area", plot_mapper, SLOT (map ())),
             "figure (); area (%1);");

          plot_mapper->setMapping
            (menu->addAction ("pie", plot_mapper, SLOT (map ())),
             "figure (); pie (%1);");

          plot_mapper->setMapping
            (menu->addAction ("hist", plot_mapper, SLOT (map ())),
             "figure (); hist (%1);");

          connect (plot_mapper, SIGNAL (mapped (const QString&)),
                   this, SLOT (relay_command (const QString&)));
        }

      menu->exec (view->mapToGlobal (qpos));
    }
}

void
variable_editor::columnmenu_requested (const QPoint& pt)
{
  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  int index = view->horizontalHeader ()->logicalIndexAt (pt);

  if (index < 0 || index > view->model ()->columnCount ())
    return;

  QString selection = selected_to_octave ();

  QList<int> coords = octave_to_coords (selection);

  bool nothingSelected = coords.isEmpty ();

  bool whole_columns_selected
    =  (nothingSelected
        ? false
        : (coords[0] == 1 && coords[1] == view->model ()->rowCount ()));

  bool current_column_selected
    = nothingSelected ? false : (coords[2] <= index+1 && coords[3] > index);

  int column_selection_count
    = nothingSelected ? 0 : (coords[3] - coords[2] + 1);

  if (! whole_columns_selected || ! current_column_selected)
    {
      view->selectColumn (index);
      column_selection_count = 1;
      current_column_selected = true;
      whole_columns_selected = true;
    }

  QString column_string
    = tr (column_selection_count > 1 ? " columns" : " column");

  QMenu *menu = new QMenu (this);

  menu->addAction (resource_manager::icon ("edit-cut"),
                   tr ("Cut") + column_string,
                   this, SLOT (cutClipboard ()));

  menu->addAction (resource_manager::icon ("edit-copy"),
                   tr ("Copy") + column_string,
                   this, SLOT (copyClipboard ()));

  menu->addAction (resource_manager::icon ("edit-paste"),
                   tr ("Paste"),
                   this, SLOT (pasteClipboard ()));

  // FIXME: Different icon for Paste Table?

  menu->addAction (resource_manager::icon ("edit-paste"),
                   tr ("Paste Table"),
                   this, SLOT (pasteTableClipboard ()));

  menu->addSeparator ();

  menu->addAction (resource_manager::icon ("edit-delete"),
                   tr ("Clear") + column_string,
                   this, SLOT (clearContent ()));

  menu->addAction (resource_manager::icon ("edit-delete"),
                   tr ("Delete") + column_string,
                   this, SLOT (delete_selected ()));

  menu->addAction (resource_manager::icon ("document-new"),
                   tr ("Variable from Selection"),
                   this, SLOT (createVariable ()));

  menu->addSeparator ();

  QSignalMapper *plot_mapper = new QSignalMapper (menu);

  plot_mapper->setMapping
    (menu->addAction ("plot", plot_mapper, SLOT (map ())),
     "figure (); plot (%1);");

  plot_mapper->setMapping
    (menu->addAction ("bar", plot_mapper, SLOT (map ())),
     "figure (); bar (%1);");

  plot_mapper->setMapping
    (menu->addAction ("stem", plot_mapper, SLOT (map ())),
     "figure (); stem (%1);");

  plot_mapper->setMapping
    (menu->addAction ("stairs", plot_mapper, SLOT (map ())),
     "figure (); stairs (%1);");

  plot_mapper->setMapping
    (menu->addAction ("area", plot_mapper, SLOT (map ())),
     "figure (); area (%1);");

  plot_mapper->setMapping
    (menu->addAction ("pie", plot_mapper, SLOT (map ())),
     "figure (); pie (%1);");

  plot_mapper->setMapping
    (menu->addAction ("hist", plot_mapper, SLOT (map ())),
     "figure (); hist (%1);");

  connect (plot_mapper, SIGNAL (mapped (const QString&)),
           this, SLOT (relay_command (const QString&)));

  QPoint menupos = pt;
  menupos.setY (view->horizontalHeader ()->height ());

  menu->exec (view->mapToGlobal (menupos));
}

void
variable_editor::rowmenu_requested (const QPoint& pt)
{
  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  int index = view->verticalHeader ()->logicalIndexAt (pt);

  if (index < 0 || index > view->model ()->columnCount ())
    return;

  QString selection = selected_to_octave ();

  QList<int> coords = octave_to_coords (selection);

  bool nothingSelected = coords.isEmpty ();

  bool whole_rows_selected
    = (nothingSelected
       ? false
       : (coords[2] == 1 && coords[3] == view->model ()->columnCount ()));

  bool current_row_selected
    = (nothingSelected ? false : (coords[0] <= index+1 && coords[1] > index));

  int rowselection_count = nothingSelected ? 0 : (coords[3] - coords[2] + 1);

  if (! whole_rows_selected || ! current_row_selected)
    {
      view->selectRow (index);
      rowselection_count = 1;
      current_row_selected = true;
      whole_rows_selected = true;
    }

  QString row_string = tr (rowselection_count > 1 ? " rows" : " row");

  QMenu *menu = new QMenu (this);

  menu->addAction (resource_manager::icon ("edit-cut"),
                   tr ("Cut") + row_string,
                   this, SLOT (cutClipboard ()));

  menu->addAction (resource_manager::icon ("edit-copy"),
                   tr ("Copy") + row_string,
                   this, SLOT (copyClipboard ()));

  menu->addAction (resource_manager::icon ("edit-paste"),
                   tr ("Paste"),
                   this, SLOT (pasteClipboard ()));

  // FIXME: Better icon for Paste Table?

  menu->addAction (resource_manager::icon ("edit-paste"),
                   tr ("Paste Table"),
                   this, SLOT (pasteTableClipboard ()));

  menu->addSeparator ();

  menu->addAction (resource_manager::icon ("edit-delete"),
                   tr ("Clear") + row_string,
                   this, SLOT (clearContent ()));

  menu->addAction (resource_manager::icon ("edit-delete"),
                   tr ("Delete") + row_string,
                   this, SLOT (delete_selected ()));

  menu->addAction (resource_manager::icon ("document-new"),
                   tr ("Variable from Selection"),
                   this, SLOT (createVariable ()));

  menu->addSeparator ();

  QSignalMapper *plot_mapper = new QSignalMapper (menu);

  plot_mapper->setMapping
    (menu->addAction ("plot", plot_mapper, SLOT (map ())),
     "figure (); plot (%1);");

  plot_mapper->setMapping
    (menu->addAction ("bar", plot_mapper, SLOT (map ())),
     "figure (); bar (%1);");

  plot_mapper->setMapping
    (menu->addAction ("stem", plot_mapper, SLOT (map ())),
     "figure (); stem (%1);");

  plot_mapper->setMapping
    (menu->addAction ("stairs", plot_mapper, SLOT (map ())),
     "figure (); stairs (%1);");

  plot_mapper->setMapping
    (menu->addAction ("area", plot_mapper, SLOT (map ())),
     "figure (); area (%1);");

  plot_mapper->setMapping
    (menu->addAction ("pie", plot_mapper, SLOT (map ())),
     "figure (); pie (%1);");

  plot_mapper->setMapping
    (menu->addAction ("hist", plot_mapper, SLOT (map ())),
     "figure (); hist (%1);");

  connect (plot_mapper, SIGNAL (mapped (const QString&)),
           this, SLOT (relay_command (const QString&)));

  QPoint menupos = pt;
  menupos.setX (view->verticalHeader ()->width ());

  // FIXME: What was the intent here?
  // setY (view->verticalHeader ()->sectionPosition (index+1) +
  //             view->verticalHeader ()->sectionSize (index));

  menu->exec (view->mapToGlobal (menupos));
}

void
variable_editor::double_click (const QModelIndex& idx)
{
  QString name = real_var_name (m_tab_widget->currentIndex ());

  QTableView *table = m_tab_widget->get_edit_view ();

  if (! table)
    return;

  variable_editor_model *model
    = qobject_cast<variable_editor_model *> (table->model ());

  if (model->requires_sub_editor (idx))
    edit_variable (name + model->subscript_expression (idx),
                   model->value_at (idx));
}

void
variable_editor::save (void)
{
  QString name = real_var_name (m_tab_widget->currentIndex ());
  QString file
    = QFileDialog::getSaveFileName (this,
                                    tr ("Save Variable %1 As").arg (name),
                                    ".", 0, 0,
                                    QFileDialog::DontUseNativeDialog);

  // FIXME: Type? binary, float-binary, ascii, text, hdf5, matlab format?
  // FIXME: Call octave_value::save_* directly?

  if (! file.isEmpty ())
    emit command_requested (QString ("save (\"%1\", \"%2\");")
                            .arg (file)
                            .arg (name));
}

void
variable_editor::clearContent (void)
{
  // FIXME: Shift?

  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QAbstractItemModel *model = view->model ();
  QItemSelectionModel *sel = view->selectionModel ();
  QList<QModelIndex> indices = sel->selectedIndexes ();

  // FIXME: Use [] for empty cells?

  for (const auto& idx : indices)
    qobject_cast<variable_editor_model *> (model)->clear_content (idx);
}

void
variable_editor::cutClipboard (void)
{
  if (! has_focus ())
    return;

  copyClipboard ();

  clearContent ();
}

void
variable_editor::copyClipboard (void)
{
  if (! has_focus ())
    return;

  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QAbstractItemModel *model = view->model ();
  QItemSelectionModel *sel = view->selectionModel ();
  QList<QModelIndex> indices = sel->selectedIndexes ();
  qSort (indices);
  if (indices.isEmpty ())
    return;

  // Convert selected items into TSV format and copy that.
  // Spreadsheet tools should understand that.

  QModelIndex previous = indices.first ();
  QString copy = model->data (previous).toString ();
  indices.removeFirst ();
  foreach (QModelIndex idx, indices)
    {
      copy.push_back (previous.row () != idx.row () ? '\n' : '\t');
      copy.append (model->data (idx).toString ());
      previous = idx;
    }
  copy.push_back ('\n');

  QClipboard *clipboard = QApplication::clipboard ();
  clipboard->setText (copy);
}

void
variable_editor::pasteClipboard (void)
{
  if (! has_focus ())
    return;

  QClipboard *clipboard = QApplication::clipboard ();
  QString text = clipboard->text ();

  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QItemSelectionModel *sel = view->selectionModel ();
  QList<QModelIndex> indices = sel->selectedIndexes ();

  variable_editor_model *model
    = static_cast<variable_editor_model *> (view->model ());

  if (indices.isEmpty ())
    {
      if (view->size () == QSize (1,1))
        model->setData (view->model ()->index (0,0), text.toDouble ());
      else if (view->size () == QSize (0,0))
        {
          model->insertColumn (0);
          model->insertRow (0);
          model->setData (view->model ()->index (0,0), text.toDouble ());
        }
    }
  else
    {
      for (int i = 0; i < indices.size (); i++)
        view->model ()->setData (indices[i], text.toDouble ());
    }

  emit updated ();
}

void variable_editor::pasteTableClipboard (void)
{
  if (! has_focus ())
    return;

  QClipboard *clipboard = QApplication::clipboard ();
  QString text = clipboard->text ();

  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QItemSelectionModel *sel = view->selectionModel ();
  QList<QModelIndex> indices = sel->selectedIndexes ();

  variable_editor_model *model
    = static_cast<variable_editor_model *> (view->model ());

  QPoint start, end;

  QPoint tabsize = QPoint (model->rowCount (), model->columnCount ());

  if (indices.isEmpty ())
    {
      start = QPoint (0,0);
      end = tabsize;
    }
  else if (indices.size () == 1)
    {
      start = QPoint (indices[0].row (), indices[0].column ());
      end = tabsize;
    }
  else
    {
      end = QPoint (0,0);
      start = tabsize;

      for (int i = 0; i < indices.size (); i++)
        {
          if (indices[i].column () < start.y ())
            start.setY (indices[i].column ());

          if (indices[i].column () > end.y ())
            end.setY (indices[i].column ());

          if (indices[i].row () < start.x ())
            start.setX (indices[i].column ());

          if (indices[i].row () > end.x ())
            end.setX (indices[i].column ());
        }
    }

  int rownum = 0;
  int colnum = 0;

  QStringList rows = text.split ('\n');
  for (const auto& row : rows)
    {
      if (rownum > end.x () - start.x ())
        continue;

      QStringList cols = row.split ('\t');
      if (cols.isEmpty ())
        continue;

      for (const auto& col : cols)
        {
          if (col.isEmpty ())
            continue;
          if (colnum > end.y () - start.y () )
            continue;

          model->setData (model->index (rownum + start.x (),
                                        colnum + start.y ()),
                          QVariant (col));

          colnum++;
        }

      colnum = 0;
      rownum++;
    }

  emit updated ();
}

void
variable_editor::createVariable (void)
{
  // FIXME: Create unnamed1..n if exist ('unnamed', 'var') is true.

  relay_command ("unnamed = %1");
}

void
variable_editor::transposeContent (void)
{
  QString name = real_var_name (m_tab_widget->currentIndex ());

  emit command_requested (QString ("%1 = %1';").arg (name));

  emit updated ();
}

void
variable_editor::up (void)
{
  QString name = real_var_name (m_tab_widget->currentIndex ());

  // FIXME: Is there a better way?

  if (name.endsWith (')') || name.endsWith ('}'))
    {
      name.remove (QRegExp ("(\\(|\\{)[^({]*(\\)|\\})$"));
      edit_variable (name, octave_value ());
    }
}

void
variable_editor::delete_selected (void)
{
  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return;

  QString selection = selected_to_octave ();
  QList<int> coords = octave_to_coords (selection);

  if (coords.isEmpty ())
    return;

  bool whole_columns_selected
    = coords[0] == 1 && coords[1] == view->model ()->rowCount ();

  bool whole_rows_selected
    = coords[2] == 1 && coords[3] == view->model ()->columnCount ();

  // Must be deleting whole columns or whole rows, and not the whole thing.

  if (whole_columns_selected == whole_rows_selected)
    return;

  if (whole_rows_selected)
    view->model ()->removeRows (coords[0], coords[1] - coords[0]);

  if (whole_columns_selected)
    view->model ()->removeColumns (coords[2], coords[3] - coords[2]);

  emit updated ();
}

void
variable_editor::relay_command (const QString& cmd)
{
  emit command_requested (cmd.arg (selected_to_octave ()));
}

QList<int>
variable_editor::octave_to_coords (QString& selection)
{
  // FIXME: Is this necessary or would it be quicker to clone the function
  // that gives us the QString?

  // Sanity check.

  if (selection.count (",") != 1)
    return QList<int> ();

  // FIXME: Why clear if object has just been created?

  QList<int> output;
  output.clear ();

  // Remove braces.

  int firstbracket = std::max (selection.indexOf ("("),
                               selection.indexOf ("{"));

  selection = selection.mid (firstbracket + 1,
                             selection.length () - (firstbracket + 2));

  QString rows = selection.left (selection.indexOf (","));
  if (! rows.contains (":"))
    {
      // Only one row.

      output.push_back (rows.toInt ());
      output.push_back (output.last ());
    }
  else
    {
      output.push_back (rows.left (rows.indexOf (":")).toInt ());
      output.push_back (rows.right (rows.length () - (rows.indexOf (":") + 1))
                        .toInt ());
    }

  QString cols;
  cols = selection.right (selection.length () - (selection.indexOf (",") + 1));
  if (cols.left (1) == " ")
    cols = cols.right (cols.length () - 1);

  if (! cols.contains (":"))
    {
      // Only one row.

      output.push_back (cols.toInt ());
      output.push_back (output.last ());
    }
  else
    {
      output.push_back (cols.left (cols.indexOf (":")).toInt ());
      output.push_back (cols.right (cols.length () - (cols.indexOf (":") + 1))
                        .toInt ());
    }

  return output;
}

// Return the real variable name from the tab addressed by 'index',
// cleaned of any '&' possibly inserted by KDE.

QString
variable_editor::real_var_name (int index)
{
  QString var_name = m_tab_widget->tabText (index);
  var_name.remove (QChar ('&'));
  return var_name;
}

QString
variable_editor::selected_to_octave (void)
{
  QString name = real_var_name (m_tab_widget->currentIndex ());

  QTableView *view = m_tab_widget->get_edit_view ();

  if (! view)
    return name;

  QItemSelectionModel *sel = view->selectionModel ();

  // Return early if nothing selected.

  if (! sel->hasSelection ())
    return name;

  QList<QModelIndex> indices = sel->selectedIndexes ();

  // FIXME: Shouldn't this be keyed to octave_idx_type?

  int32_t from_row = std::numeric_limits<int32_t>::max ();
  int32_t to_row = 0;
  int32_t from_col = std::numeric_limits<int32_t>::max ();
  int32_t to_col = 0;

  for (const auto& idx : indices)
    {
      from_row = std::min (from_row, idx.row ());
      to_row = std::max (to_row, idx.row ());
      from_col = std::min (from_col, idx.column ());
      to_col = std::max (to_col, idx.column ());
    }

  QString rows = idx_to_expr (from_row, to_row);
  QString cols = idx_to_expr (from_col, to_col);

  // FIXME: Does cell need separate handling?  Maybe use '{.,.}'?

  return QString ("%1(%2, %3)").arg (name).arg (rows).arg (cols);
}

// Also updates the font.

void variable_editor::update_colors (void)
{
  m_stylesheet = "";

  m_stylesheet += "QTableView::item{ foreground-color: "
    + m_table_colors[0].name () +" }";

  m_stylesheet += "QTableView::item{ background-color: "
    + m_table_colors[1].name () +" }";

  m_stylesheet += "QTableView::item{ selection-color: "
    + m_table_colors[2].name () +" }";

  m_stylesheet += "QTableView::item:selected{ background-color: "
    + m_table_colors[3].name () +" }";

  if (m_table_colors.length () > 4 && m_alternate_rows)
    {
      m_stylesheet
        += "QTableView::item:alternate{ background-color: "
        + m_table_colors[4].name () +" }";

      m_stylesheet
        += "QTableView::item:alternate:selected{ background-color: "
        + m_table_colors[3].name () +" }";
    }

  if (m_tab_widget->count () < 1)
    return;

  for (int i = 0; i < m_tab_widget->count (); i++)
    {
      QTableView *view = m_tab_widget->get_edit_view ();

      if (! view)
        continue;

      view->setAlternatingRowColors (m_alternate_rows);
      view->setStyleSheet (m_stylesheet);
      view->setFont (m_font);
    }

}

void
variable_editor::construct_tool_bar (void)
{
  m_tool_bar->setAllowedAreas (Qt::TopToolBarArea);

  m_tool_bar->setMovable (false);

  m_tool_bar->setObjectName ("VariableEditorToolBar");

  m_tool_bar->setWindowTitle (tr ("Variable Editor Toolbar"));

  m_tool_bar->addAction (resource_manager::icon ("document-save"),
                         tr ("Save"), this, SLOT (save ()));

  m_tool_bar->addSeparator ();

  m_tool_bar->addAction (resource_manager::icon ("edit-cut"),
                         tr ("Cut"), this, SLOT (cutClipboard ()));

  m_tool_bar->addAction (resource_manager::icon ("edit-copy"),
                         tr ("Copy"), this, SLOT (copyClipboard ()));

  m_tool_bar->addAction (resource_manager::icon ("edit-paste"),
                         tr ("Paste"), this, SLOT (pasteClipboard ()));

  // FIXME: Different icon for Paste Table?

  m_tool_bar->addAction (resource_manager::icon ("edit-paste"),
                         tr ("Paste Table"),
                         this, SLOT (pasteTableClipboard ()));

  m_tool_bar->addSeparator ();

  // FIXME: Add a print item?
  // QAction *print_action; /icons/fileprint.png
  // m_tool_bar->addSeparator ();

  QToolButton *plot_tool_button = new QToolButton (m_tool_bar);

  plot_tool_button->setText (tr ("Plot"));
  plot_tool_button->setToolTip (tr ("Plot Selected Data"));
  plot_tool_button->setIcon (resource_manager::icon ("plot-xy-curve"));

  plot_tool_button->setPopupMode (QToolButton::InstantPopup);

  QMenu *plot_menu = new QMenu (tr ("Plot"), plot_tool_button);

  plot_menu->setSeparatorsCollapsible (false);

  QSignalMapper *plot_mapper = new QSignalMapper (plot_menu);

  plot_mapper->setMapping
    (plot_menu->addAction ("plot", plot_mapper, SLOT (map ())),
     "figure (); plot (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("bar", plot_mapper, SLOT (map ())),
     "figure (); bar (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("stem", plot_mapper, SLOT (map ())),
     "figure (); stem (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("stairs", plot_mapper, SLOT (map ())),
     "figure (); stairs (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("area", plot_mapper, SLOT (map ())),
     "figure (); area (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("pie", plot_mapper, SLOT (map ())),
     "figure (); pie (%1);");

  plot_mapper->setMapping
    (plot_menu->addAction ("hist", plot_mapper, SLOT (map ())),
     "figure (); hist (%1);");

  connect (plot_mapper, SIGNAL (mapped (const QString&)),
           this, SLOT (relay_command (const QString&)));

  plot_tool_button->setMenu (plot_menu);

  m_tool_bar->addWidget (plot_tool_button);

  m_tool_bar->addSeparator ();

  m_tool_bar->addAction (QIcon (resource_manager::icon ("go-up")), tr ("Up"),
                         this, SLOT (up ()));

  // Disabled when no tab is present.

  m_tool_bar->setEnabled (false);
}
