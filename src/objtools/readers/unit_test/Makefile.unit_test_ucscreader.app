# $Id: Makefile.unit_test_ucscreader.app 432807 2014-04-18 13:52:28Z gotvyans $

APP = unit_test_ucscreader
SRC = unit_test_ucscreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = ucscreader_test_cases

WATCHERS = gotvyans
