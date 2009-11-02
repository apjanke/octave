/*

Copyright (C) 1993, 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2002,
              2003, 2004, 2005, 2006, 2007, 2008 John W. Eaton
Copyright (C) 2009 Jaroslav Hajek
Copyright (C) 2009 VZLU Prague

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

#if !defined (octave_mx_inlines_h)
#define octave_mx_inlines_h 1

#include <cstddef>
#include <cmath>

#include "quit.h"

#include "oct-cmplx.h"
#include "oct-locbuf.h"
#include "oct-inttypes.h"
#include "Array-util.h"

// Provides some commonly repeated, basic loop templates.

template <class R, class S>
inline void mx_inline_fill (size_t n, R *r, S s)
{ for (size_t i = 0; i < n; i++) r[i] = s; }

#define DEFMXUNOP(F, OP) \
template <class R, class X> \
inline void F (size_t n, R *r, const X *x) \
{ for (size_t i = 0; i < n; i++) r[i] = OP x[i]; }

DEFMXUNOP (mx_inline_uminus, -)

#define DEFMXUNOPEQ(F, OP) \
template <class R> \
inline void F (size_t n, R *r) \
{ for (size_t i = 0; i < n; i++) r[i] = OP r[i]; }

DEFMXUNOPEQ (mx_inline_uminus2, -)

#define DEFMXUNBOOLOP(F, OP) \
template <class X> \
inline void F (size_t n, bool *r, const X *x) \
{ const X zero = X(); for (size_t i = 0; i < n; i++) r[i] = x[i] OP zero; }

DEFMXUNBOOLOP (mx_inline_iszero, ==)
DEFMXUNBOOLOP (mx_inline_notzero, !=)

#define DEFMXBINOP(F, OP) \
template <class R, class X, class Y> \
inline void F (size_t n, R *r, const X *x, const Y *y) \
{ for (size_t i = 0; i < n; i++) r[i] = x[i] OP y[i]; } \
template <class R, class X, class Y> \
inline void F (size_t n, R *r, const X *x, Y y) \
{ for (size_t i = 0; i < n; i++) r[i] = x[i] OP y; } \
template <class R, class X, class Y> \
inline void F (size_t n, R *r, X x, const Y *y) \
{ for (size_t i = 0; i < n; i++) r[i] = x OP y[i]; }

DEFMXBINOP (mx_inline_add, +)
DEFMXBINOP (mx_inline_sub, -)
DEFMXBINOP (mx_inline_mul, *)
DEFMXBINOP (mx_inline_div, /)

#define DEFMXBINOPEQ(F, OP) \
template <class R, class X> \
inline void F (size_t n, R *r, const X *x) \
{ for (size_t i = 0; i < n; i++) r[i] OP x[i]; } \
template <class R, class X> \
inline void F (size_t n, R *r, X x) \
{ for (size_t i = 0; i < n; i++) r[i] OP x; }

DEFMXBINOPEQ (mx_inline_add2, +=)
DEFMXBINOPEQ (mx_inline_sub2, -=)
DEFMXBINOPEQ (mx_inline_mul2, *=)
DEFMXBINOPEQ (mx_inline_div2, /=)

#define DEFMXCMPOP(F, OP) \
template <class X, class Y> \
inline void F (size_t n, bool *r, const X *x, const Y *y) \
{ for (size_t i = 0; i < n; i++) r[i] = x[i] OP y[i]; } \
template <class X, class Y> \
inline void F (size_t n, bool *r, const X *x, Y y) \
{ for (size_t i = 0; i < n; i++) r[i] = x[i] OP y; } \
template <class X, class Y> \
inline void F (size_t n, bool *r, X x, const Y *y) \
{ for (size_t i = 0; i < n; i++) r[i] = x OP y[i]; }

DEFMXCMPOP (mx_inline_lt, <)
DEFMXCMPOP (mx_inline_le, <=)
DEFMXCMPOP (mx_inline_gt, >)
DEFMXCMPOP (mx_inline_ge, >=)
DEFMXCMPOP (mx_inline_eq, ==)
DEFMXCMPOP (mx_inline_ne, !=)

// Convert to logical value, for logical op purposes.
template <class T> inline bool logical_value (T x) { return x; }
template <class T> inline bool logical_value (const octave_int<T>& x) 
{ return x.value (); }

// NaNs in real data should generate an error. Doing it on-the-fly is faster.

#define DEFLOGCHKNAN(ARG, ZERO) \
inline bool logical_value (ARG x) \
{ if (xisnan (x)) gripe_nan_to_logical_conversion (); return x != ZERO; }

DEFLOGCHKNAN (double, 0.0)
DEFLOGCHKNAN (const Complex&, 0.0)
DEFLOGCHKNAN (float, 0.0f)
DEFLOGCHKNAN (const FloatComplex&, 0.0f)

template <class X>
void mx_inline_not (size_t n, bool *r, const X* x)
{
  for (size_t i = 0; i < n; i++)
    r[i] = ! logical_value (x[i]);
}

inline void mx_inline_not2 (size_t n, bool *r)
{
  for (size_t i = 0; i < n; i++) r[i] = ! r[i];
}

#define DEFMXBOOLOP(F, NOT1, OP, NOT2) \
template <class X, class Y> \
inline void F (size_t n, bool *r, const X *x, const Y *y) \
{ \
  for (size_t i = 0; i < n; i++) \
    r[i] = (NOT1 logical_value (x[i])) OP (NOT2 logical_value (y[i])); \
} \
template <class X, class Y> \
inline void F (size_t n, bool *r, const X *x, Y y) \
{ \
  const bool yy = (NOT2 logical_value (y)); \
  for (size_t i = 0; i < n; i++) \
    r[i] = (NOT1 logical_value (x[i])) OP yy; \
} \
template <class X, class Y> \
inline void F (size_t n, bool *r, X x, const Y *y) \
{ \
  const bool xx = (NOT1 logical_value (x)); \
  for (size_t i = 0; i < n; i++) \
    r[i] = xx OP (NOT2 logical_value (y[i])); \
}

DEFMXBOOLOP (mx_inline_and, , &, )
DEFMXBOOLOP (mx_inline_or, , |, )
DEFMXBOOLOP (mx_inline_not_and, !, &, )
DEFMXBOOLOP (mx_inline_not_or, !, |, )
DEFMXBOOLOP (mx_inline_and_not, , &, !)
DEFMXBOOLOP (mx_inline_or_not, , |, !)

#define DEFMXBOOLOPEQ(F, OP) \
template <class X> \
inline void F (size_t n, bool *r, const X *x) \
{ \
  for (size_t i = 0; i < n; i++) \
    r[i] OP logical_value (x[i]); \
} \

DEFMXBOOLOPEQ (mx_inline_and2, &=)
DEFMXBOOLOPEQ (mx_inline_or2, |=)

template <class T> 
inline bool 
mx_inline_any_nan (size_t, const T*) { return false; }

#define DEFMXANYNAN(T) \
inline bool \
mx_inline_any_nan (size_t n, const T* t) \
{ \
  for (size_t i = 0; i < n; i++) \
    if (xisnan (t[i])) return true; \
  return false; \
}

DEFMXANYNAN(double)
DEFMXANYNAN(float)
DEFMXANYNAN(Complex)
DEFMXANYNAN(FloatComplex)

// Pairwise minimums/maximums
#define DEFMXMAPPER(F, FUN) \
template <class T> \
inline void F (size_t n, T *r, const T *x, const T *y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x[i], y[i]); } \
template <class T> \
inline void F (size_t n, T *r, const T *x, T y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x[i], y); } \
template <class T> \
inline void F (size_t n, T *r, T x, const T *y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x, y[i]); }

DEFMXMAPPER (mx_inline_xmin, xmin)
DEFMXMAPPER (mx_inline_xmax, xmax)

#define DEFMXLOCALMAPPER(F, FUN, T) \
static void F (size_t n, T *r, const T *x, const T *y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x[i], y[i]); } \
static void F (size_t n, T *r, const T *x, T y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x[i], y); } \
static void F (size_t n, T *r, T x, const T *y) \
{ for (size_t i = 0; i < n; i++) r[i] = FUN (x, y[i]); }

// Appliers. Since these call the operation just once, we pass it as
// a pointer, to allow the compiler reduce number of instances.

template <class RNDA, class XNDA>
inline RNDA 
do_mx_unary_op (const XNDA& x,
                void (*op) (size_t, typename RNDA::element_type *,
                            const typename XNDA::element_type *))
{
  RNDA r (x.dims ());
  op (r.length (), r.fortran_vec (), x.data ());
  return r;
}

template <class RNDA>
inline RNDA&
do_mx_inplace_op (RNDA& r,
                  void (*op) (size_t, typename RNDA::element_type *))
{
  op (r.numel (), r.fortran_vec ());
  return r;
}


template <class RNDA, class XNDA, class YNDA>
inline RNDA 
do_mm_binary_op (const XNDA& x, const YNDA& y,
                 void (*op) (size_t, typename RNDA::element_type *,
                             const typename XNDA::element_type *,
                             const typename YNDA::element_type *),
                 const char *opname)
{
  dim_vector dx = x.dims (), dy = y.dims ();
  if (dx == dy)
    {
      RNDA r (dx);
      op (r.length (), r.fortran_vec (), x.data (), y.data ());
      return r;
    }
  else
    {
      gripe_nonconformant (opname, dx, dy);
      return RNDA ();
    }
}

template <class RNDA, class XNDA, class YS>
inline RNDA 
do_ms_binary_op (const XNDA& x, const YS& y,
                 void (*op) (size_t, typename RNDA::element_type *,
                             const typename XNDA::element_type *, YS))
{
  RNDA r (x.dims ());
  op (r.length (), r.fortran_vec (), x.data (), y);
  return r;
}

template <class RNDA, class XS, class YNDA>
inline RNDA 
do_sm_binary_op (const XS& x, const YNDA& y,
                 void (*op) (size_t, typename RNDA::element_type *, XS,
                             const typename YNDA::element_type *))
{
  RNDA r (y.dims ());
  op (r.length (), r.fortran_vec (), x, y.data ());
  return r;
}

template <class RNDA, class XNDA>
inline RNDA& 
do_mm_inplace_op (RNDA& r, const XNDA& x,
                  void (*op) (size_t, typename RNDA::element_type *,
                              const typename XNDA::element_type *),
                  const char *opname)
{
  dim_vector dr = r.dims (), dx = x.dims ();
  if (dr == dx)
    op (r.length (), r.fortran_vec (), x.data ());
  else
    gripe_nonconformant (opname, dr, dx);
  return r;
}

template <class RNDA, class XS>
inline RNDA& 
do_ms_inplace_op (RNDA& r, const XS& x,
                  void (*op) (size_t, typename RNDA::element_type *, XS))
{
  op (r.length (), r.fortran_vec (), x);
  return r;
}

template <class T1, class T2>
inline bool
mx_inline_equal (size_t n, const T1 *x, const T2 *y)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}

#define OP_DUP_FCN(OP, F, R, T) \
  static inline R * \
  F (const T *x, size_t n) \
  { \
    R *r = 0; \
    if (n > 0) \
      { \
	r = new R [n]; \
	for (size_t i = 0; i < n; i++) \
	  r[i] = OP (x[i]); \
      } \
    return r; \
  }

OP_DUP_FCN (, mx_inline_dup, double,  double)
OP_DUP_FCN (, mx_inline_dup, Complex, Complex)
OP_DUP_FCN (, mx_inline_dup, float, float)
OP_DUP_FCN (, mx_inline_dup, FloatComplex, FloatComplex)

// These should really return a bool *.  Also, they should probably be
// in with a collection of other element-by-element boolean ops.
OP_DUP_FCN (0.0 ==, mx_inline_not, double, double)
OP_DUP_FCN (0.0 ==, mx_inline_not, double, Complex)

OP_DUP_FCN (, mx_inline_make_complex, Complex, double)

OP_DUP_FCN (-, mx_inline_change_sign, double,  double)
OP_DUP_FCN (-, mx_inline_change_sign, Complex, Complex)

OP_DUP_FCN (std::abs, mx_inline_fabs_dup, double,  double)
OP_DUP_FCN (std::abs, mx_inline_cabs_dup, double,  Complex)
OP_DUP_FCN (real, mx_inline_real_dup, double,  Complex)
OP_DUP_FCN (imag, mx_inline_imag_dup, double,  Complex)
OP_DUP_FCN (conj, mx_inline_conj_dup, Complex, Complex)

OP_DUP_FCN (0.0 ==, mx_inline_not, float, float)
OP_DUP_FCN (static_cast<float>(0.0) ==, mx_inline_not, float, FloatComplex)

OP_DUP_FCN (, mx_inline_make_complex, FloatComplex, float)

OP_DUP_FCN (-, mx_inline_change_sign, float,  float)
OP_DUP_FCN (-, mx_inline_change_sign, FloatComplex, FloatComplex)

OP_DUP_FCN (std::abs, mx_inline_fabs_dup, float,  float)
OP_DUP_FCN (std::abs, mx_inline_cabs_dup, float,  FloatComplex)
OP_DUP_FCN (real, mx_inline_real_dup, float,  FloatComplex)
OP_DUP_FCN (imag, mx_inline_imag_dup, float,  FloatComplex)
OP_DUP_FCN (conj, mx_inline_conj_dup, FloatComplex, FloatComplex)

// FIXME: Due to a performance defect in g++ (<= 4.3), std::norm is slow unless
// ffast-math is on (not by default even with -O3). The following helper function
// gives the expected straightforward implementation of std::norm.
template <class T>
inline T cabsq (const std::complex<T>& c) 
{ return c.real () * c.real () + c.imag () * c.imag (); }

// default. works for integers and bool.
template <class T>
inline bool xis_true (T x) { return x; }
template <class T>
inline bool xis_false (T x) { return ! x; }
// for octave_ints
template <class T>
inline bool xis_true (const octave_int<T>& x) { return x.value (); }
template <class T>
inline bool xis_false (const octave_int<T>& x) { return ! x.value (); }
// for reals, we want to ignore NaNs.
inline bool xis_true (double x) { return ! xisnan (x) && x != 0.0; }
inline bool xis_false (double x) { return x == 0.0; }
inline bool xis_true (float x) { return ! xisnan (x) && x != 0.0f; }
inline bool xis_false (float x) { return x == 0.0f; }
// Ditto for complex.
inline bool xis_true (const Complex& x) { return ! xisnan (x) && x != 0.0; }
inline bool xis_false (const Complex& x) { return x == 0.0; }
inline bool xis_true (const FloatComplex& x) { return ! xisnan (x) && x != 0.0f; }
inline bool xis_false (const FloatComplex& x) { return x == 0.0f; }

#define OP_RED_SUM(ac, el) ac += el
#define OP_RED_PROD(ac, el) ac *= el
#define OP_RED_SUMSQ(ac, el) ac += el*el
#define OP_RED_SUMSQC(ac, el) ac += cabsq (el)

inline void op_dble_sum(double& ac, float el)
{ ac += el; }
inline void op_dble_sum(Complex& ac, const FloatComplex& el)
{ ac += el; } // FIXME: guaranteed?
template <class T>
inline void op_dble_sum(double& ac, const octave_int<T>& el)
{ ac += el.double_value (); }

// The following two implement a simple short-circuiting.
#define OP_RED_ANYC(ac, el) if (xis_true (el)) { ac = true; break; } else continue
#define OP_RED_ALLC(ac, el) if (xis_false (el)) { ac = false; break; } else continue

#define OP_RED_FCN(F, TSRC, TRES, OP, ZERO) \
template <class T> \
inline TRES \
F (const TSRC* v, octave_idx_type n) \
{ \
  TRES ac = ZERO; \
  for (octave_idx_type i = 0; i < n; i++) \
    OP(ac, v[i]); \
  return ac; \
}

#define PROMOTE_DOUBLE(T) typename subst_template_param<std::complex, T, double>::type

OP_RED_FCN (mx_inline_sum, T, T, OP_RED_SUM, 0)
OP_RED_FCN (mx_inline_dsum, T, PROMOTE_DOUBLE(T), op_dble_sum, 0.0)
OP_RED_FCN (mx_inline_count, bool, T, OP_RED_SUM, 0)
OP_RED_FCN (mx_inline_prod, T, T, OP_RED_PROD, 1)
OP_RED_FCN (mx_inline_sumsq, T, T, OP_RED_SUMSQ, 0)
OP_RED_FCN (mx_inline_sumsq, std::complex<T>, T, OP_RED_SUMSQC, 0)
OP_RED_FCN (mx_inline_any, T, bool, OP_RED_ANYC, false)
OP_RED_FCN (mx_inline_all, T, bool, OP_RED_ALLC, true)


#define OP_RED_FCN2(F, TSRC, TRES, OP, ZERO) \
template <class T> \
inline void \
F (const TSRC* v, TRES *r, octave_idx_type m, octave_idx_type n) \
{ \
  for (octave_idx_type i = 0; i < m; i++) \
    r[i] = ZERO; \
  for (octave_idx_type j = 0; j < n; j++) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        OP(r[i], v[i]); \
      v += m; \
    } \
}

OP_RED_FCN2 (mx_inline_sum, T, T, OP_RED_SUM, 0)
OP_RED_FCN2 (mx_inline_dsum, T, PROMOTE_DOUBLE(T), op_dble_sum, 0.0)
OP_RED_FCN2 (mx_inline_count, bool, T, OP_RED_SUM, 0)
OP_RED_FCN2 (mx_inline_prod, T, T, OP_RED_PROD, 1)
OP_RED_FCN2 (mx_inline_sumsq, T, T, OP_RED_SUMSQ, 0)
OP_RED_FCN2 (mx_inline_sumsq, std::complex<T>, T, OP_RED_SUMSQC, 0)

// Using the general code for any/all would sacrifice short-circuiting.
// OTOH, going by rows would sacrifice cache-coherence. The following algorithm
// will achieve both, at the cost of a temporary octave_idx_type array.

#define OP_ROW_SHORT_CIRCUIT(F, PRED, ZERO) \
template <class T> \
inline void \
F (const T* v, bool *r, octave_idx_type m, octave_idx_type n) \
{ \
  /* FIXME: it may be sub-optimal to allocate the buffer here. */ \
  OCTAVE_LOCAL_BUFFER (octave_idx_type, iact, m); \
  for (octave_idx_type i = 0; i < m; i++) iact[i] = i; \
  octave_idx_type nact = m; \
  for (octave_idx_type j = 0; j < n; j++) \
    { \
      octave_idx_type k = 0; \
      for (octave_idx_type i = 0; i < nact; i++) \
        { \
          octave_idx_type ia = iact[i]; \
          if (! PRED (v[ia])) \
            iact[k++] = ia; \
        } \
      nact = k; \
      v += m; \
    } \
  for (octave_idx_type i = 0; i < m; i++) r[i] = ! ZERO; \
  for (octave_idx_type i = 0; i < nact; i++) r[iact[i]] = ZERO; \
}

