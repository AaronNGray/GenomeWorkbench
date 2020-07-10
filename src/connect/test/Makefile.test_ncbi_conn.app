# $Id: Makefile.test_ncbi_conn.app 574061 2018-11-05 20:01:41Z lavr $

APP = test_ncbi_conn
SRC = test_ncbi_conn
LIB = xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_COPY = test_ncbi_conn.sh ../../check/ncbi_test_data
CHECK_CMD = test_ncbi_conn.sh

WATCHERS = lavr satskyse
