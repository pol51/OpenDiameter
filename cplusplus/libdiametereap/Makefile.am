## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiameternasreq/include \
              -I@top_srcdir@/libeap/include \
              -I./include \
              -D@HOST_OS@

AM_LDFLAGS = @L_LDFLAGS@

DIAMETER_EAP_SRC = src/diameter_eap_client_fsm.cxx \
                   src/diameter_eap_client_session.cxx \
                   src/diameter_eap_server_fsm.cxx \
                   src/diameter_eap_server_session.cxx \
                   src/diameter_eap_der_parser.cxx \
                   src/diameter_eap_dea_parser.cxx 

DIAMETER_EAP_HDR = include/diameter_eap_client_fsm.hxx \
                   include/diameter_eap_client_session.hxx \
                   include/diameter_eap_server_fsm.hxx \
                   include/diameter_eap_server_session.hxx \
                   include/diameter_eap_parser.hxx 


lib_LTLIBRARIES = libdiametereap.la
libdiametereap_la_SOURCES = $(DIAMETER_EAP_SRC) $(DIAMETER_EAP_HDR)
libdiametereap_la_LDFLAGS = -version-info 1:0:0 
libdiametereap_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/diametereap
include_HEADERS = $(DIAMETER_EAP_HDR)

sysconfdir = $(prefix)/etc/opendiameter/diametereap
sysconf_DATA = config/client.eap-tls.xml \
                 config/client.local.xml \
                 config/configuration.xml \
                 config/configuration.xsd \
                 config/dictionary.dtd \
                 config/dictionary.xml \
                 config/eap-tls.configuration.xsd \
                 config/server.eap-tls.xml \
                 config/server.local.xml

if compile_EAPTLS
LDADD = @L_LIBS@ \
        libdiametereap.la \
	$(top_srcdir)/libdiameter/libdiameter.la \
	$(top_srcdir)/libdiamparser/libdiamparser.la \
	$(top_srcdir)/libeap/libeap.la \
	$(top_srcdir)/libeap/libeaptls.la \
	$(top_srcdir)/libodutl/libodutl.la
else
LDADD = @L_LIBS@ \
        libdiametereap.la \
	$(top_srcdir)/libdiameter/libdiameter.la \
	$(top_srcdir)/libdiamparser/libdiamparser.la \
	$(top_srcdir)/libeap/libeap.la \
	$(top_srcdir)/libodutl/libodutl.la
endif

if compile_EAPTLS
noinst_PROGRAMS = client_test server_test client_test_tls server_test_tls

client_test_tls_SOURCES = test/client_test_tls.cxx

server_test_tls_SOURCES = test/server_test_tls.cxx
else
noinst_PROGRAMS = client_test server_test
endif

client_test_SOURCES = test/client_test.cxx

server_test_SOURCES = test/server_test.cxx

EXTRA_DIST = config
