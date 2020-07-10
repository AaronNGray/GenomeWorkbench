# $Id: Makefile.test_json_over_uttp.app 466155 2015-04-29 14:36:10Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_json_over_uttp
SRC = test_json_over_uttp
LIB = xconnserv xthrserv xconnect xutil test_boost xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

WATCHERS = sadyrovr
