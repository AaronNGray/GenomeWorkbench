# $Id: Makefile.test_convert_raw_to_delta.app 45092 2020-05-28 18:08:36Z ivanov $

APP = test_convert_raw_to_delta
SRC = test_convert_raw_to_delta

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_utils gbproj $(OBJEDIT_LIBS) xunittestutil $(XFORMAT_LIBS) xobjutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       snputil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_TIMEOUT = 3000

WATCHERS = gotvyans

