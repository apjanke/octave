// MArray-ch.cc                                            -*- C++ -*-
/*

Copyright (C) 1996 John W. Eaton

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

// Instantiate MArrays of char values.

#define NO_DIAG_ARRAY 1

#include "MArray.h"
#include "MArray.cc"

template class MArray<char>;
template class MArray2<char>;

INSTANTIATE_MARRAY_FRIENDS (char)
INSTANTIATE_MARRAY2_FRIENDS (char)

#ifndef NO_DIAG_ARRAY
template class MDiagArray<char>;

INSTANTIATE_MDIAGARRAY_FRIENDS (char)
#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
