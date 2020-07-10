# $Id: Makefile.test_value_convert.app 380269 2012-11-09 15:17:55Z vakatov $

APP = test_value_convert
SRC = test_value_convert
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =


WATCHERS = ucko
