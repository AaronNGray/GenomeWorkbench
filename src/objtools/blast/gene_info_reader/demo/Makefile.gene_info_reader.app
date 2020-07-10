# $Id: Makefile.gene_info_reader.app 553727 2017-12-20 19:27:47Z fongah2 $

WATCHERS = camacho

REQUIRES = algo

ASN_DEP = seq

APP = gene_info_reader
SRC = gene_info_reader_app

LIB_ = gene_info xobjutil seqdb blastdb $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC)) $(LMDB_LIB)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

