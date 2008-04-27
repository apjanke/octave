/*

Copyright (C) 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005,
              2006, 2007 John W. Eaton

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
#include "ov-re-mat.h"
#include "ov-flt-re-mat.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

// matrix unary ops.

DEFNDUNOP_OP (not, float_matrix, float_array, !)
DEFNDUNOP_OP (uplus, float_matrix, float_array, /* no-op */)
DEFNDUNOP_OP (uminus, float_matrix, float_array, -)

DEFUNOP (transpose, float_matrix)
{
  CAST_UNOP_ARG (const octave_float_matrix&);

  if (v.ndims () > 2)
    {
      error ("transpose not defined for N-d objects");
      return octave_value ();
    }
  else
    return octave_value (v.float_matrix_value().transpose ());
}

DEFNCUNOP_METHOD (incr, float_matrix, increment)
DEFNCUNOP_METHOD (decr, float_matrix, decrement)

// matrix by matrix ops.

DEFNDBINOP_OP (add, float_matrix, float_matrix, float_array, float_array, +)
DEFNDBINOP_OP (sub, float_matrix, float_matrix, float_array, float_array, -)

DEFBINOP_OP (mul, float_matrix, float_matrix, *)

DEFBINOP (div, float_matrix, float_matrix)
{
  CAST_BINOP_ARGS (const octave_float_matrix&, const octave_float_matrix&);
  MatrixType typ = v2.matrix_type ();
  
  FloatMatrix ret = xdiv (v1.float_matrix_value (), 
			  v2.float_matrix_value (), typ);

  v2.matrix_type (typ);
  return ret;
}

DEFBINOPX (pow, float_matrix, float_matrix)
{
  error ("can't do A ^ B for A and B both matrices");
  return octave_value ();
}

DEFBINOP (ldiv, float_matrix, float_matrix)
{
  CAST_BINOP_ARGS (const octave_float_matrix&, const octave_float_matrix&);
  MatrixType typ = v1.matrix_type ();
  
  FloatMatrix ret = xleftdiv (v1.float_matrix_value (), 
			      v2.float_matrix_value (), typ);

  v1.matrix_type (typ);
  return ret;
}

DEFNDBINOP_FN (lt, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_lt)
DEFNDBINOP_FN (le, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_le)
DEFNDBINOP_FN (eq, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_eq)
DEFNDBINOP_FN (ge, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_ge)
DEFNDBINOP_FN (gt, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_gt)
DEFNDBINOP_FN (ne, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_ne)

DEFNDBINOP_FN (el_mul, float_matrix, float_matrix, float_array, 
	       float_array, product)
DEFNDBINOP_FN (el_div, float_matrix, float_matrix, float_array, 
	       float_array, quotient)
DEFNDBINOP_FN (el_pow, float_matrix, float_matrix, float_array, 
	       float_array, elem_xpow)

DEFBINOP (el_ldiv, float_matrix, float_matrix)
{
  CAST_BINOP_ARGS (const octave_float_matrix&, const octave_float_matrix&);

  return octave_value (quotient (v2.float_array_value (), 
				 v1.float_array_value ()));
}

DEFNDBINOP_FN (el_and, float_matrix, float_matrix, float_array, 
	       float_array, mx_el_and)
DEFNDBINOP_FN (el_or,  float_matrix, float_matrix, float_array, 
	       float_array, mx_el_or)

DEFNDCATOP_FN (fm_fm, float_matrix, float_matrix, float_array, 
	       float_array, concat)

DEFNDASSIGNOP_FN (assign, float_matrix, float_matrix, float_array, assign)

DEFNDASSIGNOP_FN (dbl_assign, matrix, float_matrix, array, assign)

CONVDECL (float_matrix_to_matrix)
{
  CAST_CONV_ARG (const octave_float_matrix&);

  return new octave_matrix (v.array_value ());
}

void
install_fm_fm_ops (void)
{
  INSTALL_UNOP (op_not, octave_float_matrix, not);
  INSTALL_UNOP (op_uplus, octave_float_matrix, uplus);
  INSTALL_UNOP (op_uminus, octave_float_matrix, uminus);
  INSTALL_UNOP (op_transpose, octave_float_matrix, transpose);
  INSTALL_UNOP (op_hermitian, octave_float_matrix, transpose);

  INSTALL_NCUNOP (op_incr, octave_float_matrix, incr);
  INSTALL_NCUNOP (op_decr, octave_float_matrix, decr);

  INSTALL_BINOP (op_add, octave_float_matrix, octave_float_matrix, add);
  INSTALL_BINOP (op_sub, octave_float_matrix, octave_float_matrix, sub);
  INSTALL_BINOP (op_mul, octave_float_matrix, octave_float_matrix, mul);
  INSTALL_BINOP (op_div, octave_float_matrix, octave_float_matrix, div);
  INSTALL_BINOP (op_pow, octave_float_matrix, octave_float_matrix, pow);
  INSTALL_BINOP (op_ldiv, octave_float_matrix, octave_float_matrix, ldiv);
  INSTALL_BINOP (op_lt, octave_float_matrix, octave_float_matrix, lt);
  INSTALL_BINOP (op_le, octave_float_matrix, octave_float_matrix, le);
  INSTALL_BINOP (op_eq, octave_float_matrix, octave_float_matrix, eq);
  INSTALL_BINOP (op_ge, octave_float_matrix, octave_float_matrix, ge);
  INSTALL_BINOP (op_gt, octave_float_matrix, octave_float_matrix, gt);
  INSTALL_BINOP (op_ne, octave_float_matrix, octave_float_matrix, ne);
  INSTALL_BINOP (op_el_mul, octave_float_matrix, octave_float_matrix, el_mul);
  INSTALL_BINOP (op_el_div, octave_float_matrix, octave_float_matrix, el_div);
  INSTALL_BINOP (op_el_pow, octave_float_matrix, octave_float_matrix, el_pow);
  INSTALL_BINOP (op_el_ldiv, octave_float_matrix, octave_float_matrix, el_ldiv);
  INSTALL_BINOP (op_el_and, octave_float_matrix, octave_float_matrix, el_and);
  INSTALL_BINOP (op_el_or, octave_float_matrix, octave_float_matrix, el_or);

  INSTALL_CATOP (octave_float_matrix, octave_float_matrix, fm_fm);

  INSTALL_ASSIGNOP (op_asn_eq, octave_float_matrix, 
		    octave_float_matrix, assign);
  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, 
		    octave_float_matrix, dbl_assign);

  INSTALL_CONVOP (octave_float_matrix, octave_matrix, float_matrix_to_matrix);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
