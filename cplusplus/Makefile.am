## Process this file with automake to produce Makefile.in
MISC = $(top_srcdir)/docs/msvc-solution-usage.txt \
       $(top_srcdir)/docs/development_policy.txt \
       $(top_srcdir)/docs/coding_style.txt \
       $(top_srcdir)/docs/bugtracking-diagram.pdf \
       $(top_srcdir)/docs/framework \
       $(top_srcdir)/docs/libdiameter \
       $(top_srcdir)/docs/libdiametereap \
       $(top_srcdir)/docs/libdiameternasreq \
       $(top_srcdir)/docs/libdiametermip4 \
       $(top_srcdir)/docs/libeap \
       $(top_srcdir)/docs/libpana \
       $(top_srcdir)/docs/nasreq \
       $(top_srcdir)/opendiameter.kdevelop \
       $(top_srcdir)/tools/getopt.patch \
       $(top_srcdir)/tools/getopt1.patch \
       $(top_srcdir)/autogen.sh \
       $(top_srcdir)/RELEASE \
       $(top_srcdir)/README


EXTRA_DIST = $(MISC)

datadir = $(prefix)/share/opendiameter/doc
data_DATA = $(MISC)

SUBDIRS_LIBS = libodutl \
               libdiamparser \
	       libdiameter \
	       libdiametermip4 \
	       libeap \
	       libpana \
	       libdiametereap \
	       libdiameternasreq

SUBDIRS_APPS = applications

SUBDIRS = $(SUBDIRS_LIBS) $(SUBDIRS_APPS)
DIST_SUBDIRS = $(SUBDIRS_LIBS) $(SUBDIRS_APPS)

includedir = $(prefix)/include/opendiameter
include_HEADERS = $(top_srcdir)/include/framework.h \
                  $(top_srcdir)/include/aaa_parser_api.h \
                  $(top_srcdir)/include/aaa_global_config.h \
                  $(top_srcdir)/include/aaa_dictionary_api.h \
                  $(top_srcdir)/include/aaa_parser_defs.h \
                  $(top_srcdir)/include/resultcodes.h

dist-hook:
	rm -rf `find $(distdir) -name CVS`


