// Template array classes
/*

Copyright (C) 2000 John W. Eaton

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

#include <cassert>

#include <iostream>

#include "ArrayN.h"
#include "ArrayN-inline.h"
#include "ArrayN-idx.h"
#include "idx-vector.h"
#include "lo-error.h"

// N-dimensional array class.

template <class T>
std::ostream&
operator << (std::ostream& os, const ArrayN<T>& a)
{
  dim_vector a_dims = a.dims ();

  int n_dims = a_dims.length ();

  os << n_dims << "-dimensional array";

  if (n_dims)
    {
      os << " (";

      for (int i = 0; i < n_dims - 1; i++)
	os << a_dims(i) << "x";

      os << a_dims(n_dims-1) << ")";
    }

  os <<"\n\n";

  if (n_dims)
    {
      os << "data:";

      Array<int> ra_idx (n_dims, 0);

      // Number of times the first 2d-array is to be displayed.

      int m = 1;
      for (int i = 2; i < n_dims; i++)
	m *= a_dims(i);

      if (m == 1)
        {
          int rows = 0;
          int cols = 0;

          switch (n_dims)
            {
	    case 2:
	      rows = a_dims(0);
	      cols = a_dims(1);

	      for (int j = 0; j < rows; j++)
		{
		  ra_idx(0) = j;
		  for (int k = 0; k < cols; k++)
		    {
		      ra_idx(1) = k;
		      os << " " << a.elem(ra_idx);
		    }
		  os << "\n";
		}
	      break;

	    default:
	      rows = a_dims(0);

	      for (int k = 0; k < rows; k++)
		{
		  ra_idx(0) = k;
		  os << " " << a.elem(ra_idx);
		}
	      break;
	    }

          os << "\n";
        }
      else
        {
          int rows = a_dims(0);
          int cols = a_dims(1);

          for (int i = 0; i < m; i++)
            {
              os << "\n(:,:,";

              for (int j = 2; j < n_dims - 1; j++)
		os << ra_idx(j) + 1 << ",";

	      os << ra_idx(n_dims - 1) + 1 << ") = \n";

	      for (int j = 0; j < rows; j++)
	        {
	          ra_idx(0) = j;

	          for (int k = 0; k < cols; k++)
	            {
		      ra_idx(1) = k;
		      os << " " << a.elem(ra_idx);
		    }

	          os << "\n";
	        }

	      os << "\n";

	      if (i != m - 1)
		increment_index (ra_idx, a_dims, 2);
            }
        }
    }

  return os;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
