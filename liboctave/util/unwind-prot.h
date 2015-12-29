/*

Copyright (C) 1993-2015 John W. Eaton
Copyright (C) 2009-2010 VZLU Prague

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

#if ! defined (octave_unwind_prot_h)
#define octave_unwind_prot_h 1

#include <cstddef>

#include <stack>
#include <memory>

#include "action-container.h"

class
OCTAVE_API
unwind_protect : public action_container
{
public:

  unwind_protect (void) : lifo () { }

  // Destructor should not raise an exception, so all actions
  // registered should be exception-safe.  If you're not sure, see
  // unwind_protect_safe.

  ~unwind_protect (void) { run (); }

  virtual void add (elem *new_elem)
  {
    lifo.push (new_elem);
  }

  GCC_ATTR_DEPRECATED void add (void (*fcn) (void *), void *ptr = 0)
  {
    add (new fcn_arg_elem<void *> (fcn, ptr));
  }

  operator bool (void) const { return ! empty (); }

  GCC_ATTR_DEPRECATED void run_top (void) { run_first (); }

  void run_first (void)
  {
    if (! empty ())
      {
        // No leak on exception!
        std::auto_ptr<elem> ptr (lifo.top ());
        lifo.pop ();
        ptr->run ();
      }
  }

  GCC_ATTR_DEPRECATED void run_top (int num) { run (num); }

  GCC_ATTR_DEPRECATED void discard_top (void) { discard_first (); }

  void discard_first (void)
  {
    if (! empty ())
      {
        elem *ptr = lifo.top ();
        lifo.pop ();
        delete ptr;
      }
  }

  GCC_ATTR_DEPRECATED void discard_top (int num) { discard (num); }

  size_t size (void) const { return lifo.size (); }

protected:

  std::stack<elem *> lifo;

private:

  // No copying!

  unwind_protect (const unwind_protect&);

  unwind_protect& operator = (const unwind_protect&);
};

// Like unwind_protect, but this one will guard against the
// possibility of seeing an exception (or interrupt) in the cleanup
// actions. Not that we can do much about it, but at least we won't
// crash.

class
OCTAVE_API
unwind_protect_safe : public unwind_protect
{
private:

  static void gripe_exception (void);

public:

  unwind_protect_safe (void) : unwind_protect () { }

  ~unwind_protect_safe (void)
  {
    while (! empty ())
      {
        try
          {
            run_first ();
          }
        catch (...) // Yes, the black hole. Remember we're in a dtor.
          {
            gripe_exception ();
          }
      }
  }

private:

  // No copying!

  unwind_protect_safe (const unwind_protect_safe&);

  unwind_protect_safe& operator = (const unwind_protect_safe&);
};

#endif
