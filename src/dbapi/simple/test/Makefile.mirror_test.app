# $Id: Makefile.mirror_test.app 353711 2012-02-17 22:18:57Z ivanovp $

APP = mirror_test
SRC = mirror_test

LIB  = $(SDBAPI_LIB) xconnect xutil xncbi

LIBS = $(SDBAPI_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT
