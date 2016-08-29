/*

Copyright (C) 2016 John W. Eaton

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

#if ! defined (octave_oct_inttypes_fwd_h)
#define octave_oct_inttypes_fwd_h 1

#include "octave-config.h"

template <typename T> class octave_int;

typedef octave_int<int8_t> octave_int8;
typedef octave_int<int16_t> octave_int16;
typedef octave_int<int32_t> octave_int32;
typedef octave_int<int64_t> octave_int64;

typedef octave_int<uint8_t> octave_uint8;
typedef octave_int<uint16_t> octave_uint16;
typedef octave_int<uint32_t> octave_uint32;
typedef octave_int<uint64_t> octave_uint64;

#endif

