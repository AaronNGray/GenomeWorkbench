# $Id: Makefile.unit_test_splign.app 602973 2020-03-04 23:58:33Z vakatov $

APP = unit_test_splign

SRC = unit_test_splign

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalgoalignsplign xalgoalignutil xalgoalignnw \
    xqueryparse xalgoseq \
    $(BLAST_LIBS) \
    test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(CMPRS_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(GRPC_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included $(GRPC_OPT)

# Uncomment if you do not want it to run automatically as part of
# "make check".
CHECK_CMD = unit_test_splign -mrna-data-in mrna_in.asn -est-data-in est_in.asn -mrna-expected mrna_expected.asn -est-expected est_expected.asn
CHECK_COPY = mrna_in.asn mrna_expected.asn est_in.asn est_expected.asn

## look at the alignments produced by the unit test with
#
# unit_test_splign -mrna-data-in mrna_in.asn -mrna-out -
#
# unit_test_splign -est-data-in est_in.asn -est-out -
#
### update of expected results can be produced with
#
# unit_test_splign -mrna-data-in mrna_in.asn -mrna-out mrna_expected.asn -mrna-outfmt asn
#
# unit_test_splign -est-data-in est_in.asn -est-out est_expected.asn -est-outfmt asn

WATCHERS = kiryutin

