# $Id: Makefile.test_msg.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_msg
PROJ_TAG = test

SRC = test_msg

LIB  = gui_utils ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

