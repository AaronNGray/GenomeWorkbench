# $Id: Makefile.unit_test_gap_analysis.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_gap_analysis

SRC = unit_test_gap_analysis

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xalgoseq xvalidate xobjedit $(XFORMAT_LIBS) $(OBJREAD_LIBS) valerr taxon1 xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

# Uncomment if you do not want it to run automatically as part of
# "make check".
CHECK_CMD = unit_test_gap_analysis -basic-data gap_analysis.seq.asn -in-letter-gap-data gap_with_letters.seq.asn -mixed-gap-type-data mixed_gap_type_data.seq.asn
CHECK_COPY = gap_analysis.seq.asn gap_with_letters.seq.asn mixed_gap_type_data.seq.asn

WATCHERS = drozdov
