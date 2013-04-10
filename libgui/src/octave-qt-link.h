/*

Copyright (C) 2013 John W. Eaton
Copyright (C) 2011-2012 Jacob Dawid
Copyright (C) 2011-2012 John P. Swensen

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

#ifndef OCTAVE_QT_LINK_H
#define OCTAVE_QT_LINK_H

#include <list>
#include <string>

#include <QList>
#include <QObject>
#include <QString>

#include "octave-link.h"
#include "octave-main-thread.h"

// \class OctaveLink
// \brief Provides threadsafe access to octave.
// \author Jacob Dawid
//
// This class is a wrapper around octave and provides thread safety by
// buffering access operations to octave and executing them in the
// readline event hook, which lives in the octave thread.

class octave_qt_link : public QObject, public octave_link
{
  Q_OBJECT

public:

  octave_qt_link (octave_main_thread *mt);

  ~octave_qt_link (void);

  void execute_interpreter (void);

  bool do_exit (int status);

  bool do_edit_file (const std::string& file);

  void do_change_directory (const std::string& dir);

  void do_set_workspace (const std::list<workspace_element>& ws);
  void do_clear_workspace (void);

  void do_set_history (const string_vector& hist);
  void do_append_history (const std::string& hist_entry);
  void do_clear_history (void);

  void do_pre_input_event (void);
  void do_post_input_event (void);

  void do_enter_debugger_event (const std::string& file, int line);
  void do_execute_in_debugger_event (const std::string& file, int line);
  void do_exit_debugger_event (void);

  void do_update_breakpoint (bool insert, const std::string& file, int line);

private:

  // No copying!

  octave_qt_link (const octave_qt_link&);

  octave_qt_link& operator = (const octave_qt_link&);

  void do_insert_debugger_pointer (const std::string& file, int line);
  void do_delete_debugger_pointer (const std::string& file, int line);

  // Thread running octave_main.
  octave_main_thread *main_thread;

signals:

  void exit_signal (int status);

  void edit_file_signal (const QString& file);

  void change_directory_signal (const QString& dir);

  void set_workspace_signal (const QString& scopes,
                             const QStringList& symbols,
                             const QStringList& class_names,
                             const QStringList& dimensions,
                             const QStringList& values);

  void clear_workspace_signal (void);

  void set_history_signal (const QStringList& hist);
  void append_history_signal (const QString& hist_entry);
  void clear_history_signal (void);

  void enter_debugger_signal (void);
  void exit_debugger_signal (void);

  void update_breakpoint_marker_signal (bool insert, const QString& file,
                                        int line);

  void insert_debugger_pointer_signal (const QString&, int);
  void delete_debugger_pointer_signal (const QString&, int);
};

#endif
