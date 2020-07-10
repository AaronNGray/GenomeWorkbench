# $Id: Makefile.test_compound_id.app 466155 2015-04-29 14:36:10Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_compound_id
SRC = test_compound_id
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT

WATCHERS = sadyrovr
