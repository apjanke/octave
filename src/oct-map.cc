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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "error.h"
#include "str-vec.h"

#include "oct-map.h"
#include "utils.h"

Octave_map::Octave_map (const dim_vector& dv,
			const string_vector& key_list_arg)
  : map (), key_list (), dimensions (dv)
{
  Cell c (dv);

  for (octave_idx_type i = 0; i < key_list_arg.length (); i++)
    {
      std::string k = key_list_arg[i];
      map[k] = c;
      key_list.push_back (k);
    }
}

Cell&
Octave_map::contents (const std::string& k)
{
  maybe_add_to_key_list (k);

  return map[k];
}

Cell
Octave_map::contents (const std::string& k) const
{
  const_iterator p = seek (k);

  return p != end () ? p->second : Cell ();
}

int
Octave_map::intfield (const std::string& k, int def_val) const
{
  int retval = def_val;

  Cell c = contents (k);

  if (! c.is_empty ())
    retval = c(0).int_value ();

  return retval;
}

std::string
Octave_map::stringfield (const std::string& k,
			 const std::string& def_val) const
{
  std::string retval = def_val;

  Cell c = contents (k);

  if (! c.is_empty ())
    retval = c(0).string_value ();

  return retval;
}

string_vector
Octave_map::keys (void) const
{
  if (length () != key_list.size ())
    {
      std::cerr << "length () = " << length () << std::endl;
      std::cerr << "key_list.size () = " << key_list.size () << std::endl;
      abort ();
    }

  return string_vector (key_list);
}

Octave_map
Octave_map::transpose (void) const
{
  assert (ndims () == 2);

  dim_vector dv = dims ();

  octave_idx_type nr = dv(0);
  octave_idx_type nc = dv(1);

  dim_vector new_dims (nc, nr);

  Octave_map retval (new_dims);

  for (const_iterator p = begin (); p != end (); p++)
    retval.assign (key(p), Cell (contents(p).transpose ()));

  return retval;
}

Octave_map
Octave_map::reshape (const dim_vector& new_dims) const
{
  Octave_map retval;

  if (new_dims != dims ())
    {
      for (const_iterator p = begin (); p != end (); p++)
	retval.assign (key(p), contents(p).reshape (new_dims));

      retval.dimensions = new_dims;
    }
  else
    retval = *this;

  return retval;
}

octave_idx_type
Octave_map::numel (void) const
{
  octave_idx_type retval;

  if (empty ())
    retval = 0;
  else
    {
      Cell tmp = contents (begin ());
      retval = tmp.numel ();
    }

  return retval;
}

void
Octave_map::resize (const dim_vector& dv, bool fill)
{
  if (dv != dims ())
    {
      for (const_iterator p = begin (); p != end (); p++)
	{
	  Cell tmp = contents(p);

	  if (fill)
	    tmp.resize(dv, Cell::resize_fill_value ());
	  else
	    tmp.resize(dv);

	  dimensions = dv;

	  assign (key(p), tmp);
	}
    }
}

Octave_map
Octave_map::concat (const Octave_map& rb, const Array<octave_idx_type>& ra_idx)
{
  Octave_map retval;

  if (length() == rb.length())
    {
      for (Octave_map::const_iterator pa = begin (); pa != end (); pa++)
	{
	  Octave_map::const_iterator pb = rb.seek (key(pa));

	  if (pb == rb.end ())
	    {
	      error ("field name mismatch in structure concatenation");
	      break;
	    }
	
	  retval.assign (key(pa),
			 contents(pa).insert (rb.contents(pb), ra_idx));
	}
    }
  else
    error ("field name mismatch in structure concatenation");

  return retval;
}

static string_vector
equiv_keys (const Octave_map& a, const Octave_map& b)
{
  string_vector retval;

  string_vector a_keys = a.keys().qsort ();
  string_vector b_keys = b.keys().qsort ();

  octave_idx_type a_len = a_keys.length ();
  octave_idx_type b_len = b_keys.length ();

  if (a_len == b_len)
    {
      for (octave_idx_type i = 0; i < a_len; i++)
	{
	  if (a_keys[i] != b_keys[i])
	    return retval;
	}

      retval = a_keys;
    }
  
  return retval;
}

