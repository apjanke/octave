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

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <climits>

#include <iostream>

#include "lo-ieee.h"
#include "lo-utils.h"
#include "mx-base.h"
#include "quit.h"

#include "gripes.h"
#include "oct-obj.h"
#include "oct-lvalue.h"
#include "ops.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-base-mat.cc"
#include "ov-scalar.h"
#include "ov-re-mat.h"
#include "pr-output.h"
#include "variables.h"

#if ! defined (UCHAR_MAX)
#define UCHAR_MAX 255
#endif

template class octave_base_matrix<NDArray>;

DEFINE_OCTAVE_ALLOCATOR (octave_matrix);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_matrix, "matrix", "double");

octave_value *
octave_matrix::try_narrowing_conversion (void)
{
  octave_value *retval = 0;

  if (matrix.nelem () == 1)
    retval = new octave_scalar (matrix (0));

  return retval;
}

bool
octave_matrix::valid_as_scalar_index (void) const
{
  // XXX FIXME XXX
  return false;
}

double
octave_matrix::double_value (bool) const
{
  double retval = lo_ieee_nan_value ();

  // XXX FIXME XXX -- maybe this should be a function, valid_as_scalar()
  if (rows () > 0 && columns () > 0)
    {
      // XXX FIXME XXX -- is warn_fortran_indexing the right variable here?
      if (Vwarn_fortran_indexing)
	gripe_implicit_conversion ("real matrix", "real scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion ("real matrix", "real scalar");

  return retval;
}

// XXX FIXME XXX

Matrix
octave_matrix::matrix_value (bool) const
{
  return matrix.matrix_value ();
}

Complex
octave_matrix::complex_value (bool) const
{
  double tmp = lo_ieee_nan_value ();

  Complex retval (tmp, tmp);

  // XXX FIXME XXX -- maybe this should be a function, valid_as_scalar()
  if (rows () > 0 && columns () > 0)
    {
      // XXX FIXME XXX -- is warn_fortran_indexing the right variable here?
      if (Vwarn_fortran_indexing)
	gripe_implicit_conversion ("real matrix", "complex scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion ("real matrix", "complex scalar");

  return retval;
}

// XXX FIXME XXX

ComplexMatrix
octave_matrix::complex_matrix_value (bool) const
{
  return ComplexMatrix (matrix.matrix_value ());
}

octave_value
octave_matrix::convert_to_str_internal (bool, bool) const
{
  octave_value retval;

  int nr = matrix.rows ();
  int nc = matrix.columns ();

  if (nr == 0 && nc == 0)
    {
      char s = '\0';
      retval = octave_value (&s);
    }
  else
    {
      if (nr == 0 || nc == 0)
	{
	  char s = '\0';
	  retval = octave_value (&s);
	}
      else
	{
	  charMatrix chm (nr, nc);
	  
	  bool warned = false;

	  for (int j = 0; j < nc; j++)
	    {
	      for (int i = 0; i < nr; i++)
		{
		  OCTAVE_QUIT;

		  double d = matrix (i, j);

		  if (xisnan (d))
		    {
		      ::error ("invalid conversion from NaN to character");
		      return retval;
		    }
		  else
		    {
		      int ival = NINT (d);

		      if (ival < 0 || ival > UCHAR_MAX)
			{
			  // XXX FIXME XXX -- is there something
			  // better we could do?

			  ival = 0;

			  if (! warned)
			    {
			      ::warning ("range error for conversion to character value");
			      warned = true;
			    }
			}

		      chm (i, j) = static_cast<char> (ival);
		    }
		}
	    }

	  retval = octave_value (chm, 1);
	}
    }

  return retval;
}

void
octave_matrix::print_raw (std::ostream& os,
			  bool pr_as_read_syntax) const
{
  octave_print_internal (os, matrix, pr_as_read_syntax,
			 current_print_indent_level ());
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
