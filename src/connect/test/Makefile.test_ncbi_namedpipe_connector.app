# $Id: Makefile.test_ncbi_namedpipe_connector.app 573939 2018-11-03 21:43:25Z lavr $

APP = test_ncbi_namedpipe_connector
SRC = test_ncbi_namedpipe_connector ncbi_conntest
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS) 

CHECK_CMD = test_ncbi_namedpipe_connector.sh
CHECK_COPY = test_ncbi_namedpipe_connector.sh

WATCHERS = lavr
