# $Id: Makefile.test_math.app 371619 2012-08-09 16:47:32Z vasilche $

APP = test_math
SRC = test_math

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xutil xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

WATCHERS = dicuccio

