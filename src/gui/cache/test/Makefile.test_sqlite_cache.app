# $Id: Makefile.test_sqlite_cache.app 43615 2019-08-09 14:49:33Z vakatov $

APP = test_sqlite_cache

SRC = test_sqlite_cache

LIB  = $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

