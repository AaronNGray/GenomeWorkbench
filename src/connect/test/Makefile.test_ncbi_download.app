# $Id: Makefile.test_ncbi_download.app 574059 2018-11-05 20:01:04Z lavr $

APP = test_ncbi_download
SRC = test_ncbi_download
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_REQUIRES = in-house-resources
CHECK_COPY = test_ncbi_download.sh
CHECK_CMD = test_ncbi_download.sh

WATCHERS = lavr
