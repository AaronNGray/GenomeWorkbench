# $Id: Makefile.test_cmd_propagate_feat.app 41925 2018-11-14 17:20:29Z asztalos $

APP = test_cmd_propagate_feat
SRC = test_cmd_propagate_feat

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_utils xalgoalignutil \
       xdiscrepancy xvalidate valerr snputil taxon1 \
       $(BLAST_LIBS) $(OBJEDIT_LIBS) xalnmgr valid xunittestutil \
       $(XFORMAT_LIBS) xobjutil tables xregexp $(PCRE_LIB) test_boost \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(OBJMGR_LIBS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD = test_cmd_propagate_feat
CHECK_COPY = PropagateFeatData
CHECK_TIMEOUT = 3000

WATCHERS = asztalos

