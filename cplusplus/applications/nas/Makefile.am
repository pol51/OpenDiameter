## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

SRC = src/nasd_main.cxx \
      src/nasd_pana.cxx \
      src/nasd_diameter_eap.cxx

HDR = include/nasd_call_framework.h  \
      include/nasd_config.h \
      include/nasd_defs.h \
      include/nasd_diameter_eap.h \
      include/nasd_eap_passthrough.h \
      include/nasd_eap_backend.h \
      include/nasd_policy_script.h \
      include/nasd_pana.h

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I$(srcdir)/include \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser/include \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libpana/include \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiameternasreq/include \
              -I@top_srcdir@/libdiametereap/include \
              -I../common \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

bin_PROGRAMS = nasd
sysconfdir = $(prefix)/etc/opendiameter/nasd
nobase_sysconf_DATA = config/nasd.xml \
                      config/nasd_diameter_eap.xml \
                      config/nasd_diameter_eap_dictionary.dtd \
                      config/nasd_diameter_eap_dictionary.xml \
                      config/nasd_eap_shared_secret.bin \
                      config/nasd_pana_dictionary.dtd \
                      config/nasd_pana_dictionary.xml \
                      config/nasd_pana_paa.xml

nasd_SOURCES = $(SRC) $(HDR)
nasd_LDADD = @L_LIBS@ \
             $(top_builddir)/libpana/libpana.la \
             $(top_builddir)/libeap/libeap.la \
             $(top_builddir)/libeap/libeaparchie.la \
             $(top_builddir)/libdiameter/libdiameter.la \
             $(top_builddir)/libdiameternasreq/libdiameternasreq.la \
             $(top_builddir)/libdiametereap/libdiametereap.la \
             $(top_builddir)/libdiamparser/libdiamparser.la \
             $(top_builddir)/libodutl/libodutl.la

EXTRA_DIST = config scripts

dist-hook:
