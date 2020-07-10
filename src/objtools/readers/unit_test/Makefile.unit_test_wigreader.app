# $Id: Makefile.unit_test_wigreader.app 413785 2013-09-18 13:26:23Z ludwigf $

APP = unit_test_wigreader
SRC = unit_test_wigreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = wigreader_test_cases

WATCHERS = ludwigf
