/*

Copyright (C) 2013 John W. Eaton
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

#if !defined (workspace_model_h)
#define workspace_model_h 1

#include <QAbstractTableModel>
#include <QVector>
#include <QSemaphore>
#include <QStringList>

class workspace_model
  : public QAbstractTableModel
{
  Q_OBJECT

public:

  workspace_model (QObject *parent = 0);

  ~workspace_model (void) { }

  QVariant data (const QModelIndex& index, int role) const;

  bool setData (const QModelIndex& index, const QVariant& value,
                int role = Qt::EditRole);

  Qt::ItemFlags flags (const QModelIndex& index) const;

  QVariant headerData (int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const;

  int rowCount (const QModelIndex& parent = QModelIndex ()) const;

  int columnCount (const QModelIndex& parent = QModelIndex ()) const;

  bool is_top_level (void) const { return _top_level; }

public slots:

  void set_workspace (bool top_level,
                      const QString& scopes,
                      const QStringList& symbols,
                      const QStringList& class_names,
                      const QStringList& dimensions,
                      const QStringList& values);

  void clear_workspace (void);

signals:

  void model_changed (void);

  void rename_variable (const QString& old_name, const QString& new_name);

private:

  void clear_data (void);
  void update_table (void);

  bool _top_level;
  QString _scopes;
  QStringList _symbols;
  QStringList _class_names;
  QStringList _dimensions;
  QStringList _values;

  QStringList _columnNames;
};

#endif
