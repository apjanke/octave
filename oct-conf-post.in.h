/*

Copyright (C) 1993-2015 John W. Eaton

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

#if !defined (GNULIB_NAMESPACE)
#  define GNULIB_NAMESPACE gnulib
#endif

/* The C++ standard is evolving to allow attribute hints in a
   compiler-independent manner.  In C++ 2011 support for noreturn was
   added.  In C++ 2014 support for deprecated was added.  The Octave
   code base has been future-proofed by using macros of the form
   OCTAVE_ATTRIBUTE_NAME in place of vendor specific attribute
   mechanisms.  As compilers evolve, the underlying implementation can
   be changed with the macro definitions below.  FIXME: Update macros
   to use C++ standard attribute syntax when Octave moves to C++ 2011
   standard.  */

#if defined (__GNUC__)
   /* The following attributes are used with gcc and clang compilers.  */
#  define OCTAVE_DEPRECATED(msg) __attribute__ ((__deprecated__ (msg)))
#  define HAVE_ATTR_DEPRECATED

#  define OCTAVE_NORETURN __attribute__ ((__noreturn__))
#  define HAVE_ATTR_NORETURN

#  define OCTAVE_UNUSED __attribute__ ((__unused__))
#  define HAVE_ATTR_UNUSED
#else
#  define OCTAVE_DEPRECATED(msg)
#  define OCTAVE_NORETURN
#  define OCTAVE_UNUSED
#endif

#if ! defined (HAVE_DEV_T)
typedef short dev_t;
#endif

#if ! defined (HAVE_INO_T)
typedef unsigned long ino_t;
#endif

#if defined (_MSC_VER)
#  define __WIN32__
#  define WIN32
   /* missing parameters in macros */
#  pragma warning (disable: 4003)
   /* missing implementations in template instantiation */
#  pragma warning (disable: 4996)
   /* deprecated function names (FIXME: ???) */
#  pragma warning (disable: 4661)
#endif

#if defined (__WIN32__) && ! defined (__CYGWIN__)
#  define OCTAVE_HAVE_WINDOWS_FILESYSTEM 1
#elif defined (__CYGWIN__)
#  define OCTAVE_HAVE_WINDOWS_FILESYSTEM 1
#  define OCTAVE_HAVE_POSIX_FILESYSTEM 1
#else
#  define OCTAVE_HAVE_POSIX_FILESYSTEM 1
#endif

/* Define to 1 if we expect to have <windows.h>, Sleep, etc. */
#if defined (__WIN32__) && ! defined (__CYGWIN__)
#  define OCTAVE_USE_WINDOWS_API 1
#endif

#if defined (__APPLE__) && defined (__MACH__)
#  define OCTAVE_USE_OS_X_API 1
#endif

/* sigsetjmp is a macro, not a function. */
#if defined (sigsetjmp) && defined (HAVE_SIGLONGJMP)
#  define OCTAVE_HAVE_SIG_JUMP
#endif

#if defined (_UNICOS)
#  define F77_USES_CRAY_CALLING_CONVENTION
#endif

#if 0
#  define F77_USES_VISUAL_FORTRAN_CALLING_CONVENTION
#endif

#if defined (ENABLE_64)
#  define SIZEOF_OCTAVE_IDX_TYPE SIZEOF_INT64_T
#else
#  define SIZEOF_OCTAVE_IDX_TYPE SIZEOF_INT
#endif

/* To be able to use long doubles for 64-bit mixed arithmetics, we need
   them at least 80 bits wide and we need roundl declared in math.h.
   FIXME: Maybe substitute this by a more precise check in the future?  */
#if (SIZEOF_LONG_DOUBLE >= 10) && defined (HAVE_ROUNDL)
#  define OCTAVE_INT_USE_LONG_DOUBLE
#  if (SIZEOF_LONG_DOUBLE < 16 \
       && (defined __i386__ || defined __x86_64__) && defined __GNUC__)
#    define OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED 1
#  endif
#endif

#define OCTAVE_EMPTY_CPP_ARG

/* oct-dlldefs.h */

/* FIXME: GCC supports visibility attributes as well, even using the
   same __declspec declaration if desired.  The build system should be
   extended to support GCC and visibility attributes.  */
#if defined (_MSC_VER)
#  define OCTAVE_EXPORT __declspec(dllexport)
#  define OCTAVE_IMPORT __declspec(dllimport)
#else
   /* All other compilers, at least for now. */
#  define OCTAVE_EXPORT
#  define OCTAVE_IMPORT
#endif

/* API macro for libcruft */
#if defined (CRUFT_DLL)
#  define CRUFT_API OCTAVE_EXPORT
#else
#  define CRUFT_API OCTAVE_IMPORT
#endif

/* API macro for liboctave */
#if defined (OCTAVE_DLL)
#  define OCTAVE_API OCTAVE_EXPORT
#else
#  define OCTAVE_API OCTAVE_IMPORT
#endif

/* API macro for libinterp */
#if defined (OCTINTERP_DLL)
#  define OCTINTERP_API OCTAVE_EXPORT
#else
#  define OCTINTERP_API OCTAVE_IMPORT
#endif

/* API macro for libinterp/graphics */
#if defined (OCTGRAPHICS_DLL)
#  define OCTGRAPHICS_API OCTAVE_EXPORT
#else
#  define OCTGRAPHICS_API OCTAVE_IMPORT
#endif

/* API macro for libgui */
#if defined (OCTGUI_DLL)
#  define OCTGUI_API OCTAVE_EXPORT
#else
#  define OCTGUI_API OCTAVE_IMPORT
#endif

/* Backward compatibility */

#if defined (ENABLE_ATOMIC_REFCOUNT)
#  define USE_ATOMIC_REFCOUNT 1
#endif

#if defined (ENABLE_64)
#  define USE_64_BIT_IDX_T
#endif

#if defined (ENABLE_OPENMP)
#  define HAVE_OPENMP
#endif

#if defined (ENABLE_FLOAT_TRUNCATE)
#  define FLOAT_TRUNCATE volatile
#else
#  define FLOAT_TRUNCATE
#endif

/* oct-types.h */

#include <stdint.h>

typedef OCTAVE_IDX_TYPE octave_idx_type;

/* Tag indicating Octave config.h has been included */
#define OCTAVE_CONFIG_INCLUDED 1
