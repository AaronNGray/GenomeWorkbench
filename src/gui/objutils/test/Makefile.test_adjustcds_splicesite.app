# $Id: Makefile.test_adjustcds_splicesite.app 43380 2019-06-20 19:48:11Z asztalos $

APP = test_adjustcds_splicesite
SRC = test_adjustcds_splicesite

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

CHECK_CMD = test_adjustcds_splicesite
CHECK_COPY = SpliceSiteData
CHECK_TIMEOUT = 3000

WATCHERS = asztalos

