# $Id: Makefile.test_ncbi_file_connector.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_file_connector
SRC = test_ncbi_file_connector
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_file_connector test_ncbi_file_connector.dat /CHECK_NAME=test_ncbi_file_connector
CHECK_COPY = test_ncbi_file_connector.dat

WATCHERS = lavr
