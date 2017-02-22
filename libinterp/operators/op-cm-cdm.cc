/*

Copyright (C) 2008-2017 Jaroslav Hajek

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#define LINCLUDE "ov-cx-mat.h"
#define RINCLUDE "ov-cx-diag.h"

#define LMATRIX complex_matrix
#define RMATRIX complex_diag_matrix

#define LSHORT cm
#define RSHORT cdm

#define DEFINEDIV

#include "op-dm-template.cc"
