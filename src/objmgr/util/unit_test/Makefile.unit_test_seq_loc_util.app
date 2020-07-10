# $Id: Makefile.unit_test_seq_loc_util.app 501626 2016-05-17 17:32:10Z kornbluh $

APP = unit_test_seq_loc_util
SRC = unit_test_seq_loc_util

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xobjutil $(SOBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = vasilche grichenk
