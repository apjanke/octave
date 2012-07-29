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

#ifndef WORKSPACEMODEL_H
#define WORKSPACEMODEL_H

// Qt includes
#include <QAbstractItemModel>
#include <QVector>
#include <QSemaphore>
#include <QTimer>

#include "symbol-information.h"
#include "octave-event-observer.h"

class tree_item
{
public:
  tree_item (const QList<QVariant> &data, tree_item *parent = 0) {
    _parent_item = parent;
    _item_data = data;
  }

  tree_item (QVariant data = QVariant(), tree_item *parent = 0) {
    QList<QVariant> variantList;
    variantList << data << QVariant () << QVariant () << QVariant ();
    _parent_item = parent;
    _item_data = variantList;
  }

  ~tree_item () {
     qDeleteAll (_child_items);
  }

  void insert_child_item (int at, tree_item *item) {
    item->_parent_item = this;
    _child_items.insert (at, item);
  }

  void add_child (tree_item *item) {
    item->_parent_item = this;
    _child_items.append (item);
  }

  void delete_child_items () {
      qDeleteAll (_child_items);
      _child_items.clear ();
  }

  void remove_child (tree_item *item) {
    _child_items.removeAll (item);
  }

  QVariant data (int column) const
  {
    return _item_data[column];
  }

  void set_data (int column, QVariant data)
  {
    _item_data[column] = data;
  }

  tree_item *child (int row) {
    return _child_items[row];
  }

  int child_count () const {
    return _child_items.count();
  }

  int column_count () const
  {
    return _item_data.count();
  }

  int row () const {
    if (_parent_item)
      return _parent_item->_child_items.indexOf (const_cast<tree_item*>(this));

    return 0;
  }

  tree_item *parent ()
  {
    return _parent_item;
  }

private:
  QList<tree_item*> _child_items;
  QList<QVariant> _item_data;
  tree_item *_parent_item;
};

class workspace_model
    : public QAbstractItemModel, public octave_event_observer
{
  Q_OBJECT

public:
  workspace_model (QObject *parent = 0);
  ~workspace_model ();

  void event_accepted (octave_event *e);
  void event_reject (octave_event *e);

  QVariant data (const QModelIndex &index, int role) const;
  Qt::ItemFlags flags (const QModelIndex &index) const;
  QVariant headerData (int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const;
  QModelIndex index (int row, int column,
                    const QModelIndex &parent = QModelIndex ()) const;
  QModelIndex parent (const QModelIndex &index) const;
  int rowCount (const QModelIndex &parent = QModelIndex ()) const;
  int columnCount (const QModelIndex &parent = QModelIndex ()) const;

  void insert_top_level_item (int at, tree_item *treeItem);
  tree_item *top_level_item (int at);

public slots:
  void request_update_workspace ();

signals:
  void model_changed ();

private:
  /** Timer for periodically updating the workspace model from the current
    * symbol information. */
  QTimer _update_workspace_model_timer;

  /** Stores the current symbol information. */
  QList <symbol_information> _symbol_information;
  tree_item *_rootItem;
};

#endif // WORKSPACEMODEL_H
