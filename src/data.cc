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

#include <cfloat>
#include <cmath>

#include <string>

#include "lo-ieee.h"
#include "str-vec.h"

#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "oct-map.h"
#include "ov.h"
#include "variables.h"
#include "oct-obj.h"
#include "utils.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(x) (((x) < 0) ? (-x) : (x))
#endif

DEFUN (all, args, ,
  "all (X): are all elements of X nonzero?")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = args(0).all ();
  else
    print_usage ("all");

  return retval;
}

DEFUN (any, args, ,
  "any (X): are any elements of X nonzero?")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = args(0).any ();
  else
    print_usage ("any");

  return retval;
}

// These mapping functions may also be useful in other places, eh?

typedef double (*d_dd_fcn) (double, double);

static Matrix
map_d_m (d_dd_fcn f, double x, const Matrix& y)
{
  int nr = y.rows ();
  int nc = y.columns ();

  Matrix retval (nr, nc);

  for (int j = 0; j < nc; j++)
    for (int i = 0; i < nr; i++)
      retval (i, j) = f (x, y (i, j));

  return retval;
}

static Matrix
map_m_d (d_dd_fcn f, const Matrix& x, double y)
{
  int nr = x.rows ();
  int nc = x.columns ();

  Matrix retval (nr, nc);

  for (int j = 0; j < nc; j++)
    for (int i = 0; i < nr; i++)
      retval (i, j) = f (x (i, j), y);

  return retval;
}

static Matrix
map_m_m (d_dd_fcn f, const Matrix& x, const Matrix& y)
{
  int x_nr = x.rows ();
  int x_nc = x.columns ();

  int y_nr = y.rows ();
  int y_nc = y.columns ();

  assert (x_nr == y_nr && x_nc == y_nc);

  Matrix retval (x_nr, x_nc);

  for (int j = 0; j < x_nc; j++)
    for (int i = 0; i < x_nr; i++)
      retval (i, j) = f (x (i, j), y (i, j));

  return retval;
}

DEFUN (atan2, args, ,
  "atan2 (Y, X): atan (Y / X) in range -pi to pi")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 2 && args(0).is_defined () && args(1).is_defined ())
    {
      octave_value arg_y = args(0);
      octave_value arg_x = args(1);

      int y_nr = arg_y.rows ();
      int y_nc = arg_y.columns ();

      int x_nr = arg_x.rows ();
      int x_nc = arg_x.columns ();

      int arg_y_empty = empty_arg ("atan2", y_nr, y_nc);
      int arg_x_empty = empty_arg ("atan2", x_nr, x_nc);

      if (arg_y_empty > 0 && arg_x_empty > 0)
	return Matrix ();
      else if (arg_y_empty || arg_x_empty)
	return retval;

      int y_is_scalar = (y_nr == 1 && y_nc == 1);
      int x_is_scalar = (x_nr == 1 && x_nc == 1);

      if (y_is_scalar && x_is_scalar)
	{
	  double y = arg_y.double_value ();

	  if (! error_state)
	    {
	      double x = arg_x.double_value ();

	      if (! error_state)
		retval = atan2 (y, x);
	    }
	}
      else if (y_is_scalar)
	{
	  double y = arg_y.double_value ();

	  if (! error_state)
	    {
	      Matrix x = arg_x.matrix_value ();

	      if (! error_state)
		retval = map_d_m (atan2, y, x);
	    }
	}
      else if (x_is_scalar)
	{
	  Matrix y = arg_y.matrix_value ();

	  if (! error_state)
	    {
	      double x = arg_x.double_value ();

	      if (! error_state)
		retval = map_m_d (atan2, y, x);
	    }
	}
      else if (y_nr == x_nr && y_nc == x_nc)
	{
	  Matrix y = arg_y.matrix_value ();

	  if (! error_state)
	    {
	      Matrix x = arg_x.matrix_value ();

	      if (! error_state)
		retval = map_m_m (atan2, y, x);
	    }
	}
      else
	error ("atan2: nonconformant matrices");
    }
  else
    print_usage ("atan2");

  return retval;
}

