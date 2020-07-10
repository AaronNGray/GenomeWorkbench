# $Id: Makefile.lmdb_test2.app 540236 2017-07-05 12:52:24Z ivanov $

SRC = mtest2
APP = lmdb_test2
PROJ_TAG = test

CPPFLAGS = $(ORIG_CPPFLAGS) -I$(includedir)/util/lmdb

LIB = $(LMDB_LIB)
LIBS = $(LMDB_LIBS)

CHECK_COPY = lmdb_test.sh
CHECK_CMD  = lmdb_test.sh 2

REQUIRES = -Cygwin

WATCHERS = ivanov
