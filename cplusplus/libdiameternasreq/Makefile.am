## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser/include \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiameternasreq/include \
              -D@HOST_OS@

AM_LDFLAGS = @L_LDFLAGS@

DIAMETER_NASREQ_SRC = src/diameter_nasreq_client_fsm.cxx \
                      src/diameter_nasreq_client_session.cxx \
                      src/diameter_nasreq_server_fsm.cxx \
                      src/diameter_nasreq_server_session.cxx \
                      src/diameter_nasreq_aareq_parser.cxx \
                      src/diameter_nasreq_aaans_parser.cxx 

DIAMETER_NASREQ_HDR = include/diameter_nasreq_client_fsm.hxx \
                      include/diameter_nasreq_client_session.hxx \
                      include/diameter_nasreq_server_fsm.hxx \
                      include/diameter_nasreq_server_session.hxx \
                      include/diameter_nasreq_parser.hxx \
                      include/diameter_nasreq_authinfo.hxx

lib_LTLIBRARIES = libdiameternasreq.la
libdiameternasreq_la_SOURCES = $(DIAMETER_NASREQ_SRC) $(DIAMETER_NASREQ_HDR)
libdiameternasreq_la_LDFLAGS = -version-info 1:0:0 
libdiameternasreq_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/diameternasreq
include_HEADERS = $(DIAMETER_NASREQ_HDR)

sysconfdir = $(prefix)/etc/opendiameter/diameternasreq
sysconf_DATA = config/client.local.xml \
               config/configuration.xml \
               config/configuration.dtd \
               config/dictionary.dtd \
               config/dictionary.xml \
               config/server.local.xml

LDADD = @L_LIBS@ \
        $(top_builddir)/libdiameternasreq/libdiameternasreq.la \
	$(top_builddir)/libdiameter/libdiameter.la \
	$(top_builddir)/libdiamparser/libdiamparser.la \
	$(top_builddir)/libodutl/libodutl.la

noinst_PROGRAMS = client_test server_test

client_test_SOURCES = test/client_test.cxx

server_test_SOURCES = test/server_test.cxx

EXTRA_DIST = config

client_test_LDADD = $(top_builddir)/libdiameternasreq/libdiameternasreq.la\
	$(top_builddir)/libdiameter/libdiameter.la\
	$(top_builddir)/libdiamparser/libdiamparser.la\
	$(top_builddir)/libodutl/libodutl.la -lACEXML_Parser -lACEXML -lACE\
	-lACE_SSL -lssl
server_test_LDADD = $(top_builddir)/libdiameternasreq/libdiameternasreq.la\
	$(top_builddir)/libdiameter/libdiameter.la\
	$(top_builddir)/libdiamparser/libdiamparser.la\
	$(top_builddir)/libodutl/libodutl.la -lACEXML_Parser -lACEXML -lACE\
	-lACE_SSL -lssl
