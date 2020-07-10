# $Id: Makefile.bdbtestthr.app 184358 2010-02-26 16:33:57Z ivanov $

APP = bdbtestthr
SRC = test_bdb_thr
LIB = $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT bdb

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = kuznets

