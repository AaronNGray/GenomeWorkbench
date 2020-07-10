# $Id: Makefile.test_ncbi_ftp_connector.app 170930 2009-09-17 21:58:57Z vakatov $

APP = test_ncbi_ftp_connector
SRC = test_ncbi_ftp_connector
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD  = test_ncbi_ftp_connector.sh
CHECK_COPY = test_ncbi_ftp_connector.sh

WATCHERS = lavr
