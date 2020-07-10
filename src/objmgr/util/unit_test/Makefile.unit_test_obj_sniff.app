# $Id: Makefile.unit_test_obj_sniff.app 425920 2014-02-04 19:09:08Z vasilche $

APP = unit_test_obj_sniff
SRC = unit_test_obj_sniff

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xobjutil $(SOBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_data

WATCHERS = vasilche
