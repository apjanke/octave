/*

Copyright (C) 1996, 1997, 1998, 2000, 2002, 2003, 2004, 2005, 2006,
              2007 John W. Eaton

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

#include "Cell.h"
#include "error.h"
#include "oct-map.h"
#include "oct-obj.h"
#include "oct-lvalue.h"
#include "ov.h"
#include "pager.h"
#include "pt-arg-list.h"
#include "pt-bp.h"
#include "pt-id.h"
#include "pt-idx.h"
#include "pt-walk.h"
#include "utils.h"
#include "variables.h"

// Index expressions.

tree_index_expression::tree_index_expression (int l, int c)
  : tree_expression (l, c), expr (0), args (0), type (),
    arg_nm (), dyn_field () { }

tree_index_expression::tree_index_expression (tree_expression *e,
					      tree_argument_list *lst,
					      int l, int c, char t)
  : tree_expression (l, c), expr (e), args (0), type (),
    arg_nm (), dyn_field ()
{
  append (lst, t);
}

tree_index_expression::tree_index_expression (tree_expression *e,
					      const std::string& n,
					      int l, int c)
  : tree_expression (l, c), expr (e), args (0), type (),
    arg_nm (), dyn_field ()
{
  append (n);
}

tree_index_expression::tree_index_expression (tree_expression *e,
					      tree_expression *df,
					      int l, int c)
  : tree_expression (l, c), expr (e), args (0), type (),
    arg_nm (), dyn_field ()
{
  append (df);
}

void
tree_index_expression::append (tree_argument_list *lst, char t)
{
  args.push_back (lst);
  type.append (1, t);
  arg_nm.push_back (lst ? lst->get_arg_names () : string_vector ());
  dyn_field.push_back (static_cast<tree_expression *> (0));
}

void
tree_index_expression::append (const std::string& n)
{
  args.push_back (static_cast<tree_argument_list *> (0));
  type.append (".");
  arg_nm.push_back (n);
  dyn_field.push_back (static_cast<tree_expression *> (0));
}

void
tree_index_expression::append (tree_expression *df)
{
  args.push_back (static_cast<tree_argument_list *> (0));
  type.append (".");
  arg_nm.push_back ("");
  dyn_field.push_back (df);
}

tree_index_expression::~tree_index_expression (void)
{
  delete expr;

  while (! args.empty ())
    {
      std::list<tree_argument_list *>::iterator p = args.begin ();
      delete *p;
      args.erase (p);
    }
}

bool
tree_index_expression::has_magic_end (void) const
{
  for (std::list<tree_argument_list *>::const_iterator p = args.begin ();
       p != args.end ();
       p++)
    {
      tree_argument_list *elt = *p;

      if (elt && elt->has_magic_end ())
	return true;
    }
  
  return false;
}

// This is useful for printing the name of the variable in an indexed
// assignment.

std::string
tree_index_expression::name (void) const
{
  return expr->name ();
}

static Cell
make_subs_cell (tree_argument_list *args, const string_vector& arg_nm)
{
  Cell retval;

  octave_value_list arg_values;

  if (args)
    arg_values = args->convert_to_const_vector ();

  if (! error_state)
    {
      int n = arg_values.length ();

      if (n > 0)
	{
	  arg_values.stash_name_tags (arg_nm);

	  retval.resize (dim_vector (1, n));

	  for (int i = 0; i < n; i++)
	    retval(0,i) = arg_values(i);
	}
    }

  return retval;
}

static inline octave_value_list
make_value_list (tree_argument_list *args, const string_vector& arg_nm,
		 const octave_value *object)
{
  octave_value_list retval;

  if (args)
    retval = args->convert_to_const_vector (object);

  if (! error_state)
    {
      int n = retval.length ();

      if (n > 0)
	retval.stash_name_tags (arg_nm);
    }

  return retval;
}

std::string
tree_index_expression::get_struct_index
  (std::list<string_vector>::const_iterator p_arg_nm,
   std::list<tree_expression *>::const_iterator p_dyn_field) const
{
  std::string fn = (*p_arg_nm)(0);

  if (fn.empty ())
    {
      tree_expression *df = *p_dyn_field;

      if (df)
	{
	  octave_value t = df->rvalue ();

	  if (! error_state)
	    {
	      fn = t.string_value ();

	      if (! valid_identifier (fn))
		::error ("invalid structure field name `%s'", fn.c_str ());
	    }
	}
      else
	panic_impossible ();
    }

  return fn;
}

Octave_map
tree_index_expression::make_arg_struct (void) const
{
  int n = args.size ();

  Cell type_field (n, 1);
  Cell subs_field (n, 1);

  std::list<tree_argument_list *>::const_iterator p_args = args.begin ();
  std::list<string_vector>::const_iterator p_arg_nm = arg_nm.begin ();
  std::list<tree_expression *>::const_iterator p_dyn_field = dyn_field.begin ();

  Octave_map m;

  for (int i = 0; i < n; i++)
    {
      switch (type[i])
	{
	case '(':
	  subs_field(i) = make_subs_cell (*p_args, *p_arg_nm);
	  break;

	case '{':
	  subs_field(i) = make_subs_cell (*p_args, *p_arg_nm);
	  break;

	case '.':
	  subs_field(i) = get_struct_index (p_arg_nm, p_dyn_field);
	  break;

	default:
	  panic_impossible ();
	}

      if (error_state)
	return m;

      p_args++;
      p_arg_nm++;
      p_dyn_field++;
    }

  m.assign ("type", type_field);
  m.assign ("subs", subs_field);

  return m;
}

static void
gripe_invalid_inquiry_subscript (void)
{
  error ("invalid dimension inquiry of a non-existent value");
}

static void
gripe_indexed_cs_list (void)
{
  error ("a cs-list cannot be further indexed");
}

octave_value_list
tree_index_expression::rvalue (int nargout)
{
  octave_value_list retval;

  if (error_state)
    return retval;

  octave_value first_expr_val;

  octave_value_list first_args;

  bool have_args = false;

  if (expr->is_identifier () && type[0] == '(')
    {
      tree_identifier *id = dynamic_cast<tree_identifier *> (expr);

      if (! (id->is_variable () || args.empty ()))
	{
	  tree_argument_list *al = *(args.begin ());

	  size_t n = al ? al->length () : 0;

	  if (n > 0)
	    {
	      string_vector anm = *(arg_nm.begin ());

	      first_expr_val = id->do_lookup  (al, anm, first_args, have_args);
	    }
	}
    }

  if (! error_state)
    {
      if (first_expr_val.is_undefined ())
	first_expr_val = expr->rvalue ();

      octave_value tmp = first_expr_val;
      octave_idx_type tmpi = 0;

      std::list<octave_value_list> idx;

      int n = args.size ();

      std::list<tree_argument_list *>::iterator p_args = args.begin ();
      std::list<string_vector>::iterator p_arg_nm = arg_nm.begin ();
      std::list<tree_expression *>::iterator p_dyn_field = dyn_field.begin ();

      for (int i = 0; i < n; i++)
	{
	  if (i > 0)
	    {
	      tree_argument_list *al = *p_args;

	      if (al && al->has_magic_end ())
		{
		  // We have an expression like
		  //
		  //   x{end}.a(end)
		  //
		  // and we are looking at the argument list that
		  // contains the second (or third, etc.) "end" token,
		  // so we must evaluate everything up to the point of
		  // that argument list so we can pass the appropriate
		  // value to the built-in __end__ function.

		  octave_value_list tmp_list
		    = tmp.subsref (type.substr (tmpi, i - tmpi), idx, nargout);

		  tmp = tmp_list(0);
                  tmpi = i;
                  idx.clear ();
                  
                  if (tmp.is_cs_list ())
                    gripe_indexed_cs_list ();

		  if (error_state)
		    break;
		}
	    }

	  switch (type[i])
	    {
	    case '(':
	      if (have_args)
		{
		  idx.push_back (first_args);
		  have_args = false;
		}
	      else
		idx.push_back (make_value_list (*p_args, *p_arg_nm, &tmp));
	      break;

	    case '{':
	      idx.push_back (make_value_list (*p_args, *p_arg_nm, &tmp));
	      break;

	    case '.':
	      idx.push_back (octave_value (get_struct_index (p_arg_nm, p_dyn_field)));
	      break;

	    default:
	      panic_impossible ();
	    }

	  if (error_state)
	    break;

	  p_args++;
	  p_arg_nm++;
	  p_dyn_field++;
	}

      if (! error_state)
	retval = tmp.subsref (type.substr (tmpi, n - tmpi), idx, nargout);
    }

  return retval;
}

octave_value
tree_index_expression::rvalue (void)
{
  octave_value retval;

  octave_value_list tmp = rvalue (1);

  if (! tmp.empty ())
    retval = tmp(0);

  return retval;
}

octave_lvalue
tree_index_expression::lvalue (void)
{
  octave_lvalue retval;

  std::list<octave_value_list> idx;

  int n = args.size ();

  std::list<tree_argument_list *>::iterator p_args = args.begin ();
  std::list<string_vector>::iterator p_arg_nm = arg_nm.begin ();
  std::list<tree_expression *>::iterator p_dyn_field = dyn_field.begin ();

  retval = expr->lvalue ();

  if (! error_state)
    {
      // I think it is OK to have a copy here.

      const octave_value *tro = retval.object ();

      octave_value first_retval_object;

      if (tro)
	first_retval_object = *tro;

      octave_value tmp = first_retval_object;
      octave_idx_type tmpi = 0;
      std::list<octave_value_list> tmpidx;

      for (int i = 0; i < n; i++)
	{
          if (retval.numel () != 1)
            gripe_indexed_cs_list ();

          if (i > 0)
            {
              tree_argument_list *al = *p_args;

              if (al && al->has_magic_end ())
                {
                  // We have an expression like
                  //
                  //   x{end}.a(end)
                  //
                  // and we are looking at the argument list that
                  // contains the second (or third, etc.) "end" token,
                  // so we must evaluate everything up to the point of
                  // that argument list so we pass the appropriate
                  // value to the built-in __end__ function.

                  if (tmp.is_defined ())
                    {
                      if (tmpi < i)
                        {
                          tmp = tmp.subsref (type.substr (tmpi, i - tmpi), tmpidx, true);

                          tmpi = i;
                          tmpidx.clear ();
                        }
                    }
                  else
                    gripe_invalid_inquiry_subscript ();

                  if (tmp.is_undefined ())
                    gripe_invalid_inquiry_subscript ();
                  else if (tmp.is_cs_list ())
                    gripe_indexed_cs_list ();

                  if (error_state)
                    break;
                }
            }

	  switch (type[i])
	    {
	    case '(':
              {
                octave_value_list tidx
                  = make_value_list (*p_args, *p_arg_nm, &tmp);
                idx.push_back (tidx);
                tmpidx.push_back (tidx);
              }
              break;

	    case '{':
	      {
		octave_value_list tidx
		  = make_value_list (*p_args, *p_arg_nm, &tmp);

		if (! tidx.all_scalars ())
		  {
                    octave_idx_type nel = 1;

                    octave_idx_type nidx = tidx.length ();

                    // Possible cs-list.
                    bool has_magic_colon = tidx.has_magic_colon ();
                    dim_vector dims;

		    if (has_magic_colon)
		      {
                        if (tmp.is_defined ())
                          {
                            if (tmpi < i)
                              {
                                tmp = tmp.subsref (type.substr (tmpi, i - tmpi), tmpidx, true);
                                tmpi = i;
                                tmpidx.clear ();
                              }

                            if (tmp.is_undefined ())
                              gripe_invalid_inquiry_subscript ();
                            else if (tmp.is_cs_list ())
                              gripe_indexed_cs_list ();

                            dims = (nidx == 1) ? dim_vector (tmp.numel (), 1) : tmp.dims ();
                          }
                        else
                          gripe_invalid_inquiry_subscript ();

                        if (error_state)
                          break;
                      }

                    for (octave_idx_type j = 0; j < nidx; j++)
                      {
                        octave_value val = tidx(j);

                        if (val.is_magic_colon ())
                          nel *= dims (j);
                        else
                          nel *= val.numel ();
                      }

                    retval.numel (nel);
		  }

                if (error_state)
                  break;

		idx.push_back (tidx);
                tmpidx.push_back (tidx);

	      }
	      break;

	    case '.':
	      {
		octave_value tidx = get_struct_index (p_arg_nm, p_dyn_field);
                if (error_state)
                  break;

                if (i > 0 && type [i-1] == '(')
                  {
                    // Possible cs-list.

                    octave_value_list xidx = idx.back ();

                    if (! xidx.all_scalars ())
                      {
                        octave_idx_type nel = 1;

                        octave_idx_type nidx = xidx.length ();

                        // Possible cs-list.
                        bool has_magic_colon = xidx.has_magic_colon ();
                        dim_vector dims;

                        if (has_magic_colon)
                          {
                            // Evaluate everything up to the point preceding the last paren.
                            if (tmp.is_defined ())
                              {
                                if (tmpi < i-1)
                                  {
                                    tmpidx.pop_back ();
                                    tmp = tmp.subsref (type.substr (tmpi, i-1 - tmpi), tmpidx, true);
                                    tmpi = i - 1;
                                    tmpidx.clear ();
                                    tmpidx.push_back (xidx);
                                  }

                                if (tmp.is_undefined ())
                                  gripe_invalid_inquiry_subscript ();
                                else if (tmp.is_cs_list ())
                                  gripe_indexed_cs_list ();

                                dims = (nidx == 1) ? dim_vector (tmp.numel (), 1) : tmp.dims ();
                              }
                            else
                              gripe_invalid_inquiry_subscript ();

                            if (error_state)
                              break;
                          }

                        for (octave_idx_type j = 0; j < nidx; j++)
                          {
                            octave_value val = xidx(j);

                            if (val.is_magic_colon ())
                              nel *= dims (j);
                            else
                              nel *= val.numel ();
                          }

                        retval.numel (nel);
                      }
                  }
                else
                  {
                    // A plain struct component can also yield a list reference.
                    if (tmp.is_defined () && tmpi < i)
                        tmp = tmp.subsref (type.substr (tmpi, i - tmpi), tmpidx, true);

                    tmpi = i;
                    tmpidx.clear ();

                    if (tmp.is_cs_list ())
                      gripe_indexed_cs_list ();
                    else if (tmp.is_map ())
                      retval.numel (tmp.numel ());
                    else
                      tmp = Octave_map ();

                    if (error_state)
                      break;

                  }

                if (error_state)
                  break;

                idx.push_back (tidx);
                tmpidx.push_back (tidx);

	      }
	      break;

	    default:
	      panic_impossible ();
	    }

	  if (error_state)
	    break;

	  p_args++;
	  p_arg_nm++;
	  p_dyn_field++;
	}

      if (! error_state)
	retval.set_index (type, idx);

    }

  return retval;
}

/*
%!test
%! x = {1, 2, 3};
%! [x{:}] = deal (4, 5, 6);
%! assert (x, {4, 5, 6});

%!test
%! [x.a, x.b.c] = deal (1, 2);
%! assert (x.a == 1 && x.b.c == 2);

%!test
%! [x.a, x(2).b] = deal (1, 2);
%! assert (x(1).a == 1 && isempty (x(2).a) && isempty (x(1).b) && x(2).b == 2);

%!test
%! x = struct (zeros (0, 1), {"a", "b"});
%! x(2).b = 1;
%! assert (x(2).b == 1);

%!test
%! x = struct (zeros (0, 1), {"a", "b"});
%! x(2).b = 1;
%! assert (x(2).b == 1);
*/

tree_index_expression *
tree_index_expression::dup (symbol_table::scope_id scope,
			    symbol_table::context_id context)
{
  tree_index_expression *new_idx_expr
    = new tree_index_expression (line (), column ());

  new_idx_expr->expr = expr ? expr->dup (scope, context) : 0;

  std::list<tree_argument_list *> new_args;

  for (std::list<tree_argument_list *>::iterator p = args.begin ();
       p != args.end ();
       p++)
    {
      tree_argument_list *elt = *p;

      new_args.push_back (elt ? elt->dup (scope, context) : 0);
    }

  new_idx_expr->args = new_args;
  
  new_idx_expr->type = type;

  new_idx_expr->arg_nm = arg_nm;

  std::list<tree_expression *> new_dyn_field;

  for (std::list<tree_expression *>::iterator p = dyn_field.begin ();
       p != dyn_field.end ();
       p++)
    {
      tree_expression *elt = *p;

      new_dyn_field.push_back (elt ? elt->dup (scope, context) : 0);
    }

  new_idx_expr->dyn_field = new_dyn_field;

  new_idx_expr->copy_base (*this);
  
  return new_idx_expr;
}

void
tree_index_expression::accept (tree_walker& tw)
{
  tw.visit_index_expression (*this);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
