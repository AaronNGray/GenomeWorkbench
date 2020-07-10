# $Id: Makefile.align_filter_unit_test.app 590760 2019-08-04 21:12:21Z vakatov $

APP = align_filter_unit_test
SRC = align_filter_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalgoalignutil xalgoseq $(BLAST_LIBS) xqueryparse \
      taxon1 xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included objects

CHECK_CMD = align_filter_unit_test -data-in data/seqalign.asn -filters data/filters
CHECK_COPY = data


WATCHERS = mozese2
