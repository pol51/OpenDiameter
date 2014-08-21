## Process this file with automake to produce Makefile.in
srcdir = @srcdir@
VPATH = @srcdir@

AM_CPPFLAGS = @L_CPPFLAGS@ \
              -I@top_srcdir@/include \
              -I@top_srcdir@/libodutl/include \
              -I@top_srcdir@/libdiamparser \
              -I@top_srcdir@/libeap/include \
              -I@top_srcdir@/libpana/include \
              -I../common \
              -I@top_srcdir@/libpana/test \
              -D@HOST_OS@
AM_LDFLAGS = @L_LDFLAGS@

bin_PROGRAMS = pacd

sysconfdir = /etc/opendiameter/pac

nobase_sysconf_DATA = config/client.eap-fast.xml \
		      config/credential \
		      config/dh \
		      config/pac \
		      config/pana_auth_script \
              	     config/pana_dictionary.dtd \
             	      config/pana_dictionary.xml \
		      config/pana_pac.dtd \
        	      config/pana_pac.xml \
		      config/pana_setup.dtd \
		      config/pana_setup.xml \
 	              config/shared_secret.bin \
		      config/fast/ca-cert.pem \
		      config/fast/clt-cert.pem \
		      config/fast/clt-key.pem \
 		      config/fast/random 
 		      
sysscriptdir = /etc/opendiameter/pac

nobase_sysscript_SCRIPTS = config/pana_test_auth_script

pacd_SOURCES = pacd.cxx pacd_config.cxx pacd_config.h
pacd_LDADD = @L_LIBS@ \
             $(top_builddir)/libpana/libpana.la \
             $(top_builddir)/libeap/libeap.la \
             $(top_builddir)/libeap/libeaparchie.la \
			 $(top_builddir)/libeap/libeaptls.la \
			 $(top_builddir)/libeap/libeapfast.la \
			 $(top_builddir)/libeap/libeapfastbase.la \
             $(top_builddir)/libdiamparser/libdiamparser.la \
             $(top_builddir)/libodutl/libodutl.la

dist-hook:
