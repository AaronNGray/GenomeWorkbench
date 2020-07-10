# $Id: Makefile.unit_test_vcfwriter.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_vcfwriter
SRC = unit_test_vcfwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr \
      xobjutil gbseq entrez2cli entrez2 tables test_boost $(OBJMGR_LIBS)

#LIB = xunittestutil xobjwrite $(OBJREAD_LIBS) xobjutil gbseq xalnmgr entrez2cli entrez2 \
#	tables test_boost $(OBJMGR_LIBS)  variation_utils variation xutil xncbi

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = vcfwriter_test_cases

WATCHERS = ludwigf
