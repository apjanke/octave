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

#include "quit.h"

#define OCTAVE_CONCAT_FN(TYPE) \
  DEFNDCATOP_FN (TYPE ## _s_s, TYPE ## _scalar, TYPE ## _scalar, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _s_m, TYPE ## _scalar, TYPE ## _matrix, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _m_s, TYPE ## _matrix, TYPE ## _scalar, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _m_m, TYPE ## _matrix, TYPE ## _matrix, TYPE ## _array, TYPE ## _array, concat)

#define OCTAVE_INSTALL_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _scalar, TYPE ## _s_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, TYPE ## _s_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _scalar, TYPE ## _m_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _matrix, TYPE ## _m_m)

#define OCTAVE_S_INT_UNOPS(TYPE) \
  /* scalar unary ops. */  \
 \
  DEFUNOP_OP (s_not, TYPE ## _scalar, !) \
  DEFUNOP_OP (s_uminus, TYPE ## _scalar, -) \
  DEFUNOP_OP (s_transpose, TYPE ## _scalar, /* no-op */) \
  DEFUNOP_OP (s_hermitian, TYPE ## _scalar, /* no-op */) \
 \
  /* DEFNCUNOP_METHOD (s_incr, TYPE ## _scalar, increment) */ \
  /* DEFNCUNOP_METHOD (s_decr, TYPE ## _scalar, decrement) */

#define OCTAVE_SS_INT_ARITH_OPS(PFX, T1, T2) \
  /* scalar by scalar ops. */ \
 \
  DEFBINOP_OP (PFX ## _add, T1 ## scalar, T2 ## scalar, +) \
  DEFBINOP_OP (PFX ## _sub, T1 ## scalar, T2 ## scalar, -) \
  DEFBINOP_OP (PFX ## _mul, T1 ## scalar, T2 ## scalar, *) \
 \
  DEFBINOP (PFX ## _div, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v2.T2 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.T1 ## scalar_value () / v2.T2 ## scalar_value ()); \
  } \
 \
  DEFBINOP_FN (PFX ## _pow, T1 ## scalar, T2 ## scalar, xpow) \
 \
  DEFBINOP (PFX ## _ldiv, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v1.T1 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.T2 ## scalar_value () / v1.T1 ## scalar_value ()); \
  } \
 \
  DEFBINOP_OP (PFX ## _el_mul, T1 ## scalar, T2 ## scalar, *) \
 \
  DEFBINOP (PFX ## _el_div, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v2.T2 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.T1 ## scalar_value () / v2.T2 ## scalar_value ()); \
  } \
 \
  DEFBINOP_FN (PFX ## _el_pow, T1 ## scalar, T2 ## scalar, xpow) \
 \
  DEFBINOP (PFX ## _el_ldiv, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v1.T1 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.T2 ## scalar_value () / v1.T1 ## scalar_value ()); \
  } \

#define OCTAVE_SS_INT_BOOL_OPS(PFX, T1, T2) \
  /* DEFBINOP_OP (PFX ## _el_and, T1 ## scalar, T2 ## scalar, &&) */ \
  /* DEFBINOP_OP (PFX ## _el_or, T1 ## scalar, T2 ## scalar, ||) */

#define OCTAVE_SS_INT_CMP_OPS(PFX, T1, T2) \
  DEFBINOP_OP (PFX ## _lt, T1 ## scalar, T2 ## scalar, <) \
  DEFBINOP_OP (PFX ## _le, T1 ## scalar, T2 ## scalar, <=) \
  DEFBINOP_OP (PFX ## _eq, T1 ## scalar, T2 ## scalar, ==) \
  DEFBINOP_OP (PFX ## _ge, T1 ## scalar, T2 ## scalar, >=) \
  DEFBINOP_OP (PFX ## _gt, T1 ## scalar, T2 ## scalar, >) \
  DEFBINOP_OP (PFX ## _ne, T1 ## scalar, T2 ## scalar, !=)

#define OCTAVE_SS_POW_OPS(T1, T2) \
  octave_value \
  xpow (const octave_ ## T1& a, const octave_ ## T2& b) \
  { \
    return pow (a, b); \
  } \
 \
  octave_value \
  xpow (const octave_ ## T1& a, double b) \
  { \
    return pow (a, b); \
  } \
 \
  octave_value \
  xpow (double a, const octave_ ## T1& b) \
  { \
    return pow (a, b); \
  }

#define OCTAVE_SS_INT_OPS(TYPE) \
  OCTAVE_S_INT_UNOPS (TYPE) \
  OCTAVE_SS_POW_OPS (TYPE, TYPE) \
  OCTAVE_SS_INT_ARITH_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_SS_INT_ARITH_OPS (sx, TYPE ## _, ) \
  OCTAVE_SS_INT_ARITH_OPS (xs, , TYPE ## _) \
  OCTAVE_SS_INT_CMP_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_SS_INT_CMP_OPS (sx, TYPE ## _, ) \
  OCTAVE_SS_INT_CMP_OPS (xs, , TYPE ## _) \
  OCTAVE_SS_INT_BOOL_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_SS_INT_BOOL_OPS (sx, TYPE ## _, ) \
  OCTAVE_SS_INT_BOOL_OPS (xs, , TYPE ## _)

#define OCTAVE_SS_INT_OPS2(T1, T2) \
  OCTAVE_SS_INT_ARITH_OPS (ss, T1, T2) \
  OCTAVE_SS_INT_CMP_OPS (ss, T1, T2) \
  OCTAVE_SS_INT_BOOL_OPS (ss, T1, T2)

#define OCTAVE_SM_INT_ARITH_OPS(PFX, TS, TM) \
  /* scalar by matrix ops. */ \
 \
  DEFNDBINOP_OP (PFX ## _add, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, +) \
  DEFNDBINOP_OP (PFX ## _sub, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, -) \
  DEFNDBINOP_OP (PFX ## _mul, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, *) \
 \
  /* DEFBINOP (PFX ## _div, TS ## scalar, TM ## matrix) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); */ \
  /* */ \
  /* Matrix m1 = v1.TM ## matrix_value (); */ \
  /* Matrix m2 = v2.TM ## matrix_value (); */ \
  /* */ \
  /* return octave_value (xdiv (m1, m2)); */ \
  /* } */ \
 \
  /* DEFBINOP_FN (PFX ## _pow, TS ## scalar, TM ## matrix, xpow) */ \
 \
  DEFBINOP (PFX ## _ldiv, TS ## scalar, TM ## matrix) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); \
 \
    if (! v1.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.TS ## scalar_value () / v1.TS ## scalar_value ()); \
  } \
 \
  DEFNDBINOP_OP (PFX ## _el_mul, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, *) \
  /* DEFNDBINOP_FN (PFX ## _el_div, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, x_el_div) */ \
  DEFNDBINOP_FN (PFX ## _el_pow, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, elem_xpow) \
 \
  DEFBINOP (PFX ## _el_ldiv, TS ## scalar, TM ## matrix) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); \
 \
    if (! v1.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.TM ## array_value () / v1.TS ## scalar_value ()); \
  }

#define OCTAVE_SM_INT_CMP_OPS(PFX, TS, TM) \
  DEFNDBINOP_FN (PFX ## _lt, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_lt) \
  DEFNDBINOP_FN (PFX ## _le, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_le) \
  DEFNDBINOP_FN (PFX ## _eq, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_eq) \
  DEFNDBINOP_FN (PFX ## _ge, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_ge) \
  DEFNDBINOP_FN (PFX ## _gt, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_gt) \
  DEFNDBINOP_FN (PFX ## _ne, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_ne)

#define OCTAVE_SM_INT_BOOL_OPS(PFX, TS, TM) \
  /* DEFNDBINOP_FN (PFX ## _el_and, TS ## scalar, TYPE ## matrix, TS ## scalar, TYPE ## array, mx_el_and) */ \
  /* DEFNDBINOP_FN (PFX ## _el_or,  TS ## scalar, TYPE ## matrix, TS ## scalar, TYPE ## array, mx_el_or) */

#define OCTAVE_SM_POW_OPS(T1, T2) \
  octave_value \
  elem_xpow (const octave_ ## T1& a, const T2 ## NDArray& b) \
  { \
    T2 ## NDArray result (b.dims ()); \
    for (int i = 0; i < b.length (); i++) \
      { \
	OCTAVE_QUIT; \
	result (i) = pow (a, b(i)); \
      } \
    return octave_value (result); \
  }

#define OCTAVE_SM_CONV(TS, TM) \
  DEFCONV (TS ## s_ ## TM ## m_conv, TM ## scalar, TM ## matrix) \
  { \
    CAST_CONV_ARG (const octave_ ## TS ## scalar&); \
 \
    return new octave_ ## TM ## matrix (v.TM ## array_value ()); \
  }

#define OCTAVE_SM_INT_OPS(TYPE) \
  OCTAVE_SM_POW_OPS (TYPE, TYPE) \
  OCTAVE_SM_INT_ARITH_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_ARITH_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_INT_CMP_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_CMP_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_INT_BOOL_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_BOOL_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_CONV (TYPE ## _, TYPE ## _) \
  OCTAVE_SM_CONV (TYPE ## _, complex_)

#define OCTAVE_SM_INT_OPS2(TS, TM) \
  OCTAVE_SM_INT_ARITH_OPS (sm, TS, TM) \
  OCTAVE_SM_INT_CMP_OPS (sm, TS, TM) \
  OCTAVE_SM_INT_BOOL_OPS (sm, TS, TM)

#define OCTAVE_MS_INT_ARITH_OPS(PFX, TM, TS) \
  /* matrix by scalar ops. */ \
 \
  DEFNDBINOP_OP (PFX ## _add, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, +) \
  DEFNDBINOP_OP (PFX ## _sub, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, -) \
  DEFNDBINOP_OP (PFX ## _mul, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, *) \
 \
  DEFBINOP (PFX ## _div, TM ## matrix, TS ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); \
 \
    if (! v2.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.TM ## array_value () / v2.TS ## scalar_value ()); \
  } \
 \
  /* DEFBINOP_FN (PFX ## _pow, TM ## matrix, TS ## scalar, xpow) */ \
 \
  /* DEFBINOP (PFX ## _ldiv, TM ## matrix, TS ## scalar) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); */ \
  /* */ \
  /* Matrix m1 = v1.TM ## matrix_value (); */ \
  /* Matrix m2 = v2.TM ## matrix_value (); */ \
  /* */ \
  /* return octave_value (xleftdiv (m1, m2)); */ \
  /* } */ \
 \
  DEFNDBINOP_OP (PFX ## _el_mul, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, *) \
 \
  DEFBINOP (PFX ## _el_div, TM ## matrix, TS ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); \
 \
    if (! v2.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.TM ## array_value () / v2.TS ## scalar_value ()); \
  } \
 \
  DEFNDBINOP_FN (PFX ## _el_pow, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, elem_xpow) \
 \
  /* DEFBINOP (el_ldiv, TM ## matrix, TS ## scalar) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); */ \
  /* */ \
  /* return x_el_div (v2.TM ## _ ## TS ## scalar_value (), v1.TM ## array_value ()); */ \
  /* } */

#define OCTAVE_MS_INT_CMP_OPS(PFX, TM, TS) \
  DEFNDBINOP_FN (PFX ## _lt, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_lt) \
  DEFNDBINOP_FN (PFX ## _le, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_le) \
  DEFNDBINOP_FN (PFX ## _eq, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_eq) \
  DEFNDBINOP_FN (PFX ## _ge, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_ge) \
  DEFNDBINOP_FN (PFX ## _gt, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_gt) \
  DEFNDBINOP_FN (PFX ## _ne, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_ne) \

#define OCTAVE_MS_INT_BOOL_OPS(PFX, TM, TS) \
  /* DEFNDBINOP_FN (PFX ## _el_and, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_and) */ \
  /* DEFNDBINOP_FN (PFX ## _el_or, TM ## matrix, TS ## scalar, TM
     ## array, TS ## scalar, mx_el_or) */

#define OCTAVE_MS_INT_ASSIGN_OPS(PFX, TM, TS, TE) \
  DEFNDASSIGNOP_FN (PFX ## _assign, TM ## matrix, TS ## scalar, TE ## array, assign)

#define OCTAVE_MS_POW_OPS(T1, T2) \
octave_value elem_xpow (T1 ## NDArray a, octave_ ## T2  b) \
{ \
  T1 ## NDArray result (a.dims ()); \
  for (int i = 0; i < a.length (); i++) \
    { \
      OCTAVE_QUIT; \
      result (i) = pow (a(i), b);		\
    } \
  return octave_value (result); \
}

#define OCTAVE_MS_INT_OPS(TYPE) \
  OCTAVE_MS_POW_OPS (TYPE, TYPE) \
  OCTAVE_MS_INT_ARITH_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_ARITH_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_CMP_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_CMP_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_BOOL_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_BOOL_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_ASSIGN_OPS (ms, TYPE ## _, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_ASSIGN_OPS (mx, TYPE ## _, , )

#define OCTAVE_M_INT_UNOPS(TYPE) \
  /* matrix unary ops. */ \
 \
  DEFNDUNOP_OP (m_not, TYPE ## _matrix, TYPE ## _array, !) \
  DEFNDUNOP_OP (m_uminus, TYPE ## _matrix, TYPE ## _array, -) \
 \
  DEFUNOP (m_transpose, TYPE ## _matrix) \
  { \
    CAST_UNOP_ARG (const octave_ ## TYPE ## _matrix&); \
 \
    if (v.ndims () > 2) \
      { \
	error ("transpose not defined for N-d objects"); \
	return octave_value (); \
      } \
    else \
      return octave_value (v.TYPE ## _array_value().transpose ()); \
  } \
 \
  /* DEFNCUNOP_METHOD (m_incr, TYPE ## _matrix, increment) */ \
  /* DEFNCUNOP_METHOD (m_decr, TYPE ## _matrix, decrement) */

#define OCTAVE_MM_INT_ARITH_OPS(T1, T2) \
  /* matrix by matrix ops. */ \
 \
  DEFNDBINOP_OP (mm_add, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, +) \
  DEFNDBINOP_OP (mm_sub, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, -) \
 \
  /* DEFBINOP_OP (mm_mul, T1 ## _matrix, T2 ## _matrix, *) */ \
  /* DEFBINOP_FN (mm_div, T1 ## _matrix, T2 ## _matrix, xdiv) */ \
 \
  DEFBINOPX (mm_pow, T1 ## _matrix, T2 ## _matrix) \
  { \
    error ("can't do A ^ B for A and B both matrices"); \
    return octave_value (); \
  } \
 \
  /* DEFBINOP_FN (ldiv, T1 ## _matrix, T2 ## _matrix, xleftdiv) */ \
 \
  DEFNDBINOP_FN (mm_el_mul, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, product) \
 \
  DEFNDBINOP_FN (mm_el_div, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, quotient) \
 \
  DEFNDBINOP_FN (mm_el_pow, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, elem_xpow) \
 \
  /* DEFBINOP (mm_el_ldiv, T1 ## _matrix, T2 ## _matrix) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octavematrix&, const octavematrix&); */ \
  /* */ \
  /* return octave_value (quotient (v2.array_value (), v1.array_value ())); */ \
  /* } */

#define OCTAVE_MM_INT_CMP_OPS(T1, T2) \
  DEFNDBINOP_FN (mm_lt, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_lt) \
  DEFNDBINOP_FN (mm_le, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_le) \
  DEFNDBINOP_FN (mm_eq, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_eq) \
  DEFNDBINOP_FN (mm_ge, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_ge) \
  DEFNDBINOP_FN (mm_gt, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_gt) \
  DEFNDBINOP_FN (mm_ne, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_ne)

#define OCTAVE_MM_INT_BOOL_OPS(T1, T2) \
  DEFNDBINOP_FN (mm_el_and, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_and) \
  DEFNDBINOP_FN (mm_el_or,  T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, mx_el_or)

#define OCTAVE_MM_INT_ASSIGN_OPS(PFX, TLHS, TRHS, TE) \
  DEFNDASSIGNOP_FN (PFX ## _assign, TLHS ## matrix, TRHS ## matrix, TE ## array, assign)

#define OCTAVE_MM_POW_OPS(T1, T2) \
  octave_value \
  elem_xpow (const T1 ## NDArray& a, const T2 ## NDArray& b) \
  { \
    dim_vector a_dims = a.dims (); \
    dim_vector b_dims = b.dims (); \
    if (a_dims != b_dims) \
      { \
	gripe_nonconformant ("operator .^", a_dims, b_dims); \
	return octave_value (); \
      } \
    T1 ## NDArray result (a_dims); \
    for (int i = 0; i < a.length (); i++) \
      { \
	OCTAVE_QUIT; \
	result (i) = pow (a(i), b(i)); \
      } \
    return octave_value (result); \
  }

#define OCTAVE_MM_INT_OPS(TYPE) \
  OCTAVE_M_INT_UNOPS (TYPE) \
  OCTAVE_MM_POW_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_ARITH_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_CMP_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_BOOL_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_ASSIGN_OPS (mm, TYPE ## _, TYPE ## _, TYPE ## _) \
  OCTAVE_MM_INT_ASSIGN_OPS (mmx, TYPE ## _, , )

#define OCTAVE_MM_INT_OPS2(T1, T2) \
  OCTAVE_MM_INT_ARITH_OPS (mm, T1, T2) \
  OCTAVE_MM_INT_CMP_OPS (mm, T1, T2) \
  OCTAVE_MM_INT_BOOL_OPS (mm, T1, T2)

#define OCTAVE_RE_INT_ASSIGN_OPS(TYPE) \
  DEFNDASSIGNOP_FN (TYPE ## ms_assign, matrix, TYPE ## _scalar, array, assign) \
  DEFNDASSIGNOP_FN (TYPE ## mm_assign, matrix, TYPE ## _matrix, array, assign)

#define OCTAVE_CX_INT_ASSIGN_OPS(TYPE) \
  DEFNDASSIGNOP_FN (TYPE ## cms_assign, complex_matrix, TYPE ## _scalar, complex_array, assign) \
  DEFNDASSIGNOP_FN (TYPE ## cmm_assign, complex_matrix, TYPE ## _matrix, complex_array, assign)

#define OCTAVE_INT_OPS(TYPE) \
  OCTAVE_SS_INT_OPS (TYPE) \
  OCTAVE_SM_INT_OPS (TYPE) \
  OCTAVE_MS_INT_OPS (TYPE) \
  OCTAVE_MM_INT_OPS (TYPE) \
  OCTAVE_CONCAT_FN (TYPE) \
  OCTAVE_RE_INT_ASSIGN_OPS (TYPE) \
  OCTAVE_CX_INT_ASSIGN_OPS (TYPE)

#define OCTAVE_INSTALL_S_INT_UNOPS(TYPE) \
  INSTALL_UNOP (op_not, octave_ ## TYPE ## _scalar, s_not); \
  INSTALL_UNOP (op_uminus, octave_ ## TYPE ## _scalar, s_uminus); \
  INSTALL_UNOP (op_transpose, octave_ ## TYPE ## _scalar, s_transpose); \
  INSTALL_UNOP (op_hermitian, octave_ ## TYPE ## _scalar, s_hermitian); \
 \
  /* INSTALL_NCUNOP (op_incr, octave_ ## TYPE ## _scalar, s_incr); */ \
  /* INSTALL_NCUNOP (op_decr, octave_ ## TYPE ## _scalar, s_decr); */

#define OCTAVE_INSTALL_SS_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _mul); \
  INSTALL_BINOP (op_div, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _div); \
  INSTALL_BINOP (op_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _pow); \
  INSTALL_BINOP (op_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ldiv); \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_pow); \
  INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_ldiv);

#define OCTAVE_INSTALL_SS_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ne);

#define OCTAVE_INSTALL_SS_INT_BOOL_OPS(PFX, T1, T2) \
  /* INSTALL_BINOP (op_el_and, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_and); */ \
  /* INSTALL_BINOP (op_el_or, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_or); */

#define OCTAVE_INSTALL_SS_INT_OPS(TYPE) \
  OCTAVE_INSTALL_S_INT_UNOPS (TYPE) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (xs, , TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (xs, , TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (xs, , TYPE ## _) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_scalar, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_complex_scalar, octave_complex_matrix)

#define OCTAVE_INSTALL_SS_INT_OPS2(T1, T2) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (ss, T1, T2) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (ss, T1, T2) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (ss, T1, T2)

#define OCTAVE_INSTALL_SM_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _mul); \
  /* INSTALL_BINOP (op_div, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _div); */ \
  /* INSTALL_BINOP (op_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _pow); */ \
  INSTALL_BINOP (op_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ldiv); \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_mul); \
  /* INSTALL_BINOP (op_el_div, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_div); */ \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_pow); \
  INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_ldiv);

#define OCTAVE_INSTALL_SM_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ne);

#define OCTAVE_INSTALL_SM_INT_BOOL_OPS(PFX, T1, T2) \
  /* INSTALL_BINOP (op_el_and, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_and); */ \
  /* INSTALL_BINOP (op_el_or, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_or); */

#define OCTAVE_INSTALL_SM_INT_OPS(TYPE) \
  OCTAVE_INSTALL_SM_INT_ARITH_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_ARITH_OPS (xm, , TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (xm, , TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (xm, , TYPE ## _) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, TYPE ## _matrix_conv) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _scalar, octave_complex_matrix, complex_matrix_conv) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_matrix, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_complex_matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_SM_INT_OPS2(T1, T2) \
  OCTAVE_INSTALL_SM_INT_ARITH_OPS (sm, T1, T2) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (sm, T1, T2) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (sm, T1, T2)

#define OCTAVE_INSTALL_MS_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _mul); \
  INSTALL_BINOP (op_div, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _div); \
  /* INSTALL_BINOP (op_pow, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _pow); */ \
  /* INSTALL_BINOP (op_ldiv, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ldiv); */ \
 \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_pow); \
  /* INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_ldiv); */

#define OCTAVE_INSTALL_MS_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ne);

#define OCTAVE_INSTALL_MS_INT_BOOL_OPS(PFX, T1, T2) \
  /* INSTALL_BINOP (op_el_and, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_and); */ \
  /* INSTALL_BINOP (op_el_or, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_or); */

#define OCTAVE_INSTALL_MS_INT_ASSIGN_OPS(PFX, TLHS, TRHS) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_ ## TLHS ## matrix, octave_ ## TRHS ## scalar, PFX ## _assign)

#define OCTAVE_INSTALL_MS_INT_OPS(TYPE) \
  OCTAVE_INSTALL_MS_INT_ARITH_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_ARITH_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mx, TYPE ## _, ) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _matrix, octave_complex_scalar, octave_complex_matrix)

#define OCTAVE_INSTALL_MS_INT_OPS2(T1, T2) \
  OCTAVE_INSTALL_MS_INT_ARITH_OPS (ms, T1, T2) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (ms, T1, T2) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (ms, T1, T2)

#define OCTAVE_INSTALL_M_INT_UNOPS(TYPE) \
  INSTALL_UNOP (op_not, octave_ ## TYPE ## _matrix, m_not); \
  INSTALL_UNOP (op_uminus, octave_ ## TYPE ## _matrix, m_uminus); \
  INSTALL_UNOP (op_transpose, octave_ ## TYPE ## _matrix, m_transpose); \
  INSTALL_UNOP (op_hermitian, octave_ ## TYPE ## _matrix, m_transpose); \
 \
  /* INSTALL_NCUNOP (op_incr, octave_ ## TYPE ## _matrix, m_incr); */ \
  /* INSTALL_NCUNOP (op_decr, octave_ ## TYPE ## _matrix, m_decr); */

#define OCTAVE_INSTALL_MM_INT_ARITH_OPS(T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_sub); \
  /* INSTALL_BINOP (op_mul, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_mul); */ \
  /* INSTALL_BINOP (op_div, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_div); */ \
  INSTALL_BINOP (op_pow, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_pow); \
  /* INSTALL_BINOP (op_ldiv, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_ldiv); */ \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_pow); \
  /* INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_ldiv); */

#define OCTAVE_INSTALL_MM_INT_CMP_OPS(T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_ne);

#define OCTAVE_INSTALL_MM_INT_BOOL_OPS(T1, T2) \
  INSTALL_BINOP (op_el_and, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_and); \
  INSTALL_BINOP (op_el_or, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_or);

#define OCTAVE_INSTALL_MM_INT_ASSIGN_OPS(PFX, TLHS, TRHS) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_ ## TLHS ## matrix, octave_ ## TRHS ## matrix, PFX ## _assign)

#define OCTAVE_INSTALL_MM_INT_OPS(TYPE) \
  OCTAVE_INSTALL_M_INT_UNOPS (TYPE) \
  OCTAVE_INSTALL_MM_INT_ARITH_OPS (TYPE, TYPE) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (TYPE, TYPE) \
  OCTAVE_INSTALL_MM_INT_BOOL_OPS (TYPE, TYPE) \
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmx, TYPE ## _, ) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _matrix, octave_complex_matrix, complex_matrix_conv) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _matrix, octave_complex_matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_MM_INT_OPS2(T1, T2) \
  OCTAVE_INSTALL_MM_INT_ARITH_OPS (T1, T2) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (T1, T2) \
  OCTAVE_INSTALL_MM_INT_BOOL_OPS (T1, T2)

#define OCTAVE_INSTALL_RE_INT_ASSIGN_OPS(TYPE) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, octave_ ## TYPE ## _scalar, TYPE ## ms_assign) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, octave_ ## TYPE ## _matrix, TYPE ## mm_assign) \
  INSTALL_ASSIGNCONV (octave_scalar, octave_ ## TYPE ## _scalar, octave_matrix) \
  INSTALL_ASSIGNCONV (octave_scalar, octave_ ## TYPE ## _matrix, octave_matrix)

#define OCTAVE_INSTALL_CX_INT_ASSIGN_OPS(TYPE) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_complex_matrix, octave_ ## TYPE ## _scalar, TYPE ## cms_assign) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_complex_matrix, octave_ ## TYPE ## _matrix, TYPE ## cmm_assign) \
  INSTALL_ASSIGNCONV (octave_complex_scalar, octave_ ## TYPE ## _scalar, octave_complex_matrix) \
  INSTALL_ASSIGNCONV (octave_complex_scalar, octave_ ## TYPE ## _matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_INT_OPS(TYPE) \
  OCTAVE_INSTALL_SS_INT_OPS (TYPE) \
  OCTAVE_INSTALL_SM_INT_OPS (TYPE) \
  OCTAVE_INSTALL_MS_INT_OPS (TYPE) \
  OCTAVE_INSTALL_MM_INT_OPS (TYPE) \
  OCTAVE_INSTALL_CONCAT_FN (TYPE) \
  OCTAVE_INSTALL_RE_INT_ASSIGN_OPS (TYPE) \
  OCTAVE_INSTALL_CX_INT_ASSIGN_OPS (TYPE)

#define OCTAVE_INSTALL_SM_INT_ASSIGNCONV(TLHS, TRHS) \
  INSTALL_ASSIGNCONV (octave_ ## TLHS ## _scalar, octave_ ## TRHS ## _scalar, octave_ ## TLHS ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TLHS ## _scalar, octave_ ## TRHS ## _matrix, octave_ ## TLHS ## _matrix)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
