# $Id: Makefile.unit_test_pkg_seqedit.app 43678 2019-08-14 15:03:47Z asztalos $

APP = unit_test_pkg_seqedit
SRC = unit_test_pkg_seqedit

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB  = xvalidate xunittestutil $(XFORMAT_LIBS) xalnmgr xobjutil valid valerr taxon3 gbseq submit \
       gui_objutils variation gui_utils w_edit xobjutil valerr eutils_client xmlwrapp \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS) pkg_sequence_edit 
LIBS = $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included wxWidgets WinMain

CHECK_CMD = unit_test_pkg_seqedit
CHECK_COPY = unit_test_pkg_seqedit.ini
CHECK_TIMEOUT = 3000

WATCHERS = asztalos bollin
