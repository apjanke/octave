/*

Copyright (C) 2004 John W. Eaton

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

#if !defined (octave_intNDArray_h)
#define octave_intNDArray_h 1

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma interface
#endif

#include "MArrayN.h"
#include "boolNDArray.h"

template <class T>
class
intNDArray : public MArrayN<T>
{
public:
  
  intNDArray (void) : MArrayN<T> () { }

  intNDArray (T val) : MArrayN<T> (dim_vector (1, 1), val) { }

  intNDArray (const dim_vector& dv) : MArrayN<T> (dv) { }
  
  intNDArray (const dim_vector& dv, T val)
    : MArrayN<T> (dv, val) { }
  
  template <class U>
  explicit intNDArray (const Array<U>& a) : MArrayN<T> (a) { }

  template <class U>
  explicit intNDArray (const ArrayN<U>& a) : MArrayN<T> (a) { }

  template <class U>
  intNDArray (const MArrayN<U>& a) : MArrayN<T> (a) { }

  template <class U>
  intNDArray (const intNDArray<U>& a) : MArrayN<T> (a) { }

  intNDArray& operator = (const intNDArray<T>& a)
    {
      MArrayN<T>::operator = (a);
      return *this;
    }

  boolNDArray operator ! (void) const;

  // XXX FIXME XXX -- this is not quite the right thing.

  boolNDArray all (int dim = -1) const;
  boolNDArray any (int dim = -1) const;
  int cat (const intNDArray<T>& ra_arg, int dim, int iidx, int move);

  intNDArray squeeze (void) const
    { return intNDArray<T> (MArrayN<T>::squeeze ()); }

  intNDArray transpose (void) const
    { return intNDArray<T> (MArrayN<T>::transpose ()); }

  static void increment_index (Array<int>& ra_idx,
			       const dim_vector& dimensions,
			       int start_dimension = 0);

  static int compute_index (Array<int>& ra_idx,
			    const dim_vector& dimensions);

  static T resize_fill_value (void) { return 0; }

protected:

  intNDArray (T *d, dim_vector& dv) : MArrayN<T> (d, dv) { }
};

// i/o

template <class T>
std::ostream& operator << (std::ostream& os, const intNDArray<T>& a);

template <class T>
std::istream& operator >> (std::istream& is, intNDArray<T>& a);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
