# $Id: Makefile.unit_test_get_label.app 506381 2016-07-07 13:22:57Z bollin $

APP = unit_test_get_label
SRC = unit_test_get_label

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xobjutil $(SOBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = vasilche dicuccio bollin
