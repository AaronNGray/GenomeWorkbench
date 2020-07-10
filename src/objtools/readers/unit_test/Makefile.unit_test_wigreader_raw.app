# $Id: Makefile.unit_test_wigreader_raw.app 599873 2020-01-08 18:10:00Z ludwigf $

APP = unit_test_wigreader_raw
SRC = unit_test_wigreader_raw

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = wigreader_test_cases_raw

WATCHERS = ludwigf