DEFUN (cumprod, args, ,
  "cumprod (X): cumulative products")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      octave_value arg = args(0);

      if (arg.is_real_type ())
	{
	  Matrix tmp = arg.matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.cumprod ();
	}
      else if (arg.is_complex_type ())
	{
	  ComplexMatrix tmp = arg.complex_matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.cumprod ();
	}
      else
	{
	  gripe_wrong_type_arg ("cumprod", arg);
	  return retval;
	}
    }
  else
    print_usage ("cumprod");

  return retval;
}

DEFUN (cumsum, args, ,
  "cumsum (X): cumulative sums")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      octave_value arg = args(0);

      if (arg.is_real_type ())
	{
	  Matrix tmp = arg.matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.cumsum ();
	}
      else if (arg.is_complex_type ())
	{
	  ComplexMatrix tmp = arg.complex_matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.cumsum ();
	}
      else
	{
	  gripe_wrong_type_arg ("cumsum", arg);
	  return retval;
	}
    }
  else
    print_usage ("cumsum");

  return retval;
}

static octave_value
make_diag (const Matrix& v, int k)
{
  int nr = v.rows ();
  int nc = v.columns ();
  assert (nc == 1 || nr == 1);

  octave_value retval;

  int roff = 0;
  int coff = 0;
  if (k > 0)
    {
      roff = 0;
      coff = k;
    }
  else if (k < 0)
    {
      roff = -k;
      coff = 0;
    }

  if (nr == 1)
    {
      int n = nc + ABS (k);
      Matrix m (n, n, 0.0);
      for (int i = 0; i < nc; i++)
	m (i+roff, i+coff) = v (0, i);
      retval = octave_value (m);
    }
  else
    {
      int n = nr + ABS (k);
      Matrix m (n, n, 0.0);
      for (int i = 0; i < nr; i++)
	m (i+roff, i+coff) = v (i, 0);
      retval = octave_value (m);
    }

  return retval;
}

static octave_value
make_diag (const ComplexMatrix& v, int k)
{
  int nr = v.rows ();
  int nc = v.columns ();
  assert (nc == 1 || nr == 1);

  octave_value retval;

  int roff = 0;
  int coff = 0;
  if (k > 0)
    {
      roff = 0;
      coff = k;
    }
  else if (k < 0)
    {
      roff = -k;
      coff = 0;
    }

  if (nr == 1)
    {
      int n = nc + ABS (k);
      ComplexMatrix m (n, n, 0.0);
      for (int i = 0; i < nc; i++)
	m (i+roff, i+coff) = v (0, i);
      retval = octave_value (m);
    }
  else
    {
      int n = nr + ABS (k);
      ComplexMatrix m (n, n, 0.0);
      for (int i = 0; i < nr; i++)
	m (i+roff, i+coff) = v (i, 0);
      retval = octave_value (m);
    }

  return retval;
}

static octave_value
make_diag (const octave_value& arg)
{
  octave_value retval;

  if (arg.is_real_type ())
    {
      Matrix m = arg.matrix_value ();

      if (! error_state)
	{
	  int nr = m.rows ();
	  int nc = m.columns ();

	  if (nr == 0 || nc == 0)
	    retval = Matrix ();
	  else if (nr == 1 || nc == 1)
	    retval = make_diag (m, 0);
	  else
	    {
	      ColumnVector v = m.diag ();
	      if (v.capacity () > 0)
		retval = v;
	    }
	}
      else
	gripe_wrong_type_arg ("diag", arg);
    }
  else if (arg.is_complex_type ())
    {
      ComplexMatrix cm = arg.complex_matrix_value ();

      if (! error_state)
	{
	  int nr = cm.rows ();
	  int nc = cm.columns ();

	  if (nr == 0 || nc == 0)
	    retval = Matrix ();
	  else if (nr == 1 || nc == 1)
	    retval = make_diag (cm, 0);
	  else
	    {
	      ComplexColumnVector v = cm.diag ();
	      if (v.capacity () > 0)
		retval = v;
	    }
	}
      else
	gripe_wrong_type_arg ("diag", arg);
    }
  else
    gripe_wrong_type_arg ("diag", arg);

  return retval;
}

