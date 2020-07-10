# $Id: Makefile.hgvs2variation.app 590760 2019-08-04 21:12:21Z vakatov $


APP = hgvs2variation
SRC = hgvs2variation

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -DBOOST_ERROR_CODE_HEADER_ONLY \
           -DBOOST_SYSTEM_NO_DEPRECATED
CXXFLAGS = $(ORIG_CXXFLAGS)
LDFLAGS  = $(ORIG_LDFLAGS)

LIB_ = hgvs $(OBJREAD_LIBS) \
       xobjutil entrez2cli entrez2 xregexp $(PCRE_LIB) $(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Spirit
