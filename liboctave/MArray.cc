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

#include "MArray.h"
#include "Array-util.h"
#include "lo-error.h"

#include "MArray-defs.h"

// One dimensional array with math ops.

template <class T>
double
MArray<T>::norm (double) const
{
  (*current_liboctave_error_handler)
    ("norm: only implemented for double and complex values");

  return 0;
}

// Element by element MArray by scalar ops.

template <class T>
MArray<T>&
operator += (MArray<T>& a, const T& s)
{
  DO_VS_OP2 (T, a, +=, s)
  return a;
}

template <class T>
MArray<T>&
operator -= (MArray<T>& a, const T& s)
{
  DO_VS_OP2 (T, a, -=, s)
  return a;
}

// Element by element MArray by MArray ops.

template <class T>
MArray<T>&
operator += (MArray<T>& a, const MArray<T>& b)
{
  octave_idx_type l = a.length ();
  if (l > 0)
    {
      octave_idx_type bl = b.length ();
      if (l != bl)
	gripe_nonconformant ("operator +=", l, bl);
      else
	DO_VV_OP2 (T, a, +=, b);
    }
  return a;
}

template <class T>
MArray<T>&
operator -= (MArray<T>& a, const MArray<T>& b)
{
  octave_idx_type l = a.length ();
  if (l > 0)
    {
      octave_idx_type bl = b.length ();
      if (l != bl)
	gripe_nonconformant ("operator -=", l, bl);
      else
	DO_VV_OP2 (T, a, -=, b);
    }
  return a;
}

// Element by element MArray by scalar ops.

#define MARRAY_AS_OP(OP) \
  template <class T> \
  MArray<T> \
  operator OP (const MArray<T>& a, const T& s) \
  { \
    MArray<T> result (a.length ()); \
    T *r = result.fortran_vec (); \
    octave_idx_type l = a.length (); \
    const T *v = a.data (); \
    DO_VS_OP (r, l, v, OP, s); \
    return result; \
  }

MARRAY_AS_OP (+)
MARRAY_AS_OP (-)
MARRAY_AS_OP (*)
MARRAY_AS_OP (/)

// Element by element scalar by MArray ops.

#define MARRAY_SA_OP(OP) \
  template <class T> \
  MArray<T> \
  operator OP (const T& s, const MArray<T>& a) \
  { \
    MArray<T> result (a.length ()); \
    T *r = result.fortran_vec (); \
    octave_idx_type l = a.length (); \
    const T *v = a.data (); \
    DO_SV_OP (r, l, s, OP, v); \
    return result; \
  }

MARRAY_SA_OP(+)
MARRAY_SA_OP(-)
MARRAY_SA_OP(*)
MARRAY_SA_OP(/)

// Element by element MArray by MArray ops.

#define MARRAY_AA_OP(FCN, OP) \
  template <class T> \
  MArray<T> \
  FCN (const MArray<T>& a, const MArray<T>& b) \
  { \
    octave_idx_type l = a.length (); \
    octave_idx_type bl = b.length (); \
    if (l != bl) \
      { \
	gripe_nonconformant (#FCN, l, bl); \
	return MArray<T> (); \
      } \
    if (l == 0) \
      return MArray<T> (); \
    MArray<T> result (l); \
    T *r = result.fortran_vec (); \
    const T *x = a.data (); \
    const T *y = b.data (); \
    DO_VV_OP (r, l, x, OP, y); \
    return result; \
  }

MARRAY_AA_OP (operator +, +)
MARRAY_AA_OP (operator -, -)
MARRAY_AA_OP (product,    *)
MARRAY_AA_OP (quotient,   /)

// Unary MArray ops.

template <class T>
MArray<T>
operator + (const MArray<T>& a)
{
  return a;
}

template <class T>
MArray<T>
operator - (const MArray<T>& a)
{
  octave_idx_type l = a.length ();
  MArray<T> result (l);
  T *r = result.fortran_vec ();
  const T *x = a.data ();
  NEG_V (r, l, x);
  return result;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
