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

#include "terminal-dockwidget.h"

terminal_dock_widget::terminal_dock_widget (QTerminal *terminal, QWidget *p)
  : QDockWidget (p)
{
  setObjectName ("TerminalDockWidget");
  setWindowIcon (QIcon(":/actions/icons/logo.png"));
  setWindowTitle (tr ("Command Window"));
  setWidget (terminal);

  connect (this, SIGNAL (visibilityChanged (bool)), this, SLOT (handle_visibility_changed (bool)));
  // topLevelChanged is emitted when floating property changes (floating = true)
  connect (this, SIGNAL (topLevelChanged(bool)), this, SLOT(top_level_changed(bool)));
}

void
terminal_dock_widget::closeEvent (QCloseEvent *e)
{
  emit active_changed (false);
  QDockWidget::closeEvent (e);
}

void
terminal_dock_widget::handle_visibility_changed (bool visible)
{
  if (visible)
    emit active_changed (true);
}

// slot for signal that is emitted when floating property changes
void
terminal_dock_widget::top_level_changed (bool floating)
{
  if(floating)
    {
      setWindowFlags(Qt::Window);  // make a window from the widget when floating
      show();                      // make it visible again since setWindowFlags hides it
    }
}
