# $Id: Makefile.test_uttp.app 545644 2017-09-08 14:48:58Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

PROJ_TAG = test,grid

APP = test_uttp
SRC = test_uttp
LIB = xutil test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = sadyrovr
