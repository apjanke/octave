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
#include "ov-range.h"
#include "ov-bool.h"
#include "ov-bool-mat.h"
#include "ov-scalar.h"
#include "ov-re-mat.h"
#include "ov-str-mat.h"
#include "ov-typeinfo.h"
#include "op-int.h"
#include "ops.h"

// Concatentation of mixed integer types:

OCTAVE_CONCAT_FN2 (int8, int16)
OCTAVE_CONCAT_FN2 (int8, int32)
OCTAVE_CONCAT_FN2 (int8, int64)

OCTAVE_CONCAT_FN2 (int8, uint8)
OCTAVE_CONCAT_FN2 (int8, uint16)
OCTAVE_CONCAT_FN2 (int8, uint32)
OCTAVE_CONCAT_FN2 (int8, uint64)

OCTAVE_CONCAT_FN2 (int16, int8)
OCTAVE_CONCAT_FN2 (int16, int32)
OCTAVE_CONCAT_FN2 (int16, int64)

OCTAVE_CONCAT_FN2 (int16, uint8)
OCTAVE_CONCAT_FN2 (int16, uint16)
OCTAVE_CONCAT_FN2 (int16, uint32)
OCTAVE_CONCAT_FN2 (int16, uint64)

OCTAVE_CONCAT_FN2 (int32, int8)
OCTAVE_CONCAT_FN2 (int32, int16)
OCTAVE_CONCAT_FN2 (int32, int64)

OCTAVE_CONCAT_FN2 (int32, uint8)
OCTAVE_CONCAT_FN2 (int32, uint16)
OCTAVE_CONCAT_FN2 (int32, uint32)
OCTAVE_CONCAT_FN2 (int32, uint64)

OCTAVE_CONCAT_FN2 (int64, int8)
OCTAVE_CONCAT_FN2 (int64, int16)
OCTAVE_CONCAT_FN2 (int64, int32)

OCTAVE_CONCAT_FN2 (int64, uint8)
OCTAVE_CONCAT_FN2 (int64, uint16)
OCTAVE_CONCAT_FN2 (int64, uint32)
OCTAVE_CONCAT_FN2 (int64, uint64)

OCTAVE_CONCAT_FN2 (uint8, int8)
OCTAVE_CONCAT_FN2 (uint8, int16)
OCTAVE_CONCAT_FN2 (uint8, int32)
OCTAVE_CONCAT_FN2 (uint8, int64)

OCTAVE_CONCAT_FN2 (uint8, uint16)
OCTAVE_CONCAT_FN2 (uint8, uint32)
OCTAVE_CONCAT_FN2 (uint8, uint64)

OCTAVE_CONCAT_FN2 (uint16, int8)
OCTAVE_CONCAT_FN2 (uint16, int16)
OCTAVE_CONCAT_FN2 (uint16, int32)
OCTAVE_CONCAT_FN2 (uint16, int64)

OCTAVE_CONCAT_FN2 (uint16, uint8)
OCTAVE_CONCAT_FN2 (uint16, uint32)
OCTAVE_CONCAT_FN2 (uint16, uint64)

OCTAVE_CONCAT_FN2 (uint32, int8)
OCTAVE_CONCAT_FN2 (uint32, int16)
OCTAVE_CONCAT_FN2 (uint32, int32)
OCTAVE_CONCAT_FN2 (uint32, int64)

OCTAVE_CONCAT_FN2 (uint32, uint8)
OCTAVE_CONCAT_FN2 (uint32, uint16)
OCTAVE_CONCAT_FN2 (uint32, uint64)

OCTAVE_CONCAT_FN2 (uint64, int8)
OCTAVE_CONCAT_FN2 (uint64, int16)
OCTAVE_CONCAT_FN2 (uint64, int32)
OCTAVE_CONCAT_FN2 (uint64, int64)

OCTAVE_CONCAT_FN2 (uint64, uint8)
OCTAVE_CONCAT_FN2 (uint64, uint16)
OCTAVE_CONCAT_FN2 (uint64, uint32)

OCTAVE_INT_DOUBLE_CONCAT_FN (int8)
OCTAVE_INT_DOUBLE_CONCAT_FN (int16)
OCTAVE_INT_DOUBLE_CONCAT_FN (int32)
OCTAVE_INT_DOUBLE_CONCAT_FN (int64)

OCTAVE_INT_DOUBLE_CONCAT_FN (uint8)
OCTAVE_INT_DOUBLE_CONCAT_FN (uint16)
OCTAVE_INT_DOUBLE_CONCAT_FN (uint32)
OCTAVE_INT_DOUBLE_CONCAT_FN (uint64)

OCTAVE_DOUBLE_INT_CONCAT_FN (int8)
OCTAVE_DOUBLE_INT_CONCAT_FN (int16)
OCTAVE_DOUBLE_INT_CONCAT_FN (int32)
OCTAVE_DOUBLE_INT_CONCAT_FN (int64)
	      	   
OCTAVE_DOUBLE_INT_CONCAT_FN (uint8)
OCTAVE_DOUBLE_INT_CONCAT_FN (uint16)
OCTAVE_DOUBLE_INT_CONCAT_FN (uint32)
OCTAVE_DOUBLE_INT_CONCAT_FN (uint64)

void
install_int_concat_ops (void)
{
  OCTAVE_INSTALL_CONCAT_FN2 (int8, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (int8, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (int8, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (int8, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (int8, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (int8, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (int8, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (int16, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (int16, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (int16, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (int16, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (int16, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (int16, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (int16, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (int32, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (int32, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (int32, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (int32, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (int32, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (int32, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (int32, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (int64, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (int64, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (int64, int32);

  OCTAVE_INSTALL_CONCAT_FN2 (int64, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (int64, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (int64, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (int64, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint8, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint8, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint8, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint8, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint8, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint8, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint8, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint16, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint16, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint16, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint16, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint16, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint16, uint32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint16, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint32, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint32, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint32, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint32, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint32, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint32, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint32, uint64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint64, int8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint64, int16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint64, int32);
  OCTAVE_INSTALL_CONCAT_FN2 (uint64, int64);

  OCTAVE_INSTALL_CONCAT_FN2 (uint64, uint8);
  OCTAVE_INSTALL_CONCAT_FN2 (uint64, uint16);
  OCTAVE_INSTALL_CONCAT_FN2 (uint64, uint32);

  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (int8);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (int16);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (int32);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (int64);

  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (uint8);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (uint16);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (uint32);
  OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN (uint64);

  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (int8);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (int16);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (int32);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (int64);

  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (uint8);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (uint16);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (uint32);
  OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN (uint64);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
