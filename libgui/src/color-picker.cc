//
// This class provides a simple color picker based on tQColorButton
// by Harald Jedele, 23.03.01, GPL version 2 or any later version.
//
// Copyright (C) FZI Forschungszentrum Informatik Karlsruhe
// Copyright (C) 2013 Torsten <ttl@justmail.de>
//
// This file is part of Octave.
//
// Octave is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3 of the License, or (at your
// option) any later version.
//
// Octave is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <http://www.gnu.org/licenses/>.
//

#include "color-picker.h"

// constuctor with initial color as parameter
color_picker::color_picker (QColor old_color, QWidget* p) : QPushButton (p)
{
  _color = old_color;
  setFlat (true);
  update_button ();
  connect(this, SIGNAL (clicked ()), SLOT (select_color ()));
}

// slot for bitton clicked: selct a new color using QColorDialog
void
color_picker::select_color ()
{
  QColor new_color = QColorDialog::getColor (_color);
  if (new_color.isValid () && new_color != _color)
    {
      _color = new_color;
      update_button ();
    }
}

// draw the button with the actual color (using a stylesheet)
void color_picker::update_button ()
{
  QString css = QString("background-color: %1; border: none;" )
                        .arg(_color.name());
  setStyleSheet(css);
  repaint ();
}