OP_ROW_SHORT_CIRCUIT (mx_inline_any, xis_true, false)
OP_ROW_SHORT_CIRCUIT (mx_inline_all, xis_false, true)

#define OP_RED_FCNN(F, TSRC, TRES) \
template <class T> \
inline void \
F (const TSRC *v, TRES *r, octave_idx_type l, \
   octave_idx_type n, octave_idx_type u) \
{ \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          r[i] = F<T> (v, n); \
          v += n; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, l, n); \
          v += l*n; \
          r += l; \
        } \
    } \
}

OP_RED_FCNN (mx_inline_sum, T, T)
OP_RED_FCNN (mx_inline_dsum, T, PROMOTE_DOUBLE(T))
OP_RED_FCNN (mx_inline_count, bool, T)
OP_RED_FCNN (mx_inline_prod, T, T)
OP_RED_FCNN (mx_inline_sumsq, T, T)
OP_RED_FCNN (mx_inline_sumsq, std::complex<T>, T)
OP_RED_FCNN (mx_inline_any, T, bool)
OP_RED_FCNN (mx_inline_all, T, bool)

#define OP_CUM_FCN(F, TSRC, TRES, OP) \
template <class T> \
inline void \
F (const TSRC *v, TRES *r, octave_idx_type n) \
{ \
  if (n) \
    { \
      TRES t = r[0] = v[0]; \
      for (octave_idx_type i = 1; i < n; i++) \
        r[i] = t = t OP v[i]; \
    } \
}

