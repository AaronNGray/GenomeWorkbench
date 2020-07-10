# $Id: Makefile.hspfilter_culling_unit_test.app 553578 2017-12-18 23:33:36Z fongah2 $

APP = hspfilter_culling_unit_test
SRC = hspfilter_culling_unit_test 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
LIB = test_boost $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL))
LIBS = $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = hspfilter_culling_unit_test
CHECK_COPY = hspfilter_culling_unit_test.ini

WATCHERS = boratyng madden camacho fongah2
