# $Id: Makefile.sdbapi_test_mt_pooling.app 576105 2018-12-10 18:59:23Z ucko $

APP = sdbapi_test_mt_pooling
SRC = sdbapi_test_mt_pooling

LIB  = $(SDBAPI_LIB) xconnect xutil test_mt xncbi
LIBS = $(SDBAPI_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD =
CHECK_REQUIRES = MT in-house-resources
CHECK_TIMEOUT = 600

WATCHERS = ucko satskyse
