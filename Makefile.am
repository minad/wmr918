# $Id: Makefile.am,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
# Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = data doc src

CLEANFILES = *~

EXTRA_DIST = autogen.sh README

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
