# $Id: Makefile.dbapi_cache_test.app 184358 2010-02-26 16:33:57Z ivanov $

APP = dbapi_cache_test
SRC = dbapi_cache_test

LIB  = ncbi_xcache_dbapi dbapi dbapi_driver xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = kuznets
