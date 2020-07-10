# $Id: Makefile.test_cache_mt.app 371619 2012-08-09 16:47:32Z vasilche $

APP = test_cache_mt
SRC = test_cache_mt
LIB = xutil test_mt xncbi

REQUIRES = MT

CHECK_CMD = test_cache_mt

WATCHERS = grichenk
