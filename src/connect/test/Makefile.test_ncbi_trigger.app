# $Id: Makefile.test_ncbi_trigger.app 171333 2009-09-23 15:44:53Z ivanov $

APP = test_ncbi_trigger
SRC = test_ncbi_trigger
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

REQUIRES = MT

CHECK_CMD = test_ncbi_trigger.sh
CHECK_COPY = test_ncbi_trigger.sh

WATCHERS = lavr
