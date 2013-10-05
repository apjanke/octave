GNULIB_LINK_DEPS = \
  $(COPYSIGNF_LIBM)\
  $(COPYSIGN_LIBM)\
  $(FLOORF_LIBM)\
  $(FLOOR_LIBM)\
  $(GETHOSTNAME_LIB)\
  $(LIBSOCKET)\
  $(LIB_NANOSLEEP)\
  $(LIB_SELECT)\
  $(LTLIBINTL)\
  $(ROUNDF_LIBM)\
  $(ROUND_LIBM)\
  $(TRUNCF_LIBM)\
  $(TRUNC_LIBM)

LIBOCTAVE_LINK_DEPS = \
  $(GNULIB_LINK_DEPS) \
  $(CURL_LIBS) \
  $(SPARSE_XLIBS) \
  $(ARPACK_LIBS) \
  $(QRUPDATE_LIBS) \
  $(FFTW_XLIBS) \
  $(LAPACK_LIBS) \
  $(BLAS_LIBS) \
  $(READLINE_LIBS) \
  $(TERM_LIBS) \
  $(LIBGLOB) \
  $(REGEX_LIBS) \
  $(DL_LIBS) \
  $(PTHREAD_LIBS) \
  $(FLIBS) \
  $(LIBS)

LIBOCTAVE_LINK_OPTS = \
  $(CURL_LDFLAGS) \
  $(SPARSE_XLDFLAGS) \
  $(ARPACK_LDFLAGS) \
  $(QRUPDATE_LDFLAGS) \
  $(FFTW_XLDFLAGS)