OP_CUM_FCN (mx_inline_cumsum, T, T, +)
OP_CUM_FCN (mx_inline_cumprod, T, T, *)
OP_CUM_FCN (mx_inline_cumcount, bool, T, +)

#define OP_CUM_FCN2(F, TSRC, TRES, OP) \
template <class T> \
inline void \
F (const TSRC *v, TRES *r, octave_idx_type m, octave_idx_type n) \
{ \
  if (n) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        r[i] = v[i]; \
      const T *r0 = r; \
      for (octave_idx_type j = 1; j < n; j++) \
        { \
          r += m; v += m; \
          for (octave_idx_type i = 0; i < m; i++) \
            r[i] = r0[i] OP v[i]; \
          r0 += m; \
        } \
    } \
}

OP_CUM_FCN2 (mx_inline_cumsum, T, T, +)
OP_CUM_FCN2 (mx_inline_cumprod, T, T, *)
OP_CUM_FCN2 (mx_inline_cumcount, bool, T, *)

#define OP_CUM_FCNN(F, TSRC, TRES) \
template <class T> \
inline void \
F (const TSRC *v, TRES *r, octave_idx_type l, \
   octave_idx_type n, octave_idx_type u) \
{ \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, n); \
          v += n; r += n; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, l, n); \
          v += l*n; \
          r += l*n; \
        } \
    } \
}

