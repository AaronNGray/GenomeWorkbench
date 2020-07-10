#################################
# $Id: Makefile.test_relloc.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Aaron Ucko (ucko@ncbi.nlm.nih.gov)
#################################

# Build relative-location-computation test application "test_relloc"
#################################


APP = test_relloc
SRC = test_relloc
LIB = xobjutil $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = ucko
