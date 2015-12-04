/*

Copyright (C) 1996-2015 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include <sys/types.h>

#include "oct-group.h"

#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "oct-map.h"
#include "ov.h"
#include "oct-obj.h"
#include "utils.h"

// Group file functions.  (Why not?)

static octave_value
mk_gr_map (const octave_group& gr)
{
  octave_value retval;

  if (gr)
    {
      octave_scalar_map m;

      m.assign ("name", gr.name ());
      m.assign ("passwd", gr.passwd ());
      m.assign ("gid", static_cast<double> (gr.gid ()));
      m.assign ("mem", octave_value (gr.mem ()));

      retval = m;
    }
  else
    retval = 0;

  return retval;
}

DEFUN (getgrent, args, ,
       "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {@var{grp_struct} =} getgrent ()\n\
Return an entry from the group database, opening it if necessary.\n\
\n\
Once the end of data has been reached, @code{getgrent} returns 0.\n\
@seealso{setgrent, endgrent}\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave_group::getgrent may set msg.
  octave_value val = mk_gr_map (octave_group::getgrent (msg));

  retval(1) = msg;
  retval(0) = val;

  return retval;
}

DEFUN (getgrgid, args, ,
       "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {@var{grp_struct} =} getgrgid (@var{gid}).\n\
Return the first entry from the group database with the group ID\n\
@var{gid}.\n\
\n\
If the group ID does not exist in the database, @code{getgrgid} returns 0.\n\
@seealso{getgrnam}\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () != 1)
    print_usage ();

  double dval = args(0).double_value ();

  if (D_NINT (dval) == dval)
    {
      gid_t gid = static_cast<gid_t> (dval);

      std::string msg;

      // octave_group::getgrgid may set msg.
      octave_value val = mk_gr_map (octave_group::getgrgid (gid, msg));

      retval(1) = msg;
      retval(0) = val;
    }

  else
    error ("getgrgid: GID must be an integer");

  return retval;
}

DEFUN (getgrnam, args, ,
       "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {@var{grp_struct} =} getgrnam (@var{name})\n\
Return the first entry from the group database with the group name\n\
@var{name}.\n\
\n\
If the group name does not exist in the database, @code{getgrnam} returns 0.\n\
@seealso{getgrgid}\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () != 1)
    print_usage ();

  std::string s = args(0).string_value ();

  std::string msg;

  // octave_group::getgrnam may set msg.
  octave_value val = mk_gr_map (octave_group::getgrnam (s.c_str (), msg));

  retval(1) = msg;
  retval(0) = val;

  return retval;
}

DEFUN (setgrent, args, ,
       "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} setgrent ()\n\
Return the internal pointer to the beginning of the group database.\n\
@seealso{getgrent, endgrent}\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave_group::setgrent may set msg.
  int status = octave_group::setgrent (msg);

  retval(1) = msg;
  retval(0) = static_cast<double> (status);

  return retval;
}

DEFUN (endgrent, args, ,
       "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} endgrent ()\n\
Close the group database.\n\
@seealso{getgrent, setgrent}\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () != 0)
    print_usage ();

  std::string msg;

  // octave_group::endgrent may set msg.
  int status = octave_group::endgrent (msg);

  retval(1) = msg;
  retval(0) = static_cast<double> (status);

  return retval;
}
