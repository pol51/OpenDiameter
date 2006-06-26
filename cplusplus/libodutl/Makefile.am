## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

COMMON_HDRS = include/od_utl_exports.h \
              include/od_utl_patterns.h \
              include/od_utl_md5.h \
              include/od_utl_rbtree.h \
              include/od_utl_sha1.h \
              include/od_utl_xml_sax_parser.h \
              include/od_utl_rbtree_dbase.h

COMMON_SRC  = src/od_utl_rbtree.cxx \
              src/od_utl_md5.cxx \
              src/od_utl_sha1.cxx \
              src/od_utl_xml_sax_parser.cxx
              
includedir = $(prefix)/include/opendiameter/odutl
include_HEADERS = $(COMMON_HDRS)

lib_LTLIBRARIES = libodutl.la
libodutl_la_SOURCES = $(COMMON_SRC) $(COMMON_HDRS) $(PLATFORM_SRC)
libodutl_la_LDFLAGS = -version-info 1:0:0 
libodutl_la_AR = $(AR) -qcs

LDADD = @L_LIBS@ -lcommon

dist-hook:

