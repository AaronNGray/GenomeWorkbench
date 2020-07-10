# $Id: Makefile.test_app_job.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_app_job
PROJ_TAG = test

SRC = test_app_job test_client

CPPFLAGS = $(ORIG_CPPFLAGS)

LIB  = gui_utils tables \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(SOBJMGR_LIBS) $(COMPRESS_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = WinMain

