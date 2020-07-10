#################################
# $Id: Makefile.test_loader_patcher.app 191176 2010-05-10 16:12:20Z vakatov $
#################################

APP = test_loader_patcher
SRC = test_loader_patcher

LIB = ncbi_xloader_patcher $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = vasilche
