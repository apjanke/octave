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

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gripes.h"
#include "oct-obj.h"
#include "ov.h"
#include "ov-int8.h"
#include "ov-int16.h"
#include "ov-int32.h"
#include "ov-int64.h"
#include "ov-uint8.h"
#include "ov-uint16.h"
#include "ov-uint32.h"
#include "ov-uint64.h"
#include "ov-scalar.h"
#include "ov-re-mat.h"
#include "ov-typeinfo.h"
#include "ops.h"

// conversion ops

DEFCONVFN (scalar_to_int8, scalar, int8)
DEFCONVFN (scalar_to_int16, scalar, int16)
DEFCONVFN (scalar_to_int32, scalar, int32)
DEFCONVFN (scalar_to_int64, scalar, int64)

DEFCONVFN (scalar_to_uint8, scalar, uint8)
DEFCONVFN (scalar_to_uint16, scalar, uint16)
DEFCONVFN (scalar_to_uint32, scalar, uint32)
DEFCONVFN (scalar_to_uint64, scalar, uint64)

DEFCONVFN (matrix_to_int8, matrix, int8)
DEFCONVFN (matrix_to_int16, matrix, int16)
DEFCONVFN (matrix_to_int32, matrix, int32)
DEFCONVFN (matrix_to_int64, matrix, int64)

DEFCONVFN (matrix_to_uint8, matrix, uint8)
DEFCONVFN (matrix_to_uint16, matrix, uint16)
DEFCONVFN (matrix_to_uint32, matrix, uint32)
DEFCONVFN (matrix_to_uint64, matrix, uint64)

#define INT_CONV_FUNCTIONS(tfrom) \
  DEFCONVFN2 (tfrom ## _scalar_to_int8, tfrom, scalar, int8) \
  DEFCONVFN2 (tfrom ## _scalar_to_int16, tfrom, scalar, int16) \
  DEFCONVFN2 (tfrom ## _scalar_to_int32, tfrom, scalar, int32) \
  DEFCONVFN2 (tfrom ## _scalar_to_int64, tfrom, scalar, int64) \
 \
  DEFCONVFN2 (tfrom ## _scalar_to_uint8, tfrom, scalar, int8) \
  DEFCONVFN2 (tfrom ## _scalar_to_uint16, tfrom, scalar, int16) \
  DEFCONVFN2 (tfrom ## _scalar_to_uint32, tfrom, scalar, int32) \
  DEFCONVFN2 (tfrom ## _scalar_to_uint64, tfrom, scalar, int64) \
 \
  DEFCONVFN2 (tfrom ## _matrix_to_int8, tfrom, matrix, uint8) \
  DEFCONVFN2 (tfrom ## _matrix_to_int16, tfrom, matrix, uint16) \
  DEFCONVFN2 (tfrom ## _matrix_to_int32, tfrom, matrix, uint32) \
  DEFCONVFN2 (tfrom ## _matrix_to_int64, tfrom, matrix, uint64) \
 \
  DEFCONVFN2 (tfrom ## _matrix_to_uint8, tfrom, matrix, uint8) \
  DEFCONVFN2 (tfrom ## _matrix_to_uint16, tfrom, matrix, uint16) \
  DEFCONVFN2 (tfrom ## _matrix_to_uint32, tfrom, matrix, uint32) \
  DEFCONVFN2 (tfrom ## _matrix_to_uint64, tfrom, matrix, uint64)

INT_CONV_FUNCTIONS (int8)
INT_CONV_FUNCTIONS (int16)
INT_CONV_FUNCTIONS (int32)
INT_CONV_FUNCTIONS (int64)

INT_CONV_FUNCTIONS (uint8)
INT_CONV_FUNCTIONS (uint16)
INT_CONV_FUNCTIONS (uint32)
INT_CONV_FUNCTIONS (uint64)

#define INSTALL_INT_CONV_FUNCTIONS(tfrom) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_int8_matrix, tfrom ## _scalar_to_int8) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_int16_matrix, tfrom ## _scalar_to_int16) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_int32_matrix, tfrom ## _scalar_to_int32) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_int64_matrix, tfrom ## _scalar_to_int64) \
 \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_uint8_matrix, tfrom ## _scalar_to_uint8) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_uint16_matrix, tfrom ## _scalar_to_uint16) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_uint32_matrix, tfrom ## _scalar_to_uint32) \
  INSTALL_CONVOP (octave_ ## tfrom ## _scalar, octave_uint64_matrix, tfrom ## _scalar_to_uint64) \
 \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_int8_matrix, tfrom ## _matrix_to_int8) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_int16_matrix, tfrom ## _matrix_to_int16) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_int32_matrix, tfrom ## _matrix_to_int32) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_int64_matrix, tfrom ## _matrix_to_int64) \
 \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_uint8_matrix, tfrom ## _matrix_to_uint8) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_uint16_matrix, tfrom ## _matrix_to_uint16) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_uint32_matrix, tfrom ## _matrix_to_uint32) \
  INSTALL_CONVOP (octave_ ## tfrom ## _matrix, octave_uint64_matrix, tfrom ## _matrix_to_uint64) \

void
install_int_conv_ops (void)
{
  INSTALL_CONVOP (octave_scalar, octave_int8_matrix, scalar_to_int8);
  INSTALL_CONVOP (octave_scalar, octave_int16_matrix, scalar_to_int16);
  INSTALL_CONVOP (octave_scalar, octave_int32_matrix, scalar_to_int32);
  INSTALL_CONVOP (octave_scalar, octave_int64_matrix, scalar_to_int64);

  INSTALL_CONVOP (octave_scalar, octave_uint8_matrix, scalar_to_uint8);
  INSTALL_CONVOP (octave_scalar, octave_uint16_matrix, scalar_to_uint16);
  INSTALL_CONVOP (octave_scalar, octave_uint32_matrix, scalar_to_uint32);
  INSTALL_CONVOP (octave_scalar, octave_uint64_matrix, scalar_to_uint64);

  INSTALL_CONVOP (octave_matrix, octave_int8_matrix, matrix_to_int8);
  INSTALL_CONVOP (octave_matrix, octave_int16_matrix, matrix_to_int16);
  INSTALL_CONVOP (octave_matrix, octave_int32_matrix, matrix_to_int32);
  INSTALL_CONVOP (octave_matrix, octave_int64_matrix, matrix_to_int64);

  INSTALL_CONVOP (octave_matrix, octave_uint8_matrix, matrix_to_uint8);
  INSTALL_CONVOP (octave_matrix, octave_uint16_matrix, matrix_to_uint16);
  INSTALL_CONVOP (octave_matrix, octave_uint32_matrix, matrix_to_uint32);
  INSTALL_CONVOP (octave_matrix, octave_uint64_matrix, matrix_to_uint64);

  INSTALL_INT_CONV_FUNCTIONS (int8)
  INSTALL_INT_CONV_FUNCTIONS (int16)
  INSTALL_INT_CONV_FUNCTIONS (int32)
  INSTALL_INT_CONV_FUNCTIONS (int64)

  INSTALL_INT_CONV_FUNCTIONS (uint8)
  INSTALL_INT_CONV_FUNCTIONS (uint16)
  INSTALL_INT_CONV_FUNCTIONS (uint32)
  INSTALL_INT_CONV_FUNCTIONS (uint64)
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
