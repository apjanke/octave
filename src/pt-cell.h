/*

Copyright (C) 1999 John W. Eaton

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

#if !defined (octave_tree_cell_h)
#define octave_tree_cell_h 1

#include <iostream>

class octave_value;
class octave_value_list;
class tree_argument_list;

class tree_walker;

#include "pt-mat.h"

// General cells.

class
tree_cell : public tree_matrix
{
public:

  tree_cell (tree_argument_list *row = 0, int l = -1, int c = -1)
    : tree_matrix (row, l, c) { }

  ~tree_cell (void) { }

  bool rvalue_ok (void) const { return true; }

  octave_value rvalue (void);

  octave_value_list rvalue (int);

  tree_expression *dup (symbol_table *sym_tab);

  void accept (tree_walker& tw);

private:

  // No copying!

  tree_cell (const tree_cell&);

  tree_cell& operator = (const tree_cell&);
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
