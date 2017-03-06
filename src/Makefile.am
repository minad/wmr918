# $Id: Makefile.am,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
# Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

CLEANFILES = *~

DEFS=
INCLUDES=-I$(top_srcdir) -I. @DBI_INCDIR@
CFLAGS=-O2 -g -Wall
LIBS=-ldbi
LDFLAGS=@DBI_LIBDIR@

sbin_PROGRAMS  = wmr918
wmr918_SOURCES = wmr918.c cfg.c database.c logger.c misc.c serial.c process.c
noinst_HEADERS = wmr918.h dbrecord.h protocol.h

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
