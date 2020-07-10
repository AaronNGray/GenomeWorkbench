# $Id: Makefile.blobwriter.app 553128 2017-12-13 16:54:04Z satskyse $

APP = blobwriter
SRC = blobwriter

CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

LIB  = dbapi_util_blobstore dbapi_driver xcompress $(CMPRS_LIB) xutil xncbi
LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = ucko satskyse
