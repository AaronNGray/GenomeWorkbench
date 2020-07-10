#################################
# $Id: Makefile.test_objmgr.app 173093 2009-10-14 16:24:46Z vakatov $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr"
#################################

APP = test_objmgr
SRC = test_objmgr test_helper
LIB = $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr

WATCHERS = vasilche
