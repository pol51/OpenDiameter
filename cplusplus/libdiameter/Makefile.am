## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

INC = include/aaa_application.h \
      include/aaa_data_defs.h \
      include/aaa_log_facility.h \
      include/aaa_peer_data.h \
      include/aaa_peer_fsm.h \
      include/aaa_peer_interface.h \
      include/aaa_peer_table.h \
      include/aaa_route_framework.h \
      include/aaa_route_id_generator.h \
      include/aaa_route_msg_router.h \
      include/aaa_route_table.h \
      include/aaa_session.h \
      include/aaa_session_acct_client_fsm.h \
      include/aaa_session_acct_fsm.h \
      include/aaa_session_acct_server_fsm.h \
      include/aaa_session_attributes.h \
      include/aaa_session_auth_client_fsm.h \
      include/aaa_session_auth_fsm.h \
      include/aaa_session_auth_server_fsm.h \
      include/aaa_session_client.h \
      include/aaa_session_db.h \
      include/aaa_session_fsm.h \
      include/aaa_session_garbage_collector.h \
      include/aaa_session_msg_mux.h \
      include/aaa_session_msg_rx.h \
      include/aaa_session_server.h \
      include/aaa_session_server_factory.h \
      include/aaa_transport_ace.h \
      include/aaa_transport_collector.h \
      include/aaa_transport_interface.h \
      include/aaa_xml_data.h \
      include/diameter_api.h \
      include/aaa_session.inl \
      include/aaa_session_client.inl \
      include/aaa_session_server.inl

SRC = src/aaa_application.cxx \
      src/aaa_peer_fsm.cxx \
      src/aaa_peer_table.cxx \
      src/aaa_route_msg_router.cxx \
      src/aaa_route_table.cxx \
      src/aaa_session_acct_client_fsm.cxx \
      src/aaa_session_acct_server_fsm.cxx \
      src/aaa_session_acct_transformer.cxx \
      src/aaa_session_attributes.cxx \
      src/aaa_session_auth_client_fsm.cxx \
      src/aaa_session_auth_server_fsm.cxx \
      src/aaa_session_client.cxx \
      src/aaa_session_db.cxx \
      src/aaa_session_msg_rx.cxx \
      src/aaa_session_server.cxx \
      src/aaa_transport_collector.cxx \
      src/aaa_xml_data.cxx

AM_CPPFLAGS = @L_CPPFLAGS@ @L_ODBCFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I$(top_srcdir)/libdiamparser \
              -I$(srcdir)/include \
              -D@HOST_OS@ \
              -DNEXT_GEN
AM_LDFLAGS = @L_LDFLAGS@

lib_LTLIBRARIES = libdiameter.la
libdiameter_la_SOURCES = $(SRC) $(INC)
libdiameter_la_LDFLAGS = -version-info 1:0:0 
libdiameter_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/diameter
include_HEADERS = $(INC)

sysconfdir = $(prefix)/etc/opendiameter/diameter
sysconf_DATA = config/agent.local.xml \
               config/configuration.xml \
               config/configuration.xsd \
               config/dictionary.dtd \
               config/dictionary.xml \
               config/isp.local.xml \
               config/nas1.local.xml \
               config/nas2.local.xml \
               config/nas3.local.xml

LDADD = @L_LIBS@ \
        libdiameter.la \
        $(top_srcdir)/libdiamparser/libdiamparser.la \
        $(top_srcdir)/libodutl/libodutl.la
noinst_PROGRAMS = aaa_sample_client1 aaa_sample_server1 \
                  aaa_sample_client2 aaa_sample_server2 \
                  aaa_sample_client3 aaa_sample_server3 \
                  aaa_sample_client4 aaa_sample_server4 \
                  aaa_sample_client5 aaa_sample_server5 \
                  aaa_sample_client6 \
                  aaa_sample_agent

aaa_sample_client1_SOURCES = test/sample_client1.cxx test/sample_client1.h
aaa_sample_server1_SOURCES = test/sample_server1.cxx test/sample_server1.h
aaa_sample_client2_SOURCES = test/sample_client2.cxx
aaa_sample_server2_SOURCES = test/sample_server2.cxx
aaa_sample_client3_SOURCES = test/sample_client3.cxx
aaa_sample_server3_SOURCES = test/sample_server3.cxx
aaa_sample_client4_SOURCES = test/sample_client4.cxx
aaa_sample_server4_SOURCES = test/sample_server4.cxx
aaa_sample_client5_SOURCES = test/sample_client5.cxx
aaa_sample_server5_SOURCES = test/sample_server5.cxx
aaa_sample_client6_SOURCES = test/sample_client6.cxx
aaa_sample_agent_SOURCES = test/sample_agent.cxx

EXTRA_DIST = config
