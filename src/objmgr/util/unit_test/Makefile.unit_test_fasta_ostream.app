# $Id: Makefile.unit_test_fasta_ostream.app 501626 2016-05-17 17:32:10Z kornbluh $

APP = unit_test_fasta_ostream
SRC = unit_test_fasta_ostream

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost xobjutil $(SOBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = dicuccio ucko

