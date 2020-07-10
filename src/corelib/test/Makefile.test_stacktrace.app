# $Id: Makefile.test_stacktrace.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_stacktrace
SRC = test_stacktrace
LIB = xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_stacktrace

WATCHERS = grichenk
