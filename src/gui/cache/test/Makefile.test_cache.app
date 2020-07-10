# $Id: Makefile.test_cache.app 18176 2008-10-29 00:33:52Z dicuccio $

APP = test_cache

SRC = test_cache

CXXFLAGS = $(SQLITE3_INCLUDES) $(ORIG_CXXFLAGS)

LIB  = $(SOBJMGR_LIBS)

LIBS = $(SQLITE3_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

