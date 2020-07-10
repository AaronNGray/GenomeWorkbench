# $Id: Makefile.test_density_map.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_density_map
PROJ_TAG = test

SRC = test_density_map

CPPFLAGS = $(ORIG_CPPFLAGS)

LIB  = gui_utils xobjutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xalnmgr tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = WinMain

