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
  enum Icon
  {
    Octave,
    Terminal,
    Documentation,
    Chat,
    ChatNewMessage
  };

  ~resource_manager ();

  static resource_manager *
  instance ()
  {
    return &m_singleton;
  }

  QSettings *settings ();
  QString homePath ();
  void reloadSettings ();
  void setSettings (QString file);
  QString findTranslatorFile (QString language);
  void updateNetworkSettings ();
  void loadIcons ();
  QIcon icon (Icon icon);
  bool isFirstRun ();

  const char *octaveKeywords ();
private:
  resource_manager ();

  QSettings *m_settings;
  QString m_homePath;
  QMap <Icon, QIcon> m_icons;
  static resource_manager m_singleton;
  bool m_firstRun;
};

#endif // RESOURCEMANAGER_H
