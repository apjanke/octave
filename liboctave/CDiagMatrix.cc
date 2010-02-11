// DiagMatrix manipulations.
/*

Copyright (C) 1994, 1995, 1996, 1997, 2000, 2001, 2002, 2003, 2004,
              2005, 2007, 2008, 2009 John W. Eaton
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include "Array-util.h"
#include "lo-error.h"
#include "lo-ieee.h"
#include "mx-base.h"
#include "mx-inlines.cc"
#include "oct-cmplx.h"

// Complex Diagonal Matrix class

ComplexDiagMatrix::ComplexDiagMatrix (const DiagMatrix& a)
  : MDiagArray2<Complex> (a.rows (), a.cols ())
{
  for (octave_idx_type i = 0; i < length (); i++)
    elem (i, i) = a.elem (i, i);
}

bool
ComplexDiagMatrix::operator == (const ComplexDiagMatrix& a) const
{
  if (rows () != a.rows () || cols () != a.cols ())
    return 0;

  return mx_inline_equal (length (), data (), a.data ());
}

bool
ComplexDiagMatrix::operator != (const ComplexDiagMatrix& a) const
{
  return !(*this == a);
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (double val)
{
  for (octave_idx_type i = 0; i < length (); i++)
    elem (i, i) = val;
  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const Complex& val)
{
  for (octave_idx_type i = 0; i < length (); i++)
    elem (i, i) = val;
  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (double val, octave_idx_type beg, octave_idx_type end)
{
  if (beg < 0 || end >= length () || end < beg)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = beg; i <= end; i++)
    elem (i, i) = val;

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const Complex& val, octave_idx_type beg, octave_idx_type end)
{
  if (beg < 0 || end >= length () || end < beg)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = beg; i <= end; i++)
    elem (i, i) = val;

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ColumnVector& a)
{
  octave_idx_type len = length ();
  if (a.length () != len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < len; i++)
    elem (i, i) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ComplexColumnVector& a)
{
  octave_idx_type len = length ();
  if (a.length () != len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < len; i++)
    elem (i, i) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const RowVector& a)
{
  octave_idx_type len = length ();
  if (a.length () != len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < len; i++)
    elem (i, i) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ComplexRowVector& a)
{
  octave_idx_type len = length ();
  if (a.length () != len)
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < len; i++)
    elem (i, i) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ColumnVector& a, octave_idx_type beg)
{
  octave_idx_type a_len = a.length ();
  if (beg < 0 || beg + a_len >= length ())
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < a_len; i++)
    elem (i+beg, i+beg) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ComplexColumnVector& a, octave_idx_type beg)
{
  octave_idx_type a_len = a.length ();
  if (beg < 0 || beg + a_len >= length ())
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < a_len; i++)
    elem (i+beg, i+beg) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const RowVector& a, octave_idx_type beg)
{
  octave_idx_type a_len = a.length ();
  if (beg < 0 || beg + a_len >= length ())
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < a_len; i++)
    elem (i+beg, i+beg) = a.elem (i);

  return *this;
}

ComplexDiagMatrix&
ComplexDiagMatrix::fill (const ComplexRowVector& a, octave_idx_type beg)
{
  octave_idx_type a_len = a.length ();
  if (beg < 0 || beg + a_len >= length ())
    {
      (*current_liboctave_error_handler) ("range error for fill");
      return *this;
    }

  for (octave_idx_type i = 0; i < a_len; i++)
    elem (i+beg, i+beg) = a.elem (i);

  return *this;
}

DiagMatrix
ComplexDiagMatrix::abs (void) const
{
  DiagMatrix retval (rows (), cols ());
  for (octave_idx_type i = 0; i < rows (); i++)
    retval(i, i) = std::abs (elem (i, i));
  return retval;
}

ComplexDiagMatrix
conj (const ComplexDiagMatrix& a)
{
  ComplexDiagMatrix retval;
  octave_idx_type a_len = a.length ();
  if (a_len > 0)
    retval = ComplexDiagMatrix (mx_inline_conj_dup (a.data (), a_len),
                                a.rows (), a.cols ());
  return retval;
}

// resize is the destructive analog for this one

ComplexMatrix
ComplexDiagMatrix::extract (octave_idx_type r1, octave_idx_type c1, octave_idx_type r2, octave_idx_type c2) const
{
  if (r1 > r2) { octave_idx_type tmp = r1; r1 = r2; r2 = tmp; }
  if (c1 > c2) { octave_idx_type tmp = c1; c1 = c2; c2 = tmp; }

  octave_idx_type new_r = r2 - r1 + 1;
  octave_idx_type new_c = c2 - c1 + 1;

  ComplexMatrix result (new_r, new_c);

  for (octave_idx_type j = 0; j < new_c; j++)
    for (octave_idx_type i = 0; i < new_r; i++)
      result.elem (i, j) = elem (r1+i, c1+j);

  return result;
}

// extract row or column i.

ComplexRowVector
ComplexDiagMatrix::row (octave_idx_type i) const
{
  octave_idx_type r = rows ();
  octave_idx_type c = cols ();
  if (i < 0 || i >= r)
    {
      (*current_liboctave_error_handler) ("invalid row selection");
      return ComplexRowVector (); 
    }

  ComplexRowVector retval (c, 0.0);
  if (r <= c || (r > c && i < c))
    retval.elem (i) = elem (i, i);

  return retval;
}

ComplexRowVector
ComplexDiagMatrix::row (char *s) const
{
  if (! s)
    {
      (*current_liboctave_error_handler) ("invalid row selection");
      return ComplexRowVector (); 
    }

  char c = *s;
  if (c == 'f' || c == 'F')
    return row (static_cast<octave_idx_type>(0));
  else if (c == 'l' || c == 'L')
    return row (rows () - 1);
  else
    {
      (*current_liboctave_error_handler) ("invalid row selection");
      return ComplexRowVector ();
    }
}

ComplexColumnVector
ComplexDiagMatrix::column (octave_idx_type i) const
{
  octave_idx_type r = rows ();
  octave_idx_type c = cols ();
  if (i < 0 || i >= c)
    {
      (*current_liboctave_error_handler) ("invalid column selection");
      return ComplexColumnVector (); 
    }

  ComplexColumnVector retval (r, 0.0);
  if (r >= c || (r < c && i < r))
    retval.elem (i) = elem (i, i);

  return retval;
}

ComplexColumnVector
ComplexDiagMatrix::column (char *s) const
{
  if (! s)
    {
      (*current_liboctave_error_handler) ("invalid column selection");
      return ComplexColumnVector (); 
    }

  char c = *s;
  if (c == 'f' || c == 'F')
    return column (static_cast<octave_idx_type>(0));
  else if (c == 'l' || c == 'L')
    return column (cols () - 1);
  else
    {
      (*current_liboctave_error_handler) ("invalid column selection");
      return ComplexColumnVector (); 
    }
}

ComplexDiagMatrix
ComplexDiagMatrix::inverse (void) const
{
  octave_idx_type info;
  return inverse (info);
}

ComplexDiagMatrix
ComplexDiagMatrix::inverse (octave_idx_type& info) const
{
  octave_idx_type r = rows ();
  octave_idx_type c = cols ();
  if (r != c)
    {
      (*current_liboctave_error_handler) ("inverse requires square matrix");
      return ComplexDiagMatrix ();
    }

  ComplexDiagMatrix retval (r, c);

  info = 0;
  for (octave_idx_type i = 0; i < length (); i++)
    {
      if (elem (i, i) == 0.0)
        {
          info = -1;
          return *this;
        }
      else
        retval.elem (i, i) = 1.0 / elem (i, i);
    }

  return retval;
}

ComplexDiagMatrix
ComplexDiagMatrix::pseudo_inverse (void) const
{
  octave_idx_type r = rows ();
  octave_idx_type c = cols ();
  octave_idx_type len = length ();

  ComplexDiagMatrix retval (c, r);

  for (octave_idx_type i = 0; i < len; i++)
    {
      if (elem (i, i) != 0.0)
        retval.elem (i, i) = 1.0 / elem (i, i);
      else
        retval.elem (i, i) = 0.0;
    }

  return retval;
}

bool
ComplexDiagMatrix::all_elements_are_real (void) const
{
  return mx_inline_all_real (length (), data ());
}

// diagonal matrix by diagonal matrix -> diagonal matrix operations

ComplexDiagMatrix&
ComplexDiagMatrix::operator += (const DiagMatrix& a)
{
  octave_idx_type r = rows ();
  octave_idx_type c = cols ();

  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  if (r != a_nr || c != a_nc)
    {
      gripe_nonconformant ("operator +=", r, c, a_nr, a_nc);
      return *this;
    }

  if (r == 0 || c == 0)
    return *this;

  Complex *d = fortran_vec (); // Ensures only one reference to my privates!

  mx_inline_add2 (length (), d, a.data ());
  return *this;
}

ComplexDiagMatrix
operator * (const ComplexDiagMatrix& a, const DiagMatrix& b)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a_nc != b_nr)
    gripe_nonconformant ("operator *", a_nr, a_nc, b_nr, b_nc);

  ComplexDiagMatrix c (a_nr, b_nc);

  octave_idx_type len = c.length (), lenm = len < a_nc ? len : a_nc;

  for (octave_idx_type i = 0; i < lenm; i++)
    c.dgxelem (i) = a.dgelem (i) * b.dgelem (i);
  for (octave_idx_type i = lenm; i < len; i++)
    c.dgxelem (i) = 0.0;

  return c;
}

ComplexDiagMatrix
operator * (const DiagMatrix& a, const ComplexDiagMatrix& b)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a_nc != b_nr)
    {
      gripe_nonconformant ("operator *", a_nr, a_nc, b_nr, b_nc);
      return ComplexDiagMatrix ();
    }

  if (a_nr == 0 || a_nc == 0 || b_nc == 0)
    return ComplexDiagMatrix (a_nr, a_nc, 0.0);

  ComplexDiagMatrix c (a_nr, b_nc);

  octave_idx_type len = a_nr < b_nc ? a_nr : b_nc;

  for (octave_idx_type i = 0; i < len; i++)
    {
      double a_element = a.elem (i, i);
      Complex b_element = b.elem (i, i);

      c.elem (i, i) = a_element * b_element;
    }

  return c;
}

ComplexDiagMatrix
operator * (const ComplexDiagMatrix& a, const ComplexDiagMatrix& b)
{
  octave_idx_type a_nr = a.rows ();
  octave_idx_type a_nc = a.cols ();

  octave_idx_type b_nr = b.rows ();
  octave_idx_type b_nc = b.cols ();

  if (a_nc != b_nr)
    {
      gripe_nonconformant ("operator *", a_nr, a_nc, b_nr, b_nc);
      return ComplexDiagMatrix ();
    }

  if (a_nr == 0 || a_nc == 0 || b_nc == 0)
    return ComplexDiagMatrix (a_nr, a_nc, 0.0);

  ComplexDiagMatrix c (a_nr, b_nc);

  octave_idx_type len = a_nr < b_nc ? a_nr : b_nc;

  for (octave_idx_type i = 0; i < len; i++)
    {
      Complex a_element = a.elem (i, i);
      Complex b_element = b.elem (i, i);

      c.elem (i, i) = a_element * b_element;
    }

  return c;
}

// other operations

ComplexDET
ComplexDiagMatrix::determinant (void) const
{
  ComplexDET det (1.0);
  if (rows () != cols ())
    {
      (*current_liboctave_error_handler) ("determinant requires square matrix");
      det = ComplexDET (0.0);
    }
  else
    {
      octave_idx_type len = length ();
      for (octave_idx_type i = 0; i < len; i++)
        det *= elem (i, i);
    }

  return det;
}

double
ComplexDiagMatrix::rcond (void) const
{
  ColumnVector av = diag (0).map<double> (std::abs);
  double amx = av.max (), amn = av.min ();
  return amx == 0 ? 0.0 : amn / amx;
}

// i/o

std::ostream&
operator << (std::ostream& os, const ComplexDiagMatrix& a)
{
  Complex ZERO (0.0);
//  int field_width = os.precision () + 7;
  for (octave_idx_type i = 0; i < a.rows (); i++)
    {
      for (octave_idx_type j = 0; j < a.cols (); j++)
        {
          if (i == j)
            os << " " /* setw (field_width) */ << a.elem (i, i);
          else
            os << " " /* setw (field_width) */ << ZERO;
        }
      os << "\n";
    }
  return os;
}