OP_CUM_FCNN (mx_inline_cumsum, T, T)
OP_CUM_FCNN (mx_inline_cumprod, T, T)
OP_CUM_FCNN (mx_inline_cumcount, bool, T)

#define OP_MINMAX_FCN(F, OP) \
template <class T> \
void F (const T *v, T *r, octave_idx_type n) \
{ \
  if (! n) return; \
  T tmp = v[0]; \
  octave_idx_type i = 1; \
  if (xisnan (tmp)) \
    { \
      for (; i < n && xisnan (v[i]); i++) ; \
      if (i < n) tmp = v[i]; \
    } \
  for (; i < n; i++) \
    if (v[i] OP tmp) tmp = v[i]; \
  *r = tmp; \
} \
template <class T> \
void F (const T *v, T *r, octave_idx_type *ri, octave_idx_type n) \
{ \
  if (! n) return; \
  T tmp = v[0]; \
  octave_idx_type tmpi = 0; \
  octave_idx_type i = 1; \
  if (xisnan (tmp)) \
    { \
      for (; i < n && xisnan (v[i]); i++) ; \
      if (i < n) { tmp = v[i]; tmpi = i; } \
    } \
  for (; i < n; i++) \
    if (v[i] OP tmp) { tmp = v[i]; tmpi = i; }\
  *r = tmp; \
  *ri = tmpi; \
}

