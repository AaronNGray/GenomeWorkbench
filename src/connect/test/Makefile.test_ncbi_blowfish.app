# $Id: Makefile.test_ncbi_blowfish.app 582447 2019-03-15 16:01:11Z lavr $

APP = test_ncbi_blowfish
SRC = test_ncbi_blowfish
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
