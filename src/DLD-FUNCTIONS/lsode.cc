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

#include "LSODE.h"
#include "lo-mappers.h"

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "ov-fcn.h"
#include "pager.h"
#include "pr-output.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

#include "LSODE-opts.cc"

// Global pointer for user defined function required by lsode.
static octave_function *lsode_fcn;

// Global pointer for optional user defined jacobian function used by lsode.
static octave_function *lsode_jac;

// Is this a recursive call?
static int call_depth = 0;

ColumnVector
lsode_user_function (const ColumnVector& x, double t)
{
  ColumnVector retval;

  int nstates = x.capacity ();

  octave_value_list args;
  args(1) = t;

  Matrix m (nstates, 1);
  for (int i = 0; i < nstates; i++)
    m (i, 0) = x (i);
  octave_value state (m);
  args(0) = state;

  if (lsode_fcn)
    {
      octave_value_list tmp = lsode_fcn->do_multi_index_op (1, args);

      if (error_state)
	{
	  gripe_user_supplied_eval ("lsode");
	  return retval;
	}

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  retval = ColumnVector (tmp(0).vector_value ());

	  if (error_state || retval.length () == 0)
	    gripe_user_supplied_eval ("lsode");
	}
      else
	gripe_user_supplied_eval ("lsode");
    }

  return retval;
}

Matrix
lsode_user_jacobian (const ColumnVector& x, double t)
{
  Matrix retval;

  int nstates = x.capacity ();

  octave_value_list args;
  args(1) = t;

  Matrix m (nstates, 1);
  for (int i = 0; i < nstates; i++)
    m (i, 0) = x (i);
  octave_value state (m);
  args(0) = state;

  if (lsode_jac)
    {
      octave_value_list tmp = lsode_jac->do_multi_index_op (1, args);

      if (error_state)
	{
	  gripe_user_supplied_eval ("lsode");
	  return retval;
	}

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  retval = tmp(0).matrix_value ();

	  if (error_state || retval.length () == 0)
	    gripe_user_supplied_eval ("lsode");
	}
      else
	gripe_user_supplied_eval ("lsode");
    }

  return retval;
}

#define LSODE_ABORT() \
  do \
    { \
      unwind_protect::run_frame ("Flsode"); \
      return retval; \
    } \
  while (0)
 
#define LSODE_ABORT1(msg) \
  do \
    { \
      ::error ("lsode: " msg); \
      LSODE_ABORT (); \
    } \
  while (0)

#define LSODE_ABORT2(fmt, arg) \
  do \
    { \
      ::error ("lsode: " fmt, arg); \
      LSODE_ABORT (); \
    } \
  while (0)

DEFUN_DLD (lsode, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{x}, @var{istate}, @var{msg}]} lsode (@var{fcn}, @var{x_0}, @var{t}, @var{t_crit})\n\
Solve the set of differential equations\n\
@tex\n\
$$ {dx \\over dt} = f (x, t) $$\n\
with\n\
$$ x(t_0) = x_0 $$\n\
@end tex\n\
@ifinfo\n\
\n\
@example\n\
dx\n\
-- = f(x, t)\n\
dt
@end example\n\
\n\
with\n\
\n\
@example\n\
x(t_0) = x_0\n\
@end example\n\
\n\
@end ifinfo\n\
The solution is returned in the matrix @var{x}, with each row\n\
corresponding to an element of the vector @var{t}.  The first element\n\
of @var{t} should be @math{t_0} and should correspond to the initial\n\
state of the system @var{x_0}, so that the first row of the output\n\
is @var{x_0}.\n\
\n\
The first argument, @var{fcn}, is a string that names the function to\n\
call to compute the vector of right hand sides for the set of equations.\n\
The function must have the form\n\
\n\
@example\n\
@var{xdot} = f (@var{x}, @var{t})\n\
@end example\n\
\n\
@noindent\n\
in which @var{xdot} and @var{x} are vectors and @var{t} is a scalar.\n\
\n\
If @var{fcn} is a two-element string array, the first element names the\n\
function @math{f} described above, and the second element names a function\n\
to compute the Jacobian of @math{f}.  The Jacobian function must have the\n\
form\n\
\n\
@example\n\
@var{jac} = j (@var{x}, @var{t})\n\
@end example\n\
\n\
in which @var{jac} is the matrix of partial derivatives\n\
@tex\n\
$$ J = {\\partial f_i \\over \\partial x_j} = \\left[\\matrix{\n\
{\\partial f_1 \\over \\partial x_1}\n\
  & {\\partial f_1 \\over \\partial x_2}\n\
  & \\cdots\n\
  & {\\partial f_1 \\over \\partial x_N} \\cr\n\
{\\partial f_2 \\over \\partial x_1}\n\
  & {\\partial f_2 \\over \\partial x_2}\n\
  & \\cdots\n\
  & {\\partial f_2 \\over \\partial x_N} \\cr\n\
 \\vdots & \\vdots & \\ddots & \\vdots \\cr\n\
{\\partial f_3 \\over \\partial x_1}\n\
  & {\\partial f_3 \\over \\partial x_2}\n\
  & \\cdots\n\
  & {\\partial f_3 \\over \\partial x_N} \\cr}\\right]$$\n\
