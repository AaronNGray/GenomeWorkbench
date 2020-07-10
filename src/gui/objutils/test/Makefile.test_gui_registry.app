# $Id: Makefile.test_gui_registry.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_gui_registry
PROJ_TAG = test

SRC = test_gui_registry

LIB  = gui_objutils variation gui_utils submit xalnmgr xobjutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   tables eutils_client xmlwrapp xconnect xcompress $(SOBJMGR_LIBS) 

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)

