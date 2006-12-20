## Process this file with automake to produce Makefile.in
MISC = $(top_srcdir)/libpana/docs/architecture.txt \
       $(top_srcdir)/libpana/docs/Doxyfile \
       $(top_srcdir)/libpana/docs/figs/client_architecture.eps \
       $(top_srcdir)/libpana/docs/figs/client_architecture.jpeg \
       $(top_srcdir)/libpana/docs/figs/client_architecture.obj \
       $(top_srcdir)/libpana/docs/figs/server_architecture.eps \
       $(top_srcdir)/libpana/docs/figs/server_architecture.jpeg \
       $(top_srcdir)/libpana/docs/figs/server_architecture.obj

CONFIG = $(top_srcdir)/libpana/config

EXTRA_DIST = $(MISC) $(CONFIG)

datadir = $(prefix)/share/opendiameter/doc/pana
data_DATA = $(MISC)

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ @L_MPAFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libpana/include \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

PANA_HDRS = include/pana_auth_script.h \
            include/pana_channel.h \
            include/pana_client.h \
            include/pana_client_fsm.h \
            include/pana_config_manager.h \
            include/pana_defs.h \
            include/pana_egress.h \
            include/pana_exceptions.h \
            include/pana_exports.h \
            include/pana_fsm.h \
            include/pana_ingress.h \
            include/pana_memory_manager.h \
            include/pana_message.h \
            include/pana_node.h \
            include/pana_nonce_generator.h \
            include/pana_paa.h \
            include/pana_paa_fsm.h \
            include/pana_paa_factory.h \
            include/pana_pac_ep_key.h \
	    include/pana_parser.h \
	    include/pana_parser_avpvalue.h \
	    include/pana_parser_dict.h \
            include/pana_pmk_bootstrap.h \
            include/pana_prf_plus.h \
            include/pana_psk_bootstrap.h \
            include/pana_security_assoc.h \
            include/pana_serial_num.h \
            include/pana_session.h

PANA_SRC  = src/pana_config_manager.cxx \
            src/pana_client.cxx \
            src/pana_client_fsm.cxx \
            src/pana_egress.cxx \
            src/pana_ingress.cxx \
            src/pana_node.cxx \
            src/pana_paa.cxx \
            src/pana_paa_factory.cxx \
            src/pana_paa_fsm.cxx \
            src/pana_pac_ep_key.cxx \
            src/pana_parser.cxx \
            src/pana_parser_avpvalue.cxx \
            src/pana_parser_dict.cxx \
            src/pana_pmk_bootstrap.cxx \
            src/pana_prf_plus.cxx \
            src/pana_psk_bootstrap.cxx \
            src/pana_security_assoc.cxx \
            src/pana_session.cxx

includedir = $(prefix)/include/opendiameter/pana
include_HEADERS = $(PANA_HDRS)

sysconfdir = $(prefix)/etc/opendiameter/pana
sysconf_DATA = config/dictionary.dtd \
               config/dictionary.xml \
               config/paa.xml \
               config/pac.xml \
               config/config.dtd \
               config/pana_auth.bat \
               config/shared_secret.bin \
               config/users.dtd \
               config/users.xml

lib_LTLIBRARIES = libpana.la
libpana_la_SOURCES = $(PANA_SRC) $(PANA_HDRS) $(PLATFORM_SRC)
libpana_la_LDFLAGS = -version-info 1:0:0 
libpana_la_AR = $(AR) -qcs

LDADD = @L_LIBS@ \
        $(top_builddir)/libpana/libpana.la \
	$(top_builddir)/libodutl/libodutl.la

noinst_PROGRAMS = pana_test1 pana_test2 pana_test3

pana_test1_SOURCES = test/Test1.cxx test/user_db.cxx
pana_test1_LDADD = $(top_builddir)/libeap/libeap.la \
		$(top_builddir)/libeap/libeaparchie.la \
		$(top_builddir)/libpana/libpana.la \
                $(top_builddir)/libodutl/libodutl.la \
                @L_LIBS@

pana_test2_SOURCES = test/Test2.cxx test/user_db.cxx
pana_test2_LDADD = $(top_builddir)/libeap/libeap.la \
		$(top_builddir)/libeap/libeaparchie.la \
		$(top_builddir)/libpana/libpana.la \
                $(top_builddir)/libodutl/libodutl.la \
                @L_LIBS@

pana_test3_SOURCES = test/Test3.cxx test/user_db.cxx
pana_test3_LDADD = $(top_builddir)/libeap/libeap.la \
		$(top_builddir)/libeap/libeaparchie.la \
		$(top_builddir)/libpana/libpana.la \
                $(top_builddir)/libodutl/libodutl.la \
                @L_LIBS@

pkginclude_HEADERS = test/user_db.h

dist-hook:

