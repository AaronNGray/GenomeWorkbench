# $Id: Makefile.test_ncbi_pipe.app 171524 2009-09-24 18:21:25Z lavr $

APP = test_ncbi_pipe
SRC = test_ncbi_pipe
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = ivanov lavr
