# $Id: Makefile.dbapi_driver_check.app 553128 2017-12-13 16:54:04Z satskyse $

APP = dbapi_driver_check
SRC = dbapi_driver_check

LIB  = dbapi_driver xncbi
LIBS = $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = ucko satskyse
