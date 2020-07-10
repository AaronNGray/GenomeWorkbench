# $Id: Makefile.unit_test_pslreader.app 597331 2019-11-20 14:27:19Z ludwigf $

APP = unit_test_pslreader
SRC = unit_test_pslreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = pslreader_test_cases

WATCHERS = ludwigf
