# $Id: Makefile.unit_test_import_feattable.app 41919 2018-11-13 23:35:47Z rudnev $

APP = unit_test_import_feattable
SRC = unit_test_import_feattable

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB  = xvalidate xunittestutil $(XFORMAT_LIBS) xalnmgr xobjutil valid valerr taxon3 gbseq submit \
       gui_objutils variation gui_utils xobjutil valerr eutils_client xmlwrapp \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS) pkg_sequence_edit 
LIBS = $(VDB_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(WXWIDGETS_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included wxWidgets WinMain

CHECK_CMD = unit_test_import_feattable
CHECK_COPY = FeatTableData
CHECK_TIMEOUT = 3000

WATCHERS = asztalos bollin
