/*

Copyright (C) 1995, 1996, 1997, 2000, 2003, 2005, 2007 John W. Eaton

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

// Instantiate MArrays of float values.

#include "MArray.h"
#include "MArray.cc"
#include "fColVector.h"
#include "oct-norm.h"

template <>
OCTAVE_API float
MArray<float>::norm (float p) const
{
  return xnorm (FloatColumnVector (*this));
}

template class OCTAVE_API MArray<float>;

INSTANTIATE_MARRAY_FRIENDS (float, OCTAVE_API)

#include "MArray2.h"
#include "MArray2.cc"

template class OCTAVE_API MArray2<float>;

INSTANTIATE_MARRAY2_FRIENDS (float, OCTAVE_API)

#include "MArrayN.h"
#include "MArrayN.cc"

template class OCTAVE_API MArrayN<float>;

INSTANTIATE_MARRAYN_FRIENDS (float, OCTAVE_API)

#include "MDiagArray2.h"
#include "MDiagArray2.cc"

template class OCTAVE_API MDiagArray2<float>;

INSTANTIATE_MDIAGARRAY2_FRIENDS (float, OCTAVE_API)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
