# $Id: Makefile.bdbtest_vmerge.app 486559 2015-12-07 16:37:38Z satskyse $

APP = bdbtest_vmerge
SRC = test_bdb_vmerge
LIB = xalgovmerge $(BDB_CACHE_LIB) $(BDB_LIB) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = satskyse
