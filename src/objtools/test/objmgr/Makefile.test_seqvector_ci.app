#################################
# $Id: Makefile.test_seqvector_ci.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build object manager test application "test_objmgr"
#################################


APP = test_seqvector_ci
SRC = test_seqvector_ci
LIB = $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = run_sybase_app.sh test_seqvector_ci /CHECK_NAME=test_seqvector_ci
CHECK_TIMEOUT = 500

WATCHERS = grichenk
