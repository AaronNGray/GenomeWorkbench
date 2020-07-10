# $Id: Makefile.unit_test_seq.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_seq
SRC = unit_test_seq

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xobjutil $(OBJMGR_LIBS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD  =

WATCHERS = vasilche
