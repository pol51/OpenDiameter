## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

SRC = src/aes.c \
      src/aes_wrap.c \
      src/os.c \
      src/eap_fast_generate_pac.c \
      src/main.c

HDR = include/aes.h  \
      include/aes_wrap.h \
      include/os.h \
      include/eap_fast_generate_pac.h

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I$(srcdir)/include \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser/include \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libpana/include \
              -I@top_srcdir@/libdiameter/include \
              -I@top_srcdir@/libdiameternasreq/include \
              -I@top_srcdir@/libdiametereap/include \
              -I../common \
              -D@HOST_OS@

AM_LDFLAGS = @L_LDFLAGS@

bin_PROGRAMS = generate_pac
                      

generate_pac_SOURCES = $(SRC) $(HDR)
generate_pac_LDADD = @L_LIBS@ \
             $(top_builddir)/libpana/libpana.la \
             $(top_builddir)/libeap/libeap.la \
             $(top_builddir)/libeap/libeaparchie.la \
             $(top_builddir)/libeap/libeapfastbase.la \
             $(top_builddir)/libeap/libeapgpsk.la \
             $(top_builddir)/libdiameter/libdiameter.la \
             $(top_builddir)/libdiameternasreq/libdiameternasreq.la \
             $(top_builddir)/libdiametereap/libdiametereap.la \
             $(top_builddir)/libdiamparser/libdiamparser.la \
             $(top_builddir)/libodutl/libodutl.la

EXTRA_DIST =

dist-hook:
