# $Id: Makefile.unit_test_srcwriter.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_srcwriter
SRC = unit_test_srcwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
        tables test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = srcwriter_test_cases

WATCHERS = foleyjp
