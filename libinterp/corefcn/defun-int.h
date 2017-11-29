/*

Copyright (C) 1994-2017 John W. Eaton

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

#if ! defined (octave_defun_int_h)
#define octave_defun_int_h 1

#include "octave-config.h"

#include <string>

#include "ov-builtin.h"
#include "ov-dld-fcn.h"
#include "symtab.h"
#include "version.h"

namespace octave
{
  class interpreter;
}

class octave_value;

extern OCTINTERP_API void print_usage (void);
extern OCTINTERP_API void print_usage (const std::string&);

extern OCTINTERP_API void check_version (const std::string& version,
                                         const std::string& fcn);

extern OCTINTERP_API void
install_builtin_function (octave_builtin::fcn f, const std::string& name,
                          const std::string& file, const std::string& doc,
                          bool can_hide_function = true);

extern OCTINTERP_API void
install_builtin_function (octave_builtin::meth m, const std::string& name,
                          const std::string& file, const std::string& doc,
                          bool can_hide_function = true);

extern OCTINTERP_API void
install_dld_function (octave_dld_function::fcn f, const std::string& name,
                      const octave::dynamic_library& shl, const std::string& doc,
                      bool relative = false);

extern OCTINTERP_API void
install_dld_function (octave_dld_function::meth m, const std::string& name,
                      const octave::dynamic_library& shl, const std::string& doc,
                      bool relative = false);

extern OCTINTERP_API void
install_mex_function (void *fptr, bool fmex, const std::string& name,
                      const octave::dynamic_library& shl, bool relative = false);

extern OCTINTERP_API void
alias_builtin (const std::string& alias, const std::string& name);

extern OCTINTERP_API void
install_builtin_dispatch (const std::string& name, const std::string& klass);

// Gets the shlib of the currently executing DLD function, if any.
extern OCTINTERP_API octave::dynamic_library
get_current_shlib (void);

namespace octave
{
  // FIXME: this class doesn't seem to be used in Octave.  Is it
  // really needed?

  // This is a convenience class that calls the above function automatically at
  // construction time.  When deriving new classes, you can either use it as a
  // field or as a parent (with multiple inheritance).

  class auto_shlib : public dynamic_library
  {
  public:

    auto_shlib (void) : dynamic_library (get_current_shlib ()) { }

    auto_shlib (const dynamic_library& shl) : dynamic_library (shl) { }
  };
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED (4.4, "use 'octave::auto_shlib' instead")
typedef octave::auto_shlib octave_auto_shlib;

OCTAVE_DEPRECATED (4.4, "use 'tree_evaluator::isargout' instead")
extern OCTINTERP_API bool
defun_isargout (int, int);

OCTAVE_DEPRECATED (4.4, "use 'tree_evaluator::isargout' instead")
extern OCTINTERP_API void
defun_isargout (int, int, bool *);

#endif

#define FORWARD_DECLARE_FUNX(name)              \
  extern OCTAVE_EXPORT octave_value_list        \
  name (const octave_value_list&, int)

#define FORWARD_DECLARE_METHODX(name)                           \
  extern OCTAVE_EXPORT octave_value_list                        \
  name (octave::interpreter&, const octave_value_list&, int)

#define FORWARD_DECLARE_FUN(name)               \
  FORWARD_DECLARE_FUNX (F ## name)

#define FORWARD_DECLARE_METHOD(name)            \
  FORWARD_DECLARE_METHODX (F ## name)

#define DECLARE_FUNX(name, args_name, nargout_name)             \
  OCTAVE_EXPORT octave_value_list                               \
  name (const octave_value_list& args_name, int nargout_name)

#define DECLARE_METHODX(name, interp_name, args_name, nargout_name)     \
  OCTAVE_EXPORT octave_value_list                                       \
  name (octave::interpreter& interp_name,                               \
        const octave_value_list& args_name, int nargout_name)

#define DECLARE_FUN(name, args_name, nargout_name)      \
  DECLARE_FUNX (F ## name, args_name, nargout_name)

#define DECLARE_METHOD(name, interp_name, args_name, nargout_name)      \
  DECLARE_METHODX (F ## name, interp_name, args_name, nargout_name)

#define DECLARE_STATIC_FUNX(name, args_name, nargout_name)      \
  static octave_value_list                                      \
  name (const octave_value_list& args_name, int nargout_name)

#define DECLARE_STATIC_METHODX(name, interp_name, args_name, nargout_name) \
  static octave_value_list                                              \
  name (octave::interpreter& interp_name,                               \
        const octave_value_list& args_name, int nargout_name)

#define DECLARE_STATIC_FUN(name, args_name, nargout_name)       \
  DECLARE_STATIC_FUNX (F ## name, args_name, nargout_name)

#define DECLARE_STATIC_METHOD(name, interp_name, args_name, nargout_name) \
  DECLARE_STATIC_METHODX (F ## name, interp_name, args_name, nargout_name)

// Define the code that will be used to insert the new function into
// the symbol table.  We look for this name instead of the actual
// function so that we can easily install the doc std::string too.

typedef bool (*octave_dld_fcn_installer) (const octave::dynamic_library&, bool relative);

typedef octave_function *
  (*octave_dld_fcn_getter) (const octave::dynamic_library&, bool relative);

#if defined (OCTAVE_SOURCE)
#  define DEFINE_FUN_INSTALLER_FUN(name, doc)                           \
  DEFINE_FUNX_INSTALLER_FUN(#name, F ## name, G ## name, "external-doc")
#else
#  define DEFINE_FUN_INSTALLER_FUN(name, doc)                   \
  DEFINE_FUNX_INSTALLER_FUN(#name, F ## name, G ## name, doc)
#endif

#define DEFINE_FUNX_INSTALLER_FUN(name, fname, gname, doc)              \
  extern "C"                                                            \
  OCTAVE_EXPORT                                                         \
  octave_function *                                                     \
  gname (const octave::dynamic_library& shl, bool relative)             \
  {                                                                     \
    check_version (OCTAVE_API_VERSION, name);                           \
                                                                        \
    octave_dld_function *fcn                                            \
      = octave_dld_function::create (fname, shl, name, doc);            \
                                                                        \
    if (relative)                                                       \
      fcn->mark_relative ();                                            \
                                                                        \
    return fcn;                                                         \
  }

#endif
