# $Id: Makefile.test_ncbistr.app 350435 2012-01-20 15:09:46Z grichenk $

APP = test_ncbistr
SRC = test_ncbistr
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =

WATCHERS = ivanov
