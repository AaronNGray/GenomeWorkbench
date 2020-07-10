# $Id: Makefile.pssmenginefreqratios_unit_test.app 553487 2017-12-18 14:23:38Z fongah2 $

APP = pssmenginefreqratios_unit_test
SRC = pssmenginefreqratios_unit_test 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -I$(srcdir)/../../api
LIB = blast_unit_test_util test_boost $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL)) 
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = pssmenginefreqratios_unit_test
CHECK_COPY = pssmenginefreqratios_unit_test.ini

WATCHERS = boratyng madden camacho fongah2
