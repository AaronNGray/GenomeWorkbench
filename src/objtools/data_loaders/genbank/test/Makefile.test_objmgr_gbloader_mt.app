#################################
# $Id: Makefile.test_objmgr_gbloader_mt.app 590760 2019-08-04 21:12:21Z vakatov $
#################################

REQUIRES = dbapi FreeTDS

APP = test_objmgr_gbloader_mt
SRC = test_objmgr_gbloader_mt
LIB = test_mt ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = all_readers.sh test_objmgr_gbloader_mt /CHECK_NAME=test_objmgr_gbloader_mt
CHECK_COPY = all_readers.sh
CHECK_TIMEOUT = 400

WATCHERS = vasilche
