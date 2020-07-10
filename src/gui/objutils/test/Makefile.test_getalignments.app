# $Id: Makefile.test_getalignments.app 41919 2018-11-13 23:35:47Z rudnev $

SRC = test_getalignments
APP = test_getalignments
PROJ_TAG = test

LIB = gui_objutils variation gui_opengl gui_utils \
      xalgoalignutil $(BLAST_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil \
	  submit gbseq tables valerr entrezgene \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	  biotree eutils_client xmlwrapp xconnect $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)