OP_MINMAX_FCN (mx_inline_min, <)
OP_MINMAX_FCN (mx_inline_max, >)

// Row reductions will be slightly complicated.  We will proceed with checks
// for NaNs until we detect that no row will yield a NaN, in which case we
// proceed to a faster code.

#define OP_MINMAX_FCN2(F, OP) \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type m, octave_idx_type n) \
{ \
  if (! n) return; \
  bool nan = false; \
  octave_idx_type j = 0; \
  for (octave_idx_type i = 0; i < m; i++) \
    {  \
      r[i] = v[i]; \
      if (xisnan (v[i])) nan = true;  \
    } \
  j++; v += m; \
  while (nan && j < n) \
    { \
      nan = false; \
      for (octave_idx_type i = 0; i < m; i++) \
        {  \
          if (xisnan (v[i])) \
            nan = true;  \
          else if (xisnan (r[i]) || v[i] OP r[i]) \
            r[i] = v[i]; \
        } \
      j++; v += m; \
    } \
  while (j < n) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        if (v[i] OP r[i]) r[i] = v[i]; \
      j++; v += m; \
    } \
} \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type *ri, \
   octave_idx_type m, octave_idx_type n) \
{ \
  if (! n) return; \
  bool nan = false; \
  octave_idx_type j = 0; \
  for (octave_idx_type i = 0; i < m; i++) \
    {  \
      r[i] = v[i]; ri[i] = j; \
      if (xisnan (v[i])) nan = true;  \
    } \
  j++; v += m; \
  while (nan && j < n) \
    { \
      nan = false; \
      for (octave_idx_type i = 0; i < m; i++) \
        {  \
          if (xisnan (v[i])) \
            nan = true;  \
          else if (xisnan (r[i]) || v[i] OP r[i]) \
            { r[i] = v[i]; ri[i] = j; } \
        } \
      j++; v += m; \
    } \
  while (j < n) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        if (v[i] OP r[i]) \
          { r[i] = v[i]; ri[i] = j; } \
      j++; v += m; \
    } \
}

OP_MINMAX_FCN2 (mx_inline_min, <)
OP_MINMAX_FCN2 (mx_inline_max, >)

