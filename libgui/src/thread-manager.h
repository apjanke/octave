/*

Copyright (C) 2013-2017 John W. Eaton

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

#if ! defined (octave_thread_manager_h)
#define octave_thread_manager_h 1

#include "octave-config.h"

#include "oct-refcount.h"

class octave_base_thread_manager
{
public:

  friend class octave_thread_manager;

  octave_base_thread_manager (void) : m_count (1) { }

  octave_base_thread_manager (const octave_base_thread_manager&)
    : m_count (1)
  { }

  virtual ~octave_base_thread_manager (void) = default;

  virtual void register_current_thread (void) = 0;

  virtual void interrupt (void) = 0;

protected:

  octave::refcount<int> m_count;
};

class octave_thread_manager
{
public:

  octave_thread_manager (void);

  ~octave_thread_manager (void)
  {
    if (--m_rep->m_count == 0)
      delete m_rep;
  }

  octave_thread_manager (const octave_thread_manager& tm) : m_rep (tm.m_rep) { }

  octave_thread_manager& operator = (const octave_thread_manager& tm)
  {
    if (m_rep != tm.m_rep)
      {
        if (--m_rep->m_count == 0)
          delete m_rep;

        m_rep = tm.m_rep;
        m_rep->m_count++;
      }

    return *this;
  }

  void register_current_thread (void) { m_rep->register_current_thread (); }

  void interrupt (void) { m_rep->interrupt (); }

  static void block_interrupt_signal (void);

  static void unblock_interrupt_signal (void);

private:

  octave_base_thread_manager *m_rep;

  static octave_base_thread_manager * create_rep (void);
};

#endif
