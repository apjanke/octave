// Matrix manipulations.
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

#include <iostream>
#include <string>

#include "lo-error.h"
#include "str-vec.h"
#include "mx-base.h"
#include "mx-inlines.cc"

// charMatrix class.

charMatrix::charMatrix (char c)
  : MArray2<char> ()
{
  octave_idx_type nc = 1;
  octave_idx_type nr = 1;

  resize (nr, nc);

  elem (0, 0) = c;
}

charMatrix::charMatrix (const char *s)
  : MArray2<char> ()
{
  octave_idx_type nc = s ? strlen (s) : 0;
  octave_idx_type nr = s && nc > 0 ? 1 : 0;

  resize (nr, nc);

  for (octave_idx_type i = 0; i < nc; i++)
    elem (0, i) = s[i];
}

charMatrix::charMatrix (const std::string& s)
  : MArray2<char> ()
{
  octave_idx_type nc = s.length ();
  octave_idx_type nr = nc > 0 ? 1 : 0;

  resize (nr, nc);

  for (octave_idx_type i = 0; i < nc; i++)
    elem (0, i) = s[i];
}

charMatrix::charMatrix (const string_vector& s)
  : MArray2<char> (s.length (), s.max_length (), 0)
{
  octave_idx_type nr = rows ();

  for (octave_idx_type i = 0; i < nr; i++)
    {
      octave_idx_type nc = s[i].length ();
      for (octave_idx_type j = 0; j < nc; j++)
	elem (i, j) = s[i][j];
    }
}

bool
charMatrix::operator == (const charMatrix& a) const
{
  if (rows () != a.rows () || cols () != a.cols ())
    return 0;

  return mx_inline_equal (data (), a.data (), length ());
}

bool
charMatrix::operator != (const charMatrix& a) const
{
  return !(*this == a);
}

charMatrix&
charMatrix::insert (const char *s, octave_idx_type r, octave_idx_type c)
{
  if (s)
    {
      octave_idx_type s_len = strlen (s);

      if (r < 0 || r >= rows () || c < 0 || c + s_len - 1 > cols ())
	{
	  (*current_liboctave_error_handler) ("range error for insert");
	  return *this;
	}

      for (octave_idx_type i = 0; i < s_len; i++)
	elem (r, c+i) = s[i];
    }
  return *this;
}

charMatrix&
charMatrix::insert (const charMatrix& a, octave_idx_type r, octave_idx_type c)
{
  Array2<char>::insert (a, r, c);
  return *this;
}

std::string
charMatrix::row_as_string (octave_idx_type r, bool strip_ws, bool raw) const 
{
  std::string retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();

  if (r == 0 && nr == 0 && nc == 0)
    return retval;

  if (r < 0 || r >= nr)
    {
      (*current_liboctave_error_handler) ("range error for row_as_string");
      return retval;
    }

  retval.resize (nc, '\0');

  for (octave_idx_type i = 0; i < nc; i++)
    retval[i] = elem (r, i);

  if (! raw)
    {
      if (strip_ws)
	{
	  while (--nc >= 0)
	    {
	      char c = retval[nc];
	      if (c && c != ' ')
		break;
	    }
	}
      else
	{
	  while (--nc >= 0)
	    if (retval[nc])
	      break;
	}

      retval.resize (nc+1);
    }

  return retval;
}

charMatrix
charMatrix::extract (octave_idx_type r1, octave_idx_type c1, octave_idx_type r2, octave_idx_type c2) const
{
  if (r1 > r2) { octave_idx_type tmp = r1; r1 = r2; r2 = tmp; }
  if (c1 > c2) { octave_idx_type tmp = c1; c1 = c2; c2 = tmp; }

  octave_idx_type new_r = r2 - r1 + 1;
  octave_idx_type new_c = c2 - c1 + 1;

  charMatrix result (new_r, new_c);

  for (octave_idx_type j = 0; j < new_c; j++)
    for (octave_idx_type i = 0; i < new_r; i++)
      result.elem (i, j) = elem (r1+i, c1+j);

  return result;
}

// FIXME Do these really belong here?  Maybe they should be
// in a base class?

boolMatrix
charMatrix::all (int dim) const
{
  MX_ALL_OP (dim);
}

boolMatrix
charMatrix::any (int dim) const
{
  MX_ANY_OP (dim);
}

MS_CMP_OPS(charMatrix, , char, )
MS_BOOL_OPS(charMatrix, char, 0)

SM_CMP_OPS(char, , charMatrix, )
SM_BOOL_OPS(char, charMatrix, 0)

MM_CMP_OPS(charMatrix, , charMatrix, )
MM_BOOL_OPS(charMatrix, charMatrix, 0)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