#define OP_MINMAX_FCNN(F) \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type l, \
   octave_idx_type n, octave_idx_type u) \
{ \
  if (! n) return; \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, n); \
          v += n; r++; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, l, n); \
          v += l*n; \
          r += l; \
        } \
    } \
} \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type *ri, \
   octave_idx_type l, octave_idx_type n, octave_idx_type u) \
{ \
  if (! n) return; \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, ri, n); \
          v += n; r++; ri++; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, ri, l, n); \
          v += l*n; \
          r += l; ri += l; \
        } \
    } \
}

OP_MINMAX_FCNN (mx_inline_min)
OP_MINMAX_FCNN (mx_inline_max)

#define OP_CUMMINMAX_FCN(F, OP) \
template <class T> \
void F (const T *v, T *r, octave_idx_type n) \
{ \
  if (! n) return; \
  T tmp = v[0]; \
  octave_idx_type i = 1, j = 0; \
  if (xisnan (tmp)) \
    { \
      for (; i < n && xisnan (v[i]); i++) ; \
      for (; j < i; j++) r[j] = tmp; \
      if (i < n) tmp = v[i]; \
    } \
  for (; i < n; i++) \
    if (v[i] OP tmp) \
      { \
        for (; j < i; j++) r[j] = tmp; \
        tmp = v[i]; \
      } \
  for (; j < i; j++) r[j] = tmp; \
} \
template <class T> \
void F (const T *v, T *r, octave_idx_type *ri, octave_idx_type n) \
{ \
  if (! n) return; \
  T tmp = v[0]; octave_idx_type tmpi = 0; \
  octave_idx_type i = 1, j = 0; \
  if (xisnan (tmp)) \
    { \
      for (; i < n && xisnan (v[i]); i++) ; \
      for (; j < i; j++) { r[j] = tmp; ri[j] = tmpi; } \
      if (i < n) { tmp = v[i]; tmpi = i; } \
    } \
  for (; i < n; i++) \
    if (v[i] OP tmp) \
      { \
        for (; j < i; j++) { r[j] = tmp; ri[j] = tmpi; } \
        tmp = v[i]; tmpi = i; \
      } \
  for (; j < i; j++) { r[j] = tmp; ri[j] = tmpi; } \
}

OP_CUMMINMAX_FCN (mx_inline_cummin, <)
OP_CUMMINMAX_FCN (mx_inline_cummax, >)

// Row reductions will be slightly complicated.  We will proceed with checks
// for NaNs until we detect that no row will yield a NaN, in which case we
// proceed to a faster code.

#define OP_CUMMINMAX_FCN2(F, OP) \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type m, octave_idx_type n) \
{ \
  if (! n) return; \
  bool nan = false; \
  const T *r0; \
  octave_idx_type j = 0; \
  for (octave_idx_type i = 0; i < m; i++) \
    {  \
      r[i] = v[i]; \
      if (xisnan (v[i])) nan = true;  \
    } \
  j++; v += m; r0 = r; r += m; \
  while (nan && j < n) \
    { \
      nan = false; \
      for (octave_idx_type i = 0; i < m; i++) \
        {  \
          if (xisnan (v[i])) \
            { r[i] = r0[i]; nan = true; } \
          else if (xisnan (r0[i]) || v[i] OP r0[i]) \
            r[i] = v[i]; \
        } \
      j++; v += m; r0 = r; r += m; \
    } \
  while (j < n) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        if (v[i] OP r0[i]) \
          r[i] = v[i]; \
        else \
          r[i] = r0[i]; \
      j++; v += m; r0 = r; r += m; \
    } \
} \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type *ri, \
   octave_idx_type m, octave_idx_type n) \
{ \
  if (! n) return; \
  bool nan = false; \
  const T *r0; const octave_idx_type *r0i; \
  octave_idx_type j = 0; \
  for (octave_idx_type i = 0; i < m; i++) \
    {  \
      r[i] = v[i]; ri[i] = 0; \
      if (xisnan (v[i])) nan = true;  \
    } \
  j++; v += m; r0 = r; r += m; r0i = ri; ri += m;  \
  while (nan && j < n) \
    { \
      nan = false; \
      for (octave_idx_type i = 0; i < m; i++) \
        {  \
          if (xisnan (v[i])) \
            { r[i] = r0[i]; ri[i] = r0i[i]; nan = true; } \
          else if (xisnan (r0[i]) || v[i] OP r0[i]) \
            { r[i] = v[i]; ri[i] = j; }\
        } \
      j++; v += m; r0 = r; r += m; r0i = ri; ri += m;  \
    } \
  while (j < n) \
    { \
      for (octave_idx_type i = 0; i < m; i++) \
        if (v[i] OP r0[i]) \
          { r[i] = v[i]; ri[i] = j; } \
        else \
          { r[i] = r0[i]; ri[i] = r0i[i]; } \
      j++; v += m; r0 = r; r += m; r0i = ri; ri += m;  \
    } \
}

OP_CUMMINMAX_FCN2 (mx_inline_cummin, <)
OP_CUMMINMAX_FCN2 (mx_inline_cummax, >)

