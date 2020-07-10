# $Id: Makefile.test_psg_client_impl.app 608056 2020-05-11 16:29:19Z ivanov $

APP = test_psg_client_impl
SRC = test_psg_client_impl
LIB = psg_client $(SEQ_LIBS) pub medline biblio general xser xconnserv xconnect test_boost xutil xncbi

LIBS = $(PSG_CLIENT_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) $(LIBUV_INCLUDE) \
           $(NGHTTP2_INCLUDE)

REQUIRES = MT Boost.Test.Included LIBUV NGHTTP2

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_psg_client_impl

WATCHERS = sadyrovr
