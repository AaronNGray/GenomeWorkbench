# $Id: Makefile.test_netcache_api.app 589064 2019-07-05 15:10:48Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_netcache_api
SRC = test_netcache_api
LIB = xconnserv xthrserv xconnect xutil test_boost xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_netcache_api
CHECK_TIMEOUT = 1200

WATCHERS = sadyrovr gouriano fukanchi
