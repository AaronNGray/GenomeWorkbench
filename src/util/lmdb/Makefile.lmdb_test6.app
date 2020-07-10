# $Id: Makefile.lmdb_test6.app 540236 2017-07-05 12:52:24Z ivanov $

SRC = mtest6
APP = lmdb_test6
PROJ_TAG = test

CPPFLAGS = $(ORIG_CPPFLAGS) -I$(includedir)/util/lmdb

LIB = $(LMDB_LIB)
LIBS = $(LMDB_LIBS)

CHECK_COPY = lmdb_test.sh
CHECK_CMD  = lmdb_test.sh 6

REQUIRES = -Cygwin

WATCHERS = ivanov
