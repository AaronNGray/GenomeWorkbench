#################################
# $Id: Makefile.test_objmgr_gbloader.app 590760 2019-08-04 21:12:21Z vakatov $
#################################

REQUIRES = bdb dbapi FreeTDS

APP = test_objmgr_gbloader
SRC = test_objmgr_gbloader
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CHECK_CMD = all_readers.sh test_objmgr_gbloader /CHECK_NAME=test_objmgr_gbloader
CHECK_COPY = all_readers.sh

WATCHERS = vasilche
