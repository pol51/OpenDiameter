## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ @L_ODBCFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiametermip4/include \
              -I./include \
              -D@HOST_OS@

AM_LDFLAGS = @L_LDFLAGS@

DIAMETER_MIP_SRC = src/diameter_mip4_amr_parser.cxx \
                   src/diameter_mip4_ama_parser.cxx \
                   src/diameter_mip4_har_parser.cxx \
                   src/diameter_mip4_haa_parser.cxx \
		   src/diameter_mip4_ha_client_fsm.cxx \
		   src/diameter_mip4_ha_server_fsm.cxx \
		   src/diameter_mip4_fa_client_fsm.cxx \
		   src/diameter_mip4_aaas_server_fsm.cxx \
		   src/diameter_mip4_aaas_client_fsm.cxx 

DIAMETER_MIP_HDR =  include/diameter_mip4_ha_client_session.hxx \
                   include/diameter_mip4_ha_server_session.hxx \
		   include/diameter_mip4_ha_client_fsm.hxx \
		   include/diameter_mip4_ha_server_fsm.hxx \
		   include/diameter_mip4_ha_client_interface.hxx \
		   include/diameter_mip4_ha_server_interface.hxx \
		   include/mip4_diameter_ha_client_interface.hxx \
                   include/diameter_mip4_fa_client_interface.hxx \
                   include/diameter_mip4_fa_client_fsm.hxx \
                   include/diameter_mip4_fa_client_session.hxx \
		   include/diameter_mip4_aaas_server_fsm.hxx \
		   include/diameter_mip4_aaas_server_session.hxx \
		   include/diameter_mip4_aaas_client_fsm.hxx \
		   include/diameter_mip4_aaas_client_session.hxx \
		   include/diameter_mip4_aaas_server_interface.hxx \
		   include/diameter_mip4_aaas_server_sint_interface.hxx \
		   include/mip4_diameter_fa_client_interface.hxx \
		   include/mip4_diameter_ha_client_interface.hxx \
                   include/diameter_mip4_parser.hxx 

lib_LTLIBRARIES = libdiametermip4.la
libdiametermip4_la_SOURCES = $(DIAMETER_MIP_SRC) $(DIAMETER_MIP_HDR)
libdiametermip4_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/diametermip4
include_HEADERS = $(DIAMETER_MIP_HDR)

sysconfdir = $(prefix)/etc/opendiameter/diametermip4
sysconf_DATA = config/aaas_s.local.xml \
               config/aaas_s2.local.xml \
               config/client.local.xml \
               config/configuration.xml \
               config/configuration.xsd \
               config/dictionary.dtd \
               config/dictionary.xml \
               config/fac.local.xml \
               config/hac.local.xml \
               config/has.local.xml \
               config/hosts \
               config/server.local.xml

LDADD = @L_LIBS@ \
        libdiametermip4.la \
	$(top_srcdir)/libdiameter/libdiameter.la \
	$(top_srcdir)/libdiamparser/libdiamparser.la \
	$(top_srcdir)/libodutl/libodutl.la

noinst_PROGRAMS = hac_test aaas_server_test has_test fac_test

hac_test_SOURCES = test/hac_test.cxx \
	           test/xyz_ha_client_session.hxx


aaas_server_test_SOURCES = test/aaas_server_test.cxx \
                           test/diameter_specific_aaas_server_interface.hxx

has_test_SOURCES = test/has_test.cxx \
	           test/xyz_ha_server_session.hxx

fac_test_SOURCES = test/fac_test.cxx \
	           test/xyz_fa_client_session.hxx

# Distribution

EXTRA_DIST = config
