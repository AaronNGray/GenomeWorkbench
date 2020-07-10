# $Id: Makefile.test_ncbi_core.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_core
SRC = test_ncbi_core
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
