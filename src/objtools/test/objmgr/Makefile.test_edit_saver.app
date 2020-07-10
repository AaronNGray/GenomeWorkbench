#################################
# $Id: Makefile.test_edit_saver.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Maxim Didenko (didenko@ncbi.nlm.nih.gov)
#################################

APP = test_edit_saver
SRC = test_edit_saver
LIB = xobjutil ncbi_xloader_patcher $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_edit_saver -gi 45678
CHECK_CMD = test_edit_saver -gi 21225451 

WATCHERS = vasilche
