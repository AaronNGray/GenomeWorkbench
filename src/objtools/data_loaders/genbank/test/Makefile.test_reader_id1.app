# $Id: Makefile.test_reader_id1.app 590760 2019-08-04 21:12:21Z vakatov $



APP = test_reader_id1
SRC = test_reader_id1
LIB = $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = vasilche
