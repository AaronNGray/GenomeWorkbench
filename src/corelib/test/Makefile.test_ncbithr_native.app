# $Id: Makefile.test_ncbithr_native.app 178673 2009-12-15 17:34:00Z grichenk $

APP = test_ncbithr_native
SRC = test_ncbithr_native
LIB = xncbi

REQUIRES = MT

CHECK_CMD = test_ncbithr_native
CHECK_CMD = test_ncbithr_native -favorwriters

WATCHERS = grichenk
