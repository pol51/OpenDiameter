## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

SRC = src/aaad_main.cxx \
      src/aaad_diameter_eap.cxx 
     
HDR = include/aaad_config.h \
      include/aaad_defs.h \
      include/aaad_call_framework.h \
      include/aaad_user_db.h \
      include/aaad_diameter_eap.h

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@srcdir@/include \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libdiamparser/include \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiametereap/include \
              -I@top_srcdir@/libdiameternasreq/include \
              -I@top_srcdir@/libodutl/include \
              -I../common \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

# Distribution
bin_PROGRAMS = aaad
sysconfdir = $(prefix)/etc/opendiameter/aaad
nobase_sysconf_DATA = config/aaad.xml \
                      config/aaad.xsd \
                      config/aaad_diameter_dictionary.dtd \
                      config/aaad_diameter_dictionary.xml \
                      config/aaad_diameter_server.xml \
                      config/aaad_diameter_server.xsd \
                      config/aaad_eap_shared_secret.bin \
                      config/aaad_user_db.xml \
                      config/aaad_user_db.xsd

aaad_SOURCES = $(SRC) $(HDR) 
aaad_LDADD = @L_LIBS@ \
             $(top_srcdir)/libeap/libeap.la \
             $(top_srcdir)/libeap/libeaparchie.la \
             $(top_srcdir)/libdiametereap/libdiametereap.la \
             $(top_srcdir)/libdiameter/libdiameter.la \
             $(top_srcdir)/libdiamparser/libdiamparser.la \
             $(top_srcdir)/libodutl/libodutl.la

EXTRA_DIST = config

dist-hook:

