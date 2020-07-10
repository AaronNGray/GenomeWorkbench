# $Id: Makefile.unit_test_field_resolver.app 45050 2020-05-18 17:58:56Z asztalos $

APP = unit_test_field_resolver
SRC = unit_test_field_resolver

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xvalidate xunittestutil $(XFORMAT_LIBS) xalnmgr xobjutil valid valerr taxon3 gbseq submit \
       gui_objutils snputil variation gui_utils xobjedit xobjutil valerr \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) \
       $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD = unit_test_field_resolver
CHECK_COPY = gene.asn
CHECK_TIMEOUT = 3000

WATCHERS = asztalos evgeniev
