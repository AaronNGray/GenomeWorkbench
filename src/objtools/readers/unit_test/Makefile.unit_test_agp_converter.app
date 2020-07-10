# $Id: Makefile.unit_test_agp_converter.app 582359 2019-03-14 10:42:44Z kornbluh $

APP = unit_test_agp_converter
SRC = unit_test_agp_converter

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = agp_converter_test_cases

WATCHERS = bollin ludwigf drozdov

