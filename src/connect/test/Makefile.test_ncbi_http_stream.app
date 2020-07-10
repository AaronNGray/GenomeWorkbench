# $Id: Makefile.test_ncbi_http_stream.app 574668 2018-11-16 13:43:21Z lavr $

APP = test_ncbi_http_stream
SRC = test_ncbi_http_stream
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_http_stream https://www.ncbi.nlm.nih.gov/Service/index.html / /Service/index.html /CHECK_NAME=test_ncbi_http_stream

WATCHERS = lavr
