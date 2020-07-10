# $Id: Makefile.test_regexp.app 173093 2009-10-14 16:24:46Z vakatov $

APP = test_regexp
SRC = test_regexp

LIB = xregexp $(PCRE_LIB) xutil xncbi
LIBS = $(PCRE_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(PCRE_INCLUDE) $(ORIG_CPPFLAGS)

CHECK_CMD = test_regexp Abc Ef Ghh Ooo Pppk /CHECK_NAME=test_regexp

WATCHERS = ivanov
