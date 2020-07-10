# $Id: Makefile.bl2seq_unit_test.app 601536 2020-02-10 15:02:01Z boratyng $

APP = bl2seq_unit_test
SRC = bl2seq_unit_test

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
LIB = blast_unit_test_util test_boost $(BLAST_INPUT_LIBS) \
    $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL)) 
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)


CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = bl2seq_unit_test
CHECK_COPY = bl2seq_unit_test.ini

WATCHERS = boratyng madden camacho fongah2 
