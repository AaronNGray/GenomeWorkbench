# $Id: Makefile.test_ncbi_socket.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_socket
SRC = test_ncbi_socket
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_socket.sh
CHECK_COPY = test_ncbi_socket.sh

WATCHERS = lavr
