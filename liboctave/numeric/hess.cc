/*

Copyright (C) 1994-2016 John W. Eaton

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "CMatrix.h"
#include "dMatrix.h"
#include "fCMatrix.h"
#include "fMatrix.h"
#include "hess.h"
#include "lo-error.h"
#include "lo-lapack-proto.h"

namespace octave
{
namespace math
{

template <>
octave_idx_type
hess<Matrix>::init (const Matrix& a)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (a_nr != a_nc)
    (*current_liboctave_error_handler) ("hess: requires square matrix");

  char job = 'N';
  char side = 'R';

  octave_idx_type n = a_nc;
  octave_idx_type lwork = 32 * n;
  octave_idx_type info;
  octave_idx_type ilo;
  octave_idx_type ihi;

  hess_mat = a;
  double *h = hess_mat.fortran_vec ();

  Array<double> scale (dim_vector (n, 1));
  double *pscale = scale.fortran_vec ();

  F77_XFCN (dgebal, DGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1),
                             n, h, n, ilo, ihi, pscale, info
                             F77_CHAR_ARG_LEN (1)));

  Array<double> tau (dim_vector (n-1, 1));
  double *ptau = tau.fortran_vec ();

  Array<double> work (dim_vector (lwork, 1));
  double *pwork = work.fortran_vec ();

  F77_XFCN (dgehrd, DGEHRD, (n, ilo, ihi, h, n, ptau, pwork,
                             lwork, info));

  unitary_hess_mat = hess_mat;
  double *z = unitary_hess_mat.fortran_vec ();

  F77_XFCN (dorghr, DORGHR, (n, ilo, ihi, z, n, ptau, pwork,
                             lwork, info));

  F77_XFCN (dgebak, DGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                             F77_CONST_CHAR_ARG2 (&side, 1),
                             n, ilo, ihi, pscale, n, z,
                             n, info
                             F77_CHAR_ARG_LEN (1)
                             F77_CHAR_ARG_LEN (1)));

  // If someone thinks of a more graceful way of doing
  // this (or faster for that matter :-)), please let
  // me know!

  if (n > 2)
    for (octave_idx_type j = 0; j < a_nc; j++)
      for (octave_idx_type i = j+2; i < a_nr; i++)
        hess_mat.elem (i, j) = 0;

  return info;
}

template <>
octave_idx_type
hess<FloatMatrix>::init (const FloatMatrix& a)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (a_nr != a_nc)
    (*current_liboctave_error_handler) ("hess: requires square matrix");

  char job = 'N';
  char side = 'R';

  octave_idx_type n = a_nc;
  octave_idx_type lwork = 32 * n;
  octave_idx_type info;
  octave_idx_type ilo;
  octave_idx_type ihi;

  hess_mat = a;
  float *h = hess_mat.fortran_vec ();

  Array<float> scale (dim_vector (n, 1));
  float *pscale = scale.fortran_vec ();

  F77_XFCN (sgebal, SGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1),
                             n, h, n, ilo, ihi, pscale, info
                             F77_CHAR_ARG_LEN (1)));

  Array<float> tau (dim_vector (n-1, 1));
  float *ptau = tau.fortran_vec ();

  Array<float> work (dim_vector (lwork, 1));
  float *pwork = work.fortran_vec ();

  F77_XFCN (sgehrd, SGEHRD, (n, ilo, ihi, h, n, ptau, pwork,
                             lwork, info));

  unitary_hess_mat = hess_mat;
  float *z = unitary_hess_mat.fortran_vec ();

  F77_XFCN (sorghr, SORGHR, (n, ilo, ihi, z, n, ptau, pwork,
                             lwork, info));

  F77_XFCN (sgebak, SGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                             F77_CONST_CHAR_ARG2 (&side, 1),
                             n, ilo, ihi, pscale, n, z,
                             n, info
                             F77_CHAR_ARG_LEN (1)
                             F77_CHAR_ARG_LEN (1)));

  // If someone thinks of a more graceful way of doing
  // this (or faster for that matter :-)), please let
  // me know!

  if (n > 2)
    for (octave_idx_type j = 0; j < a_nc; j++)
      for (octave_idx_type i = j+2; i < a_nr; i++)
        hess_mat.elem (i, j) = 0;

  return info;
}

template <>
octave_idx_type
hess<ComplexMatrix>::init (const ComplexMatrix& a)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (a_nr != a_nc)
    (*current_liboctave_error_handler) ("hess: requires square matrix");

  char job = 'N';
  char side = 'R';

  octave_idx_type n = a_nc;
  octave_idx_type lwork = 32 * n;
  octave_idx_type info;
  octave_idx_type ilo;
  octave_idx_type ihi;

  hess_mat = a;
  Complex *h = hess_mat.fortran_vec ();

  Array<double> scale (dim_vector (n, 1));
  double *pscale = scale.fortran_vec ();

  F77_XFCN (zgebal, ZGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1),
                             n, F77_DBLE_CMPLX_ARG (h), n, ilo, ihi, pscale, info
                             F77_CHAR_ARG_LEN (1)));

  Array<Complex> tau (dim_vector (n-1, 1));
  Complex *ptau = tau.fortran_vec ();

  Array<Complex> work (dim_vector (lwork, 1));
  Complex *pwork = work.fortran_vec ();

  F77_XFCN (zgehrd, ZGEHRD, (n, ilo, ihi, F77_DBLE_CMPLX_ARG (h), n, F77_DBLE_CMPLX_ARG (ptau), F77_DBLE_CMPLX_ARG (pwork), lwork, info));

  unitary_hess_mat = hess_mat;
  Complex *z = unitary_hess_mat.fortran_vec ();

  F77_XFCN (zunghr, ZUNGHR, (n, ilo, ihi, F77_DBLE_CMPLX_ARG (z), n, F77_DBLE_CMPLX_ARG (ptau), F77_DBLE_CMPLX_ARG (pwork),
                             lwork, info));

  F77_XFCN (zgebak, ZGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                             F77_CONST_CHAR_ARG2 (&side, 1),
                             n, ilo, ihi, pscale, n, F77_DBLE_CMPLX_ARG (z), n, info
                             F77_CHAR_ARG_LEN (1)
                             F77_CHAR_ARG_LEN (1)));

  // If someone thinks of a more graceful way of
  // doing this (or faster for that matter :-)),
  // please let me know!

  if (n > 2)
    for (octave_idx_type j = 0; j < a_nc; j++)
      for (octave_idx_type i = j+2; i < a_nr; i++)
        hess_mat.elem (i, j) = 0;

  return info;
}

template <>
octave_idx_type
hess<FloatComplexMatrix>::init (const FloatComplexMatrix& a)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (a_nr != a_nc)
    {
      (*current_liboctave_error_handler) ("hess: requires square matrix");
      return -1;
    }

  char job = 'N';
  char side = 'R';

  octave_idx_type n = a_nc;
  octave_idx_type lwork = 32 * n;
  octave_idx_type info;
  octave_idx_type ilo;
  octave_idx_type ihi;

  hess_mat = a;
  FloatComplex *h = hess_mat.fortran_vec ();

  Array<float> scale (dim_vector (n, 1));
  float *pscale = scale.fortran_vec ();

  F77_XFCN (cgebal, CGEBAL, (F77_CONST_CHAR_ARG2 (&job, 1),
                             n, F77_CMPLX_ARG (h), n, ilo, ihi, pscale, info
                             F77_CHAR_ARG_LEN (1)));

  Array<FloatComplex> tau (dim_vector (n-1, 1));
  FloatComplex *ptau = tau.fortran_vec ();

  Array<FloatComplex> work (dim_vector (lwork, 1));
  FloatComplex *pwork = work.fortran_vec ();

  F77_XFCN (cgehrd, CGEHRD, (n, ilo, ihi, F77_CMPLX_ARG (h), n, F77_CMPLX_ARG (ptau), F77_CMPLX_ARG (pwork), lwork, info));

  unitary_hess_mat = hess_mat;
  FloatComplex *z = unitary_hess_mat.fortran_vec ();

  F77_XFCN (cunghr, CUNGHR, (n, ilo, ihi, F77_CMPLX_ARG (z), n, F77_CMPLX_ARG (ptau), F77_CMPLX_ARG (pwork),
                             lwork, info));

  F77_XFCN (cgebak, CGEBAK, (F77_CONST_CHAR_ARG2 (&job, 1),
                             F77_CONST_CHAR_ARG2 (&side, 1),
                             n, ilo, ihi, pscale, n, F77_CMPLX_ARG (z), n, info
                             F77_CHAR_ARG_LEN (1)
                             F77_CHAR_ARG_LEN (1)));

  // If someone thinks of a more graceful way of
  // doing this (or faster for that matter :-)),
  // please let me know!

  if (n > 2)
    for (octave_idx_type j = 0; j < a_nc; j++)
      for (octave_idx_type i = j+2; i < a_nr; i++)
        hess_mat.elem (i, j) = 0;

  return info;
}

}
}
