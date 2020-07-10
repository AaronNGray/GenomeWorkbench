# $Id: Makefile.test_ncbi_heapmgr.app 419798 2013-11-29 17:26:59Z ucko $

APP = test_ncbi_heapmgr
SRC = test_ncbi_heapmgr
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
# LINK = purify g++

CHECK_CMD =

WATCHERS = lavr
