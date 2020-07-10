# $Id: Makefile.gene_info_writer.app 553727 2017-12-20 19:27:47Z fongah2 $

APP = gene_info_writer
SRC = gene_info_writer_app
LIB_ = gene_info_writer gene_info seqdb xobjutil blastdb $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC)) $(LMDB_LIB)

CFLAGS    = $(FAST_CFLAGS)
CXXFLAGS  = $(FAST_CXXFLAGS)
LDFLAGS   = $(FAST_LDFLAGS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

WATCHERS = madden camacho
