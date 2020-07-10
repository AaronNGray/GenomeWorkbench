# $Id: Makefile.unit_test_defline.app 361177 2012-04-30 17:23:35Z bollin $

APP = unit_test_defline
SRC = unit_test_defline

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xobjutil $(SOBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = bollin