@end tex\n\
@ifinfo\n\
\n\
@example\n\
             | df_1  df_1       df_1 |\n\
             | ----  ----  ...  ---- |\n\
             | dx_1  dx_2       dx_N |\n\
             |                       |\n\
             | df_2  df_2       df_2 |\n\
             | ----  ----  ...  ---- |\n\
      df_i   | dx_1  dx_2       dx_N |\n\
jac = ---- = |                       |\n\
      dx_j   |  .    .     .    .    |\n\
             |  .    .      .   .    |\n\
             |  .    .       .  .    |\n\
             |                       |\n\
             | df_N  df_N       df_N |\n\
             | ----  ----  ...  ---- |\n\
             | dx_1  dx_2       dx_N |\n\
@end example\n\
\n\
@end ifinfo\n\
\n\
The second and third arguments specify the intial state of the system,\n\
@math{x_0}, and the initial value of the independent variable @math{t_0}.\n\
\n\
The fourth argument is optional, and may be used to specify a set of\n\
times that the ODE solver should not integrate past.  It is useful for\n\
avoiding difficulties with singularities and points where there is a\n\
discontinuity in the derivative.\n\
\n\
After a successful computation, the value of @var{istate} will be 2\n\
(consistent with the Fortran version of @sc{Lsode}).\n\
\n\
If the computation is not successful, @var{istate} will be something\n\
other than 2 and @var{msg} will contain additional information.\n\
\n\
You can use the function @code{lsode_options} to set optional\n\
parameters for @code{lsode}.\n\
@end deftypefn\n\
@seealso{daspk, dassl, dasrt, odessa}")
{
  octave_value_list retval;

  unwind_protect::begin_frame ("Flsode");

  unwind_protect_int (call_depth);
  call_depth++;

  if (call_depth > 1)
    LSODE_ABORT1 ("invalid recursive call");

  int nargin = args.length ();

  if (nargin > 2 && nargin < 5 && nargout < 4)
    {
      lsode_fcn = 0;
      lsode_jac = 0;

      octave_value f_arg = args(0);

      switch (f_arg.rows ())
	{
	case 1:
	  lsode_fcn = extract_function
	    (f_arg, "lsode", "__lsode_fcn__",
	     "function xdot = __lsode_fcn__ (x, t) xdot = ",
	     "; endfunction");
	  break;

	case 2:
	  {
	    string_vector tmp = f_arg.all_strings ();

	    if (! error_state)
	      {
		lsode_fcn = extract_function
		  (tmp(0), "lsode", "__lsode_fcn__",
		   "function xdot = __lsode_fcn__ (x, t) xdot = ",
		   "; endfunction");

		if (lsode_fcn)
		  {
		    lsode_jac = extract_function
		      (tmp(1), "lsode", "__lsode_jac__",
		       "function jac = __lsode_jac__ (x, t) jac = ",
		       "; endfunction");

		    if (! lsode_jac)
		      lsode_fcn = 0;
		  }
	      }
	  }
	  break;

	default:
	  LSODE_ABORT1
	    ("first arg should be a string or 2-element string array");
	}

      if (error_state || ! lsode_fcn)
	LSODE_ABORT ();

      ColumnVector state (args(1).vector_value ());

      if (error_state)
	LSODE_ABORT1 ("expecting state vector as second argument");

      ColumnVector out_times (args(2).vector_value ());

      if (error_state)
	LSODE_ABORT1 ("expecting output time vector as third argument");

      ColumnVector crit_times;

      int crit_times_set = 0;
      if (nargin > 3)
	{
	  crit_times = ColumnVector (args(3).vector_value ());

	  if (error_state)
	    LSODE_ABORT1 ("expecting critical time vector as fourth argument");

	  crit_times_set = 1;
	}

      double tzero = out_times (0);

      ODEFunc func (lsode_user_function);
      if (lsode_jac)
	func.set_jacobian_function (lsode_user_jacobian);

      LSODE ode (state, tzero, func);

      ode.copy (lsode_opts);

      Matrix output;
      if (crit_times_set)
	output = ode.integrate (out_times, crit_times);
      else
	output = ode.integrate (out_times);

      if (! error_state)
	{
	  std::string msg = ode.error_message ();

	  retval(2) = msg;
	  retval(1) = static_cast<double> (ode.integration_state ());

	  if (ode.integration_ok ())
	    retval(0) = output;
	  else
	    {
	      retval(0) = Matrix ();

	      if (nargout < 2)
		error ("lsode: %s", msg.c_str ());
	    }
	}
    }
  else
    print_usage ("lsode");

  unwind_protect::run_frame ("Flsode");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
