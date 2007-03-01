## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libpana/include \
              -I../common \
	      -I@top_srcdir@/libpana/test \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

# Distribution
if compile_EAPTLS
noinst_PROGRAMS = pacd client_pana_eaptls
else
noinst_PROGRAMS = pacd
endif

pacd_SOURCES = pacd.cxx pacd_config.cxx pacd_config.h
pacd_LDADD = @L_LIBS@ \
             $(top_builddir)/libpana/libpana.la \
             $(top_builddir)/libeap/libeap.la \
             $(top_builddir)/libeap/libeaparchie.la \
             $(top_builddir)/libdiamparser/libdiamparser.la \
             $(top_builddir)/libodutl/libodutl.la

if compile_EAPTLS
client_pana_eaptls_SOURCES = client_pana_eap-tls.cxx
client_pana_eaptls_LDADD = @L_LIBS@ \
             $(top_builddir)/libpana/libpana.la \
             $(top_builddir)/libeap/libeap.la \
             $(top_builddir)/libeap/libeaparchie.la \
             $(top_builddir)/libeap/libeaptls.la \
             $(top_builddir)/libdiamparser/libdiamparser.la \
             $(top_builddir)/libodutl/libodutl.la

endif

dist-hook:
