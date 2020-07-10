# $Id: Makefile.demo2.app 184358 2010-02-26 16:33:57Z ivanov $

APP = bdb_demo2
SRC = demo2
LIB = bdb xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = kuznets
