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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <string>

#include <sys/types.h>

#include "oct-group.h"

#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "oct-map.h"
#include "ov.h"
#include "ovl.h"
#include "utils.h"

// Group file functions.  (Why not?)

static octave_value
mk_gr_map (const octave::sys::group& gr)
{
  octave_value retval;

  if (gr)
    {
      octave_scalar_map m;

      m.assign ("name", gr.name ());
      m.assign ("passwd", gr.passwd ());
      m.assign ("gid", static_cast<double> (gr.gid ()));
      m.assign ("mem", octave_value (gr.mem ()));

      retval = ovl (m);
    }
  else
    retval = ovl (0);

  return retval;
}

DEFUN (getgrent, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{grp_struct} =} getgrent ()
Return an entry from the group database, opening it if necessary.

Once the end of data has been reached, @code{getgrent} returns 0.
@seealso{setgrent, endgrent}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave::sys::group::getgrent may set msg.
  octave_value val = mk_gr_map (octave::sys::group::getgrent (msg));

  return ovl (val, msg);
}

DEFUN (getgrgid, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{grp_struct} =} getgrgid (@var{gid}).
Return the first entry from the group database with the group ID
@var{gid}.

If the group ID does not exist in the database, @code{getgrgid} returns 0.
@seealso{getgrnam}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  double dval = args(0).double_value ();

  if (octave::math::x_nint (dval) != dval)
    error ("getgrgid: GID must be an integer");

  gid_t gid = static_cast<gid_t> (dval);

  std::string msg;

  // octave::sys::group::getgrgid may set msg.
  octave_value val = mk_gr_map (octave::sys::group::getgrgid (gid, msg));

  return ovl (val, msg);
}

DEFUN (getgrnam, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {@var{grp_struct} =} getgrnam (@var{name})
Return the first entry from the group database with the group name
@var{name}.

If the group name does not exist in the database, @code{getgrnam} returns 0.
@seealso{getgrgid}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  std::string s = args(0).string_value ();

  std::string msg;

  // octave::sys::group::getgrnam may set msg.
  octave_value val = mk_gr_map (octave::sys::group::getgrnam (s.c_str (), msg));

  return ovl (val, msg);
}

DEFUN (setgrent, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} setgrent ()
Return the internal pointer to the beginning of the group database.
@seealso{getgrent, endgrent}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave::sys::group::setgrent may set msg.
  int status = octave::sys::group::setgrent (msg);

  return ovl (static_cast<double> (status), msg);
}

DEFUN (endgrent, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} endgrent ()
Close the group database.
@seealso{getgrent, setgrent}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave::sys::group::endgrent may set msg.
  int status = octave::sys::group::endgrent (msg);

  return ovl (static_cast<double> (status), msg);
}
