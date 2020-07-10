# $Id: Makefile.unit_test_cmd_del_bioseq.app 45092 2020-05-28 18:08:36Z ivanov $

APP = unit_test_cmd_del_bioseq
SRC = unit_test_cmd_del_bioseq

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_utils gbproj xalgoalignutil entrezgene biotree xdiscrepancy xvalidate valerr snputil taxon1 $(BLAST_LIBS) $(OBJEDIT_LIBS) xalnmgr macro valid xunittestutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(XFORMAT_LIBS) xobjutil tables xregexp $(PCRE_LIB) test_boost \
       $(OBJMGR_LIBS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = 
CHECK_TIMEOUT = 3000

WATCHERS = gotvyans
