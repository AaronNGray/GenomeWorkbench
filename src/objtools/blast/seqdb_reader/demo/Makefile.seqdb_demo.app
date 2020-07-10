# $Id: Makefile.seqdb_demo.app 553727 2017-12-20 19:27:47Z fongah2 $

APP = seqdb_demo
SRC = seqdb_demo
LIB_ = seqdb xobjutil blastdb $(SOBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC)) $(LMDB_LIB)

CFLAGS    = $(FAST_CFLAGS)
CXXFLAGS  = $(FAST_CXXFLAGS)
LDFLAGS   = $(FAST_LDFLAGS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = madden camacho