Octave_map&
Octave_map::maybe_delete_elements (const octave_value_list& idx)
{
  string_vector t_keys = keys();
  octave_idx_type len = t_keys.length ();

  if (len > 0)
    {
      for (octave_idx_type i = 0; i < len; i++)
	{
	  std::string k = t_keys[i];

	  map[k] = contents (k).assign (idx, Cell());

	  if (error_state)
	    break;
	}

      if (!error_state)
	dimensions = contents(t_keys[0]).dims();
    }

  return *this;
}

Octave_map&
Octave_map::assign (const octave_value_list& idx, const Octave_map& rhs)
{
  string_vector t_keys = empty () ? rhs.keys () : equiv_keys (*this, rhs);

  if (! t_keys.empty ())
    {
      octave_idx_type len = t_keys.length ();

      for (octave_idx_type i = 0; i < len; i++)
	{
	  std::string k = t_keys[i];

	  Cell t_rhs = rhs.contents (k);

	  assign (idx, k, t_rhs);

	  if (error_state)
	    break;
	}
    }
  else
    error ("field name mismatch in structure assignment");

  return *this;
}

static dim_vector
common_size (const dim_vector& a, const dim_vector& b)
{
  dim_vector retval;

  octave_idx_type a_len = a.length ();
  octave_idx_type b_len = b.length ();

  octave_idx_type new_len = std::max (a_len, b_len);
  octave_idx_type min_len = std::min (a_len, b_len);

  retval.resize (new_len);

  for (octave_idx_type i = 0; i < min_len; i++)
    retval(i) = std::max (a(i), b(i));

  if (a_len < b_len)
    {
      for (octave_idx_type i = min_len; i < b_len; i++)
	retval(i) = b(i);
    }
  else if (a_len > b_len)
    {
      for (octave_idx_type i = min_len; i < a_len; i++)
	retval(i) = a(i);
    }

  return retval;
}

Octave_map&
Octave_map::assign (const octave_value_list& idx, const std::string& k,
		    const Cell& rhs)
{
  Cell tmp = map[k];

  octave_value fill_value = Matrix ();

  tmp.assign (idx, rhs, fill_value);

  if (! error_state)
    {
      dim_vector rhs_dims = tmp.dims ();

      dim_vector curr_dims = dims ();

      dim_vector new_dims = common_size (rhs_dims, curr_dims);

      if (new_dims != rhs_dims)
	{
	  tmp.resize (new_dims, fill_value);
	}

      if (new_dims != curr_dims)
	{
	  for (iterator p = begin (); p != end (); p++)
	    contents(p).resize (new_dims, fill_value);
	}

      dimensions = new_dims;

      maybe_add_to_key_list (k);

      map[k] = tmp;
    }

  return *this;
}

Octave_map&
Octave_map::assign (const std::string& k, const octave_value& rhs)
{
  if (empty ())
    {
      maybe_add_to_key_list (k);

      map[k] = Cell (rhs);

      dimensions = dim_vector (1, 1);
    }
  else
    {
      dim_vector dv = dims ();

      if (dv.all_ones ())
	{
	  maybe_add_to_key_list (k);

	  map[k] = Cell (rhs);
	}
      else
	error ("invalid structure assignment");
    }

  return *this;
}

Octave_map&
Octave_map::assign (const std::string& k, const Cell& rhs)
{
  if (empty ())
    {
      maybe_add_to_key_list (k);

      map[k] = rhs;

      dimensions = rhs.dims ();
    }
  else
    {
      if (dims () == rhs.dims ())
	{
	  maybe_add_to_key_list (k);

	  map[k] = rhs;
	}
      else
	error ("invalid structure assignment");
    }

  return *this;
}

Octave_map
Octave_map::index (const octave_value_list& idx)
{
  Octave_map retval;

  if (idx.length () > 0)
    {
      for (iterator p = begin (); p != end (); p++)
	{
	  Cell tmp = contents(p).index (idx);

	  if (error_state)
	    break;

	  retval.assign (key(p), tmp);
	}
    }
  else
    retval = *this;

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
