# $Id: Makefile.test_qparse.app 371619 2012-08-09 16:47:32Z vasilche $

APP = test_qparse
SRC = test_qparse

CPPFLAGS = $(ORIG_CPPFLAGS) 

LIB  = xqueryparse xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = kuznets

