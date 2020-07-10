# $Id: Makefile.unit_test_microarrayreader.app 597513 2019-11-22 13:54:51Z ludwigf $

APP = unit_test_microarrayreader
SRC = unit_test_microarrayreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = microarrayreader_test_cases

WATCHERS = ludwigf
