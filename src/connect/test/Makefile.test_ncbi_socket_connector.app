# $Id: Makefile.test_ncbi_socket_connector.app 375781 2012-09-24 15:07:33Z lavr $

APP = test_ncbi_socket_connector
SRC = test_ncbi_socket_connector ncbi_conntest
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_socket_connector.sh
CHECK_COPY = test_ncbi_socket_connector.sh

WATCHERS = lavr
