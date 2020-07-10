#################################
# $Id: Makefile.test_objmgr_basic.app 173093 2009-10-14 16:24:46Z vakatov $
#################################

APP = test_objmgr_basic
SRC = test_objmgr_basic
LIB = $(SOBJMGR_LIBS)

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_objmgr_basic

WATCHERS = vasilche
