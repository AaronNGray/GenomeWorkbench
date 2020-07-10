# $Id: Makefile.bdbtest_bvstore.app 184358 2010-02-26 16:33:57Z ivanov $

APP = bdbtest_bvstore
SRC = test_bdb_bvstore
LIB =  $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT bdb

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = kuznets
