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

#ifndef WORKSPACEVIEW_H
#define WORKSPACEVIEW_H

#include <QDockWidget>
#include <QTreeView>
#include <QSemaphore>
#include "octavelink.h"

class WorkspaceView:public QDockWidget
{
  Q_OBJECT
public:
  WorkspaceView (QWidget * parent = 0);

public slots:
  void handleVisibilityChanged (bool visible);

signals:
  /** Custom signal that tells if a user has clicke away that dock widget. */
  void activeChanged (bool active);

protected:
  void closeEvent (QCloseEvent *event);

private:
  QTreeView *m_workspaceTreeView;
};

#endif // WORKSPACEVIEW_H
