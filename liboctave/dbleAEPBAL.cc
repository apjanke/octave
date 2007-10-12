/*

Copyright (C) 1996, 1997 John W. Eaton

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

#include "dbleAEPBAL.h"
#include "f77-fcn.h"

extern "C"
{
  F77_RET_T
  F77_FUNC (dgebal, DGEBAL) (F77_CONST_CHAR_ARG_DECL,
			     const octave_idx_type&, double*, const octave_idx_type&, octave_idx_type&,
			     octave_idx_type&, double*, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (dgebak, DGEBAK) (F77_CONST_CHAR_ARG_DECL,
			     F77_CONST_CHAR_ARG_DECL,
			     const octave_idx_type&, const octave_idx_type&, const octave_idx_type&, double*,
			     const octave_idx_type&, double*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL
			     F77_CHAR_ARG_LEN_DECL);
}

octave_idx_type
AEPBALANCE::init (const Matrix& a, const std::string& balance_job)
{
  octave_idx_type n = a.cols ();

  if (a.rows () != n)
    {
      (*current_liboctave_error_handler) ("AEPBALANCE requires square matrix");
      return -1;
    }

  octave_idx_type info;
  octave_idx_type ilo;
  octave_idx_type ihi;

  Array<double> scale (n);
  double *pscale = scale.fortran_vec ();

  balanced_mat = a;
  double *p_balanced_mat = balanced_mat.fortran_vec ();

  char job = balance_job[0];

  F77_XFCN (dgebal, DGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1),
			     n, p_balanced_mat, n, ilo, ihi, pscale, info
			     F77_CHAR_ARG_LEN (1)));

  if (f77_exception_encountered)
    (*current_liboctave_error_handler) ("unrecoverable error in dgebal");
  else
    {
      balancing_mat = Matrix (n, n, 0.0);
      for (octave_idx_type i = 0; i < n; i++)
	balancing_mat.elem (i ,i) = 1.0;

      double *p_balancing_mat = balancing_mat.fortran_vec ();

      char side = 'R';

      F77_XFCN (dgebak, DGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
				 F77_CONST_CHAR_ARG2 (&side, 1),
				 n, ilo, ihi, pscale, n,
				 p_balancing_mat, n, info
				 F77_CHAR_ARG_LEN (1)
				 F77_CHAR_ARG_LEN (1)));

      if (f77_exception_encountered)
	(*current_liboctave_error_handler) ("unrecoverable error in dgebak");
    }

  return info;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
