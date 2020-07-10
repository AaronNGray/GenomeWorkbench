# $Id: Makefile.test_ncbi_namerd.app 538010 2017-06-07 12:25:03Z mcelhany $

APP = test_ncbi_namerd
SRC = test_ncbi_namerd

LIB = connect
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_ncbi_namerd.sh
CHECK_COPY = test_ncbi_namerd.sh ../../check/ncbi_test_data
CHECK_TIMEOUT = 30

WATCHERS = lavr mcelhany
