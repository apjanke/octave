/*

Copyright (C) 2011-2016 Michael Goffioul

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

#if ! defined (octave_Menu_h)
#define octave_Menu_h 1

#include "MenuContainer.h"
#include "Object.h"

class QAction;
class QMenu;
class QWidget;

namespace QtHandles
{

  class Menu : public Object, public MenuContainer
  {
    Q_OBJECT

  public:
    Menu (const graphics_object& go, QAction* action, Object* parent);
    ~Menu (void);

    static Menu* create (const graphics_object& go);

    Container* innerContainer (void) { return 0; }

    QWidget* menu (void);

  protected:
    void update (int pId);

  private slots:
    void actionTriggered (void);
    void actionHovered (void);

  private:
    void updateSiblingPositions (void);

  private:
    QWidget* m_parent;
    QAction* m_separator;
  };

}

#endif
