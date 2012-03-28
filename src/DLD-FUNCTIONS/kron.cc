/*

Copyright (C) 2002-2012 John W. Eaton

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

// Author: Paul Kienzle <pkienzle@users.sf.net>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "dMatrix.h"
#include "fMatrix.h"
#include "CMatrix.h"
#include "fCMatrix.h"

#include "dSparse.h"
#include "CSparse.h"

#include "dDiagMatrix.h"
#include "fDiagMatrix.h"
#include "CDiagMatrix.h"
#include "fCDiagMatrix.h"

#include "PermMatrix.h"

#include "mx-inlines.cc"
#include "quit.h"

#include "defun-dld.h"
#include "error.h"
#include "oct-obj.h"

template <class R, class T>
static MArray<T>
kron (const MArray<R>& a, const MArray<T>& b)
{
  assert (a.ndims () == 2);
  assert (b.ndims () == 2);

  octave_idx_type nra = a.rows (), nrb = b.rows ();
  octave_idx_type nca = a.cols (), ncb = b.cols ();

  MArray<T> c (dim_vector (nra*nrb, nca*ncb));
  T *cv = c.fortran_vec ();

  for (octave_idx_type ja = 0; ja < nca; ja++)
    for (octave_idx_type jb = 0; jb < ncb; jb++)
      for (octave_idx_type ia = 0; ia < nra; ia++)
        {
          octave_quit ();
          mx_inline_mul (nrb, cv, a(ia, ja), b.data () + nrb*jb);
          cv += nrb;
        }

  return c;
}

template <class R, class T>
static MArray<T>
kron (const MDiagArray2<R>& a, const MArray<T>& b)
{
  assert (b.ndims () == 2);

  octave_idx_type nra = a.rows (), nrb = b.rows (), dla = a.diag_length ();
  octave_idx_type nca = a.cols (), ncb = b.cols ();

  MArray<T> c (dim_vector (nra*nrb, nca*ncb), T());

  for (octave_idx_type ja = 0; ja < dla; ja++)
    for (octave_idx_type jb = 0; jb < ncb; jb++)
      {
        octave_quit ();
        mx_inline_mul (nrb, &c.xelem(ja*nrb, ja*ncb + jb), a.dgelem (ja), b.data () + nrb*jb);
      }

  return c;
}

template <class T>
static MSparse<T>
kron (const MSparse<T>& A, const MSparse<T>& B)
{
  octave_idx_type idx = 0;
  MSparse<T> C (A.rows () * B.rows (), A.columns () * B.columns (),
                A.nnz () * B.nnz ());

  C.cidx (0) = 0;

  for (octave_idx_type Aj = 0; Aj < A.columns (); Aj++)
    for (octave_idx_type Bj = 0; Bj < B.columns (); Bj++)
      {
        octave_quit ();
        for (octave_idx_type Ai = A.cidx (Aj); Ai < A.cidx (Aj+1); Ai++)
          {
            octave_idx_type Ci = A.ridx(Ai) * B.rows ();
            const T v = A.data (Ai);

            for (octave_idx_type Bi = B.cidx (Bj); Bi < B.cidx (Bj+1); Bi++)
              {
                C.data (idx) = v * B.data (Bi);
                C.ridx (idx++) = Ci + B.ridx (Bi);
              }
          }
        C.cidx (Aj * B.columns () + Bj + 1) = idx;
      }

  return C;
}

static PermMatrix
kron (const PermMatrix& a, const PermMatrix& b)
{
  octave_idx_type na = a.rows (), nb = b.rows ();
  const octave_idx_type *pa = a.data (), *pb = b.data ();
  PermMatrix c(na*nb); // Row permutation.
  octave_idx_type *pc = c.fortran_vec ();

  bool cola = a.is_col_perm (), colb = b.is_col_perm ();
  if (cola && colb)
    {
      for (octave_idx_type i = 0; i < na; i++)
        for (octave_idx_type j = 0; j < nb; j++)
          pc[pa[i]*nb+pb[j]] = i*nb+j;
    }
  else if (cola)
    {
      for (octave_idx_type i = 0; i < na; i++)
        for (octave_idx_type j = 0; j < nb; j++)
          pc[pa[i]*nb+j] = i*nb+pb[j];
    }
  else if (colb)
    {
      for (octave_idx_type i = 0; i < na; i++)
        for (octave_idx_type j = 0; j < nb; j++)
          pc[i*nb+pb[j]] = pa[i]*nb+j;
    }
  else
    {
      for (octave_idx_type i = 0; i < na; i++)
        for (octave_idx_type j = 0; j < nb; j++)
          pc[i*nb+j] = pa[i]*nb+pb[j];
    }

  return c;
}


template <class MTA, class MTB>
octave_value
do_kron (const octave_value& a, const octave_value& b)
{
  MTA am = octave_value_extract<MTA> (a);
  MTB bm = octave_value_extract<MTB> (b);
  return octave_value (kron (am, bm));
}

octave_value
dispatch_kron (const octave_value& a, const octave_value& b)
{
  octave_value retval;
  if (a.is_perm_matrix () && b.is_perm_matrix ())
    retval = do_kron<PermMatrix, PermMatrix> (a, b);
  else if (a.is_diag_matrix ())
    {
      if (b.is_diag_matrix () && a.rows () == a.columns ()
          && b.rows () == b.columns ())
        {
          octave_value_list tmp;
          tmp(0) = a.diag ();
          tmp(1) = b.diag ();
          tmp = dispatch_kron (tmp, 1);
          if (tmp.length () == 1)
            retval = tmp(0).diag ();
        }
      else if (a.is_single_type () || b.is_single_type ())
        {
          if (a.is_complex_type ())
            retval = do_kron<FloatComplexDiagMatrix, FloatComplexMatrix> (a, b);
          else if (b.is_complex_type ())
            retval = do_kron<FloatDiagMatrix, FloatComplexMatrix> (a, b);
          else
            retval = do_kron<FloatDiagMatrix, FloatMatrix> (a, b);
        }
      else
        {
          if (a.is_complex_type ())
            retval = do_kron<ComplexDiagMatrix, ComplexMatrix> (a, b);
          else if (b.is_complex_type ())
            retval = do_kron<DiagMatrix, ComplexMatrix> (a, b);
          else
            retval = do_kron<DiagMatrix, Matrix> (a, b);
        }
    }
  else if (a.is_sparse_type () || b.is_sparse_type ())
    {
      if (a.is_complex_type () || b.is_complex_type ())
        retval = do_kron<SparseComplexMatrix, SparseComplexMatrix> (a, b);
      else
        retval = do_kron<SparseMatrix, SparseMatrix> (a, b);
    }
  else if (a.is_single_type () || b.is_single_type ())
    {
      if (a.is_complex_type ())
        retval = do_kron<FloatComplexMatrix, FloatComplexMatrix> (a, b);
      else if (b.is_complex_type ())
        retval = do_kron<FloatMatrix, FloatComplexMatrix> (a, b);
      else
        retval = do_kron<FloatMatrix, FloatMatrix> (a, b);
    }
  else
    {
      if (a.is_complex_type ())
        retval = do_kron<ComplexMatrix, ComplexMatrix> (a, b);
      else if (b.is_complex_type ())
        retval = do_kron<Matrix, ComplexMatrix> (a, b);
      else
        retval = do_kron<Matrix, Matrix> (a, b);
    }
  return retval;
}


DEFUN_DLD (kron, args, , "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {} kron (@var{A}, @var{B})\n\
@deftypefnx {Loadable Function} {} kron (@var{A1}, @var{A2}, @dots{})\n\
Form the Kronecker product of two or more matrices, defined block by \n\
block as\n\
\n\
@example\n\
x = [ a(i,j)*b ]\n\
@end example\n\
\n\
For example:\n\
\n\
@example\n\
@group\n\
kron (1:4, ones (3, 1))\n\
     @result{}  1  2  3  4\n\
         1  2  3  4\n\
         1  2  3  4\n\
@end group\n\
@end example\n\
\n\
If there are more than two input arguments @var{A1}, @var{A2}, @dots{}, \n\
@var{An} the Kronecker product is computed as\n\
\n\
@example\n\
kron (kron (@var{A1}, @var{A2}), @dots{}, @var{An})\n\
@end example\n\
\n\
@noindent\n\
Since the Kronecker product is associative, this is well-defined.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin >= 2)
    {
      octave_value a = args(0), b = args(1);
      retval = dispatch_kron (a, b);
      for (octave_idx_type i = 2; i < nargin; i++)
        retval = dispatch_kron (retval, args(i));
    }
  else
    print_usage ();

  return retval;
}


/*
%!test
%! x = ones (2);
%! assert (kron (x, x), ones (4));

%!shared x, y, z
%! x =  [1, 2];
%! y =  [-1, -2];
%! z =  [1,  2,  3,  4; 1,  2,  3,  4; 1,  2,  3,  4];
%!assert (kron (1:4, ones (3, 1)), z)
%!assert (kron (x, y, z), kron (kron (x, y), z))
%!assert (kron (x, y, z), kron (x, kron (y, z)))
*/
