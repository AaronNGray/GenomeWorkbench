# $Id: Makefile.test_ncbi_http_upload.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_http_upload
SRC = test_ncbi_http_upload

LIB = xconnect xncbi $(NCBIATOMIC_LIB)
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_http_upload.sh
CHECK_COPY = test_ncbi_http_upload.sh ../../check/ncbi_test_data

WATCHERS = lavr
