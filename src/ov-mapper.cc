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

#if defined (__GNUG__) && ! defined (NO_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "ov-mapper.h"
#include "ov.h"

DEFINE_OCTAVE_ALLOCATOR (octave_mapper);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_mapper,
				     "built-in mapper function");

static bool
any_element_less_than (const Matrix& a, double val)
{
  int nr = a.rows ();
  int nc = a.columns ();

  for (int j = 0; j < nc; j++)
    for (int i = 0; i < nr; i++)
      if (a (i, j) < val)
	return true;

  return false;
}

static bool
any_element_greater_than (const Matrix& a, double val)
{
  int nr = a.rows ();
  int nc = a.columns ();

  for (int j = 0; j < nc; j++)
    for (int i = 0; i < nr; i++)
      if (a (i, j) > val)
	return true;

  return false;
}

// In most cases, we could use the map member function from the Matrix
// classes, but as currently implemented, they don't allow us to
// detect errors and abort properly.  So use these macros to do the
// looping here instead.

#define MAPPER_LOOP_2(T, F, M, CONV, R) \
  do \
    { \
      int nr = M.rows (); \
      int nc = M.cols (); \
 \
      T result (nr, nc); \
 \
      for (int j = 0; j < nc; j++) \
	{ \
	   for (int i = 0; i < nr; i++) \
	     { \
		result (i, j) = CONV (F (M (i, j))); \
 \
		if (error_state) \
		  return retval; \
	     } \
	} \
      retval = R; \
    } \
  while (0)

#define MAPPER_LOOP_1(T, F, M, CONV) \
  MAPPER_LOOP_2 (T, F, M, CONV, result)

#define MAPPER_LOOP(T, F, M) \
  MAPPER_LOOP_1 (T, F, M, )

octave_value
octave_mapper::apply (const octave_value& arg) const
{
  octave_value retval;

  if (arg.is_real_type ())
    {
      if (arg.is_scalar_type ())
	{
	  double d = arg.double_value ();

	  if (can_ret_cmplx_for_real && (d < lower_limit || d > upper_limit))
	    {
	      if (c_c_map_fcn)
		retval = c_c_map_fcn (Complex (d));
	      else
		error ("%s: unable to handle real arguments",
		       name().c_str ());
	    }
	  else if (d_d_map_fcn)
	    retval = d_d_map_fcn (d);
	  else if (d_b_map_fcn)
	    retval = d_b_map_fcn (d);
	  else
	    error ("%s: unable to handle real arguments",
		   name().c_str ());
	}
      else
	{
	  Matrix m = arg.matrix_value ();

	  if (error_state)
	    return retval;

	  if (can_ret_cmplx_for_real
	      && (any_element_less_than (m, lower_limit)
		  || any_element_greater_than (m, upper_limit)))
	    {
	      if (c_c_map_fcn)
		MAPPER_LOOP (ComplexMatrix, c_c_map_fcn, m);
	      else
		error ("%s: unable to handle real arguments",
		       name().c_str ());
	    }
	  else if (d_d_map_fcn)
	    MAPPER_LOOP (Matrix, d_d_map_fcn, m);
	  else if (d_b_map_fcn)
	    MAPPER_LOOP (boolMatrix, d_b_map_fcn, m);
	  else
	    error ("%s: unable to handle real arguments",
		   name().c_str ());
	}
    }
  else if (arg.is_complex_type ())
    {
      if (arg.is_scalar_type ())
	{
	  Complex c = arg.complex_value ();

	  if (d_c_map_fcn)
	    retval = d_c_map_fcn (c);
	  else if (c_c_map_fcn)
	    retval = c_c_map_fcn (c);
	  else if (c_b_map_fcn)
	    retval = c_b_map_fcn (c);
	  else
	    error ("%s: unable to handle complex arguments",
		   name().c_str ());
	}
      else
	{
	  ComplexMatrix cm = arg.complex_matrix_value ();

	  if (error_state)
	    return retval;

	  if (d_c_map_fcn)
	    MAPPER_LOOP (Matrix, d_c_map_fcn, cm);
	  else if (c_c_map_fcn)
	    MAPPER_LOOP (ComplexMatrix, c_c_map_fcn, cm);
	  else if (c_b_map_fcn)
	    MAPPER_LOOP (boolMatrix, c_b_map_fcn, cm);
	  else
	    error ("%s: unable to handle complex arguments",
		   name().c_str ());
	}
    }
  else if (ch_map_fcn)
    {
      // XXX FIXME XXX -- this could be done in a better way...

      octave_value tmp = arg.convert_to_str ();

      if (! error_state)
	{
	  charMatrix chm = tmp.char_matrix_value ();

	  if (! error_state)
	    {
	      switch (ch_map_flag)
		{
		case 0:
		  MAPPER_LOOP_1 (boolMatrix, ch_map_fcn, chm, bool);
		  break;

		case 1:
		  MAPPER_LOOP (Matrix, ch_map_fcn, chm);
		  break;

		case 2:
		  MAPPER_LOOP_2 (charMatrix, ch_map_fcn, chm, ,
				 octave_value (result, true));
		  break;

		default:
		  panic_impossible ();
		  break;
		}
	    }
	}
    }
  else
    gripe_wrong_type_arg ("mapper", arg);

  return retval;
}

octave_value_list
octave_mapper::subsref (const std::string type,
			const SLList<octave_value_list>& idx,
			int nargout)
{
  octave_value_list retval;

  switch (type[0])
    {
    case '(':
      retval = do_multi_index_op (nargout, idx.front ());
      break;

    case '{':
    case '.':
      {
	std::string nm = type_name ();
	error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
      }
      break;

    default:
      panic_impossible ();
    }

  return retval;

  // XXX FIXME XXX
  //  return retval.next_subsref (type, idx);
}

octave_value_list
octave_mapper::do_multi_index_op (int, const octave_value_list& args)
{
  octave_value retval;

  if (error_state)
    return retval;

  int nargin = args.length ();

  if (nargin > 1)
    ::error ("%s: too many arguments", name().c_str ());
  else if (nargin < 1)
    ::error ("%s: too few arguments", name().c_str ());
  else
    {
      if (args(0).is_defined ())
	retval = apply (args(0));
      else
	::error ("%s: argument undefined", name().c_str ());
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
