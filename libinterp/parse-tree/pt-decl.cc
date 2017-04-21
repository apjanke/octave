/*

Copyright (C) 1996-2017 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "defun.h"
#include "error.h"
#include "pt-cmd.h"
#include "ov.h"
#include "oct-lvalue.h"
#include "pt-bp.h"
#include "pt-decl.h"
#include "pt-exp.h"
#include "pt-id.h"
#include "pt-walk.h"
#include "utils.h"
#include "variables.h"

namespace octave
{
  // Declarations (global, static, etc.).

  tree_decl_elt::~tree_decl_elt (void)
  {
    delete id;
    delete expr;
  }

  tree_decl_elt *
  tree_decl_elt::dup (symbol_table::scope_id scope,
                      symbol_table::context_id context) const
  {
    return new tree_decl_elt (id ? id->dup (scope, context) : 0,
                              expr ? expr->dup (scope, context) : 0);
  }

  // Initializer lists for declaration statements.

  tree_decl_init_list *
  tree_decl_init_list::dup (symbol_table::scope_id scope,
                            symbol_table::context_id context) const
  {
    tree_decl_init_list *new_dil = new tree_decl_init_list ();

    for (const tree_decl_elt* elt : *this)
      new_dil->append (elt ? elt->dup (scope, context) : 0);

    return new_dil;
  }

  // Base class for declaration commands (global, static).

  tree_decl_command::~tree_decl_command (void)
  {
    delete init_list;
  }

  // Global.

  tree_command *
  tree_global_command::dup (symbol_table::scope_id scope,
                            symbol_table::context_id context) const
  {
    return
      new tree_global_command (init_list ? init_list->dup (scope, context) : 0,
                               line (), column ());
  }

  // Static.

  tree_command *
  tree_persistent_command::dup (symbol_table::scope_id scope,
                                symbol_table::context_id context) const
  {
    return
      new tree_persistent_command (init_list ? init_list->dup (scope, context)
                                   : 0,
                                   line (), column ());
  }
}
