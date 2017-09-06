/*

Copyright (C) 2013-2017 John W. Eaton
Copyright (C) 2011-2016 Jacob Dawid

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

#if ! defined (octave_workspace_view_h)
#define octave_workspace_view_h 1

#include <QItemDelegate>
#include <QTableView>
#include <QSemaphore>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <QSignalMapper>

#include "octave-dock-widget.h"
#include "workspace-model.h"

class workspace_view : public octave_dock_widget
{
  Q_OBJECT

public:

  workspace_view (QWidget *parent = nullptr);

  ~workspace_view (void) = default;

public slots:

  void setModel (workspace_model *model);

  void notice_settings (const QSettings *);

  void save_settings (void);

signals:

  /** signal that user had requested a command on a variable */
  void command_requested (const QString& cmd);

  /// signal that user wants to edit a variable
  void edit_variable_signal (const QString&);

protected:

  void closeEvent (QCloseEvent *event);

protected slots:

  void filter_update (const QString& expression);
  void filter_activate (bool enable);
  void update_filter_history (void);

  void header_contextmenu_requested (const QPoint& mpos);

  void toggle_header (int column);

  void contextmenu_requested (const QPoint& pos);

  void handle_contextmenu_copy (void);
  void handle_contextmenu_copy_value (void);
  void handle_contextmenu_rename (void);
  void handle_contextmenu_edit (void);
  void handle_contextmenu_disp (void);
  void handle_contextmenu_plot (void);
  void handle_contextmenu_stem (void);
  void handle_contextmenu_filter (void);

  void handle_model_changed (void);

  void copyClipboard (void);
  void selectAll (void);

private:

  void relay_contextmenu_command (const QString& cmdname);

  QString get_var_name (QModelIndex index);

  QTableView *m_view;
  int m_view_previous_row_count;
  workspace_model *m_model;

  QSortFilterProxyModel m_filter_model;
  QCheckBox *m_filter_checkbox;
  QComboBox *m_filter;
  QWidget *m_filter_widget;
  bool m_filter_shown;

  enum { MaxFilterHistory = 10 };

  QStringList m_columns_shown;
  QStringList m_columns_shown_keys;
  QSignalMapper *m_sig_mapper;
};

#endif
