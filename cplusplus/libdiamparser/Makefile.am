## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I$(srcdir)/include \
              -I@top_srcdir@/include \
	      -D@HOST_OS@ 
AM_LDFLAGS = @L_LDFLAGS@

PARSER_SRC = src/aaa_avplist.cxx \
	     src/aaa_dict_mngr.cxx \
             src/aaa_parser.cxx \
	     src/aaa_parser_avp.cxx \
	     src/aaa_avp_header.cxx \
             src/aaa_error_status.cxx \
	     src/aaa_parser_q_avplist.cxx \
	     src/aaa_xml_parsing.cxx \
	     src/aaa_memory_manager.cxx \
	     src/aaa_parser_avpvalue.cxx \
	     src/aaa_msg_to_xml.cxx

PARSER_HDR = include/aaa_parser_avp.h \
             include/aaa_parser_avpvalue.h \
             include/aaa_avp_header.h \
	     include/aaa_comlist.h \
	     include/aaa_error_status.h \
	     include/aaa_parser_q_avplist.h \
	     include/aaa_avplist.h \
	     include/aaa_g_avplist.h \
	     include/aaa_q_avplist.h \
             include/aaa_memory_manager.h \
	     include/aaa_xml_parsing.h \
	     include/aaa_msg_to_xml.h \
             include/diameter_parser.h \
	     include/diameter_parser_api.h

lib_LTLIBRARIES = libdiamparser.la
libdiamparser_la_SOURCES = $(PARSER_SRC) $(PARSER_HDR)
libdiamparser_la_LDFLAGS = -version-info 1:0:0
libdiamparser_la_AR = $(AR) -qcs

includedir = $(prefix)/include/opendiameter/parser
include_HEADERS = $(PARSER_HDR)

sysconfdir = $(prefix)/etc/opendiameter/diameter
sysconf_DATA = config/dictionary.xml \
               config/dictionary.dtd

LDADD = @L_LIBS@ $(top_builddir)/libdiamparser/libdiamparser.la
noinst_PROGRAMS = parser_test1 parser_test2 \
                  parser_test3 parser_test4 \
                  parser_test5

parser_test1_SOURCES = test/aaa_test1.cxx
parser_test2_SOURCES = test/aaa_test2.cxx
parser_test3_SOURCES = test/aaa_test3.cxx
parser_test4_SOURCES = test/aaa_test1.cxx
parser_test5_SOURCES = test/aaa_test5.cxx

EXTRA_DIST = config/dictionary.xml config/dictionary.dtd

dist-hook:

