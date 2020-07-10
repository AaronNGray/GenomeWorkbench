# $Id: Makefile.test_fw.app 448737 2014-10-08 16:23:00Z lavr $

APP = test_fw
SRC = test_fw
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = lavr
