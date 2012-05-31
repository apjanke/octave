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

#ifndef FILEEDITORINTERFACE_H
#define FILEEDITORINTERFACE_H

#include <QDockWidget>

class QTerminal;
class main_window;

class file_editor_interface : public QDockWidget
{
  Q_OBJECT

  public:
    file_editor_interface (QTerminal *terminal, main_window *mainWindow)
      : QDockWidget ((QWidget*)mainWindow) // QDockWidget constructor is explicit, hence the cast.
    {
      setObjectName ("FileEditor");
      m_terminal = terminal;
      m_mainWindow = mainWindow;

      connect (this, SIGNAL (visibilityChanged (bool)), this, SLOT (handle_visibility_changed (bool)));
    }

    virtual ~file_editor_interface () { }
  public slots:
    virtual void request_new_file () = 0;
    virtual void request_open_file () = 0;
    virtual void request_open_file (QString fileName) = 0;

  signals:
      void active_changed (bool active);

  protected:
    QTerminal* m_terminal;
    main_window* m_mainWindow;

    void closeEvent (QCloseEvent *event)
    {
      emit active_changed (false);
      QDockWidget::closeEvent (event);
    }

  protected slots:
    void handle_visibility_changed (bool visible)
    {
      if (visible)
        emit active_changed (true);
    }
};

#endif // FILEEDITORINTERFACE_H
