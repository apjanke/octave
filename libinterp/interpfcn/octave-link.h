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

#if !defined (octave_link_h)
#define octave_link_h 1

#include <string>

#include "event-queue.h"

class octave_mutex;
class string_vector;
class workspace_element;

// \class OctaveLink
// \brief Provides threadsafe access to octave.
// \author Jacob Dawid
//
// This class is a wrapper around octave and provides thread safety by
// buffering access operations to octave and executing them in the
// readline event hook, which lives in the octave thread.

class octave_link
{
protected:

  octave_link (void);

public:

  virtual ~octave_link (void) { }

  static void generate_events (void)
  {
    if (enabled ())
      instance->do_generate_events ();
  }

  // If disable is TRUE, then no additional events will be processed
  // other than exit.

  static void process_events (bool disable = false)
  {
    if (enabled ())
      {
        if (disable)
          instance->link_enabled = false;

        instance->do_process_events ();
      }
  }

  static void discard_events (void)
  {
    if (enabled ())
      instance->do_discard_events ();
  }

  static bool exit (int status)
  {
    bool retval = false;

    if (instance_ok ())
      retval = instance->do_exit (status);

    return retval;
  }

  template <class T>
  static void post_event (T *obj, void (T::*method) (void))
  {
    if (enabled ())
      instance->do_post_event (obj, method);
  }

  template <class T, class A>
  static void post_event (T *obj, void (T::*method) (A), A arg)
  {
    if (enabled ())
      instance->do_post_event (obj, method, arg);
  }

  template <class T, class A>
  static void post_event (T *obj, void (T::*method) (const A&), const A& arg)
  {
    if (enabled ())
      instance->do_post_event (obj, method, arg);
  }

  static void entered_readline_hook (void)
  {
    if (enabled ())
      instance->do_entered_readline_hook ();
  }

  static void finished_readline_hook (void)
  {
    if (enabled ())
      instance->do_finished_readline_hook ();
  }

  static bool
  edit_file (const std::string& file)
  {
    return enabled () ? instance->do_edit_file (file) : false;
  }

  static void change_directory (const std::string& dir)
  {
    if (enabled ())
      instance->do_change_directory (dir);
  }

  static void set_workspace (const std::list<workspace_element>& ws)
  {
    if (enabled ())
      instance->do_set_workspace (ws);
  }

  static void clear_workspace (void)
  {
    if (enabled ())
      instance->do_clear_workspace ();
  }

  static void set_history (const string_vector& hist)
  {
    if (enabled ())
      instance->do_set_history (hist);
  }

  static void append_history (const std::string& hist_entry)
  {
    if (enabled ())
      instance->do_append_history (hist_entry);
  }

  static void clear_history (void)
  {
    if (enabled ())
      instance->do_clear_history ();
  }

  static void pre_input_event (void)
  {
    if (enabled ())
      instance->do_pre_input_event ();
  }

  static void post_input_event (void)
  {
    if (enabled ())
      instance->do_post_input_event ();
  }

  static void enter_debugger_event (const std::string& file, int line)
  {
    if (enabled ())
      {
        instance->debugging = true;

        instance->do_enter_debugger_event (file, line);
      }
  }

  static void execute_in_debugger_event (const std::string& file, int line)
  {
    if (enabled ())
      instance->do_execute_in_debugger_event (file, line);
  }

  static void exit_debugger_event (void)
  {
    if (enabled () && instance->debugging)
      {
        instance->debugging = false;

        instance->do_exit_debugger_event ();
      }
  }

  static void
  update_breakpoint (bool insert, const std::string& file, int line)
  {
    if (enabled ())
      instance->do_update_breakpoint (insert, file, line);
  }

  static void connect_link (octave_link *);

private:

  static octave_link *instance;

  // No copying!

  octave_link (const octave_link&);

  octave_link& operator = (const octave_link&);

  static bool instance_ok (void) { return instance != 0; }

  static bool enabled (void)
  {
    return instance_ok () ? instance->link_enabled : false;
  }

protected:

  // Semaphore to lock access to the event queue.
  octave_mutex *event_queue_mutex;

  // Event Queue.
  event_queue gui_event_queue;

  bool debugging;
  bool link_enabled;

  void do_generate_events (void);
  void do_process_events (void);
  void do_discard_events (void);

  template <class T>
  void do_post_event (T *obj, void (T::*method) (void))
  {
    gui_event_queue.add_method (obj, method);
  }

  template <class T, class A>
  void do_post_event (T *obj, void (T::*method) (A), A arg)
  {
    gui_event_queue.add_method (obj, method, arg);
  }

  template <class T, class A>
  void do_post_event (T *obj, void (T::*method) (const A&), const A& arg)
  {
    gui_event_queue.add_method (obj, method, arg);
  }

  void do_entered_readline_hook (void) { }
  void do_finished_readline_hook (void) { }

  virtual bool do_exit (int status) = 0;

  virtual bool do_edit_file (const std::string& file) = 0;

  virtual void do_change_directory (const std::string& dir) = 0;

  virtual void
  do_set_workspace (const std::list<workspace_element>& ws) = 0;

  virtual void do_clear_workspace (void) = 0;

  virtual void do_set_history (const string_vector& hist) = 0;
  virtual void do_append_history (const std::string& hist_entry) = 0;
  virtual void do_clear_history (void) = 0;

  virtual void do_pre_input_event (void) = 0;
  virtual void do_post_input_event (void) = 0;

  virtual void
  do_enter_debugger_event (const std::string& file, int line) = 0;

  virtual void
  do_execute_in_debugger_event (const std::string& file, int line) = 0;

  virtual void do_exit_debugger_event (void) = 0;

  virtual void do_update_breakpoint (bool insert,
                                     const std::string& file, int line) = 0;
};

#endif // OCTAVELINK_H
