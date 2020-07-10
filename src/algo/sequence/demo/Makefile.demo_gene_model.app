# $Id: Makefile.demo_gene_model.app 590760 2019-08-04 21:12:21Z vakatov $

WATCHERS = dicuccio

SRC = demo_gene_model
APP = demo_gene_model

CPPFLAGS = $(ORIG_CPPFLAGS)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

LIB = xalgoseq xalnmgr tables xregexp $(PCRE_LIB) xobjutil taxon1 $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