#define OP_CUMMINMAX_FCNN(F) \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type l, \
   octave_idx_type n, octave_idx_type u) \
{ \
  if (! n) return; \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, n); \
          v += n; r += n; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, l, n); \
          v += l*n; \
          r += l*n; \
        } \
    } \
} \
template <class T> \
inline void \
F (const T *v, T *r, octave_idx_type *ri, \
   octave_idx_type l, octave_idx_type n, octave_idx_type u) \
{ \
  if (! n) return; \
  if (l == 1) \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, ri, n); \
          v += n; r += n; ri += n; \
        } \
    } \
  else \
    { \
      for (octave_idx_type i = 0; i < u; i++) \
        { \
          F (v, r, ri, l, n); \
          v += l*n; \
          r += l*n; ri += l*n; \
        } \
    } \
}

OP_CUMMINMAX_FCNN (mx_inline_cummin)
OP_CUMMINMAX_FCNN (mx_inline_cummax)

template <class T>
void mx_inline_diff (const T *v, T *r, octave_idx_type n,
                     octave_idx_type order)
{
  switch (order)
    {
    case 1:
      for (octave_idx_type i = 0; i < n-1; i++)
        r[i] = v[i+1] - v[i];
      break;
    case 2:
      if (n > 1)
	{
	  T lst = v[1] - v[0];
          for (octave_idx_type i = 0; i < n-2; i++)
            {
              T dif = v[i+2] - v[i+1];
              r[i] = dif - lst;
              lst = dif;
            }
        }
      break;
    default:
        {
          OCTAVE_LOCAL_BUFFER (T, buf, n-1);

          for (octave_idx_type i = 0; i < n-1; i++)
            buf[i] = v[i+1] - v[i];

          for (octave_idx_type o = 2; o <= order; o++)
            {
              for (octave_idx_type i = 0; i < n-o; i++)
                buf[i] = buf[i+1] - buf[i];
            }

          for (octave_idx_type i = 0; i < n-order; i++)
            r[i] = buf[i];
        }
    }
}

template <class T>
void mx_inline_diff (const T *v, T *r, 
                     octave_idx_type m, octave_idx_type n,
                     octave_idx_type order)
{
  switch (order)
    {
    case 1:
      for (octave_idx_type i = 0; i < m*(n-1); i++)
        r[i] = v[i+m] - v[i];
      break;
    case 2:
      for (octave_idx_type i = 0; i < n-2; i++)
        {
          for (octave_idx_type j = i*m; j < i*m+m; j++)
            r[j] = (v[j+m+m] - v[j+m]) + (v[j+m] - v[j]);
        }
      break;
    default:
        {
          OCTAVE_LOCAL_BUFFER (T, buf, n-1);

          for (octave_idx_type j = 0; j < m; j++)
            {
              for (octave_idx_type i = 0; i < n-1; i++)
                buf[i] = v[i*m+j+m] - v[i*m+j];

              for (octave_idx_type o = 2; o <= order; o++)
                {
                  for (octave_idx_type i = 0; i < n-o; i++)
                    buf[i] = buf[i+1] - buf[i];
                }

              for (octave_idx_type i = 0; i < n-order; i++)
                r[i*m+j] = buf[i];
            }
        }
    }
}

template <class T>
inline void
mx_inline_diff (const T *v, T *r,
                octave_idx_type l, octave_idx_type n, octave_idx_type u,
                octave_idx_type order)
{
  if (! n) return;
  if (l == 1)
    {
      for (octave_idx_type i = 0; i < u; i++)
        {
          mx_inline_diff (v, r, n, order);
          v += n; r += n-order;
        }
    }
  else
    {
      for (octave_idx_type i = 0; i < u; i++)
        {
          mx_inline_diff (v, r, l, n, order);
          v += l*n;
          r += l*(n-order);
        }
    }
}

// Assistant function

inline void
get_extent_triplet (const dim_vector& dims, int& dim,
                    octave_idx_type& l, octave_idx_type& n,
                    octave_idx_type& u)
{
  octave_idx_type ndims = dims.length ();
  if (dim >= ndims)
    {
      l = dims.numel ();
      n = 1;
      u = 1;
    }
  else
    {
      if (dim < 0)
        dim = dims.first_non_singleton ();

      // calculate extent triplet.
      l = 1, n = dims(dim), u = 1;
      for (octave_idx_type i = 0; i < dim; i++) 
        l *= dims (i);
      for (octave_idx_type i = dim + 1; i < ndims; i++)
        u *= dims (i);
    }
}

// Appliers.
// FIXME: is this the best design? C++ gives a lot of options here...
// maybe it can be done without an explicit parameter?

