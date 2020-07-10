# $Id: Makefile.unit_test_gtfreader.app 414075 2013-09-20 12:44:23Z ludwigf $

APP = unit_test_gtfreader
SRC = unit_test_gtfreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = gtfreader_test_cases

WATCHERS = ludwigf
