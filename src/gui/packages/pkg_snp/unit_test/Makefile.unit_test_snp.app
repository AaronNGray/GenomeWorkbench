# $Id: Makefile.unit_test_snp.app 41919 2018-11-13 23:35:47Z rudnev $

APP = unit_test_snp
SRC = main test_snp_filters \
      ..\filter\snp_filter

LIB  = w_controls gui_objutils variation gui_utils \
       xalnmgr xobjutil valerr tables eutils_client \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   xmlwrapp xconnect $(OBJMGR_LIBS) 

LIBS = $(BOOST_LIBPATH) $(BOOST_TEST_UTF_LIBS) $(NETWORK_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = Boost.Test

