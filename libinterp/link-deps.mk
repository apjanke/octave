include $(top_srcdir)/liboctave/link-deps.mk

if AMCOND_ENABLE_DYNAMIC_LINKING
  LIBOCTINTERP_LINK_DEPS =
else
  LIBOCTINTERP_LINK_DEPS = $(DLDFCN_LIBS)
endif

LIBOCTINTERP_LINK_DEPS += \
  $(FT2_LIBS) \
  $(HDF5_LIBS) \
  $(Z_LIBS) \
  $(FFTW_XLIBS) \
  $(REGEX_LIBS) \
  $(OPENGL_LIBS) \
  $(X11_LIBS) \
  $(CARBON_LIBS) \
  $(LLVM_LIBS) \
  $(JAVA_LIBS) \
  $(LAPACK_LIBS)

LIBOCTINTERP_LINK_OPTS = \
  $(FT2_LDFLAGS) \
  $(HDF5_LDFLAGS) \
  $(Z_LDFLAGS) \
  $(REGEX_LDFLAGS) \
  $(FFTW_XLDFLAGS) \
  $(LLVM_LDFLAGS)

OCT_LINK_DEPS =

OCT_LINK_OPTS = $(LDFLAGS)

if AMCOND_LINK_ALL_DEPS
  LIBOCTINTERP_LINK_DEPS += $(LIBOCTAVE_LINK_DEPS)
  LIBOCTINTERP_LINK_OPTS += $(LIBOCTAVE_LINK_OPTS)

  OCTAVE_LINK_DEPS = $(LIBOCTINTERP_LINK_DEPS)
  OCTAVE_LINK_OPTS = $(LIBOCTINTERP_LINK_OPTS)

  OCT_LINK_DEPS += $(LIBOCTINTERP_LINK_DEPS)
  OCT_LINK_OPTS += $(LIBOCTINTERP_LINK_OPTS)
endif
