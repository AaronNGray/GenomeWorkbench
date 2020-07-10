# $Id: Makefile.test_ncbi_namedpipe.app 573939 2018-11-03 21:43:25Z lavr $

APP = test_ncbi_namedpipe
SRC = test_ncbi_namedpipe
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_CMD = test_ncbi_namedpipe.sh
CHECK_COPY = test_ncbi_namedpipe.sh

WATCHERS = ivanov lavr
