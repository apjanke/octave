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

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QSettings>
#include <QDesktopServices>
#include <QMap>
#include <QIcon>

class resource_manager
{
public:
  enum icon
  {
    octave,
    terminal,
    documentation,
    chat,
    chat_new_message
  };

  ~resource_manager ();

  static resource_manager *
  instance ()
  {
    return &_singleton;
  }

  QSettings *get_settings ();
  QString get_home_path ();
  void reload_settings ();
  void set_settings (QString file);
  QString find_translator_file (QString language);
  void update_network_settings ();
  void load_icons ();
  QIcon get_icon (icon i);
  bool is_first_run ();
  const char *octave_keywords ();

private:
  resource_manager ();

  QSettings *_settings;
  QString _home_path;
  QMap <icon, QIcon> _icons;
  static resource_manager _singleton;
  bool _first_run;
};

#endif // RESOURCEMANAGER_H
