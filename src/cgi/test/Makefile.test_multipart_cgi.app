# $Id: Makefile.test_multipart_cgi.app 505858 2016-06-29 16:55:21Z elisovdn $

APP = test_multipart.cgi
SRC = test_multipart_cgi
LIB = xcgi$(STATIC) xutil$(STATIC) xncbi$(STATIC)

WATCHERS = ucko
