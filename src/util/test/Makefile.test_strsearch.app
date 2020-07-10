#################################
# $Id: Makefile.test_strsearch.app 177963 2009-12-07 19:08:54Z ucko $
#################################

# Build test application "test_strsearch"
#################################
APP = test_strsearch
SRC = test_strsearch
LIB = xutil test_boost xncbi

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = Boost.Test.Included

CHECK_CMD = test_strsearch

WATCHERS = ivanov
