/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 Jacob Dawid (jacob.dawid@googlemail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TerminalHighlighter.h"
#include "TerminalView.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringListModel>
#include <QStringList>
#include <QScrollBar>

TerminalView::TerminalView (QWidget * parent)
  : QPlainTextEdit (parent), Terminal ()
{
  setFont (QFont ("Monospace", 10));
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_terminalEmulation = TerminalEmulation::newTerminalEmulation (this);

  TerminalHighlighter *terminalHighlighter = new TerminalHighlighter ();
  terminalHighlighter->setDocument (document ());
}

TerminalView::~TerminalView ()
{
}

QTextCursor
TerminalView::textCursor ()
{
  return QPlainTextEdit::textCursor();
}

void
TerminalView::setTextCursor (const QTextCursor &cursor)
{
  QPlainTextEdit::setTextCursor (cursor);
}

void
TerminalView::bell ()
{
}

void
TerminalView::keyPressEvent (QKeyEvent * keyEvent)
{
  if(m_terminalEmulation)
    m_terminalEmulation->processKeyEvent (keyEvent);
}

void
TerminalView::mousePressEvent (QMouseEvent *mouseEvent)
{
  // TODO: Pass mouse events to the terminal emulation.
  mouseEvent->accept();
}

void
TerminalView::mouseDoubleClickEvent (QMouseEvent *mouseEvent)
{
  // TODO: Pass mouse events to the terminal emulation.
  mouseEvent->accept();
}

void
TerminalView::wheelEvent (QWheelEvent *wheelEvent)
{
  // TODO: Pass mouse events to the terminal emulation.
  wheelEvent->accept();
}
