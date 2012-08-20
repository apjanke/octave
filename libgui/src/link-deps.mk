include $(top_srcdir)/libinterp/link-deps.mk

if AMCOND_ENABLE_DYNAMIC_LINKING
  LIBOCTGUI_LINK_DEPS =
else
  LIBOCTGUI_LINK_DEPS = $(DLDFCN_LIBS)
endif

LIBOCTGUI_LINK_DEPS += \
  $(QT_LIBS)

LIBOCTGUI_LINK_OPTS = \
  $(QT_LDFLAGS)

if AMCOND_LINK_ALL_DEPS
  LIBOCTGUI_LINK_DEPS += $(LIBOCTINTERP_LINK_DEPS)
  LIBOCTGUI_LINK_OPTS += $(LIBOCTINTERP_LINK_OPTS)

  OCTAVE_GUI_LINK_DEPS = $(LIBOCTGUI_LINK_DEPS)
  OCTAVE_GUI_LINK_OPTS = $(LIBOCTGUI_LINK_OPTS)
endif
