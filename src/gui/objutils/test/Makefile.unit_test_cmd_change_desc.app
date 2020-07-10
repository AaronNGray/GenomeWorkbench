# $Id: Makefile.unit_test_cmd_change_desc.app 41925 2018-11-14 17:20:29Z asztalos $

APP = unit_test_cmd_change_desc
SRC = unit_test_cmd_change_desc

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_utils xvalidate valerr \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(BLAST_LIBS) $(OBJEDIT_LIBS) xalnmgr macro valid xunittestutil \
       $(XFORMAT_LIBS) xobjutil tables xregexp $(PCRE_LIB) test_boost \
       $(OBJMGR_LIBS)

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD = unit_test_cmd_change_desc
CHECK_COPY = 
CHECK_TIMEOUT = 3000

WATCHERS = asztalos

