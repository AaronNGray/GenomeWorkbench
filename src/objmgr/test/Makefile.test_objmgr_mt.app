#################################
# $Id: Makefile.test_objmgr_mt.app 173093 2009-10-14 16:24:46Z vakatov $
# Author:  Aleksey Grichenko (grichenk@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr_mt"
#################################

APP = test_objmgr_mt
SRC = test_objmgr_mt test_helper
LIB = test_mt $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_mt
CHECK_TIMEOUT = 600

WATCHERS = vasilche
