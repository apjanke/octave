/*

Copyright (C) 1996, 1997, 2000, 2001, 2002, 2004, 2005, 2006, 2007,
              2008, 2009 John W. Eaton

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

#include "error.h"
#include "defun.h"
#include "oct-obj.h"
#include "ov.h"
#include "pt-binop.h"
#include "pt-bp.h"
#include "pt-walk.h"
#include "variables.h"

// TRUE means we mark | and & expressions for braindead short-circuit
// behavior.
static bool Vdo_braindead_shortcircuit_evaluation;

// Binary expressions.
 
octave_value_list
tree_binary_expression::rvalue (int nargout)
{
  octave_value_list retval;

  if (nargout > 1)
    error ("binary operator `%s': invalid number of output arguments",
           oper () . c_str ());
  else
    retval = rvalue1 (nargout);

  return retval;
}

octave_value
tree_binary_expression::rvalue1 (int)
{
  octave_value retval;

  if (error_state)
    return retval;

  if (Vdo_braindead_shortcircuit_evaluation
      && eligible_for_braindead_shortcircuit)
    {
      if (op_lhs)
        {
          octave_value a = op_lhs->rvalue1 ();

          if (! error_state)
            {
              if (a.ndims () == 2 && a.rows () == 1 && a.columns () == 1)
                {
                  bool result = false;

                  bool a_true = a.is_true ();

                  if (! error_state)
                    {
                      if (a_true)
                        {
                          if (etype == octave_value::op_el_or)
                            {
                              result = true;
                              goto done;
                            }
                        }
                      else
                        {
                          if (etype == octave_value::op_el_and)
                            goto done;
                        }

                      if (op_rhs)
                        {
                          octave_value b = op_rhs->rvalue1 ();

                          if (! error_state)
                            result = b.is_true ();
                        }

                    done:

                      if (! error_state)
                        return octave_value (result);
                    }
                }
            }
        }
    }

  if (op_lhs)
    {
      octave_value a = op_lhs->rvalue1 ();

      if (! error_state && a.is_defined () && op_rhs)
        {
          octave_value b = op_rhs->rvalue1 ();

          if (! error_state && b.is_defined ())
            {
              retval = ::do_binary_op (etype, a, b);

              if (error_state)
                retval = octave_value ();
            }
        }
    }

  return retval;
}

std::string
tree_binary_expression::oper (void) const
{
  return octave_value::binary_op_as_string (etype);
}

tree_expression *
tree_binary_expression::dup (symbol_table::scope_id scope,
                             symbol_table::context_id context) const
{
  tree_binary_expression *new_be
    = new tree_binary_expression (op_lhs ? op_lhs->dup (scope, context) : 0,
                                  op_rhs ? op_rhs->dup (scope, context) : 0,
                                  line (), column (), etype);

  new_be->copy_base (*this);

  return new_be;
}

void
tree_binary_expression::accept (tree_walker& tw)
{
  tw.visit_binary_expression (*this);
}

// Boolean expressions.
 
octave_value_list
tree_boolean_expression::rvalue (int nargout)
{
  octave_value_list retval;

  if (nargout > 1)
    error ("binary operator `%s': invalid number of output arguments",
           oper () . c_str ());
  else
    retval = rvalue1 (nargout);

  return retval;
}

octave_value
tree_boolean_expression::rvalue1 (int)
{
  octave_value retval;

  if (error_state)
    return retval;

  bool result = false;

  if (op_lhs)
    {
      octave_value a = op_lhs->rvalue1 ();

      if (! error_state)
        {
          bool a_true = a.is_true ();

          if (! error_state)
            {
              if (a_true)
                {
                  if (etype == bool_or)
                    {
                      result = true;
                      goto done;
                    }
                }
              else
                {
                  if (etype == bool_and)
                    goto done;
                }

              if (op_rhs)
                {
                  octave_value b = op_rhs->rvalue1 ();

                  if (! error_state)
                    result = b.is_true ();
                }

            done:

              if (! error_state)
                retval = octave_value (result);
            }
        }
    }

  return retval;
}

std::string
tree_boolean_expression::oper (void) const
{
  std::string retval = "<unknown>";

  switch (etype)
    {
    case bool_and:
      retval = "&&";
      break;

    case bool_or:
      retval = "||";
      break;

    default:
      break;
    }

  return retval;
}

tree_expression *
tree_boolean_expression::dup (symbol_table::scope_id scope,
                              symbol_table::context_id context) const
{
  tree_boolean_expression *new_be
    = new tree_boolean_expression (op_lhs ? op_lhs->dup (scope, context) : 0,
                                   op_rhs ? op_rhs->dup (scope, context) : 0,
                                   line (), column (), etype);

  new_be->copy_base (*this);

  return new_be;
}

DEFUN (do_braindead_shortcircuit_evaluation, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn  {Built-in Function} {@var{val} =} do_braindead_shortcircuit_evaluation ()\n\
@deftypefnx  {Built-in Function} {@var{old_val} =} do_braindead_shortcircuit_evaluation (@var{new_val})\n\
Query or set the internal variable that controls whether Octave will\n\
do short-circuit evaluation of @samp{|} and @samp{&} operators inside the\n\
conditions of if or while statements.\n\
\n\
This feature is only provided for compatibility with Matlab and should\n\
not be used unless you are porting old code that relies on this feature.\n\
\n\
To obtain short-circuit behavior for logical expressions in new programs,\n\
you should always use the @samp{&&} and @samp{||} operators.\n\
@end deftypefn")
{
  return SET_INTERNAL_VARIABLE (do_braindead_shortcircuit_evaluation);
}
