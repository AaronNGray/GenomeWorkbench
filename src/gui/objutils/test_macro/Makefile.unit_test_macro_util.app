# $Id: Makefile.unit_test_macro_util.app 44651 2020-02-14 16:53:07Z ucko $

APP = unit_test_macro_util
SRC = unit_test_macro_util
CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_objutils gui_utils xdiscrepancy eutils_client xmlwrapp \
       xobjwrite xvalidate xunittestutil xalnmgr $(XFORMAT_LIBS) xobjutil \
       macro gencoll_client valerr taxon1 ncbi_xcache_netcache sqlitewrapp \
       xhtml xqueryparse xregexp $(PCRE_LIB) tables test_boost $(OBJMGR_LIBS)

LIBS = $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) \
       $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD = unit_test_macro_util
CHECK_TIMEOUT = 3000

WATCHERS = asztalos evgeniev
