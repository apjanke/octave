// N-D Array  manipulations.
/*

Copyright (C) 2003 John W. Eaton

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

#include "chNDArray.h"
#include "mx-base.h"
#include "lo-ieee.h"

#include "ArrayN-inline.h"

// XXX FIXME XXX -- this is not quite the right thing.

boolMatrix
charNDArray::all (int dim) const
{
  boolMatrix retval;

  if (dimensions.length () == 2)
    {
      charMatrix tmp = matrix_value ();
      retval = tmp.all (dim);
    }
  else
    (*current_liboctave_error_handler)
      ("all is not yet implemented for N-d Arrays");

  return retval;
}

boolMatrix
charNDArray::any (int dim) const
{
  boolMatrix retval;

  if (dimensions.length () == 2)
    {
      charMatrix tmp = matrix_value ();
      retval = tmp.any (dim);
    }
  else
    (*current_liboctave_error_handler)
      ("any is not yet implemented for N-d Arrays");

  return retval;
}

charMatrix
charNDArray::matrix_value (void) const
{
  charMatrix retval;

  int nd = ndims ();

  switch (nd)
    {
    case 1:
      retval = charMatrix (Array2<char> (*this, dimensions(0), 1));
      break;

    case 2:
      retval = charMatrix (Array2<char> (*this, dimensions(0),
					       dimensions(1)));
      break;

    default:
      (*current_liboctave_error_handler)
	("invalid converstion of charNDArray to charMatrix");
      break;
    }

  return retval;
}

void
charNDArray::increment_index (Array<int>& ra_idx,
			      const dim_vector& dimensions,
			      int start_dimension)
{
  ::increment_index (ra_idx, dimensions, start_dimension);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
