# $Id: Makefile.test_ncbi_hmac.app 390081 2013-02-23 17:13:45Z lavr $

APP = test_ncbi_hmac
SRC = test_ncbi_hmac
LIB = xutil xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
