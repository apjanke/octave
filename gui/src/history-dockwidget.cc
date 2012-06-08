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

#include "history-dockwidget.h"
#include <QVBoxLayout>

history_dock_widget::history_dock_widget (QWidget * parent)
  : QDockWidget (parent), octave_event_observer ()
{
  setObjectName ("HistoryDockWidget");
  construct ();
}

void
history_dock_widget::event_accepted (octave_event *e)
{
  if (dynamic_cast <octave_update_history_event*> (e))
    {
      // Determine the client's (our) history length and the one of the server.
      int clientHistoryLength = _history_model->rowCount ();
      int serverHistoryLength = command_history::length ();

      // If were behind the server, iterate through all new entries and add
      // them to our history.
      if (clientHistoryLength < serverHistoryLength)
        {
          for (int i = clientHistoryLength; i < serverHistoryLength; i++)
            {
              _history_model->insertRow (0);
              _history_model->setData (_history_model->index (0),
                QString (command_history::get_entry (i).c_str ()));
            }
        }
    }

  // Post a new update event in a given time. This prevents flooding the
  // event queue.
  _update_history_model_timer.start ();
  delete e;
}

void
history_dock_widget::event_reject (octave_event *e)
{
  delete e;
}

void
history_dock_widget::construct ()
{
  _history_model = new QStringListModel ();
  _sort_filter_proxy_model.setSourceModel (_history_model);
  _history_list_view = new QListView (this);
  _history_list_view->setModel (&_sort_filter_proxy_model);
  _history_list_view->setAlternatingRowColors (true);
  _history_list_view->setEditTriggers (QAbstractItemView::NoEditTriggers);
  _history_list_view->setStatusTip (tr ("Doubleclick a command to transfer it to the terminal."));
  _filter_line_edit = new QLineEdit (this);
  _filter_line_edit->setStatusTip (tr ("Enter text to filter the command history."));
  QVBoxLayout *layout = new QVBoxLayout ();

  setWindowTitle (tr ("Command History"));
  setWidget (new QWidget ());

  layout->addWidget (_history_list_view);
  layout->addWidget (_filter_line_edit);
  layout->setMargin (2);

  widget ()->setLayout (layout);

  connect (_filter_line_edit,
           SIGNAL (textEdited (QString)),
           &_sort_filter_proxy_model,
           SLOT (setFilterWildcard (QString)));

  connect (_history_list_view,
           SIGNAL (doubleClicked (QModelIndex)),
           this,
           SLOT (handle_double_click (QModelIndex)));

  connect (this,
           SIGNAL (visibilityChanged (bool)),
           this,
           SLOT (handle_visibility_changed (bool)));

  _update_history_model_timer.setInterval (200);
  _update_history_model_timer.setSingleShot (true);

  connect (&_update_history_model_timer,
           SIGNAL (timeout ()),
           this,
           SLOT (request_history_model_update ()));

  _update_history_model_timer.start ();
}

void
history_dock_widget::handle_double_click (QModelIndex modelIndex)
{
  emit command_double_clicked (modelIndex.data().toString());
}

void
history_dock_widget::handle_visibility_changed (bool visible)
{
  if (visible)
    emit active_changed (true);
}

void
history_dock_widget::request_history_model_update ()
{
  octave_link::instance ()
      ->post_event (new octave_update_history_event (*this));
}

void
history_dock_widget::closeEvent (QCloseEvent *event)
{
  emit active_changed (false);
  QDockWidget::closeEvent (event);
}
