# $Id: Makefile.test_id1_client.app 191176 2010-05-10 16:12:20Z vakatov $

APP = test_id1_client
SRC = test_id1_client
LIB = id1cli id1 seqset $(SEQ_LIBS) pub medline biblio general \
      xser xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = ucko
