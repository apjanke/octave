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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "dbleCHOL.h"
#include "f77-fcn.h"
#include "lo-error.h"

extern "C"
{
  F77_RET_T
  F77_FUNC (dpotrf, DPOTRF) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&,
			     double*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);
}

octave_idx_type
CHOL::init (const Matrix& a)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (a_nr != a_nc)
    {
      (*current_liboctave_error_handler) ("CHOL requires square matrix");
      return -1;
    }

  octave_idx_type n = a_nc;
  octave_idx_type info;

  chol_mat = a;
  double *h = chol_mat.fortran_vec ();

  F77_XFCN (dpotrf, DPOTRF, (F77_CONST_CHAR_ARG2 ("U", 1),
			     n, h, n, info
			     F77_CHAR_ARG_LEN (1)));

  if (f77_exception_encountered)
    (*current_liboctave_error_handler) ("unrecoverable error in dpotrf");
  else
    {
      // If someone thinks of a more graceful way of doing this (or
      // faster for that matter :-)), please let me know!

      if (n > 1)
	for (octave_idx_type j = 0; j < a_nc; j++)
	  for (octave_idx_type i = j+1; i < a_nr; i++)
	    chol_mat.elem (i, j) = 0.0;
    }

  return info;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
