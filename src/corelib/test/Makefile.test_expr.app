# $Id: Makefile.test_expr.app 571649 2018-10-01 10:14:16Z ivanov $

APP = test_expr
SRC = test_expr

LIB = test_boost$(STATIC) xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =


WATCHERS = vakatov
