# $Id: Makefile.test_ncbi_conn_stream_mt.app 561784 2018-04-11 21:22:30Z lavr $

APP = test_ncbi_conn_stream_mt
SRC = test_ncbi_conn_stream_mt

LIB = xconnect test_mt xncbi
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_conn_stream_mt.sh
CHECK_COPY = test_ncbi_conn_stream_mt.sh ../../check/ncbi_test_data

WATCHERS = lavr
