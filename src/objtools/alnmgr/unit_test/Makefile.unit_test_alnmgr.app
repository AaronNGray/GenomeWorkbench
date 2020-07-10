# $Id: Makefile.unit_test_alnmgr.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_alnmgr
SRC = unit_test_alnmgr

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalnmgr xobjutil submit tables test_boost $(OBJMGR_LIBS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_COPY = data

CHECK_CMD =

WATCHERS = grichenk
