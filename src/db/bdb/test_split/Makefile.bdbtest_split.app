# $Id: Makefile.bdbtest_split.app 486559 2015-12-07 16:37:38Z satskyse $

APP = bdbtest_split
SRC = test_bdb_split
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = satskyse
