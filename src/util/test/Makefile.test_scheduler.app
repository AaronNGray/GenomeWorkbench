# $Id: Makefile.test_scheduler.app 370257 2012-07-27 14:56:37Z ivanovp $

APP = test_scheduler
SRC = test_scheduler

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xutil test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =


WATCHERS = vakatov
