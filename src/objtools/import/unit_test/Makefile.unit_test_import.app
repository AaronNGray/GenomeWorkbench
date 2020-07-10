# $Id: Makefile.unit_test_import.app 571257 2018-09-24 14:08:24Z ludwigf $

APP = unit_test_import
SRC = unit_test_import

LIB  = xunittestutil xobjimport xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

WATCHERS = ludwigf
