# config.make -- autoconf rules to remake the Makefile, c-auto.h, etc.
##ifdef HOSTNAME
##ac_dir = $(gnu)/share/autoconf
##autoconf = $(ac_dir)/acspecific.m4 $(ac_dir)/acgeneral.m4 $(ac_dir)/acsite.m4
##autoheader = $(ac_dir)/acconfig.h $(ac_dir)/autoheader.m4
##
### I define $(autoconf) to acgeneral.m4 and the other Autoconf files, so
### configure automatically gets remade in the sources with a new Autoconf
### release.  But it would be bad for installers with Autoconf to remake
### configure (not to mention require Autoconf), so I take out the variable
### $(autoconf) definition before release.
### 
### BTW, xt.ac isn't really required for dvipsk or dviljk, but it doesn't
### seem worth the trouble.
### 
##configure_in = $(srcdir)/configure.in $(kpathsea_srcdir)/common.ac \
##  $(kpathsea_srcdir)/withenable.ac $(kpathsea_srcdir)/xt.ac \
##  $(kpathsea_srcdir)/acklibtool.m4
##$(srcdir)/configure: $(configure_in) $(autoconf)
##	cd $(srcdir) && autoconf
##endif

config.status: $(srcdir)/configure
	$(SHELL) $(srcdir)/configure --no-create

Makefile: $(srcdir)/Makefile.in config.status $(top_srcdir)/../make/*.make
	$(SHELL) config.status

# This rule isn't used for the top-level Makefile, but it doesn't hurt.
# We don't depend on config.status because configure always rewrites
# config.status, even when it doesn't change. Thus it might be newer
# than c-auto.h when we don't need to remake the latter.
c-auto.h: stamp-auto
stamp-auto: $(srcdir)/c-auto.h.in
	$(SHELL) config.status
	date >$(srcdir)/stamp-auto

##ifdef HOSTNAME
### autoheader reads acconfig.h (and c-auto.h.top) automatically.
##$(srcdir)/c-auto.h.in: $(srcdir)/stamp-auto.in
##$(srcdir)/stamp-auto.in: $(configure_in) $(autoheader) \
##  $(kpathsea_srcdir)/acconfig.h
##	cd $(srcdir) && autoheader --localdir=$(kpathsea_srcdir)
##	date >$(srcdir)/stamp-auto.in
##endif

# End of config.make.
