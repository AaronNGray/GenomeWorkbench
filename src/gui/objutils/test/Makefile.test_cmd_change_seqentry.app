# $Id: Makefile.test_cmd_change_seqentry.app 41919 2018-11-13 23:35:47Z rudnev $

APP = test_cmd_change_seqentry
PROJ_TAG = test

SRC = test_cmd_change_seqentry

LIB  = gui_objutils variation gui_utils xdiscrepancy macro xvalidate taxon1 \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xcleanup taxon3 xobjedit xalnmgr xobjutil submit gbseq tables valerr snputil $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(DL_LIBS) $(ORIG_LIBS)
