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

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gripes.h"
#include "ov.h"
#include "ov-re-mat.h"
#include "ov-scalar.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

// matrix by scalar ops.

DEFBINOP_OP (add, matrix, scalar, +)
DEFBINOP_OP (sub, matrix, scalar, -)
DEFBINOP_OP (mul, matrix, scalar, *)

DEFBINOP (div, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  double d = v2.double_value ();

  if (d == 0.0)
    gripe_divide_by_zero ();

  return octave_value (v1.matrix_value () / d);
}

DEFBINOP_FN (pow, matrix, scalar, xpow)

DEFBINOP (ldiv, matrix, scalar)
{
  BINOP_NONCONFORMANT ("operator \\");
}

DEFBINOP_FN (lt, matrix, scalar, mx_el_lt)
DEFBINOP_FN (le, matrix, scalar, mx_el_le)
DEFBINOP_FN (eq, matrix, scalar, mx_el_eq)
DEFBINOP_FN (ge, matrix, scalar, mx_el_ge)
DEFBINOP_FN (gt, matrix, scalar, mx_el_gt)
DEFBINOP_FN (ne, matrix, scalar, mx_el_ne)

DEFBINOP_OP (el_mul, matrix, scalar, *)

DEFBINOP (el_div, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  double d = v2.double_value ();

  if (d == 0.0)
    gripe_divide_by_zero ();

  return octave_value (v1.matrix_value () / d);
}

DEFBINOP_FN (el_pow, matrix, scalar, elem_xpow)

DEFBINOP (el_ldiv, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  return x_el_div (v2.double_value (), v1.matrix_value ());
}

DEFBINOP_FN (el_and, matrix, scalar, mx_el_and)
DEFBINOP_FN (el_or, matrix, scalar, mx_el_or)

DEFASSIGNOP_FN (assign, matrix, scalar, assign)

void
install_m_s_ops (void)
{
  INSTALL_BINOP (add, octave_matrix, octave_scalar, add);
  INSTALL_BINOP (sub, octave_matrix, octave_scalar, sub);
  INSTALL_BINOP (mul, octave_matrix, octave_scalar, mul);
  INSTALL_BINOP (div, octave_matrix, octave_scalar, div);
  INSTALL_BINOP (pow, octave_matrix, octave_scalar, pow);
  INSTALL_BINOP (ldiv, octave_matrix, octave_scalar, ldiv);
  INSTALL_BINOP (lt, octave_matrix, octave_scalar, lt);
  INSTALL_BINOP (le, octave_matrix, octave_scalar, le);
  INSTALL_BINOP (eq, octave_matrix, octave_scalar, eq);
  INSTALL_BINOP (ge, octave_matrix, octave_scalar, ge);
  INSTALL_BINOP (gt, octave_matrix, octave_scalar, gt);
  INSTALL_BINOP (ne, octave_matrix, octave_scalar, ne);
  INSTALL_BINOP (el_mul, octave_matrix, octave_scalar, el_mul);
  INSTALL_BINOP (el_div, octave_matrix, octave_scalar, el_div);
  INSTALL_BINOP (el_pow, octave_matrix, octave_scalar, el_pow);
  INSTALL_BINOP (el_ldiv, octave_matrix, octave_scalar, el_ldiv);
  INSTALL_BINOP (el_and, octave_matrix, octave_scalar, el_and);
  INSTALL_BINOP (el_or, octave_matrix, octave_scalar, el_or);

  INSTALL_ASSIGNOP (asn_eq, octave_matrix, octave_scalar, assign);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
