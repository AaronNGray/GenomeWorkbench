# $Id: Makefile.ntlookup_unit_test.app 601536 2020-02-10 15:02:01Z boratyng $

APP = ntlookup_unit_test
SRC = ntlookup_unit_test 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -I$(srcdir)/../../api
LIB = blast_unit_test_util test_boost $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL)) 
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = ntlookup_unit_test
CHECK_COPY = data ntlookup_unit_test.ini

WATCHERS = boratyng morgulis madden camacho fongah2
