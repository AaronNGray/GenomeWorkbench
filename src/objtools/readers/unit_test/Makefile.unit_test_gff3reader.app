# $Id: Makefile.unit_test_gff3reader.app 413597 2013-09-17 13:35:23Z ludwigf $

APP = unit_test_gff3reader
SRC = unit_test_gff3reader

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = gff3reader_test_cases

WATCHERS = ludwigf
