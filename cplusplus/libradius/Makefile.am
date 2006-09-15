## Process this file with automake to produce Makefile.in
EXTRA_DIST = $(top_srcdir)/libradius/config

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libradius/include \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

RADIUS_HDRS = include/radius_attribute.h \
              include/radius_defs.h \
              include/radius_dictionary.h \
              include/radius_packet.h \
              include/radius_parser.h

RADIUS_SRC  = src/radius_dictionary.cxx \
              src/radius_parser.cxx

lib_LTLIBRARIES = libradius.la
libradius_la_SOURCES = $(RADIUS_SRC) $(RADIUS_HDRS) $(PLATFORM_SRC)
libradius_la_LDFLAGS = -version-info 1:0:0 
libradius_la_AR = $(AR) -qcs

LDADD = @L_LIBS@ $(top_builddir)/libradius/libradius.la

noinst_PROGRAMS = radius_parser_test radius_dict_test

radius_dict_test_SOURCES = test/DictTest.cxx
radius_dict_test_LDADD = @L_LIBS@ \
                         $(top_builddir)/libradius/libradius.la \
			 $(top_srcdir)/libdiamparser/libdiamparser.la \
			 $(top_srcdir)/libodutl/libodutl.la

radius_parser_test_SOURCES = test/ParserTest.cxx 
radius_parser_test_LDADD = @L_LIBS@ \
                           $(top_builddir)/libradius/libradius.la \
			   $(top_srcdir)/libdiamparser/libdiamparser.la \
			   $(top_srcdir)/libodutl/libodutl.la

dist-hook:

