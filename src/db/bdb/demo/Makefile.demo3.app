# $Id: Makefile.demo3.app 184358 2010-02-26 16:33:57Z ivanov $

APP = bdb_demo3
SRC = demo3
LIB = bdb xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BERKELEYDB_INCLUDE)

WATCHERS = kuznets
