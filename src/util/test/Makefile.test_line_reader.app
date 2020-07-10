#################################
# $Id: Makefile.test_line_reader.app 489927 2016-01-19 21:29:09Z elisovdn $

APP = test_line_reader
SRC = test_line_reader
CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
LIB = xutil test_boost xncbi

REQUIRES = Boost.Test.Included
CHECK_REQUIRES = in-house-resources

CHECK_CMD = test_line_reader -selftest /CHECK_NAME=test_line_reader

WATCHERS = ucko
