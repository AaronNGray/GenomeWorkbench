# $Id: Makefile.test_ncbi_buffer.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_buffer
SRC = test_ncbi_buffer
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_buffer.sh
CHECK_COPY = test_ncbi_buffer.sh


WATCHERS = lavr