static octave_value
make_diag (const octave_value& a, const octave_value& b)
{
  octave_value retval;

  int k = b.nint_value ();

  if (error_state)
    {
      error ("diag: invalid second argument");      
      return retval;
    }

  if (a.is_real_type ())
    {
      Matrix m = a.matrix_value ();

      if (! error_state)
	{
	  int nr = m.rows ();
	  int nc = m.columns ();

	  if (nr == 0 || nc == 0)
	    retval = Matrix ();
	  else if (nr == 1 || nc == 1)
	    retval = make_diag (m, k);
	  else
	    {
	      ColumnVector d = m.diag (k);
	      retval = d;
	    }
	}
    }
  else if (a.is_complex_type ())
    {
      ComplexMatrix cm = a.complex_matrix_value ();

      if (! error_state)
	{
	  int nr = cm.rows ();
	  int nc = cm.columns ();

	  if (nr == 0 || nc == 0)
	    retval = Matrix ();
	  else if (nr == 1 || nc == 1)
	    retval = make_diag (cm, k);
	  else
	    {
	      ComplexColumnVector d = cm.diag (k);
	      retval = d;
	    }
	}
    }
  else
    gripe_wrong_type_arg ("diag", a);

  return retval;
}

DEFUN (diag, args, ,
  "diag (X [,k]): form/extract diagonals")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = make_diag (args(0));
  else if (nargin == 2 && args(0).is_defined () && args(1).is_defined ())
    retval = make_diag (args(0), args(1));
  else
    print_usage ("diag");

  return retval;
}

DEFUN (prod, args, ,
  "prod (X): products")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      octave_value arg = args(0);

      if (arg.is_real_type ())
	{
	  Matrix tmp = arg.matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.prod ();
	}
      else if (arg.is_complex_type ())
	{
	  ComplexMatrix tmp = arg.complex_matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.prod ();
	}
      else
	{
	  gripe_wrong_type_arg ("prod", arg);
	  return retval;
	}
    }
  else
    print_usage ("prod");

  return retval;
}

DEFUN (length, args, ,
  "length (x): return the `length' of the object X\n\
\n\
For matrix objects, the length is the number of rows or columns,\n\
whichever is greater (this odd definition is used for compatibility\n\
with Matlab).\n\
\n\
See also: size, rows, columns, is_scalar, is_vector, is_matrix")
{
  octave_value retval;

  if (args.length () == 1)
    {
      int len = args(0).length ();

      if (! error_state)
	retval = static_cast<double> (len);
    }
  else
    print_usage ("length");

  return retval;
}

DEFUN (size, args, nargout,
  "[m, n] = size (x): return rows and columns of X\n\
\n\
d = size (x): return number of rows and columns of x as a row vector\n\
\n\
m = size (x, 1): return number of rows in x\n\
m = size (x, 2): return number of columns in x")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1 && nargout < 3)
    {
      int nr = args(0).rows ();
      int nc = args(0).columns ();

      if (nargout == 0 || nargout == 1)
	{
	  Matrix m (1, 2);
	  m (0, 0) = nr;
	  m (0, 1) = nc;
	  retval = m;
	}
      else if (nargout == 2)
	{
	  retval(1) = static_cast<double> (nc);
	  retval(0) = static_cast<double> (nr);
	}
    }
  else if (nargin == 2 && nargout < 2)
    {
      int nd = args(1).nint_value ();

      if (error_state)
	error ("size: expecting scalar as second argument");
      else
	{
	  if (nd == 1)
	    retval(0) = static_cast<double> (args(0).rows ());
	  else if (nd == 2)
	    retval(0) = static_cast<double> (args(0).columns ());
	  else
	    error ("size: invalid second argument -- expecting 1 or 2");
	}
    }
  else
    print_usage ("size");

  return retval;
}

DEFUN (sum, args, ,
  "sum (X): sum of elements")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      octave_value arg = args(0);

      if (arg.is_real_type ())
	{
	  Matrix tmp = arg.matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.sum ();
	}
      else if (arg.is_complex_type ())
	{
	  ComplexMatrix tmp = arg.complex_matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.sum ();
	}
      else
	{
	  gripe_wrong_type_arg ("sum", arg);
	  return retval;
	}
    }
  else
    print_usage ("sum");

  return retval;
}

