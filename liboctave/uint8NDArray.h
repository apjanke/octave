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

#if !defined (octave_uint8NDArray_h)
#define octave_uint8NDArray_h 1

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma interface
#endif

#include "intNDArray.h"
#include "mx-op-defs.h"
#include "oct-inttypes.h"

typedef intNDArray<octave_uint8> uint8NDArray;

OCTAVE_INT_CONCAT_DECL (octave_uint8)

NDS_CMP_OP_DECLS (uint8NDArray, octave_uint8)
NDS_BOOL_OP_DECLS (uint8NDArray, octave_uint8)

SND_CMP_OP_DECLS (octave_uint8, uint8NDArray)
SND_BOOL_OP_DECLS (octave_uint8, uint8NDArray)

NDND_CMP_OP_DECLS (uint8NDArray, uint8NDArray)
NDND_BOOL_OP_DECLS (uint8NDArray, uint8NDArray)

MARRAY_FORWARD_DEFS (MArrayN, uint8NDArray, octave_uint8)

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
