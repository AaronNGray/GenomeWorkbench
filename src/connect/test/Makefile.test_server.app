# $Id: Makefile.test_server.app 370257 2012-07-27 14:56:37Z ivanovp $

APP = test_server
SRC = test_server
LIB = xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify -best-effort CC

REQUIRES = MT

CHECK_CMD =
CHECK_TIMEOUT = 400

WATCHERS = vakatov