DEFUN (sumsq, args, ,
  "sumsq (X): sum of squares of elements.\n\
\n\
This function is equivalent to computing\n\
\n\
  sum (X .* conj (X))\n\
\n\
but it uses less memory and avoids calling conj if X is real.")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      octave_value arg = args(0);

      if (arg.is_real_type ())
	{
	  Matrix tmp = arg.matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.sumsq ();
	}
      else if (arg.is_complex_type ())
	{
	  ComplexMatrix tmp = arg.complex_matrix_value ();

	  if (! error_state)
	    retval(0) = tmp.sumsq ();
	}
      else
	{
	  gripe_wrong_type_arg ("sumsq", arg);
	  return retval;
	}
    }
  else
    print_usage ("sumsq");

  return retval;
}

DEFUN (is_bool, args, ,
  "is_bool (x): return nonzero if x is a boolean object")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_bool_type ();
  else
    print_usage ("is_bool");

  return retval;
}

DEFALIAS (islogical, is_bool);

DEFUN (is_complex, args, ,
  "is_complex (x): return nonzero if x is a complex-valued numeric object")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_complex_type ();
  else
    print_usage ("is_complex");

  return retval;
}

DEFUN (isreal, args, ,
  "isreal (x): return nonzero if x is a real-valued numeric object")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_real_type ();
  else
    print_usage ("isreal");

  return retval;
}

DEFUN (isempty, args, ,
  "isempty (x): return nonzero if x is an empty matrix, string, or list")
{
  double retval = 0.0;

  if (args.length () == 1)
    {
      octave_value arg = args(0);

      if (arg.is_matrix_type ())
	retval = static_cast<double> (arg.rows () == 0 || arg.columns () == 0);
      else if (arg.is_list () || arg.is_string ())
	retval = static_cast<double> (arg.length () == 0);
    }
  else
    print_usage ("isempty");

  return retval;
}

DEFUN (isnumeric, args, ,
  "isnumeric (x): return nonzero if x is a numeric object")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_numeric_type ();
  else
    print_usage ("isnumeric");

  return retval;
}

DEFUN (is_list, args, ,
  "is_list (x): return nonzero if x is a list")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_list ();
  else
    print_usage ("is_list");

  return retval;
}

DEFUN (is_matrix, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Usage} {} is_matrix (@var{a})\n\
Return 1 if @var{a} is a matrix.  Otherwise, return 0.\n\
@end deftypefn")
{
  double retval = 0.0;

  if (args.length () == 1)
    {
      octave_value arg = args(0);

      if (arg.is_scalar_type () || arg.is_range ())
	retval = 1.0;
      else if (arg.is_matrix_type ())
	retval = static_cast<double> (arg.rows () >= 1 && arg.columns () >= 1);
    }
  else
    print_usage ("is_matrix");

  return retval;
}

DEFUN (is_struct, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} is_struct (@var{expr})\n\
Return 1 if the value of the expression @var{expr} is a structure.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_map ();
  else
    print_usage ("is_struct");

  return retval;
}

DEFUN (struct_elements, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} struct_elements (@var{struct})\n\
Return a list of strings naming the elements of the structure\n\
@var{struct}.  It is an error to call @code{struct_elements} with an\n\
argument that is not a structure.\n\
@end deftypefn")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      if (args (0).is_map ())
	{
	  Octave_map m = args(0).map_value ();
	  retval(0) = m.make_name_list ();
	}
      else
	gripe_wrong_type_arg ("struct_elements", args (0));
    }
  else
    print_usage ("struct_elements");

  return retval;
}

DEFUN (struct_contains, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} struct_contains (@var{expr}, @var{name})\n\
Return 1 if the expression @var{expr} is a structure and it includes an\n\
element named @var{name}.  The first argument must be a structure and\n\
the second must be a string.\n\
@end deftypefn")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 2)
    {
      retval = 0.0;

      // XXX FIXME XXX -- should this work for all types that can do
      // structure reference operations?

      if (args(0).is_map () && args(1).is_string ())
	{
	  string s = args(1).string_value ();
	  octave_value tmp = args(0).do_struct_elt_index_op (s, true);
	  retval = static_cast<double> (tmp.is_defined ());
	}
      else
	print_usage ("struct_contains");
    }
  else
    print_usage ("struct_contains");

  return retval;
}

