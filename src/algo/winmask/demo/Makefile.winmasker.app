# $Id: Makefile.winmasker.app 553811 2017-12-21 19:13:00Z fongah2 $

WATCHERS = dicuccio

ASN_DEP = seq

APP = winmasker
SRC = main win_mask_app
LIB = xalgowinmask \
	  xblast xnetblastcli xnetblast scoremat seqdb blastdb tables \
	  $(OBJREAD_LIBS) xobjutil $(OBJMGR_LIBS) $(LMDB_LIB)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

