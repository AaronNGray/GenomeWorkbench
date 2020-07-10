# $Id: Makefile.cgi_io_test.app 505858 2016-06-29 16:55:21Z elisovdn $

APP = cgi_io_test
SRC = cgi_io_test

LIB = xcgi xutil xncbi

## ...or, for FastCGI:
#
# LIB = xfcgi xutil xncbi
# LIBS = $(FASTCGI_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = grichenk
