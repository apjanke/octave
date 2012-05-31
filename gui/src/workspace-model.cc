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

#include "workspace-model.h"
#include <QTreeWidget>
#include <QTime>
#include "octave-link.h"

workspace_model::workspace_model(QObject *parent)
  : QAbstractItemModel(parent)
{
  QList<QVariant> rootData;
  rootData << tr ("Name") << tr ("Type") << tr ("Value");
  _rootItem = new TreeItem(rootData);
}

workspace_model::~workspace_model()
{
  delete _rootItem;
}

QModelIndex
workspace_model::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  TreeItem *parentItem;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<TreeItem*>(parent.internalPointer());

  TreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex
workspace_model::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
  TreeItem *parentItem = childItem->parent();

  if (parentItem == _rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int
workspace_model::rowCount(const QModelIndex &parent) const
{
  TreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<TreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

int
workspace_model::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
  else
    return _rootItem->columnCount();
}

void
workspace_model::insertTopLevelItem(int at, TreeItem *treeItem)
{
  _rootItem->insertChildItem(at, treeItem);
}

TreeItem *
workspace_model::topLevelItem (int at)
{
  return _rootItem->child(at);
}

Qt::ItemFlags
workspace_model::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant
workspace_model::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return _rootItem->data(section);

  return QVariant();
}

QVariant
workspace_model::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();

  TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

  return item->data(index.column());
}


void
workspace_model::updateFromSymbolTable ()
{
  topLevelItem (0)->deleteChildItems ();
  topLevelItem (1)->deleteChildItems ();
  topLevelItem (2)->deleteChildItems ();
  topLevelItem (3)->deleteChildItems ();

  octave_link::instance ()-> acquireSymbolInformation();
  const QList <SymbolInformation>& symbolInformation = octave_link::instance() ->symbolInformation ();

  foreach (const SymbolInformation& s, symbolInformation)
    {
      TreeItem *child = new TreeItem ();

      child->setData (0, s._symbol);
      child->setData (1, s._type);
      child->setData (2, s._value);

      switch (s._scope)
        {
          case SymbolInformation::Local:       topLevelItem (0)->addChild (child); break;
          case SymbolInformation::Global:      topLevelItem (1)->addChild (child); break;
          case SymbolInformation::Persistent:  topLevelItem (2)->addChild (child); break;
          case SymbolInformation::Hidden:      topLevelItem (3)->addChild (child); break;
        }
    }

  octave_link::instance ()-> releaseSymbolInformation();

  reset();
  emit expandRequest();
}
