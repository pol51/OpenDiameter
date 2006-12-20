## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser/include \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiametercc/include \
              -D@HOST_OS@

AM_LDFLAGS = @L_LDFLAGS@

DIAMETER_CC_SRC = src/diameter_cc_client_fsm.cxx \
                      src/diameter_cc_client_session.cxx \
                      src/diameter_cc_server_fsm.cxx \
                      src/diameter_cc_server_session.cxx \
                      src/diameter_cc_req_parser.cxx \
                      src/diameter_cc_ans_parser.cxx 

DIAMETER_CC_HDR = include/diameter_cc_client_fsm.h \
                      include/diameter_cc_client_session.h \
                      include/diameter_cc_server_fsm.h \
                      include/diameter_cc_server_session.h \
                      include/diameter_cc_parser.h \
                      include/diameter_cc_account.h \
                      include/diameter_cc_accounts.h \
                      include/diameter_cc_application.h

lib_LTLIBRARIES = libdiametercc.la
libdiametercc_la_SOURCES = $(DIAMETER_CC_SRC) $(DIAMETER_CC_HDR)
libdiametercc_la_LDFLAGS = -version-info 1:0:0 
libdiametercc_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/diametercc
include_HEADERS = $(DIAMETER_CC_HDR)

sysconfdir = $(prefix)/etc/opendiameter/diametercc
sysconf_DATA = config/client.local.xml \
               config/configuration.xml \
               config/configuration.dtd \
               config/dictionary.dtd \
               config/dictionary.xml \
               config/server.local.xml

LDADD = @L_LIBS@ \
        $(top_builddir)/libdiametercc/libdiametercc.la \
	$(top_builddir)/libdiameter/libdiameter.la \
	$(top_builddir)/libdiamparser/libdiamparser.la \
	$(top_builddir)/libodutl/libodutl.la

noinst_PROGRAMS = cc_client_test cc_server_test

cc_client_test_SOURCES = test/cc_client_test.cxx

cc_server_test_SOURCES = test/cc_server_test.cxx

EXTRA_DIST = config

cc_client_test_LDADD = $(top_builddir)/libdiametercc/libdiametercc.la\
	$(top_builddir)/libdiameter/libdiameter.la\
	$(top_builddir)/libdiamparser/libdiamparser.la\
	$(top_builddir)/libodutl/libodutl.la -lACEXML_Parser -lACEXML -lACE\
	-lACE_SSL -lssl
cc_server_test_LDADD = $(top_builddir)/libdiametercc/libdiametercc.la\
	$(top_builddir)/libdiameter/libdiameter.la\
	$(top_builddir)/libdiamparser/libdiamparser.la\
	$(top_builddir)/libodutl/libodutl.la -lACEXML_Parser -lACEXML -lACE\
	-lACE_SSL -lssl
