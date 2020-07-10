# $Id: Makefile.test_ncbi_server_info.app 568161 2018-07-31 18:52:23Z lavr $

APP = test_ncbi_server_info
SRC = test_ncbi_server_info
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_server_info.sh
CHECK_COPY = test_ncbi_server_info.sh

WATCHERS = lavr
