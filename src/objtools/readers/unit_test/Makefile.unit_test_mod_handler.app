# $Id: Makefile.unit_test_mod_handler.app 575496 2018-11-29 19:17:09Z foleyjp $

APP = unit_test_mod_handler
SRC = unit_test_mod_handler

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = mod_handler_test_cases

WATCHERS = foleyjp
