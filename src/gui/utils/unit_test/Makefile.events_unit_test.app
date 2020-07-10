# $Id: Makefile.events_unit_test.app 41919 2018-11-13 23:35:47Z rudnev $

APP = events_unit_test
SRC = events_tools events_unit_test
PROJ_TAG = test

LIB = gui_utils xutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil \
      xncbi
LIBS = $(BOOST_LIBPATH) $(BOOST_TEST_UTF_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test

