/*

Copyright (C) 2012-2017 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_event_queue_h)
#define octave_event_queue_h 1

#include "octave-config.h"

#include <queue>
#include <memory>

#include "action-container.h"

namespace octave
{
  class
  event_queue : public action_container
  {
  public:

    event_queue (void) : fifo () { }

    // No copying!

    event_queue (const event_queue&) = delete;

    event_queue& operator = (const event_queue&) = delete;

    // Destructor should not raise an exception, so all actions
    // registered should be exception-safe.  If you're not sure, see
    // event_queue_safe.

    ~event_queue (void) { run (); }

    void add (elem *new_elem)
    {
      fifo.push (new_elem);
    }

    void run_first (void)
    {
      if (! empty ())
        {
          // No leak on exception!
          std::unique_ptr<elem> ptr (fifo.front ());
          fifo.pop ();
          ptr->run ();
        }
    }

    void discard_first (void)
    {
      if (! empty ())
        {
          elem *ptr = fifo.front ();
          fifo.pop ();
          delete ptr;
        }
    }

    size_t size (void) const { return fifo.size (); }

  protected:

    std::queue<elem *> fifo;
  };

  // Like event_queue, but this one will guard against the
  // possibility of seeing an exception (or interrupt) in the cleanup actions.
  // Not that we can do much about it, but at least we won't crash.

  class
  event_queue_safe : public event_queue
  {
  private:

    void warn_unhandled_exception (void) const;

  public:

    event_queue_safe (void) : event_queue () { }

    // No copying!

    event_queue_safe (const event_queue_safe&) = delete;

    event_queue_safe& operator = (const event_queue_safe&) = delete;

    ~event_queue_safe (void)
    {
      while (! empty ())
        {
          try
            {
              run_first ();
            }
          catch (...) // Yes, the black hole.  Remember we're in a dtor.
            {
              warn_unhandled_exception ();
            }
        }
    }
  };
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED (4.4, "use 'octave::event_queue' instead")
typedef octave::event_queue event_queue;

OCTAVE_DEPRECATED (4.4, "use 'octave::event_queue_safe' instead")
typedef octave::event_queue_safe event_queue_safe;

#endif

#endif
