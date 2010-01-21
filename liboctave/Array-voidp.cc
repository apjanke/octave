/*

Copyright (C) 1996, 1997, 2000, 2003, 2005, 2007, 2008, 2009 John W. Eaton

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

#include <string>

// Instantiate Arrays of void *.

#include "Array.h"
#include "Array.cc"

NO_INSTANTIATE_ARRAY_SORT (void *);

INSTANTIATE_ARRAY (void *, OCTAVE_API);

#include "Array2.h"

template class OCTAVE_API Array2<void *>;

#include "Array3.h"

template class OCTAVE_API Array3<void *>;
