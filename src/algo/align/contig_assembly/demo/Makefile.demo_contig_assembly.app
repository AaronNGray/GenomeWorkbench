# $Id: Makefile.demo_contig_assembly.app 590760 2019-08-04 21:12:21Z vakatov $

WATCHERS = jcherry

ASN_DEP = seq

APP = demo_contig_assembly
SRC = demo_contig_assembly
LIB = xalgocontig_assembly xalgoalignnw xalgoseq xregexp $(PCRE_LIB) \
      $(BLAST_LIBS) taxon1 $(OBJMGR_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = objects

