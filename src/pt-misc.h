/*

Copyright (C) 1994, 1995, 1996, 1997, 1999, 2002, 2003, 2004, 2005,
              2006, 2007 John W. Eaton

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

#if !defined (octave_tree_misc_h)
#define octave_tree_misc_h 1

class Cell;

class octave_value;
class octave_value_list;

class tree_identifier;
class tree_index_expression;
class tree_va_return_list;

class tree_walker;

#include "base-list.h"
#include "pt-decl.h"

// Parameter lists.  Used to hold the list of input and output
// parameters in a function definition.  Elements are identifiers
// only.

class
tree_parameter_list : public octave_base_list<tree_decl_elt *>
{
public:

  tree_parameter_list (void)
    : marked_for_varargs (0) { }

  tree_parameter_list (tree_decl_elt *t)
    : marked_for_varargs (0) { append (t); }

  ~tree_parameter_list (void);

  void mark_as_formal_parameters (void);

  void mark_varargs (void) { marked_for_varargs = 1; }

  bool takes_varargs (void) const { return marked_for_varargs != 0; }

  void mark_varargs_only (void) { marked_for_varargs = -1; }

  bool varargs_only (void) { return (marked_for_varargs < 0); }

  void initialize_undefined_elements (const std::string& warnfor,
				      int nargout, const octave_value& val);

  void define_from_arg_vector (const octave_value_list& args);

  void undefine (void);

  bool is_defined (void);

  octave_value_list convert_to_const_vector (const Cell& varargout);

  tree_parameter_list *dup (symbol_table *sym_tab);

  void accept (tree_walker& tw);

private:

  int marked_for_varargs;

  // No copying!

  tree_parameter_list (const tree_parameter_list&);

  tree_parameter_list& operator = (const tree_parameter_list&);
};

// Return lists.  Used to hold the right hand sides of multiple
// assignment expressions.

class
tree_return_list : public octave_base_list<tree_index_expression *>
{
public:

  tree_return_list (void) { }

  tree_return_list (tree_index_expression *t) { append (t); }

  ~tree_return_list (void);

  tree_return_list *dup (symbol_table *sym_tab);

  void accept (tree_walker& tw);

private:

  // No copying!

  tree_return_list (const tree_return_list&);

  tree_return_list& operator = (const tree_return_list&);
};

class
tree_va_return_list : public octave_base_list<octave_value>
{
public:

  tree_va_return_list (void) { }

  ~tree_va_return_list (void) { }

private:

  // No copying!

  tree_va_return_list (const tree_va_return_list&);

  tree_va_return_list& operator = (const tree_va_return_list&);
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
