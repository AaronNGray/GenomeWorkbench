# $Id: Makefile.unit_test_gene_model.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_gene_model
SRC = unit_test_gene_model

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xalgoseq xvalidate xobjedit $(XFORMAT_LIBS) $(OBJREAD_LIBS) valerr taxon1 xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

# Uncomment if you do not want it to run automatically as part of
# "make check".
CHECK_CMD = unit_test_gene_model -data-in alignments.asn -data-expected annotations.asn -seqdata-expected seqdata.asn -combined-data-expected combined_annot.asn -combined-with-omission-expected combined_with_omission.asn /CHECK_NAME=unit_test_gene_model
CHECK_COPY = alignments.asn annotations.asn seqdata.asn combined_annot.asn combined_with_omission.asn

WATCHERS = dicuccio chetvern mozese2
