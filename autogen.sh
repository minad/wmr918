#! /bin/sh
# $Id: autogen.sh,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
aclocal
autoheader
automake -a
autoconf

