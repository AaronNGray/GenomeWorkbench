# $Id: Makefile.unit_test_mod_adder.app 575638 2018-12-03 15:52:02Z foleyjp $

APP = unit_test_mod_adder
SRC = unit_test_mod_adder

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = mod_adder_test_cases

WATCHERS = foleyjp
