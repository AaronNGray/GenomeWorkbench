# $Id: Makefile.test_weakref.app 370257 2012-07-27 14:56:37Z ivanovp $

APP = test_weakref
SRC = test_weakref

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = satskyse
