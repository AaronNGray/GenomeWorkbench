# $Id: Makefile.test_netservice_params.app 544025 2017-08-16 16:12:22Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_netservice_params
SRC = test_netservice_params
LIB = xconnserv xthrserv xconnect xutil test_boost xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_netservice_params

WATCHERS = sadyrovr
