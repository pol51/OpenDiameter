## Process this file with automake to produce Makefile.in
#
# @date June 20, 2014
# @author Ron Brash (ron.brash@gmail.com)
# @brief Updated Makefile.am
# @note If new directories or projects are to be added, they must be also
# added inside of configure.in and assorted Autotools must be ran again:
# $ aclocal; autoreconf -f; automake --add-missing;
#
docdir = $(prefix)/share/opendiameter/docs
configdir = /etc/opendiameter/

INSTALL_DOCS = $(top_srcdir)/docs/msvc-solution-usage.txt \
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
			   
INSTALL_CONFIGS = $(top_srcdir)/applications/pana/config \
			   $(top_srcdir)/applications/nas/config \
			   $(top_srcdir)/applications/aaa/config

# Add any libraries to this list in tandem to the list in configure.in
SUBDIRS_LIBS =  libodutl \
				libeap \
				libpana \
				libdiamparser \
				libdiameter \
				libdiametermip4 \
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
	
# Added this method to handle installation of directories - its more reliable
# than expected Autotools to work correctly
install-data-local:
	@echo; echo "Installing documentation files"
	if [ -d "$(docdir)" ]; then \
		echo "Documentation directory exists..."; \
	else \
		mkdir -p $(docdir)/; \
	fi
	cp -R $(INSTALL_DOCS) $(docdir)/
	chmod 751 -R $(docdir) $(docdir)
	
	@echo; echo "Installing configuration files"
	if [ -d "$(configdir)" ]; then \
		echo "Configuration directory exists..."; \
	else \
		mkdir -p $(configdir)/; \
		mkdir -p $(configdir)/aaa/config; \
		mkdir -p $(configdir)/nas/config; \
		mkdir -p $(configdir)/pac/config; \
	fi

	cp -R $(INSTALL_CONFIGS) $(configdir)/
	chmod 751 -R $(configdir) $(configdir)	
	
	@echo; echo "Binaries for applications are stored in their respective directories \
	within this source directory [applications/*]"
	
	@echo; echo "No SysV or startup scripts have been created or installed*]"

# Added this method to handle uninstallation of directories - its more reliable
# than expected Autotools to work correctly
uninstall-local:
	rm -rf $(docdir)/
	rm -rf $(configdir)/
