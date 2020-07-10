# $Id: Makefile.test_label.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_label
PROJ_TAG = test

SRC = test_label

LIB  = gui_objutils variation snputil gui_utils \
	   xalgoalignutil $(BLAST_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil \
	   submit gbseq tables valerr entrezgene \
	   eutils_client xmlwrapp xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   biotree $(OBJMGR_LIBS) 

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)
