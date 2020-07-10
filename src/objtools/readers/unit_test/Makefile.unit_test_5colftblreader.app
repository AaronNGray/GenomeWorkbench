# $Id: Makefile.unit_test_5colftblreader.app 533503 2017-04-17 14:42:41Z foleyjp $

APP = unit_test_5colftblreader
SRC = unit_test_5colftblreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = 5colftblreader_test_cases

WATCHERS = foleyjp
