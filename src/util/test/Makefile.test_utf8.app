#################################
# $Id: Makefile.test_utf8.app 209388 2010-10-26 13:49:49Z gouriano $

APP = test_utf8
SRC = test_utf8
LIB = xutil test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_utf8_u2a.txt test_utf8.ini

WATCHERS = gouriano
