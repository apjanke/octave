/*

Copyright (C) 1996, 1997 John W. Eaton

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

#if !defined (octave_Bounds_h)
#define octave_Bounds_h 1

#include <iostream>

#include "dColVector.h"

class
Bounds
{
public:

  Bounds (void)
    : lb (), ub () { }

  Bounds (octave_idx_type n)
    : lb (n, 0.0), ub (n, 0.0) { }

  Bounds (const ColumnVector l, const ColumnVector u)
    : lb (l), ub (u)
      {
        if (lb.capacity () != ub.capacity ())
	  {
	    error ("inconsistent sizes for lower and upper bounds");
	    return;
	  }
      }

  Bounds (const Bounds& a)
    : lb (a.lb), ub (a.ub) { }

  Bounds& operator = (const Bounds& a)
    {
      if (this != &a)
	{
	  lb = a.lower_bounds ();
	  ub = a.upper_bounds ();
	}
      return *this;
    }

  ~Bounds (void) { }

  Bounds& resize (octave_idx_type n)
    {
      lb.resize (n);
      ub.resize (n);

      return *this;
    }

  double lower_bound (octave_idx_type index) const { return lb.elem (index); }
  double upper_bound (octave_idx_type index) const { return ub.elem (index); }

  ColumnVector lower_bounds (void) const { return lb; }
  ColumnVector upper_bounds (void) const { return ub; }

  octave_idx_type size (void) const { return lb.capacity (); }

  Bounds& set_bound (octave_idx_type index, double low, double high)
    {
      lb.elem (index) = low;
      ub.elem (index) = high;
      return *this;
    }

  Bounds& set_bounds (double low, double high)
    {
      lb.fill (low);
      ub.fill (high);
      return *this;
    }

  Bounds& set_bounds (const ColumnVector lb, const ColumnVector ub);

  Bounds& set_lower_bound (octave_idx_type index, double low)
    {
      lb.elem (index) = low;
      return *this;
    }

  Bounds& set_upper_bound (octave_idx_type index, double high)
    {
      ub.elem (index) = high;
      return *this;
    }

  Bounds& set_lower_bounds (double low)
    {
      lb.fill (low);
      return *this;
    }

  Bounds& set_upper_bounds (double high)
    {
      ub.fill (high);
      return *this;
    }

  Bounds& set_lower_bounds (const ColumnVector lb);
  Bounds& set_upper_bounds (const ColumnVector ub);

  friend std::ostream& operator << (std::ostream& os, const Bounds& b);

protected:

  ColumnVector lb;
  ColumnVector ub;

private:

  void error (const char *msg);
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
