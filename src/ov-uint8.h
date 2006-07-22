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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_uint8_h)
#define octave_uint8_h 1

#define OCTAVE_VALUE_INT_MATRIX_T octave_uint8_matrix
#define OCTAVE_INT_NDARRAY_T uint8NDArray
#define OCTAVE_VALUE_INT_NDARRAY_EXTRACTOR_FUNCTION uint8_array_value

#define OCTAVE_VALUE_INT_SCALAR_T octave_uint8_scalar
#define OCTAVE_INT_T octave_uint8
#define OCTAVE_VALUE_INT_SCALAR_EXTRACTOR_FUNCTION uint8_scalar_value

#define OCTAVE_TYPE_PREDICATE_FUNCTION is_uint8_type

#define OCTAVE_INT_MX_CLASS mxUINT8_CLASS

#include "ov-intx.h"

#undef OCTAVE_VALUE_INT_MATRIX_T
#undef OCTAVE_INT_NDARRAY_T
#undef OCTAVE_VALUE_INT_NDARRAY_EXTRACTOR_FUNCTION

#undef OCTAVE_VALUE_INT_SCALAR_T
#undef OCTAVE_INT_T
#undef OCTAVE_VALUE_INT_SCALAR_EXTRACTOR_FUNCTION

#undef OCTAVE_TYPE_PREDICATE_FUNCTION

#undef OCTAVE_INT_MX_CLASS

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
