# $Id: Makefile.test_ncbi_http_session.app 573757 2018-10-31 20:18:17Z lavr $

APP = test_ncbi_http_session
SRC = test_ncbi_http_session
LIB = xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = 

WATCHERS = lavr
