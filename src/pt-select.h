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

#if !defined (octave_tree_select_h)
#define octave_tree_select_h 1

#if defined (__GNUG__)
#pragma interface
#endif

#include <SLList.h>

class expression;
class tree_statement_list;

class tree_walker;

#include "pt-cmd.h"

// If.

class
tree_if_clause
{
public:

  tree_if_clause (void)
    : expr (0), list (0) { }

  tree_if_clause (tree_statement_list *l)
    : expr (0), list (l) { }

  tree_if_clause (tree_expression *e, tree_statement_list *l)
    : expr (e), list (l) { }

  ~tree_if_clause (void);

  bool is_else_clause (void)
    { return ! expr; }

  int eval (void);

  tree_expression *condition (void) { return expr; }

  tree_statement_list *commands (void) { return list; }

  void accept (tree_walker& tw);

private:

  // The condition to test.
  tree_expression *expr;

  // The list of statements to evaluate if expr is true.
  tree_statement_list *list;
};

class
tree_if_command_list : public SLList<tree_if_clause *>
{
public:

  tree_if_command_list (void)
    : SLList<tree_if_clause *> () { }

  tree_if_command_list (tree_if_clause *t)
    : SLList<tree_if_clause *> () { append (t); }

  ~tree_if_command_list (void)
    {
      while (! empty ())
	{
	  tree_if_clause *t = remove_front ();
	  delete t;
	}
    }

  void eval (void);

  void accept (tree_walker& tw);
};

class
tree_if_command : public tree_command
{
public:

  tree_if_command (int l = -1, int c = -1)
    : tree_command (l, c), list (0) { }

  tree_if_command (tree_if_command_list *lst, int l = -1, int c = -1)
    : tree_command (l, c), list (lst) { }

  ~tree_if_command (void);

  void eval (void);

  tree_if_command_list *cmd_list (void) { return list; }

  void accept (tree_walker& tw);

private:

  // List of if commands (if, elseif, elseif, ... else, endif)
  tree_if_command_list *list;
};

// Switch.

class
tree_switch_case
{
public:

  tree_switch_case (void)
    : label (0), list (0) { }

  tree_switch_case (tree_statement_list *l)
    : label (0), list (l) { }

  tree_switch_case (tree_expression *e, tree_statement_list *l)
    : label (e), list (l) { }

  ~tree_switch_case (void);

  bool is_default_case (void)
    { return ! label; }

  bool label_matches (const octave_value& val);

  int eval (const octave_value& val);

  void eval_error (void);

  tree_expression *case_label (void) { return label; }

  tree_statement_list *commands (void) { return list; }

  void accept (tree_walker& tw);

private:

  // The case label.
  tree_expression *label;

  // The list of statements to evaluate if the label matches.
  tree_statement_list *list;
};

class
tree_switch_case_list : public SLList<tree_switch_case *>
{
public:

  tree_switch_case_list (void)
    : SLList<tree_switch_case *> () { }

  tree_switch_case_list (tree_switch_case *t)
    : SLList<tree_switch_case *> () { append (t); }

  ~tree_switch_case_list (void)
    {
      while (! empty ())
	{
	  tree_switch_case *t = remove_front ();
	  delete t;
	}
    }

  void eval (const octave_value& val);

  void accept (tree_walker& tw);
};

class
tree_switch_command : public tree_command
{
public:

  tree_switch_command (int l = -1, int c = -1)
    : tree_command (l, c), expr (0), list (0) { }

  tree_switch_command (tree_expression *e, tree_switch_case_list *lst,
		       int l = -1, int c = -1)
    : tree_command (l, c), expr (e), list (lst) { }

  ~tree_switch_command (void);

  void eval (void);

  void eval_error (void);

  tree_expression *switch_value (void) { return expr; }

  tree_switch_case_list *case_list (void) { return list; }

  void accept (tree_walker& tw);

private:

  // Value on which to switch.
  tree_expression *expr;

  // List of cases (case 1, case 2, ..., default)
  tree_switch_case_list *list;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
