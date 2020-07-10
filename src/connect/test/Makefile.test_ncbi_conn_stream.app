# $Id: Makefile.test_ncbi_conn_stream.app 561803 2018-04-12 01:45:24Z ucko $

APP = test_ncbi_conn_stream
SRC = test_ncbi_conn_stream
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
