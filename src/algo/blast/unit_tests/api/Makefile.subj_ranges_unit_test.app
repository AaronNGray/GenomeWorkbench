# $Id: Makefile.subj_ranges_unit_test.app 553487 2017-12-18 14:23:38Z fongah2 $

APP = subj_ranges_unit_test
SRC = subj_ranges_unit_test 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
LIB = test_boost $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL)) 
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = subj_ranges_unit_test
CHECK_COPY = subj_ranges_unit_test.ini

WATCHERS = boratyng madden camacho fongah2 
