# $Id: Makefile.test_ncbi_connutil_misc.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbi_connutil_misc
SRC = test_ncbi_connutil_misc
LIB = connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(C_LIBS)
LINK = $(C_LINK)
#LINK = purify $(C_LINK)

CHECK_CMD =

WATCHERS = lavr
