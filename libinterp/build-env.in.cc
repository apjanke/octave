// %NO_EDIT_WARNING%
/*

Copyright (C) 1996-2015 John W. Eaton

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
#  include "config.h"
#endif

#include "build-env.h"

namespace octave
{
  namespace build_env
  {
    const char *AMD_CPPFLAGS = %OCTAVE_CONF_AMD_CPPFLAGS%;

    const char *AMD_LDFLAGS = %OCTAVE_CONF_AMD_LDFLAGS%;

    const char *AMD_LIBS = %OCTAVE_CONF_AMD_LIBS%;

    const char *ARFLAGS = %OCTAVE_CONF_ARFLAGS%;

    const char *AR = %OCTAVE_CONF_AR%;

    const char *ARPACK_CPPFLAGS = %OCTAVE_CONF_ARPACK_CPPFLAGS%;

    const char *ARPACK_LDFLAGS = %OCTAVE_CONF_ARPACK_LDFLAGS%;

    const char *ARPACK_LIBS = %OCTAVE_CONF_ARPACK_LIBS%;

    const char *BLAS_LIBS = %OCTAVE_CONF_BLAS_LIBS%;

    const char *CAMD_CPPFLAGS = %OCTAVE_CONF_CAMD_CPPFLAGS%;

    const char *CAMD_LDFLAGS = %OCTAVE_CONF_CAMD_LDFLAGS%;

    const char *CAMD_LIBS = %OCTAVE_CONF_CAMD_LIBS%;

    const char *CARBON_LIBS = %OCTAVE_CONF_CARBON_LIBS%;

    const char *CC = %OCTAVE_CONF_CC%;

    const char *CCOLAMD_CPPFLAGS = %OCTAVE_CONF_CCOLAMD_CPPFLAGS%;

    const char *CCOLAMD_LDFLAGS = %OCTAVE_CONF_CCOLAMD_LDFLAGS%;

    const char *CCOLAMD_LIBS = %OCTAVE_CONF_CCOLAMD_LIBS%;

    const char *CFLAGS = %OCTAVE_CONF_CFLAGS%;

    const char *CHOLMOD_CPPFLAGS = %OCTAVE_CONF_CHOLMOD_CPPFLAGS%;

    const char *CHOLMOD_LDFLAGS = %OCTAVE_CONF_CHOLMOD_LDFLAGS%;

    const char *CHOLMOD_LIBS = %OCTAVE_CONF_CHOLMOD_LIBS%;

    const char *COLAMD_CPPFLAGS = %OCTAVE_CONF_COLAMD_CPPFLAGS%;

    const char *COLAMD_LDFLAGS = %OCTAVE_CONF_COLAMD_LDFLAGS%;

    const char *COLAMD_LIBS = %OCTAVE_CONF_COLAMD_LIBS%;

    const char *CPICFLAG = %OCTAVE_CONF_CPICFLAG%;

    const char *CPPFLAGS = %OCTAVE_CONF_CPPFLAGS%;

    const char *CURL_CPPFLAGS = %OCTAVE_CONF_CURL_CPPFLAGS%;

    const char *CURL_LDFLAGS = %OCTAVE_CONF_CURL_LDFLAGS%;

    const char *CURL_LIBS = %OCTAVE_CONF_CURL_LIBS%;

    const char *CXSPARSE_CPPFLAGS = %OCTAVE_CONF_CXSPARSE_CPPFLAGS%;

    const char *CXSPARSE_LDFLAGS = %OCTAVE_CONF_CXSPARSE_LDFLAGS%;

    const char *CXSPARSE_LIBS = %OCTAVE_CONF_CXSPARSE_LIBS%;

    const char *CXXCPP = %OCTAVE_CONF_CXXCPP%;

    const char *CXXFLAGS = %OCTAVE_CONF_CXXFLAGS%;

    const char *CXXPICFLAG = %OCTAVE_CONF_CXXPICFLAG%;

    const char *CXX = %OCTAVE_CONF_CXX%;

    const char *DEFAULT_PAGER = %OCTAVE_CONF_DEFAULT_PAGER%;

    const char *DEFS = %OCTAVE_CONF_DEFS%;

    const char *DL_LD = %OCTAVE_CONF_DL_LD%;

    const char *DL_LDFLAGS = %OCTAVE_CONF_DL_LDFLAGS%;

    const char *DL_LIBS = %OCTAVE_CONF_DL_LIBS%;

    const char *EXEEXT = %OCTAVE_CONF_EXEEXT%;

    const char *GCC_VERSION = %OCTAVE_CONF_GCC_VERSION%;

    const char *GXX_VERSION = %OCTAVE_CONF_GXX_VERSION%;

    const char *F77 = %OCTAVE_CONF_F77%;

    const char *F77_FLOAT_STORE_FLAG = %OCTAVE_CONF_F77_FLOAT_STORE_FLAG%;

    const char *F77_INTEGER_8_FLAG = %OCTAVE_CONF_F77_INTEGER_8_FLAG%;

    const char *FFLAGS = %OCTAVE_CONF_FFLAGS%;

    const char *FFTW3_CPPFLAGS = %OCTAVE_CONF_FFTW3_CPPFLAGS%;

    const char *FFTW3_LDFLAGS = %OCTAVE_CONF_FFTW3_LDFLAGS%;

    const char *FFTW3_LIBS = %OCTAVE_CONF_FFTW3_LIBS%;

    const char *FFTW3F_CPPFLAGS = %OCTAVE_CONF_FFTW3F_CPPFLAGS%;

    const char *FFTW3F_LDFLAGS = %OCTAVE_CONF_FFTW3F_LDFLAGS%;

    const char *FFTW3F_LIBS = %OCTAVE_CONF_FFTW3F_LIBS%;

    const char *FLIBS = %OCTAVE_CONF_FLIBS%;

    const char *FLTK_CPPFLAGS = %OCTAVE_CONF_FLTK_CPPFLAGS%;

    const char *FLTK_LDFLAGS = %OCTAVE_CONF_FLTK_LDFLAGS%;

    const char *FLTK_LIBS = %OCTAVE_CONF_FLTK_LIBS%;

    const char *FONTCONFIG_CPPFLAGS = %OCTAVE_CONF_FONTCONFIG_CPPFLAGS%;

    const char *FONTCONFIG_LIBS = %OCTAVE_CONF_FONTCONFIG_LIBS%;

    const char *FPICFLAG = %OCTAVE_CONF_FPICFLAG%;

    const char *FT2_CPPFLAGS = %OCTAVE_CONF_FT2_CPPFLAGS%;

    const char *FT2_LIBS = %OCTAVE_CONF_FT2_LIBS%;

    const char *GLPK_CPPFLAGS = %OCTAVE_CONF_GLPK_CPPFLAGS%;

    const char *GLPK_LDFLAGS = %OCTAVE_CONF_GLPK_LDFLAGS%;

    const char *GLPK_LIBS = %OCTAVE_CONF_GLPK_LIBS%;

    const char *GNUPLOT = %OCTAVE_CONF_GNUPLOT%;

    const char *HDF5_CPPFLAGS = %OCTAVE_CONF_HDF5_CPPFLAGS%;

    const char *HDF5_LDFLAGS = %OCTAVE_CONF_HDF5_LDFLAGS%;

    const char *HDF5_LIBS = %OCTAVE_CONF_HDF5_LIBS%;

    const char *INCLUDEDIR = %OCTAVE_CONF_INCLUDEDIR%;

    const char *LAPACK_LIBS = %OCTAVE_CONF_LAPACK_LIBS%;

    const char *LDFLAGS = %OCTAVE_CONF_LDFLAGS%;

    const char *LD_CXX = %OCTAVE_CONF_LD_CXX%;

    const char *LD_STATIC_FLAG = %OCTAVE_CONF_LD_STATIC_FLAG%;

    const char *LEXLIB = %OCTAVE_CONF_LEXLIB%;

    const char *LEX = %OCTAVE_CONF_LEX%;

    const char *LFLAGS = %OCTAVE_CONF_LFLAGS%;

    const char *LIBEXT = %OCTAVE_CONF_LIBEXT%;

    const char *LIBOCTAVE = %OCTAVE_CONF_LIBOCTAVE%;

    const char *LIBOCTINTERP = %OCTAVE_CONF_LIBOCTINTERP%;

    const char *LIBS = %OCTAVE_CONF_LIBS%;

    const char *LN_S = %OCTAVE_CONF_LN_S%;

    const char *MAGICK_CPPFLAGS = %OCTAVE_CONF_MAGICK_CPPFLAGS%;

    const char *MAGICK_LDFLAGS = %OCTAVE_CONF_MAGICK_LDFLAGS%;

    const char *MAGICK_LIBS = %OCTAVE_CONF_MAGICK_LIBS%;

    const char *LLVM_CPPFLAGS = %OCTAVE_CONF_LLVM_CPPFLAGS%;

    const char *LLVM_LDFLAGS = %OCTAVE_CONF_LLVM_LDFLAGS%;

    const char *LLVM_LIBS = %OCTAVE_CONF_LLVM_LIBS%;

    const char *MKOCTFILE_DL_LDFLAGS = %OCTAVE_CONF_MKOCTFILE_DL_LDFLAGS%;

    const char *OCTAVE_LINK_DEPS = %OCTAVE_CONF_OCTAVE_LINK_DEPS%;

    const char *OCTAVE_LINK_OPTS = %OCTAVE_CONF_OCTAVE_LINK_OPTS%;

    const char *OCTINCLUDEDIR = %OCTAVE_CONF_OCTINCLUDEDIR%;

    const char *OCTLIBDIR = %OCTAVE_CONF_OCTLIBDIR%;

    const char *OCT_LINK_DEPS = %OCTAVE_CONF_OCT_LINK_DEPS%;

    const char *OCT_LINK_OPTS = %OCTAVE_CONF_OCT_LINK_OPTS%;

    const char *OPENGL_LIBS = %OCTAVE_CONF_OPENGL_LIBS%;

    const char *OSMESA_CPPFLAGS = %OCTAVE_CONF_OSMESA_CPPFLAGS%;

    const char *OSMESA_LDFLAGS = %OCTAVE_CONF_OSMESA_LDFLAGS%;

    const char *OSMESA_LIBS = %OCTAVE_CONF_OSMESA_LIBS%;

    const char *PCRE_CPPFLAGS = %OCTAVE_CONF_PCRE_CPPFLAGS%;

    const char *PCRE_LIBS = %OCTAVE_CONF_PCRE_LIBS%;

    const char *PREFIX = %OCTAVE_CONF_PREFIX%;

    const char *PTHREAD_CFLAGS = %OCTAVE_CONF_PTHREAD_CFLAGS%;

    const char *PTHREAD_LIBS = %OCTAVE_CONF_PTHREAD_LIBS%;

    const char *QHULL_CPPFLAGS = %OCTAVE_CONF_QHULL_CPPFLAGS%;

    const char *QHULL_LDFLAGS = %OCTAVE_CONF_QHULL_LDFLAGS%;

    const char *QHULL_LIBS = %OCTAVE_CONF_QHULL_LIBS%;

    const char *QRUPDATE_CPPFLAGS = %OCTAVE_CONF_QRUPDATE_CPPFLAGS%;

    const char *QRUPDATE_LDFLAGS = %OCTAVE_CONF_QRUPDATE_LDFLAGS%;

    const char *QRUPDATE_LIBS = %OCTAVE_CONF_QRUPDATE_LIBS%;

    const char *QT_CPPFLAGS = %OCTAVE_CONF_QT_CPPFLAGS%;

    const char *QT_LDFLAGS = %OCTAVE_CONF_QT_LDFLAGS%;

    const char *QT_LIBS = %OCTAVE_CONF_QT_LIBS%;

    const char *RANLIB = %OCTAVE_CONF_RANLIB%;

    const char *RDYNAMIC_FLAG = %OCTAVE_CONF_RDYNAMIC_FLAG%;

    const char *READLINE_LIBS = %OCTAVE_CONF_READLINE_LIBS%;

    const char *SED = %OCTAVE_CONF_SED%;

    const char *SHARED_LIBS = %OCTAVE_CONF_SHARED_LIBS%;

    const char *SHLEXT = %OCTAVE_CONF_SHLEXT%;

    const char *SHLEXT_VER = %OCTAVE_CONF_SHLEXT_VER%;

    const char *SH_LD = %OCTAVE_CONF_SH_LD%;

    const char *SH_LDFLAGS = %OCTAVE_CONF_SH_LDFLAGS%;

    const char *STATIC_LIBS = %OCTAVE_CONF_STATIC_LIBS%;

    const char *TERM_LIBS = %OCTAVE_CONF_TERM_LIBS%;

    const char *UMFPACK_CPPFLAGS = %OCTAVE_CONF_UMFPACK_CPPFLAGS%;

    const char *UMFPACK_LDFLAGS = %OCTAVE_CONF_UMFPACK_LDFLAGS%;

    const char *UMFPACK_LIBS = %OCTAVE_CONF_UMFPACK_LIBS%;

    const char *WARN_CFLAGS = %OCTAVE_CONF_WARN_CFLAGS%;

    const char *WARN_CXXFLAGS = %OCTAVE_CONF_WARN_CXXFLAGS%;

    const char *X11_INCFLAGS = %OCTAVE_CONF_X11_INCFLAGS%;

    const char *X11_LIBS = %OCTAVE_CONF_X11_LIBS%;

    const char *XTRA_CFLAGS = %OCTAVE_CONF_XTRA_CFLAGS%;

    const char *XTRA_CXXFLAGS = %OCTAVE_CONF_XTRA_CXXFLAGS%;

    const char *YACC = %OCTAVE_CONF_YACC%;

    const char *YFLAGS = %OCTAVE_CONF_YFLAGS%;

    const char *Z_CPPFLAGS = %OCTAVE_CONF_Z_CPPFLAGS%;

    const char *Z_LDFLAGS = %OCTAVE_CONF_Z_LDFLAGS%;

    const char *Z_LIBS = %OCTAVE_CONF_Z_LIBS%;

    const char *config_opts = %OCTAVE_CONF_config_opts%;
  };
};
