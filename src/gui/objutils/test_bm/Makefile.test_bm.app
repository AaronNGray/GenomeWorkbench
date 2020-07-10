# $Id: Makefile.test_bm.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_bm

PROJ_TAG = test

SRC = test_bm

LIB = gui_objutils variation snputil gui_utils \
      xalgoalignutil $(BLAST_LIBS) xalnmgr xobjutil \
      valerr entrezgene biotree eutils_client xmlwrapp xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      $(OBJMGR_LIBS) 

LIBS = $(DL_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
