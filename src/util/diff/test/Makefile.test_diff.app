# $Id: Makefile.test_diff.app 462132 2015-03-16 18:49:07Z vasilche $

APP = test_diff
SRC = test_diff

LIB = xdiff xutil xncbi
LIBS = $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS)

CHECK_COPY = testdata
CHECK_CMD = 

WATCHERS = ivanov
