# $Id: Makefile.test_ncbi_linkerd.app 543990 2017-08-16 11:40:42Z mcelhany $

# Temporarily disable on Windows due to missing devops support.
REQUIRES = -MSWin

APP = test_ncbi_linkerd
SRC = test_ncbi_linkerd

LIB = connssl connect
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_ncbi_linkerd.sh
CHECK_COPY = test_ncbi_linkerd.sh ../../check/ncbi_test_data
CHECK_TIMEOUT = 30

WATCHERS = lavr mcelhany
