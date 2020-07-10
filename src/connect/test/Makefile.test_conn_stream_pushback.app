# $Id: Makefile.test_conn_stream_pushback.app 561784 2018-04-11 21:22:30Z lavr $

APP = test_conn_stream_pushback
SRC = test_conn_stream_pushback
LIB = xconnect xpbacktest xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_conn_stream_pushback.sh
CHECK_COPY = test_conn_stream_pushback.sh

WATCHERS = lavr
