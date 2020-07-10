# $Id: Makefile.unit_test_bedreader.app 530006 2017-03-09 17:17:07Z foleyjp $

APP = unit_test_bedreader
SRC = unit_test_bedreader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = bedreader_test_cases

WATCHERS = ludwigf gotvyans foleyjp
