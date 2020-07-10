# $Id: Makefile.test_ncbi_disp.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_disp
SRC = test_ncbi_disp
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =
CHECK_TIMEOUT = 30

WATCHERS = lavr
