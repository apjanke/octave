/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2012 Jacob Dawid (jacob.dawid@googlemail.com)
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

#ifndef OCTAVEQTEVENTLISTENER_H
#define OCTAVEQTEVENTLISTENER_H

#include <QObject>
#include <QString>
#include "octave-event-listener.h"

class octave_qt_event_listener
    : public QObject, public octave_event_listener
{
    Q_OBJECT
  public:
    octave_qt_event_listener (QObject *parent = 0);

    void current_directory_has_changed (std::string directory);

  signals:
    void current_directory_changed (QString directory);
};

#endif // OCTAVEQTEVENTLISTENER_H
