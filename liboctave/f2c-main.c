/*

Copyright (C) 1996, 1997 John W. Eaton

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

#include <assert.h>


/* Dummy Fortran main declaration, needed in order to link to some
   Fortran libraries.  See the AC_F77_DUMMY_MAIN macro documentation. 
   This function should never be called. */

#ifdef F77_DUMMY_MAIN
#  ifdef __cplusplus
extern "C"
#  endif
int F77_DUMMY_MAIN() { assert(0); return 1; }
#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/

