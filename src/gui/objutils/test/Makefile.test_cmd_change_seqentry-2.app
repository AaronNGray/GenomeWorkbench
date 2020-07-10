# $Id: Makefile.test_cmd_change_seqentry-2.app 45092 2020-05-28 18:08:36Z ivanov $

APP = test_cmd_change_seqentry-2
SRC = test_cmd_change_seqentry-2

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_utils xdiscrepancy xvalidate \
       valerr xcleanup taxon3 xobjedit xalnmgr biotree taxon1 snputil macro $(OBJEDIT_LIBS) \
       xunittestutil $(XFORMAT_LIBS) xobjutil tables xregexp $(PCRE_LIB) \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       test_boost gbproj xalgoalignutil entrezgene $(BLAST_LIBS) $(OBJMGR_LIBS)

LIBS = $(WXWIDGETS_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_cmd_change_seqentry-2.asn
CHECK_TIMEOUT = 3000

WATCHERS = gotvyans

