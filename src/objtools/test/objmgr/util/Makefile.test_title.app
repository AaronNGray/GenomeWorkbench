#################################
# $Id: Makefile.test_title.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build title-computation test application "test_title"
#################################


APP = test_title
SRC = test_title
LIB = xobjutil $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = ucko
