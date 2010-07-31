/*

Copyright (C) 2008, 2009 Jaroslav Hajek

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

#include "ov-null-mat.h"
#include "ops.h"
#include "defun.h"

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_null_matrix, "null_matrix", "double");

const octave_value octave_null_matrix::instance (new octave_null_matrix ());

static octave_base_value *
default_null_matrix_numeric_conversion_function (const octave_base_value& a)
{
  // The cast is not necessary?
  // CAST_CONV_ARG (const octave_null_matrix&);

  return a.empty_clone ();
}

octave_base_value::type_conv_info
octave_null_matrix::numeric_conversion_function (void) const
{
  return octave_base_value::type_conv_info (default_null_matrix_numeric_conversion_function, 
                                            octave_matrix::static_type_id ());
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_null_str, "null_string", "char");

const octave_value octave_null_str::instance (new octave_null_str ());

static octave_base_value *
default_null_str_numeric_conversion_function (const octave_base_value& a)
{
  // The cast is not necessary?
  // CAST_CONV_ARG (const octave_null_str&);

  return a.empty_clone ();
}

octave_base_value::type_conv_info
octave_null_str::numeric_conversion_function (void) const
{
  return octave_base_value::type_conv_info (default_null_str_numeric_conversion_function, 
                                            octave_char_matrix_str::static_type_id ());
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_null_sq_str, "null_sq_string", "char");

const octave_value octave_null_sq_str::instance (new octave_null_sq_str ());

static octave_base_value *
default_null_sq_str_numeric_conversion_function (const octave_base_value& a)
{
  // The cast is not necessary?
  // CAST_CONV_ARG (const octave_null_sq_str&);

  return a.empty_clone ();
}

octave_base_value::type_conv_info
octave_null_sq_str::numeric_conversion_function (void) const
{
  return octave_base_value::type_conv_info (default_null_sq_str_numeric_conversion_function, 
                                            octave_char_matrix_sq_str::static_type_id ());
}

DEFUN (isnull, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} isnull (@var{x})\n\
Return 1 if @var{x} is a special null matrix, string or single quoted string.\n\
Indexed assignment with such a value as right-hand side should delete array\n\
elements.  This function should be used when overloading indexed assignment for\n\
user-defined classes instead of @code{isempty}, to distinguish the cases:\n\
@table @asis\n\
@item @code{A(I) = []}\n\
This should delete elements if @code{I} is nonempty.\n\
\n\
@item @code{X = []; A(I) = X}\n\
This should give an error if @code{I} is nonempty.\n\
@end table\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin == 1 && args(0).is_defined ())
    retval = args(0).is_null_value ();
  else
    print_usage ();

  return retval;
}

