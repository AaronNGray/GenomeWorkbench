# $Id: Makefile.unit_test_gvfreader.app 413428 2013-09-16 13:59:48Z ludwigf $

APP = unit_test_gvfreader
SRC = unit_test_gvfreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = gvfreader_test_cases

WATCHERS = ludwigf
