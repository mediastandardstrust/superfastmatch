AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = libsuperfastmatch-@SUPERFASTMATCH_API_VERSION@.la
libsuperfastmatch_@SUPERFASTMATCH_API_VERSION@_la_SOURCES = superfastmatch/document.cc

libsuperfastmatch_@SUPERFASTMATCH_API_VERSION@_la_LDFLAGS = -version-info $(SUPERFASTMATCH_SO_VERSION)

superfastmatch_includedir = $(includedir)/superfastmatch-$(SUPERFASTMATCH_API_VERSION)
nobase_superfastmatch_include_HEADERS = superfastmatch.h \
                                 		superfastmatch/document.h

superfastmatch_libincludedir = $(libdir)/superfastmatch-$(SUPERFASTMATCH_API_VERSION)/include
nodist_superfastmatch_libinclude_HEADERS = superfastmatchconfig.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = superfastmatch-$(SUPERFASTMATCH_API_VERSION).pc

dist_noinst_SCRIPTS = autogen.sh
