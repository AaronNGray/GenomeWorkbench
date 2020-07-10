#################################
# $Id: Makefile.test_compile_time.app 588279 2019-06-19 22:47:27Z gotvyans $

APP = test_compile_time
SRC = test_compile_time
LIB = xutil test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_utf8_u2a.txt test_utf8.ini

WATCHERS = gotvyans