static octave_value
fill_matrix (const octave_value_list& args, double val, const char *fcn)
{
  octave_value retval;

  int nargin = args.length ();

  switch (nargin)
    {
    case 0:
      retval = 0.0;
      break;

    case 1:
      {
	int nr, nc;
	get_dimensions (args(0), fcn, nr, nc);

	if (! error_state)
	  retval = Matrix (nr, nc, val);
      }
      break;

    case 2:
      {
	int nr, nc;
	get_dimensions (args(0), args(1), fcn, nr, nc);

	if (! error_state)
	  retval = Matrix (nr, nc, val);
      }
      break;

    default:
      print_usage (fcn);
      break;
    }

  return retval;
}

DEFUN (ones, args, ,
  "ones (N), ones (N, M), ones (X): create a matrix of all ones")
{
  return fill_matrix (args, 1.0, "ones");
}

DEFUN (zeros, args, ,
  "zeros (N), zeros (N, M), zeros (X): create a matrix of all zeros")
{
  return fill_matrix (args, 0.0, "zeros");
}

static Matrix
identity_matrix (int nr, int nc)
{
  Matrix m (nr, nc, 0.0);

  if (nr > 0 && nc > 0)
    {
      int n = MIN (nr, nc);
      for (int i = 0; i < n; i++)
	m (i, i) = 1.0;
    }

  return m;
}

DEFUN (eye, args, ,
  "eye (N), eye (N, M), eye (X): create an identity matrix")
{
  octave_value retval;

  int nargin = args.length ();

  switch (nargin)
    {
    case 0:
      retval = 1.0;
      break;

    case 1:
      {
	int nr, nc;
	get_dimensions (args(0), "eye", nr, nc);

	if (! error_state)
	  retval = identity_matrix (nr, nc);
      }
      break;

    case 2:
      {
	int nr, nc;
	get_dimensions (args(0), args(1), "eye", nr, nc);

	if (! error_state)
	  retval = identity_matrix (nr, nc);
      }
      break;

    default:
      print_usage ("eye");
      break;
    }

  return retval;
}

DEFUN (linspace, args, ,
  "usage: linspace (x1, x2, n)\n\
\n\
Return a vector of n equally spaced points between x1 and x2\n\
inclusive.\n\
\n\
If the final argument is omitted, n = 100 is assumed.\n\
\n\
All three arguments must be scalars.\n\
\n\
See also: logspace")
{
  octave_value_list retval;

  int nargin = args.length ();

  int npoints = 100;

  if (nargin != 2 && nargin != 3)
    {
      print_usage ("linspace");
      return retval;
    }

  if (nargin == 3)
    npoints = args(2).nint_value ();

  if (! error_state)
    {
      octave_value arg_1 = args(0);
      octave_value arg_2 = args(1);

      if (arg_1.is_complex_type () || arg_2.is_complex_type ())
	{
	  Complex x1 = arg_1.complex_value ();
	  Complex x2 = arg_2.complex_value ();

	  if (! error_state)
	    {
	      ComplexRowVector rv = linspace (x1, x2, npoints);

	      if (! error_state)
		retval (0) = octave_value (rv, 0);
	    }
	}
      else
	{
	  double x1 = arg_1.double_value ();
	  double x2 = arg_2.double_value ();

	  if (! error_state)
	    {
	      RowVector rv = linspace (x1, x2, npoints);

	      if (! error_state)
		retval (0) = octave_value (rv, 0);
	    }
	}
    }

  return retval;
}

