# $Id: Makefile.test_threaded_client.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_threaded_client
SRC = test_threaded_client
LIB = xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify -best-effort CC

REQUIRES = MT

# (neither of these can contain make variables)
CHECK_CMD = test_threaded_client_server.sh
CHECK_COPY = test_threaded_client_server.sh

WATCHERS = ucko
