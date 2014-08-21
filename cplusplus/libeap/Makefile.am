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
		include/eap_gpsk_crypto.hxx \
		include/eap_gpsk_fsm.hxx \
		include/eap_gpsk_parser.hxx \
		include/eap_gpsk.hxx \
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
		include/eap_archie.hxx \
		include/eap_gpsk_crypto.hxx \
		include/eap_gpsk_fsm.hxx \
		include/eap_gpsk_parser.hxx \
		include/eap_gpsk.hxx
endif

# Added code for EAP FAST 
if compile_EAPFAST
FAST_BASE_HDR = include/base/aes.h \
		include/base/aes_wrap.h \
		include/base/eap_fast_common.h \
		include/base/eap_fast_pac.h \
		include/base/md5.h \
		include/base/openssl_patches.h \
		include/base/os.h \
		include/base/sha1.h
		
FAST_SRC = eap-fast/eap_fast_mng.cxx \
          eap-fast/eap_fast_crypto.cxx \
          eap-fast/eap_fast_fsm.cxx \
          eap-fast/eap_fast_xml_data.cxx \
          eap-fast/eap_fast_xml_parser.cxx

FAST_BASE_SRC = eap-fast/base/aes.c \
          eap-fast/base/aes_wrap.c \
          eap-fast/base/os.c \
          eap-fast/base/sha1.c \
          eap-fast/base/md5.c \
          eap-fast/base/eap_fast_common.c \
          eap-fast/base/openssl_patches.c \
          eap-fast/base/eap_fast_pac.c

endif 

ARCHIE_SRC = eap-archie/eap_archie_fsm.cxx

GPSK_SRC = eap-gpsk/eap_gpsk_fsm.cxx

if compile_EAPTLS
TLS_SRC = eap-tls/eap_tls_mng.cxx \
          eap-tls/eap_tls_crypto.cxx \
          eap-tls/eap_tls_fsm.cxx \
          eap-tls/eap_tls_xml_data.cxx \
          eap-tls/eap_tls_xml_parser.cxx
endif

includedir = $(prefix)/include/opendiameter/eap

if compile_EAPFAST
include_HEADERS = $(CORE_HDR) $(FAST_BASE_HDR)
else
include_HEADERS = $(CORE_HDR)
endif

sysconfdir =   /etc/opendiameter/eap
sysconf_DATA = config/client.eap-tls.xml \
               config/server.eap-tls.xml \
               config/client.eap-fast.xml \
			   config/server.eap-fast.xml

if compile_EAPTLS
EAP_TLS_LTLIBTRARIES = libeaptls.la
endif

if compile_EAPFAST
EAP_FAST_LTLIBTRARIES = libeapfast.la libeapfastbase.la
endif 

lib_LTLIBRARIES = libeap.la libeaparchie.la libeapgpsk.la $(EAP_FAST_LTLIBTRARIES) $(EAP_TLS_LTLIBTRARIES)

libeap_la_SOURCES = $(CORE_SRC) $(CORE_HDR)
libeaparchie_la_SOURCES = $(ARCHIE_SRC)
libeapgpsk_la_SOURCES = $(GPSK_SRC)

if compile_EAPFAST
libeapfast_la_SOURCES = $(FAST_SRC) $(FAST_HDR)
libeapfastbase_la_SOURCES = $(FAST_BASE_SRC)
endif

if compile_EAPTLS
libeaptls_la_SOURCES = $(TLS_SRC) $(CORE_HDR)
endif

libeap_la_LDFLAGS = -version-info 1:0:0 
libeap_la_AR = $(AR) -qcs

libeaparchie_la_LDFLAGS = -version-info 1:0:0 
libeaparchie_la_AR = $(AR) -qcs

libeapgpsk_la_LDFLAGS = -version-info 1:0:0 
libeapgpsk_la_AR = $(AR) -qcs

if compile_EAPTLS
libeaptls_la_LDFLAGS = -version-info 1:0:0 
libeaptls_la_AR = $(AR) -qcs

EAP_TLS_LDD = $(top_builddir)/libeap/libeaptls.la
endif

if compile_EAPFAST
libeapfast_la_LDFLAGS = -version-info 1:0:0 
libeapfast_la_AR = $(AR) -qcs
libeapfastbase_la_LDFLAGS = -version-info 1:0:0 
libeapfastbase_la_AR = $(AR) -qcs

EAP_FAST_LDD = $(top_builddir)/libeap/libeapfast.la \
			   $(top_builddir)/libeap/libeapfastbase.la
endif

LDADD = @L_LIBS@\
		$(top_builddir)/libeap/libeap.la \
        $(top_builddir)/libeap/libeaparchie.la \
        $(top_builddir)/libeap/libeapgpsk.la \
        $(EAP_FAST_LDD) \
        $(EAP_TLS_LDD)

if compile_EAPTLS
tls_PROGS = md5_test archie_test gpsk_test tls_test
endif

if compile_EAPFAST
fast_test_PROGS = md5_test archie_test gpsk_test fast_test
endif

noinst_PROGRAMS = md5_test archie_test gpsk_test $(fast_test_PROGS) $(tls_PROGS)

md5_test_SOURCES = tests/md5_test.cxx

archie_test_SOURCES = tests/archie_test.cxx 

gpsk_test_SOURCES = tests/gpsk_test.cxx 

if compile_EAPTLS
tls_test_SOURCES = tests/tls_test.cxx
endif

if compile_EAPFAST
fast_test_SOURCES = tests/fast_test.cxx
endif

EXTRA_DIST = config