void
symbols_of_data (void)
{

#define IMAGINARY_DOC_STRING "-*- texinfo -*-\n\
@defvr {Built-in Variable} I\n\
@defvrx {Built-in Variable} J\n\
@defvrx {Built-in Variable} i\n\
@defvrx {Built-in Variable} j\n\
A pure imaginary number, defined as\n\
@iftex\n\
@tex\n\
  $\\sqrt{-1}$.\n\
@end tex\n\
@end iftex\n\
@ifinfo\n\
  @code{sqrt (-1)}.\n\
@end ifinfo\n\
The @code{I} and @code{J} forms are true constants, and cannot be\n\
modified.  The @code{i} and @code{j} forms are like ordinary variables,\n\
and may be used for other purposes.  However, unlike other variables,\n\
they once again assume their special predefined values if they are\n\
cleared @xref{Status of Variables}.\n\
@end defvr"

#define INFINITY_DOC_STRING "-*- texinfo -*-\n\
@defvr {Built-in Variable} Inf\n\
@defvrx {Built-in Variable} inf\n\
Infinity.  This is the result of an operation like 1/0, or an operation\n\
that results in a floating point overflow.\n\
@end defvr"

#define NAN_DOC_STRING "-*- texinfo -*-\n\
@defvr {Built-in Variable} NaN\n\
@defvrx {Built-in Variable} nan\n\
Not a number.  This is the result of an operation like\n\
@iftex\n\
@tex\n\
$0/0$, or $\\infty - \\infty$,\n\
@end tex\n\
@end iftex\n\
@ifinfo\n\
0/0, or @samp{Inf - Inf},\n\
@end ifinfo\n\
or any operation with a NaN.\n\
\n\
Note that NaN always compares not equal to NaN.  This behavior is\n\
specified by the IEEE standard for floating point arithmetic.  To\n\
find NaN values, you must use the @code{isnan} function.\n\
@end defvr"

  DEFCONST (I, Complex (0.0, 1.0),
    IMAGINARY_DOC_STRING);

  DEFCONST (Inf, octave_Inf,
    INFINITY_DOC_STRING);

  DEFCONST (J, Complex (0.0, 1.0),
    IMAGINARY_DOC_STRING);

  DEFCONST (NaN, octave_NaN,
    NAN_DOC_STRING);

#if defined (M_E)
  double e_val = M_E;
#else
  double e_val = exp (1.0);
#endif

  DEFCONST (e, e_val,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} e\n\
The base of natural logarithms.  The constant\n\
@iftex\n\
@tex\n\
 $e$\n\
@end tex\n\
@end iftex\n\
@ifinfo\n\
 @var{e}\n\
@end ifinfo\n\
 satisfies the equation\n\
@iftex\n\
@tex\n\
 $\\log (e) = 1$.\n\
@end tex\n\
@end iftex\n\
@ifinfo\n\
 @code{log} (@var{e}) = 1.\n\
@end ifinfo\n\
@end defvr");

  DEFCONST (eps, DBL_EPSILON,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} eps\n\
The machine precision.  More precisely, @code{eps} is the largest\n\
relative spacing between any two adjacent numbers in the machine's\n\
floating point system.  This number is obviously system-dependent.  On\n\
machines that support 64 bit IEEE floating point arithmetic, @code{eps}\n\
is approximately\n\
@ifinfo\n\
 2.2204e-16.\n\
@end ifinfo\n\
@iftex\n\
@tex\n\
 $2.2204\\times10^{-16}$.\n\
@end tex\n\
@end iftex\n\
@end defvr");

  DEFCONST (false, false,
    "logical false value");

  DEFCONST (i, Complex (0.0, 1.0),
    IMAGINARY_DOC_STRING);

  DEFCONST (inf, octave_Inf,
    INFINITY_DOC_STRING);

  DEFCONST (j, Complex (0.0, 1.0),
    IMAGINARY_DOC_STRING);

  DEFCONST (nan, octave_NaN,
    NAN_DOC_STRING);

#if defined (M_PI)
  double pi_val = M_PI;
#else
  double pi_val = 4.0 * atan (1.0);
#endif

  DEFCONST (pi, pi_val,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} pi\n\
The ratio of the circumference of a circle to its diameter.\n\
Internally, @code{pi} is computed as @samp{4.0 * atan (1.0)}.\n\
@end defvr");

  DEFCONST (realmax, DBL_MAX,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} realmax\n\
The largest floating point number that is representable.  The actual\n\
value is system-dependent.  On machines that support 64 bit IEEE\n\
floating point arithmetic, @code{realmax} is approximately\n\
@ifinfo\n\
 1.7977e+308\n\
@end ifinfo\n\
@iftex\n\
@tex\n\
 $1.7977\\times10^{308}$.\n\
@end tex\n\
@end iftex\n\
@end defvr");

  DEFCONST (realmin, DBL_MIN,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} realmin\n\
The smallest floating point number that is representable.  The actual\n\
value is system-dependent.  On machines that support 64 bit IEEE\n\
floating point arithmetic, @code{realmin} is approximately\n\
@ifinfo\n\
 2.2251e-308\n\
@end ifinfo\n\
@iftex\n\
@tex\n\
 $2.2251\\times10^{-308}$.\n\
@end tex\n\
@end iftex\n\
@end defvr");

  DEFCONST (true, true,
    "logical true value");

}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
