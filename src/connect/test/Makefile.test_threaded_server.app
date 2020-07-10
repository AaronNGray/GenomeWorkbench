# $Id: Makefile.test_threaded_server.app 184358 2010-02-26 16:33:57Z ivanov $

APP = test_threaded_server
SRC = test_threaded_server
LIB = xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify -best-effort CC

REQUIRES = MT

# Tested along with test_threaded_client from its makefile.

WATCHERS = ucko
