/*

Copyright (C) 1996-2017 John W. Eaton

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

#if ! defined (octave_ov_mex_fcn_h)
#define octave_ov_mex_fcn_h 1

#include "octave-config.h"

#include <string>

#include "oct-shlib.h"

#include "ov-fcn.h"
#include "ov-builtin.h"
#include "ov-typeinfo.h"

class octave_value;
class octave_value_list;

// Dynamically-linked functions.

class
octave_mex_function : public octave_function
{
public:

  octave_mex_function (void)
    : m_mex_fcn_ptr (), m_exit_fcn_ptr (), m_is_fmex (), m_sh_lib (),
      m_time_checked (), m_is_system_fcn_file () { }

  octave_mex_function (void *fptr, bool fmex,
                       const octave::dynamic_library& shl,
                       const std::string& nm = "");

  // No copying!

  octave_mex_function (const octave_mex_function& fn) = delete;

  octave_mex_function& operator = (const octave_mex_function& fn) = delete;

  ~octave_mex_function (void);

  octave_value subsref (const std::string& type,
                        const std::list<octave_value_list>& idx)
  {
    octave_value_list tmp = subsref (type, idx, 1);
    return tmp.length () > 0 ? tmp(0) : octave_value ();
  }

  octave_value_list subsref (const std::string& type,
                             const std::list<octave_value_list>& idx,
                             int nargout);

  octave_function * function_value (bool = false) { return this; }

  const octave_function * function_value (bool = false) const { return this; }

  void mark_fcn_file_up_to_date (const octave::sys::time& t)
  {
    m_time_checked = t;
  }

  std::string fcn_file_name (void) const;

  octave::sys::time time_parsed (void) const;

  octave::sys::time time_checked (void) const { return m_time_checked; }

  bool is_system_fcn_file (void) const { return m_is_system_fcn_file; }

  bool is_builtin_function (void) const { return false; }

  bool is_mex_function (void) const { return true; }

  octave_value_list
  do_multi_index_op (int nargout, const octave_value_list& args);

  void atexit (void (*fcn) (void)) { m_exit_fcn_ptr = fcn; }

  octave::dynamic_library get_shlib (void) const { return m_sh_lib; }

  void *mex_fcn_ptr (void) const { return m_mex_fcn_ptr; }

  bool is_fmex (void) const { return m_is_fmex; }

private:

  void *m_mex_fcn_ptr;

  void (*m_exit_fcn_ptr) (void);

  bool m_is_fmex;

  octave::dynamic_library m_sh_lib;

  // The time the file was last checked to see if it needs to be
  // parsed again.
  mutable octave::sys::time m_time_checked;

  // True if this function came from a file that is considered to be a
  // system function.  This affects whether we check the time stamp
  // on the file to see if it has changed.
  bool m_is_system_fcn_file;

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

#endif
