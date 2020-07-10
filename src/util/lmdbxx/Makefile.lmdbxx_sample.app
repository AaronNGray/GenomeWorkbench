# $Id: Makefile.lmdbxx_sample.app 593136 2019-09-12 17:04:57Z lavr $

REQUIRES = LMDB

SRC = example
APP = lmdbxx_sample
PROJ_TAG = demo

CPPFLAGS = -I$(includedir)/util/lmdbxx $(LMDB_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = $(LMDB_LIB) xncbi
LIBS = $(LMDB_LIBS) $(ORIG_LIBS)

WATCHERS = ivanov
