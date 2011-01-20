/*

Copyright (C) 2004-2011 David Bateman
Copyright (C) 1998-2004 Andy Adler

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

#include "gripes.h"
#include "oct-obj.h"
#include "ov.h"
#include "ov-typeinfo.h"
#include "ov-cx-mat.h"
#include "ov-complex.h"
#include "ops.h"
#include "xpow.h"

#include "sparse-xpow.h"
#include "sparse-xdiv.h"
#include "ov-cx-sparse.h"

// sparse complex matrix by complex scalar ops.

DEFBINOP_OP (add, sparse_complex_matrix, complex, +)
DEFBINOP_OP (sub, sparse_complex_matrix, complex, -)
DEFBINOP_OP (mul, sparse_complex_matrix, complex, *)

DEFBINOP (div, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (const octave_sparse_complex_matrix&,
                   const octave_complex&);

  Complex d = v2.complex_value ();
  octave_value retval;

  if (d == 0.0)
    gripe_divide_by_zero ();

  retval = octave_value (v1.sparse_complex_matrix_value () / d);

  return retval;
}

DEFBINOP (pow, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (const octave_sparse_complex_matrix&,
                   const octave_complex&);
  return xpow (v1.complex_matrix_value (), v2.complex_value ());
}

DEFBINOP (ldiv, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (const octave_sparse_complex_matrix&, const octave_complex&);

  if (v1.rows() == 1 && v1.columns() == 1)
    {
      Complex d = v1.complex_value ();

      if (d == 0.0)
        gripe_divide_by_zero ();

      return octave_value (SparseComplexMatrix (1, 1, v2.complex_value () / d));
    }
  else
    {
      MatrixType typ = v1.matrix_type ();
      SparseComplexMatrix m1 = v1.sparse_complex_matrix_value ();
      ComplexMatrix m2 = ComplexMatrix (1, 1, v2.complex_value ());
      ComplexMatrix ret = xleftdiv (m1, m2, typ);
      v1.matrix_type (typ);
      return ret;
    }
}

DEFBINOP_FN (lt, sparse_complex_matrix, complex, mx_el_lt)
DEFBINOP_FN (le, sparse_complex_matrix, complex, mx_el_le)
DEFBINOP_FN (eq, sparse_complex_matrix, complex, mx_el_eq)
DEFBINOP_FN (ge, sparse_complex_matrix, complex, mx_el_ge)
DEFBINOP_FN (gt, sparse_complex_matrix, complex, mx_el_gt)
DEFBINOP_FN (ne, sparse_complex_matrix, complex, mx_el_ne)

DEFBINOP_OP (el_mul, sparse_complex_matrix, complex, *)

DEFBINOP (el_div, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (const octave_sparse_complex_matrix&,
                   const octave_complex&);

  octave_value retval;

  Complex d = v2.complex_value ();

  if (d == 0.0)
    gripe_divide_by_zero ();

  retval =  octave_value (v1.sparse_complex_matrix_value () / d);

  return retval;
}

DEFBINOP_FN (el_pow, sparse_complex_matrix, complex, elem_xpow)

DEFBINOP (el_ldiv, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (const octave_sparse_complex_matrix&,
                   const octave_complex&);

  return octave_value
    (x_el_div (v2.complex_value (), v1.sparse_complex_matrix_value ()));
}

DEFBINOP_FN (el_and, sparse_complex_matrix, complex, mx_el_and)
DEFBINOP_FN (el_or,  sparse_complex_matrix, complex, mx_el_or)

DEFCATOP (scm_cs, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (octave_sparse_complex_matrix&, const octave_complex&);
  SparseComplexMatrix tmp (1, 1, v2.complex_value ());
  return octave_value
    (v1.sparse_complex_matrix_value (). concat (tmp, ra_idx));
}

DEFASSIGNOP (assign, sparse_complex_matrix, complex)
{
  CAST_BINOP_ARGS (octave_sparse_complex_matrix&, const octave_complex&);

  SparseComplexMatrix tmp (1, 1, v2.complex_value ());
  v1.assign (idx, tmp);
  return octave_value ();
}

void
install_scm_cs_ops (void)
{
  INSTALL_BINOP (op_add, octave_sparse_complex_matrix, octave_complex, add);
  INSTALL_BINOP (op_sub, octave_sparse_complex_matrix, octave_complex, sub);
  INSTALL_BINOP (op_mul, octave_sparse_complex_matrix, octave_complex, mul);
  INSTALL_BINOP (op_div, octave_sparse_complex_matrix, octave_complex, div);
  INSTALL_BINOP (op_pow, octave_sparse_complex_matrix, octave_complex, pow);
  INSTALL_BINOP (op_ldiv, octave_sparse_complex_matrix, octave_complex,
                 ldiv);
  INSTALL_BINOP (op_lt, octave_sparse_complex_matrix, octave_complex, lt);
  INSTALL_BINOP (op_le, octave_sparse_complex_matrix, octave_complex, le);
  INSTALL_BINOP (op_eq, octave_sparse_complex_matrix, octave_complex, eq);
  INSTALL_BINOP (op_ge, octave_sparse_complex_matrix, octave_complex, ge);
  INSTALL_BINOP (op_gt, octave_sparse_complex_matrix, octave_complex, gt);
  INSTALL_BINOP (op_ne, octave_sparse_complex_matrix, octave_complex, ne);
  INSTALL_BINOP (op_el_mul, octave_sparse_complex_matrix, octave_complex,
                 el_mul);
  INSTALL_BINOP (op_el_div, octave_sparse_complex_matrix, octave_complex,
                 el_div);
  INSTALL_BINOP (op_el_pow, octave_sparse_complex_matrix, octave_complex,
                 el_pow);
  INSTALL_BINOP (op_el_ldiv, octave_sparse_complex_matrix, octave_complex,
                 el_ldiv);
  INSTALL_BINOP (op_el_and, octave_sparse_complex_matrix, octave_complex,
                 el_and);
  INSTALL_BINOP (op_el_or, octave_sparse_complex_matrix, octave_complex,
                 el_or);

  INSTALL_CATOP (octave_sparse_complex_matrix, octave_complex, scm_cs);

  INSTALL_ASSIGNOP (op_asn_eq, octave_sparse_complex_matrix, octave_complex,
                    assign);
}
