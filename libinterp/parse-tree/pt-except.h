/*

Copyright (C) 1996-2017 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_pt_except_h)
#define octave_pt_except_h 1

#include "octave-config.h"

#include "pt-cmd.h"
#include "pt-id.h"
#include "pt-walk.h"

namespace octave
{
  class comment_list;
  class tree_statement_list;

  // Simple exception handling.

  class tree_try_catch_command : public tree_command
  {
  public:

    tree_try_catch_command (int l = -1, int c = -1)
      : tree_command (l, c),
        try_code (nullptr), catch_code (nullptr), expr_id (nullptr),
        lead_comm (nullptr), mid_comm (nullptr), trail_comm (nullptr) { }

    tree_try_catch_command (tree_statement_list *tc, tree_statement_list *cc,
                            tree_identifier *id,
                            comment_list *cl = nullptr,
                            comment_list *cm = nullptr,
                            comment_list *ct = nullptr,
                            int l = -1, int c = -1)
      : tree_command (l, c), try_code (tc), catch_code (cc), expr_id (id),
        lead_comm (cl), mid_comm (cm), trail_comm (ct) { }

    // No copying!

    tree_try_catch_command (const tree_try_catch_command&) = delete;

    tree_try_catch_command& operator = (const tree_try_catch_command&) = delete;

    ~tree_try_catch_command (void);

    tree_identifier * identifier (void) { return expr_id; }

    tree_statement_list * body (void) { return try_code; }

    tree_statement_list * cleanup (void) { return catch_code; }

    comment_list * leading_comment (void) { return lead_comm; }

    comment_list * middle_comment (void) { return mid_comm; }

    comment_list * trailing_comment (void) { return trail_comm; }

    void accept (tree_walker& tw)
    {
      tw.visit_try_catch_command (*this);
    }

  private:

    // The first block of code to attempt to execute.
    tree_statement_list *try_code;

    // The code to execute if an error occurs in the first block.
    tree_statement_list *catch_code;

    // Identifier to modify.
    tree_identifier *expr_id;

    // Comment preceding TRY token.
    comment_list *lead_comm;

    // Comment preceding CATCH token.
    comment_list *mid_comm;

    // Comment preceding END_TRY_CATCH token.
    comment_list *trail_comm;
  };

  // Simple exception handling.

  class tree_unwind_protect_command : public tree_command
  {
  public:

    tree_unwind_protect_command (int l = -1, int c = -1)
      : tree_command (l, c),
        unwind_protect_code (nullptr), cleanup_code (nullptr),
        lead_comm (nullptr), mid_comm (nullptr), trail_comm (nullptr) { }

    tree_unwind_protect_command (tree_statement_list *tc,
                                 tree_statement_list *cc,
                                 comment_list *cl = nullptr,
                                 comment_list *cm = nullptr,
                                 comment_list *ct = nullptr,
                                 int l = -1, int c = -1)
      : tree_command (l, c), unwind_protect_code (tc), cleanup_code (cc),
        lead_comm (cl), mid_comm (cm), trail_comm (ct) { }

    // No copying!

    tree_unwind_protect_command (const tree_unwind_protect_command&) = delete;

    tree_unwind_protect_command&
    operator = (const tree_unwind_protect_command&) = delete;

    ~tree_unwind_protect_command (void);

    tree_statement_list * body (void) { return unwind_protect_code; }

    tree_statement_list * cleanup (void) { return cleanup_code; }

    comment_list * leading_comment (void) { return lead_comm; }

    comment_list * middle_comment (void) { return mid_comm; }

    comment_list * trailing_comment (void) { return trail_comm; }

    void accept (tree_walker& tw)
    {
      tw.visit_unwind_protect_command (*this);
    }

  private:

    // The first body of code to attempt to execute.
    tree_statement_list *unwind_protect_code;

    // The body of code to execute no matter what happens in the first
    // body of code.
    tree_statement_list *cleanup_code;

    // Comment preceding UNWIND_PROTECT token.
    comment_list *lead_comm;

    // Comment preceding UNWIND_PROTECT_CLEANUP token.
    comment_list *mid_comm;

    // Comment preceding END_UNWIND_PROTECT token.
    comment_list *trail_comm;
  };
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED (4.4, "use 'octave::tree_try_catch_command' instead")
typedef octave::tree_try_catch_command tree_try_catch_command;

OCTAVE_DEPRECATED (4.4, "use 'octave::tree_unwind_protect_command' instead")
typedef octave::tree_unwind_protect_command tree_unwind_protect_command;

#endif

#endif
