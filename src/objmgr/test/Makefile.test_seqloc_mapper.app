# $Id: Makefile.test_seqloc_mapper.app 414552 2013-09-25 10:39:46Z meric $

APP = test_seqloc_mapper

SRC = test_seqloc_mapper

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = gencoll_client genome_collection seqset $(OBJREAD_LIBS) \
      xalnmgr $(XFORMAT_LIBS) xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD = test_seqloc_mapper

WATCHERS = grichenk
