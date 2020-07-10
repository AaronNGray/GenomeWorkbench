# $Id: Makefile.unit_test_gff3flybasewriter.app 598141 2019-12-04 15:41:51Z foleyjp $

APP = unit_test_gff3flybasewriter
SRC = unit_test_gff3flybasewriter
LIB = xunittestutil xobjwrite variation_utils $(OBJEDIT_LIBS) $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
        tables test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included MT

CHECK_CMD  =
CHECK_COPY = gff3flybasewriter_test_cases

WATCHERS = foleyjp