template <class ArrayType, class T>
inline ArrayType
do_mx_red_op (const Array<T>& src, int dim,
              void (*mx_red_op) (const T *, typename ArrayType::element_type *,
                                 octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  // M*b inconsistency: sum([]) = 0 etc.
  if (dims.length () == 2 && dims(0) == 0 && dims(1) == 0)
    dims (1) = 1;

  get_extent_triplet (dims, dim, l, n, u);

  // Reduction operation reduces the array size.
  if (dim < dims.length ()) dims(dim) = 1;
  dims.chop_trailing_singletons ();

  ArrayType ret (dims);
  mx_red_op (src.data (), ret.fortran_vec (), l, n, u);

  return ret;
}

template <class ArrayType, class T>
inline ArrayType
do_mx_cum_op (const Array<T>& src, int dim,
              void (*mx_cum_op) (const T *, typename ArrayType::element_type *,
                                 octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  get_extent_triplet (dims, dim, l, n, u);

  // Cumulative operation doesn't reduce the array size.
  ArrayType ret (dims);
  mx_cum_op (src.data (), ret.fortran_vec (), l, n, u);

  return ret;
}

template <class ArrayType>
inline ArrayType
do_mx_minmax_op (const ArrayType& src, int dim,
                 void (*mx_minmax_op) (const typename ArrayType::element_type *, 
                                       typename ArrayType::element_type *,
                                       octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  get_extent_triplet (dims, dim, l, n, u);

  // If the dimension is zero, we don't do anything.
  if (dim < dims.length () && dims(dim) != 0) dims(dim) = 1;
  dims.chop_trailing_singletons ();

  ArrayType ret (dims);
  mx_minmax_op (src.data (), ret.fortran_vec (), l, n, u);

  return ret;
}

template <class ArrayType>
inline ArrayType
do_mx_minmax_op (const ArrayType& src, Array<octave_idx_type>& idx, int dim,
                 void (*mx_minmax_op) (const typename ArrayType::element_type *, 
                                       typename ArrayType::element_type *,
                                       octave_idx_type *,
                                       octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  get_extent_triplet (dims, dim, l, n, u);

  // If the dimension is zero, we don't do anything.
  if (dim < dims.length () && dims(dim) != 0) dims(dim) = 1;
  dims.chop_trailing_singletons ();

  ArrayType ret (dims);
  if (idx.dims () != dims) idx = Array<octave_idx_type> (dims);

  mx_minmax_op (src.data (), ret.fortran_vec (), idx.fortran_vec (),
                l, n, u);

  return ret;
}

template <class ArrayType>
inline ArrayType
do_mx_cumminmax_op (const ArrayType& src, int dim,
                    void (*mx_cumminmax_op) (const typename ArrayType::element_type *, 
                                             typename ArrayType::element_type *,
                                             octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  get_extent_triplet (dims, dim, l, n, u);

  ArrayType ret (dims);
  mx_cumminmax_op (src.data (), ret.fortran_vec (), l, n, u);

  return ret;
}

template <class ArrayType>
inline ArrayType
do_mx_cumminmax_op (const ArrayType& src, Array<octave_idx_type>& idx, int dim,
                    void (*mx_cumminmax_op) (const typename ArrayType::element_type *, 
                                             typename ArrayType::element_type *,
                                             octave_idx_type *,
                                             octave_idx_type, octave_idx_type, octave_idx_type))
{
  octave_idx_type l, n, u;
  dim_vector dims = src.dims ();
  get_extent_triplet (dims, dim, l, n, u);

  ArrayType ret (dims);
  if (idx.dims () != dims) idx = Array<octave_idx_type> (dims);

  mx_cumminmax_op (src.data (), ret.fortran_vec (), idx.fortran_vec (),
                   l, n, u);

  return ret;
}

template <class ArrayType>
inline ArrayType
do_mx_diff_op (const ArrayType& src, int dim, octave_idx_type order,
               void (*mx_diff_op) (const typename ArrayType::element_type *, 
                                   typename ArrayType::element_type *,
                                   octave_idx_type, octave_idx_type, octave_idx_type,
                                   octave_idx_type))
{
  octave_idx_type l, n, u;
  if (order <= 0)
    return src;

  dim_vector dims = src.dims ();

  get_extent_triplet (dims, dim, l, n, u);
  if (dim >= dims.length ())
    dims.resize (dim+1, 1);

  if (dims(dim) <= order)
    {
      dims (dim) = 0;
      return ArrayType (dims);
    }
  else
    {
      dims(dim) -= order;
    }

  ArrayType ret (dims);
  mx_diff_op (src.data (), ret.fortran_vec (), l, n, u, order);

  return ret;
}

// Fast extra-precise summation. According to
// T. Ogita, S. M. Rump, S. Oishi:
// Accurate Sum And Dot Product,
// SIAM J. Sci. Computing, Vol. 26, 2005

template <class T>
inline void twosum_accum (T& s, T& e, 
                          const T& x)
{
  T s1 = s + x, t = s1 - s, e1 = (s - (s1 - t)) + (x - t);
  s = s1;
  e += e1;
}

template <class T>
inline T
mx_inline_xsum (const T *v, octave_idx_type n) 
{
  T s = 0, e = 0;
  for (octave_idx_type i = 0; i < n; i++)
    twosum_accum (s, e, v[i]);

  return s + e;
}

template <class T>
inline void
mx_inline_xsum (const T *v, T *r, 
                octave_idx_type m, octave_idx_type n)
{
  OCTAVE_LOCAL_BUFFER (T, e, m);
  for (octave_idx_type i = 0; i < m; i++)
    e[i] = r[i] = T ();

  for (octave_idx_type j = 0; j < n; j++)
    {
      for (octave_idx_type i = 0; i < m; i++)
        twosum_accum (r[i], e[i], v[i]);

      v += m;
    }

  for (octave_idx_type i = 0; i < m; i++)
    r[i] += e[i];
}

OP_RED_FCNN (mx_inline_xsum, T, T)

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
