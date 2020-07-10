# $Id: Makefile.test_range_coll.app 371619 2012-08-09 16:47:32Z vasilche $

APP = test_range_coll
SRC = test_range_coll

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xutil xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

WATCHERS = dicuccio

