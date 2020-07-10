# $Id: Makefile.test_ncbi_sendmail.app 453808 2014-12-08 16:41:22Z lavr $

APP = test_ncbi_sendmail
SRC = test_ncbi_sendmail
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
