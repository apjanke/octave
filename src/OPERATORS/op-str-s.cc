/*

Copyright (C) 2003 John W. Eaton

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

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gripes.h"
#include "oct-obj.h"
#include "ov.h"
#include "ov-scalar.h"
#include "ov-str-mat.h"
#include "ov-typeinfo.h"
#include "ops.h"

DEFASSIGNOP (assign, char_matrix_str, octave_scalar)
{
  CAST_BINOP_ARGS (octave_char_matrix_str&, const octave_scalar&);

  octave_value tmp = v2.convert_to_str_internal (false, false);

  if (! error_state)
    v1.assign (idx, tmp.char_matrix_value ());

  return octave_value ();
}

DEFCATOP (str_s, char_matrix_str, scalar)
{
  CAST_BINOP_ARGS (const octave_char_matrix_str&,
		   const octave_scalar&);

  if (Vwarn_num_to_str)
    gripe_implicit_conversion (v2.type_name (), v1.type_name ());

  return octave_value (concat (v1.char_array_value (), v2.array_value (), 
			       ra_idx), true);
}

DEFCATOP (s_str, scalar, char_matrix_str)
{
  CAST_BINOP_ARGS (const octave_scalar&,
		   const octave_char_matrix_str&);

  if (Vwarn_num_to_str)
    gripe_implicit_conversion (v1.type_name (), v2.type_name ());

  return octave_value (concat (v1.array_value (), v2.char_array_value (), 
			       ra_idx), true);
}

void
install_str_s_ops (void)
{
  INSTALL_ASSIGNOP (op_asn_eq, octave_char_matrix_str, octave_scalar, assign);

  INSTALL_CATOP (octave_char_matrix_str, octave_scalar, str_s);
  INSTALL_CATOP (octave_scalar, octave_char_matrix_str, s_str);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
