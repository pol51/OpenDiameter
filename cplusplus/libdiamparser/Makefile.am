## Process this file with automake to produce Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ -I@top_srcdir@/include -D@HOST_OS@ 
AM_LDFLAGS = @L_LDFLAGS@

PARSER_SRC = avplist.cxx \
             q_avplist.cxx \
	     comlist.cxx \
	     g_avplist.cxx \
             parser.cxx \
	     avp_container.cxx \
	     parser_avp.cxx \
	     avp_header.cxx \
             error_status.cxx \
	     parser_q_avplist.cxx \
	     xml_parsing.cxx \
	     memory_manager.cxx \
	     parser_avpvalue.cxx 

PARSER_HDR = parser_avp.h \
             parser_avpvalue.h \
	     parser.h \
             avp_header.h \
	     comlist.h \
	     error_status.h \
	     parser_q_avplist.h \
	     avplist.h \
	     g_avplist.h \
	     q_avplist.h \
             memory_manager.h \
	     xml_parsing.h \
	     diameter_parser_api.h

lib_LTLIBRARIES = libdiamparser.la
libdiamparser_la_SOURCES = $(PARSER_SRC) $(PARSER_HDR) 
libdiamparser_la_LDFLAGS = -version-info 1:0:0
libdiamparser_la_AR = $(AR) -qcs

sysconfdir = $(prefix)/etc/opendiameter/parser
sysconf_DATA = dictionary.xml dictionary.dtd

includedir = $(prefix)/include/opendiameter/parser
include_HEADERS = $(PARSER_HDR)

LDADD = @L_LIBS@ libdiamparser.la
noinst_PROGRAMS = parser_test1 parser_test2 \
                  parser_test3 parser_test4 \
                  parser_test5

parser_test1_SOURCES = test1.cxx
parser_test2_SOURCES = test2.cxx
parser_test3_SOURCES = test3.cxx
parser_test4_SOURCES = test1.cxx
parser_test5_SOURCES = test5.cxx

parser_test4_CPPFLAGS = $(AM_CPPFLAGS) -DPROTOID_TEST

EXTRA_DIST = dictionary.xml dictionary.dtd

dist-hook:

