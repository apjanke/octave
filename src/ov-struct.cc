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

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include "Cell.h"
#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "oct-lvalue.h"
#include "ov-list.h"
#include "ov-struct.h"
#include "unwind-prot.h"
#include "variables.h"

DEFINE_OCTAVE_ALLOCATOR(octave_struct);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(octave_struct, "struct");

Cell
octave_struct::dotref (const octave_value_list& idx)
{
  Cell retval;

  assert (idx.length () == 1);

  std::string nm = idx(0).string_value ();

  Octave_map::const_iterator p = map.seek (nm);

  if (p != map.end ())
    retval = map.contents (p);
  else
    error ("structure has no member `%s'", nm.c_str ());

  return retval;
}

#if 0
static void
gripe_invalid_index (void)
{
  error ("invalid index for structure array");
}
#endif

static void
gripe_invalid_index_for_assignment (void)
{
  error ("invalid index for structure array assignment");
}

static void
gripe_invalid_index_type (const std::string& nm, char t)
{
  error ("%s cannot be indexed with %c", nm.c_str (), t);
}

static void
gripe_failed_assignment (void)
{
  error ("assignment to structure element failed");
}

octave_value
octave_struct::subsref (const std::string& type,
			const std::list<octave_value_list>& idx)
{
  octave_value retval;

  int skip = 1;

  switch (type[0])
    {
    case '(':
      {
	if (type.length () > 1 && type[1] == '.')
	  {
	    std::list<octave_value_list>::const_iterator p = idx.begin ();
	    octave_value_list key_idx = *++p;

	    Cell tmp = dotref (key_idx);

	    if (! error_state)
	      {
		Cell t = tmp.index (idx.front ());

		retval = (t.length () == 1) ? t(0) : octave_value (t, true);

		// We handled two index elements, so tell
		// next_subsref to skip both of them.

		skip++;
	      }
	  }
	else
	  retval = map.index (idx.front ());
      }
      break;

    case '.':
      {
	Cell t = dotref (idx.front ());

	retval = (t.length () == 1) ? t(0) : octave_value (t, true);
      }
      break;

    case '{':
      gripe_invalid_index_type (type_name (), type[0]);
      break;

    default:
      panic_impossible ();
    }

  if (! error_state)
    retval = retval.next_subsref (type, idx, skip);

  return retval;
}

octave_value
octave_struct::numeric_conv (const Cell& val,
			     const std::string& type)
{
  octave_value retval;

  if (val.length () == 1)
    {
      retval = val(0);

      if (type.length () > 0 && type[0] == '.' && ! retval.is_map ())
	retval = Octave_map ();
    }
  else
    gripe_invalid_index_for_assignment ();

  return retval;
}

octave_value
octave_struct::subsasgn (const std::string& type,
			 const std::list<octave_value_list>& idx,
			 const octave_value& rhs)
{
  octave_value retval;

  int n = type.length ();

  octave_value t_rhs = rhs;

  if (n > 1 && ! (type.length () == 2 && type[0] == '(' && type[1] == '.'))
    {
      switch (type[0])
	{
	case '(':
	  {
	    if (type.length () > 1 && type[1] == '.')
	      {
		std::list<octave_value_list>::const_iterator p = idx.begin ();
		octave_value_list t_idx = *p;

		octave_value_list key_idx = *++p;

		assert (key_idx.length () == 1);

		std::string key = key_idx(0).string_value ();

		octave_value u;

		if (! map.contains (key))
		  u = octave_value::empty_conv (type.substr (2), rhs);
		else
		  {
		    Cell map_val = map[key];

		    Cell map_elt = map_val.index (idx.front (), true);

		    u = numeric_conv (map_elt, type.substr (2));
		  }

		if (! error_state)
		  {
		    std::list<octave_value_list> next_idx (idx);

		    // We handled two index elements, so subsasgn to
		    // needs to skip both of them.

		    next_idx.erase (next_idx.begin ());
		    next_idx.erase (next_idx.begin ());

		    u.make_unique ();

		    t_rhs = u.subsasgn (type.substr (2), next_idx, rhs);
		  }
	      }
	    else
	      gripe_invalid_index_for_assignment ();
	  }
	  break;

	case '.':
	  {
	    octave_value_list key_idx = idx.front ();

	    assert (key_idx.length () == 1);

	    std::string key = key_idx(0).string_value ();

	    octave_value u;

	    if (! map.contains (key))
	      u = octave_value::empty_conv (type.substr (1), rhs);
	    else
	      {
		Cell map_val = map[key];

		u = numeric_conv (map_val, type.substr (1));
	      }

	    if (! error_state)
	      {
		std::list<octave_value_list> next_idx (idx);

		next_idx.erase (next_idx.begin ());

		u.make_unique ();

		t_rhs = u.subsasgn (type.substr (1), next_idx, rhs);
	      }
	  }
	  break;

	case '{':
	  gripe_invalid_index_type (type_name (), type[0]);
	  break;

	default:
	  panic_impossible ();
	}
    }

  if (! error_state)
    {
      switch (type[0])
	{
	case '(':
	  {
	    if (n > 1 && type[1] == '.')
	      {
		std::list<octave_value_list>::const_iterator p = idx.begin ();
		octave_value_list key_idx = *++p;

		assert (key_idx.length () == 1);

		std::string key = key_idx(0).string_value ();

		if (! error_state)
		  {
		    map.assign (idx.front (), key, t_rhs);

		    if (! error_state)
		      retval = octave_value (this, count + 1);
		    else
		      gripe_failed_assignment ();
		  }
		else
		  gripe_failed_assignment ();
	      }
	    else
	      {
		Octave_map rhs_map = t_rhs.map_value ();

		if (! error_state)
		  {
		    map.assign (idx.front (), rhs_map);

		    if (! error_state)
		      retval = octave_value (this, count + 1);
		    else
		      gripe_failed_assignment ();
		  }
		else
		  error ("invalid structure assignment");
	      }
	  }
	  break;

	case '.':
	  {
	    octave_value_list key_idx = idx.front ();

	    assert (key_idx.length () == 1);

	    std::string key = key_idx(0).string_value ();

	    map.assign (key, t_rhs);

	    if (! error_state)
	      retval = octave_value (this, count + 1);
	    else
	      gripe_failed_assignment ();
	  }
	  break;

	case '{':
	  gripe_invalid_index_type (type_name (), type[0]);
	  break;

	default:
	  panic_impossible ();
	}
    }
  else
    gripe_failed_assignment ();

  return retval;
}

void
octave_struct::print (std::ostream& os, bool) const
{
  print_raw (os);
}

void
octave_struct::print_raw (std::ostream& os, bool) const
{
  // XXX FIXME XXX -- would be nice to print the output in some
  // standard order.  Maybe all substructures first, maybe
  // alphabetize entries, etc.

  unwind_protect::begin_frame ("octave_struct_print");

  unwind_protect_int (Vstruct_levels_to_print);

  if (Vstruct_levels_to_print >= 0)
    {
      bool print_keys_only = (Vstruct_levels_to_print == 0);

      Vstruct_levels_to_print--;

      indent (os);
      os << "{";
      newline (os);

      increment_indent_level ();

      int n = map.numel ();

      if (n > 1 && print_keys_only)
	{
	  indent (os);
	  dim_vector dv = dims ();
	  os << dv.str () << " struct array containing the fields:";
	  newline (os);
	  newline (os);

	  increment_indent_level ();
	}

      for (Octave_map::const_iterator p = map.begin (); p != map.end (); p++)
	{
	  std::string key = map.key (p);
	  Cell val = map.contents (p);

	  octave_value tmp = (n == 1) ? val(0) : octave_value (val, true);

	  if (print_keys_only)
	    {
	      indent (os);
	      os << key;
	      if (n == 1)
		{
		  dim_vector dv = tmp.dims ();
		  os << ": " << dv.str () << " " << tmp.type_name ();
		}
	      newline (os);
	    }
	  else
	    tmp.print_with_name (os, key);
	}

      if (n > 1 && print_keys_only)
	decrement_indent_level ();

      decrement_indent_level ();

      indent (os);
      os << "}";
      newline (os);
    }
  else
    {
      indent (os);
      os << "<structure>";
      newline (os);
    }

  unwind_protect::run_frame ("octave_struct_print");
}

bool
octave_struct::print_name_tag (std::ostream& os, const std::string& name) const
{
  bool retval = false;

  indent (os);

  if (Vstruct_levels_to_print < 0)
    os << name << " = ";
  else
    {
      os << name << " =";
      newline (os);
      retval = true;
    }

  return retval;
}

DEFUN (isstruct, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} isstruct (@var{expr})\n\
Return 1 if the value of the expression @var{expr} is a structure.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).is_map ();
  else
    print_usage ("isstruct");

  return retval;
}

DEFUN (fieldnames, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} fieldnames (@var{struct})\n\
Return a cell array of strings naming the elements of the structure\n\
@var{struct}.  It is an error to call @code{fieldnames} with an\n\
argument that is not a structure.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      if (args(0).is_map ())
	{
	  Octave_map m = args(0).map_value ();
	  retval = Cell (m.keys ());
	}
      else
	gripe_wrong_type_arg ("fieldnames", args(0));
    }
  else
    print_usage ("fieldnames");

  return retval;
}

DEFUN (isfield, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} isfield (@var{expr}, @var{name})\n\
Return true if the expression @var{expr} is a structure and it includes an\n\
element named @var{name}.  The first argument must be a structure and\n\
the second must be a string.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin == 2)
    {
      retval = false;

      // XXX FIXME XXX -- should this work for all types that can do
      // structure reference operations?

      if (args(0).is_map () && args(1).is_string ())
	{
	  std::string key = args(1).string_value ();

	  Octave_map m = args(0).map_value ();

	  retval = m.contains (key) != 0;
	}
      else
	print_usage ("isfield");
    }
  else
    print_usage ("isfield");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
