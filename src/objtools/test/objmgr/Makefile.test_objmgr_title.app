#################################
# $Id: Makefile.test_objmgr_title.app 505858 2016-06-29 16:55:21Z elisovdn $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build title-computation test application "test_title"
#################################


APP = test_objmgr_title
SRC = test_objmgr_title
LIB = $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = vasilche
