#################################
# $Id: Makefile.test_compress_archive.app 393391 2013-03-25 15:18:29Z ivanov $

APP = test_compress_archive
SRC = test_compress_archive
LIB = xcompress xutil xncbi
LIBS = $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS)

CHECK_CMD = test_compress_archive test all

WATCHERS = ivanov
