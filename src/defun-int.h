/*

Copyright (C) 1996, 1997 John W. Eaton

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

#if !defined (octave_defun_int_h)
#define octave_defun_int_h 1

#include <string>

#include "ov-builtin.h"
#include "ov-dld-fcn.h"
#include "ov-mapper.h"
#include "symtab.h"
#include "version.h"

class octave_value;

extern OCTINTERP_API void print_usage (void);
extern OCTINTERP_API void print_usage (const std::string&) GCC_ATTR_DEPRECATED;

extern OCTINTERP_API void check_version (const std::string& version, const std::string& fcn);

extern OCTINTERP_API void
install_builtin_mapper (octave_mapper *mf);

extern OCTINTERP_API void
install_builtin_function (octave_builtin::fcn f, const std::string& name,
			  const std::string& doc, bool is_text_fcn = false,
			  bool can_hide_function = true);

extern OCTINTERP_API void
install_dld_function (octave_dld_function::fcn f, const std::string& name,
		      const octave_shlib& shl,
		      const std::string& doc, bool is_text_fcn = false,
		      bool relative = false);

extern OCTINTERP_API void
install_mex_function (void *fptr, bool fmex, const std::string& name,
		      const octave_shlib& shl, bool is_text_fcn = false,
		      bool relative = false);

extern OCTINTERP_API void
alias_builtin (const std::string& alias, const std::string& name);

#define DECLARE_FUNX(name, args_name, nargout_name) \
  OCTAVE_EXPORT octave_value_list \
  name (const octave_value_list& args_name, int nargout_name)

#define DECLARE_FUN(name, args_name, nargout_name) \
  DECLARE_FUNX (F ## name, args_name, nargout_name)

// Define the code that will be used to insert the new function into
// the symbol table.  We look for this name instead of the actual
// function so that we can easily install the doc std::string too.

typedef bool (*octave_dld_fcn_installer) (const octave_shlib&, bool relative);

#define DEFINE_FUN_INSTALLER_FUN(name, doc) \
  DEFINE_FUN_INSTALLER_FUN2(name, doc, CXX_ABI)

#define DEFINE_FUN_INSTALLER_FUN2(name, doc, cxx_abi) \
  DEFINE_FUN_INSTALLER_FUN3(name, doc, cxx_abi)

#define DEFINE_FUN_INSTALLER_FUN3(name, doc, cxx_abi) \
  DEFINE_FUNX_INSTALLER_FUN3(#name, F ## name, FS ## name, doc, cxx_abi)

#define DEFINE_FUNX_INSTALLER_FUN(name, fname, fsname, doc) \
  DEFINE_FUNX_INSTALLER_FUN2(name, fname, fsname, doc, CXX_ABI)

#define DEFINE_FUNX_INSTALLER_FUN2(name, fname, fsname, doc, cxx_abi) \
  DEFINE_FUNX_INSTALLER_FUN3(name, fname, fsname, doc, cxx_abi)

#define DEFINE_FUNX_INSTALLER_FUN3(name, fname, fsname, doc, cxx_abi) \
  extern "C" \
  OCTAVE_EXPORT \
  bool \
  fsname ## _ ## cxx_abi (const octave_shlib& shl, bool relative) \
  { \
    check_version (OCTAVE_API_VERSION, name); \
    install_dld_function (fname, name, shl, doc, false, relative); \
    return error_state ? false : true; \
  }

// MAKE_BUILTINS is defined to extract function names and related
// information and create the *.df files that are eventually used to
// create the builtins.cc file.

#if defined (MAKE_BUILTINS)

// Generate code to install name in the symbol table.  The script
// mkdefs will create a .def file for every .cc file that uses DEFUN,
// or DEFCMD.

#define DEFUN_INTERNAL(name, args_name, nargout_name, is_text_fcn, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFUN_INTERNAL (name, args_name, nargout_name, is_text_fcn, doc) \
  END_INSTALL_BUILTIN

#define DEFCONSTFUN_INTERNAL(name, args_name, nargout_name, is_text_fcn, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFCONSTFUN_INTERNAL (name, args_name, nargout_name, is_text_fcn, doc) \
  END_INSTALL_BUILTIN

#define DEFUNX_INTERNAL(name, fname, args_name, nargout_name, \
			is_text_fcn, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFUNX_INTERNAL (name, fname, args_name, nargout_name, is_text_fcn, doc) \
  END_INSTALL_BUILTIN

// Generate code to install name in the symbol table.  The script
// mkdefs will create a .def file for every .cc file that uses
// DEFUN_DLD.

#define DEFUN_DLD_INTERNAL(name, args_name, nargout_name, is_text_fcn, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFUN_DLD_INTERNAL (name, args_name, nargout_name, is_text_fcn, doc) \
  END_INSTALL_BUILTIN

#define DEFUNX_DLD_INTERNAL(name, fname, args_name, nargout_name, \
			    is_text_fcn, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFUNX_DLD_INTERNAL (name, fname, args_name, nargout_name, is_text_fcn, doc) \
  END_INSTALL_BUILTIN

// Generate code for making another name for an existing function.

#define DEFALIAS_INTERNAL(alias, name) \
  BEGIN_INSTALL_BUILTIN \
    XDEFALIAS_INTERNAL(alias, name) \
  END_INSTALL_BUILTIN

#define DEFUN_MAPPER_INTERNAL(name, ch_map, d_b_map, c_b_map, d_d_map, \
			      d_c_map, c_c_map, lo, hi, \
			      ch_map_flag, can_ret_cmplx_for_real, doc) \
  BEGIN_INSTALL_BUILTIN \
    XDEFUN_MAPPER_INTERNAL(name, ch_map, d_b_map, c_b_map, d_d_map, \
		           d_c_map, c_c_map, lo, hi, \
			   ch_map_flag, can_ret_cmplx_for_real, doc) \
  END_INSTALL_BUILTIN

#else /* ! MAKE_BUILTINS */

// Generate the first line of the function definition.  This ensures
// that the internal functions all have the same signature.

#define DEFUN_INTERNAL(name, args_name, nargout_name, is_text_fcn, doc) \
  DECLARE_FUN (name, args_name, nargout_name)

#define DEFCONSTFUN_INTERNAL(name, args_name, nargout_name, is_text_fcn, doc) \
  DECLARE_FUN (name, args_name, nargout_name)

#define DEFUNX_INTERNAL(name, fname, args_name, nargout_name, \
			is_text_fcn, doc) \
  DECLARE_FUNX (fname, args_name, nargout_name)

// No definition is required for an alias.

#define DEFALIAS_INTERNAL(alias, name)

// How mapper functions are actually installed.

// FIXME -- Really want to avoid the following casts, since
// (as always with casts) it may mask some real errors...

#define DEFUN_MAPPER_INTERNAL(name, ch_map, d_b_map, c_b_map, d_d_map, \
			      d_c_map, c_c_map, lo, hi, \
			      ch_map_flag, can_ret_cmplx_for_real, doc) \
  install_builtin_mapper \
    (new octave_mapper \
     (ch_map, d_b_map, c_b_map, d_d_map, d_c_map, c_c_map, \
      lo, hi, ch_map_flag, can_ret_cmplx_for_real, #name, doc))

#endif /* ! MAKE_BUILTINS */

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
