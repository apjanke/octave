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

// Instantiate Arrays of octave_values.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Array.h"


#include "Array.cc"

#include "Array2.h"
#include "Array2.cc"

#include "DiagArray2.h"
#include "DiagArray2.cc"

#include "oct-obj.h"

octave_value 
resize_fill_value (const octave_value& x)
{
  static octave_value retval = octave_value (Matrix ());
  return retval;
}

template class Array<octave_value>;


template int assign (Array<octave_value>&, const Array<octave_value>&);

template int assign (Array<octave_value>&,
		     const Array<octave_value>&, const octave_value&);


template class Array2<octave_value>;

template int assign (Array2<octave_value>&, const Array2<octave_value>&);

template int assign (Array2<octave_value>&,
		     const Array2<octave_value>&, const octave_value&);

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
