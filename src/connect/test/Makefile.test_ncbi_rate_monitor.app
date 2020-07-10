# $Id: Makefile.test_ncbi_rate_monitor.app 339204 2011-09-27 15:34:33Z lavr $

APP = test_ncbi_rate_monitor
SRC = test_ncbi_rate_monitor
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD =

WATCHERS = lavr
