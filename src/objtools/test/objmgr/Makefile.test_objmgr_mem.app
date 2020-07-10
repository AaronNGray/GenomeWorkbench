#################################
# $Id: Makefile.test_objmgr_mem.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr_mem"
#################################


APP = test_objmgr_mem
SRC = test_objmgr_mem
LIB = $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_mem

WATCHERS = vasilche
