# $Id: Makefile.unit_test_gff3reader_genbank.app 538311 2017-06-09 13:30:55Z ludwigf $

APP = unit_test_gff3reader_genbank
SRC = unit_test_gff3reader_genbank

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = gff3reader_test_cases_genbank

WATCHERS = ludwigf
