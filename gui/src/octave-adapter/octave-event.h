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

#ifndef OCTAVEEVENT_H
#define OCTAVEEVENT_H

#include <string>
#include "octave-event-observer.h"
#include "oct-env.h"
#include "pt-eval.h"
#include "toplev.h"

#include <readline/readline.h>

/**
  * \class octave_event
  * \brief Base class for an octave event.
  * In order to make communication with octave threadsafe, comunication is
  * implemented via events. An application may create events and post them,
  * however there is no guarantee events will be processed in a given time.
  *
  * In order to create an event, there must be an event observer. The event
  * observer will be given the opportunity to react on the event as soon as
  * it has been processed in the octave thread. Accepting and ignoring takes
  * places in the octave thread.
  */
class octave_event
{
  public:
    octave_event (octave_event_observer& o)
      : _octave_event_observer (o)
    { }

    virtual ~octave_event ()
    { }

    /** Performs what is necessary to make this event happen.
      * This code is thread-safe since it will be executed in the octave thread.
      * However, you should take care to keep this code as short as possible. */
    virtual bool perform () const = 0;

    /**
      * Accepts this event. This allows the event observer to react properly
      * onto the event.
      */
    void accept ()
    { _octave_event_observer.event_accepted (this); }

    /**
      * Rejects this event. This allows the event observer to react properly
      * onto the event.
      */
    void reject ()
    { _octave_event_observer.event_reject (this); }

  private:
    octave_event_observer& _octave_event_observer;
};

class octave_update_history_event : public octave_event
{
  public:
    /** Creates a new octave_exit_event. */
    octave_update_history_event (octave_event_observer& o)
      : octave_event (o)
    { }

    bool perform () const
    { return true; /* Always grant. */ }
};

class octave_update_workspace_event : public octave_event
{
  public:
    /** Creates a new octave_exit_event. */
    octave_update_workspace_event (octave_event_observer& o)
      : octave_event (o)
    { }

    bool perform () const
    { return true; /* Always grant. */ }
};

/** Implements an octave exit event. */
class octave_exit_event : public octave_event
{
  public:
    /** Creates a new octave_exit_event. */
    octave_exit_event (octave_event_observer& o)
      : octave_event (o)
    { }

    bool perform () const
    { clean_up_and_exit (0); return true; }
};

/** Implements a change directory events. */
class octave_change_directory_event : public octave_event
{
  public:
    /** Creates a new octave_change_directory_event. */
    octave_change_directory_event (octave_event_observer& o,
                                   std::string directory)
      : octave_event (o)
    { _directory = directory; }

    bool perform () const
    { return octave_env::chdir (_directory); }

  private:
    std::string _directory;
};

class octave_debug_step_into_event : public octave_event
{
  public:
    /** Creates a new octave_debug_step_into_event. */
    octave_debug_step_into_event (octave_event_observer& o)
      : octave_event (o) { }

    bool perform () const
    {
      tree_evaluator::dbstep_flag = -1;
      return true;
    }
};

class octave_debug_step_over_event : public octave_event
{
  public:
    /** Creates a new octave_debug_step_over_event. */
    octave_debug_step_over_event (octave_event_observer& o)
      : octave_event (o) { }

    bool perform () const
    {
      tree_evaluator::dbstep_flag = 1;
      return true;
    }
};

class octave_debug_step_out_event : public octave_event
{
  public:
    /** Creates a new octave_debug_step_out_event. */
    octave_debug_step_out_event (octave_event_observer& o)
      : octave_event (o) { }

    bool perform () const
    {
      tree_evaluator::dbstep_flag = -2;
      return true;
    }
};

class octave_debug_continue_event : public octave_event
{
  public:
    /** Creates a new octave_debug_step_out_event. */
    octave_debug_continue_event (octave_event_observer& o)
      : octave_event (o) { }

    bool perform () const
    {
      tree_evaluator::dbstep_flag = 0;
      return true;
    }
};

class octave_debug_quit_event : public octave_event
{
  public:
    /** Creates a new octave_debug_step_out_event. */
    octave_debug_quit_event (octave_event_observer& o)
      : octave_event (o) { }

    bool perform () const
    {
      tree_evaluator::dbstep_flag = 0;
      return true;
    }
};

#endif // OCTAVEEVENT_H
