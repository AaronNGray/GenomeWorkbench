# $Id: Makefile.unit_test_pslwriter.app 594841 2019-10-10 13:44:35Z ludwigf $

APP = unit_test_pslwriter
SRC = unit_test_pslwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJEDIT_LIBS) $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
        tables test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = pslwriter_test_cases

WATCHERS = ludwigf
