/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include <iomanip>
#include <iostream>

#include "NLEqn.h"

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "ov-fcn.h"
#include "pager.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

#include "NLEqn-opts.cc"

// Global pointer for user defined function required by hybrd1.
static octave_function *fsolve_fcn;

// Is this a recursive call?
static int call_depth = 0;

int
hybrd_info_to_fsolve_info (int info)
{
  switch (info)
    {
    case -1:
      info = -2;
      break;

    case 0:
      info = -1;
      break;

    case 1:
      break;

    case 2:
      info = 4;
      break;

    case 3:
    case 4:
    case 5:
      info = 3;
      break;

    default:
      panic_impossible ();
      break;
    }

  return info;
}

ColumnVector
fsolve_user_function (const ColumnVector& x)
{
  ColumnVector retval;

  int n = x.capacity ();

  octave_value_list args;
  args.resize (1);

  if (n > 1)
    {
      Matrix m (n, 1);
      for (int i = 0; i < n; i++)
	m (i, 0) = x (i);
      octave_value vars (m);
      args(0) = vars;
    }
  else
    {
      double d = x (0);
      octave_value vars (d);
      args(0) = vars;
    }

  if (fsolve_fcn)
    {
      octave_value_list tmp = fsolve_fcn->do_multi_index_op (1, args);
      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  retval = ColumnVector (tmp(0).vector_value ());

	  if (error_state || retval.length () <= 0)
	    gripe_user_supplied_eval ("fsolve");
	}
      else
	gripe_user_supplied_eval ("fsolve");
    }

  return retval;
}

#define FSOLVE_ABORT() \
  do \
    { \
      unwind_protect::run_frame ("Ffsolve"); \
      return retval; \
    } \
  while (0)

#define FSOLVE_ABORT1(msg) \
  do \
    { \
      ::error ("fsolve: " msg); \
      FSOLVE_ABORT (); \
    } \
  while (0)

#define FSOLVE_ABORT2(fmt, arg) \
  do \
    { \
      ::error ("fsolve: " fmt, arg); \
      FSOLVE_ABORT (); \
    } \
  while (0)

DEFUN_DLD (fsolve, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{x}, @var{info}, @var{msg}] =} fsolve (@var{fcn}, @var{x0})\n\
Given @var{fcn}, the name of a function of the form @code{f (@var{x})}\n\
and an initial starting point @var{x0}, @code{fsolve} solves the set of\n\
equations such that @code{f(@var{x}) == 0}.\n\
\n\
You can use the function @code{fsolve_options} to set optional\n\
parameters for @code{fsolve}.\n\
@end deftypefn")
{
  octave_value_list retval;

  unwind_protect::begin_frame ("Ffsolve");

  unwind_protect_int (call_depth);
  call_depth++;

  if (call_depth > 1)
    FSOLVE_ABORT1 ("invalid recursive call");

  int nargin = args.length ();

  if (nargin == 2 && nargout < 4)
    {
      fsolve_fcn = extract_function (args(0), "fsolve", "__fsolve_fcn__",
				    "function y = __fsolve_fcn__ (x) y = ",
				    "; endfunction");
      if (! fsolve_fcn)
	FSOLVE_ABORT ();

      ColumnVector x (args(1).vector_value ());

      if (error_state)
	FSOLVE_ABORT1 ("expecting vector as second argument");

      if (nargin > 2)
	warning ("fsolve: ignoring extra arguments");

      if (nargout > 2)
	warning ("fsolve: can't compute path output yet");

      NLFunc nleqn_fcn (fsolve_user_function);
      NLEqn nleqn (x, nleqn_fcn);
      nleqn.copy (fsolve_opts);

      int info;
      ColumnVector soln = nleqn.solve (info);

      if (! error_state)
	{
	  std::string msg = nleqn.error_message ();

	  retval(2) = msg;
	  retval(1) = static_cast<double> (hybrd_info_to_fsolve_info (info));

	  if (nleqn.solution_ok ())
	    retval(0) = soln;
	  else
	    {
	      retval(0) = Matrix ();

	      if (nargout < 2)
		error ("fsolve: %s", msg.c_str ());
	    }
	}
    }
  else
    print_usage ("fsolve");

  unwind_protect::run_frame ("Ffsolve");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
