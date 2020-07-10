# $Id: Makefile.test_random.app 415936 2013-10-22 18:10:13Z satskyse $

APP = test_random
SRC = test_random

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xutil xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD = test_random

WATCHERS = satskyse

