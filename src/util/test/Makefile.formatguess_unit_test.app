# $Id: Makefile.formatguess_unit_test.app 529995 2017-03-09 17:06:51Z foleyjp $

APP = formatguess_unit_test
SRC = formatguess_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xutil xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

WATCHERS = vakatov foleyjp
