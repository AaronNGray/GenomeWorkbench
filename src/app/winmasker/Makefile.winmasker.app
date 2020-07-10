# $Id: Makefile.winmasker.app 590760 2019-08-04 21:12:21Z vakatov $

WATCHERS = morgulis camacho mozese2 fongah2

REQUIRES = objects algo

ASN_DEP = seq

APP = windowmasker
SRC = main win_mask_app win_mask_sdust_masker

LIB = xalgowinmask xalgodustmask blast composition_adjustment \
        seqmasks_io seqdb blastdb tables $(OBJREAD_LIBS) xobjutil \
	$(OBJMGR_LIBS:%=%$(STATIC)) $(LMDB_LIB)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BLAST_THIRD_PARTY_INCLUDE)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

PROJ_TAG = gbench
