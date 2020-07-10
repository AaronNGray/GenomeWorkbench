# $Id: Makefile.unit_test_vcfreader.app 530006 2017-03-09 17:17:07Z foleyjp $

APP = unit_test_vcfreader
SRC = unit_test_vcfreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = vcfreader_test_cases

WATCHERS = ludwigf foleyjp
