# $Id: Makefile.test_ncbi_service.app 574061 2018-11-05 20:01:41Z lavr $

APP = test_ncbi_service
SRC = test_ncbi_service
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_service bounce

WATCHERS = lavr
