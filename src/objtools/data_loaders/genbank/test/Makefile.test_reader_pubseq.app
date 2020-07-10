#################################
# $Id: Makefile.test_reader_pubseq.app 191176 2010-05-10 16:12:20Z vakatov $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build serialization test application "serialtest"
#################################

REQUIRES = PubSeqOS

APP = test_reader_pubseq
SRC = test_reader_pubseq

LIB = $(GENBANK_READER_PUBSEQOS_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = vasilche
