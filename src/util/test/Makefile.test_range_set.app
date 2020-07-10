# $Id: Makefile.test_range_set.app 601385 2020-02-06 16:31:49Z vasilche $

APP = test_range_set
SRC = test_range_set

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xutil xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

WATCHERS = dicuccio

