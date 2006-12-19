## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libeap/include \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

if compile_EAPTLS
CORE_SRC = eapcore/eap_fsm.cxx \
	eapcore/eap_peerfsm.cxx \
	eapcore/eap_authfsm.cxx \
        eapcore/eap_standalone_authfsm.cxx \
        eapcore/eap_backend_authfsm.cxx \
        eapcore/eap_passthrough_authfsm.cxx \
	eapcore/eap_identity.cxx \
	eapcore/eap_method_registrar.cxx \
	eapcore/eap_policy.cxx \
        eapcore/eap_notification.cxx \
	eapcore/eap_md5.cxx 

CORE_HDR = include/eap_api.h \
        include/eap.hxx \
        include/eap_authfsm.hxx \
        include/eap_fsm.hxx \
        include/eap_identity.hxx \
        include/eap_log.hxx \
        include/eap_md5.hxx \
        include/eap_method_registrar.hxx \
        include/eap_notification.hxx \
        include/eap_parser.hxx \
        include/eap_peerfsm.hxx \
        include/eap_policy.hxx \
	include/eap_archie_crypto.hxx \
	include/eap_archie_fsm.hxx \
	include/eap_archie_parser.hxx \
	include/eap_archie.hxx \
	include/eap_tls.hxx \
	include/eap_tls_mng.hxx \
	include/eap_tls_session.hxx \
	include/eap_tls_parser.hxx \
	include/eap_tls_fsm.hxx \
	include/eap_tls_xml_data.hxx \
	include/eap_tls_xml_parser.hxx \
	include/eap_tls_data_defs.hxx
else
CORE_SRC = eapcore/eap_fsm.cxx \
	eapcore/eap_peerfsm.cxx \
	eapcore/eap_authfsm.cxx \
        eapcore/eap_standalone_authfsm.cxx \
        eapcore/eap_backend_authfsm.cxx \
        eapcore/eap_passthrough_authfsm.cxx \
	eapcore/eap_identity.cxx \
	eapcore/eap_method_registrar.cxx \
	eapcore/eap_policy.cxx \
        eapcore/eap_notification.cxx \
	eapcore/eap_md5.cxx

CORE_HDR = include/eap_api.h \
        include/eap.hxx \
        include/eap_authfsm.hxx \
        include/eap_fsm.hxx \
        include/eap_identity.hxx \
        include/eap_log.hxx \
        include/eap_md5.hxx \
        include/eap_method_registrar.hxx \
        include/eap_notification.hxx \
        include/eap_parser.hxx \
        include/eap_peerfsm.hxx \
        include/eap_policy.hxx \
	include/eap_archie_crypto.hxx \
	include/eap_archie_fsm.hxx \
	include/eap_archie_parser.hxx \
	include/eap_archie.hxx
endif

ARCHIE_SRC = eap-archie/eap_archie_fsm.cxx

if compile_EAPTLS
TLS_SRC = eap-tls/eap_tls_mng.cxx \
          eap-tls/eap_tls_crypto.cxx \
          eap-tls/eap_tls_fsm.cxx \
          eap-tls/eap_tls_xml_data.cxx \
          eap-tls/eap_tls_xml_parser.cxx
endif

includedir = $(prefix)/include/opendiameter/eap
include_HEADERS = $(CORE_HDR)

sysconfdir = $(prefix)/etc/opendiameter/eap
sysconf_DATA = config/client.eap-tls.xml \
               config/server.eap-tls.xml

if compile_EAPTLS
lib_LTLIBRARIES = libeap.la libeaparchie.la libeaptls.la
else
lib_LTLIBRARIES = libeap.la libeaparchie.la
endif

libeap_la_SOURCES = $(CORE_SRC) $(CORE_HDR)
libeaparchie_la_SOURCES = $(ARCHIE_SRC)

if compile_EAPTLS
libeaptls_la_SOURCES = $(TLS_SRC)
endif

libeap_la_LDFLAGS = -version-info 1:0:0 
libeap_la_AR = $(AR) -qcs
libeaparchie_la_LDFLAGS = -version-info 1:0:0 
libeaparchie_la_AR = $(AR) -qcs

if compile_EAPTLS
libeaptls_la_LDFLAGS = -version-info 1:0:0 
libeaptls_la_AR = $(AR) -qcs
LDADD = @L_LIBS@ \
        $(top_builddir)/libeap/libeap.la \
        $(top_builddir)/libeap/libeaparchie.la \
        $(top_builddir)/libeap/libeaptls.la \
        $(top_builddir)/libdiamparser/libdiamparser.la
else
LDADD = @L_LIBS@ \
        $(top_builddir)/libeap/libeap.la \
        $(top_builddir)/libeap/libeaparchie.la
endif

if compile_EAPTLS
noinst_PROGRAMS = md5_test archie_test tls_test
else
noinst_PROGRAMS = md5_test archie_test
endif

md5_test_SOURCES = tests/md5_test.cxx

archie_test_SOURCES = tests/archie_test.cxx 

if compile_EAPTLS
tls_test_SOURCES = tests/tls_test.cxx
endif

EXTRA_DIST = config
