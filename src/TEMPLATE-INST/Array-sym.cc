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

// Instantiate Arrays of octave_child objects.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Array.h"
#include "Array.cc"

#include "oct-obj.h"
#include "symtab.h"

typedef symbol_record* symbol_record_ptr;

INSTANTIATE_ARRAY (symbol_record_ptr, OCTINTERP_API);

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
