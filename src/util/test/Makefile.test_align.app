# $Id: Makefile.test_align.app 178693 2009-12-15 19:37:13Z vakatov $

APP = test_align
SRC = test_align

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = todorov
