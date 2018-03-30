/*

Copyright (C) 2008-2018 Jaroslav Hajek

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#define LINCLUDE "ov-flt-re-mat.h"
#define RINCLUDE "ov-flt-re-diag.h"

#define LMATRIX float_matrix
#define RMATRIX float_diag_matrix

#define LSHORT fm
#define RSHORT fdm

#define DEFINEDIV

#include "op-dm-template.cc"
