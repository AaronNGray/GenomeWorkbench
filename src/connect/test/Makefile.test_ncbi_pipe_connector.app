# $Id: Makefile.test_ncbi_pipe_connector.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_pipe_connector
SRC = test_ncbi_pipe_connector
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
