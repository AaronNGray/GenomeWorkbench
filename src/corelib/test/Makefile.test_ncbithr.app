# $Id: Makefile.test_ncbithr.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbithr
SRC = test_ncbithr
LIB = xncbi

REQUIRES = MT

CHECK_CMD = test_ncbithr
CHECK_CMD = test_ncbithr -favorwriters

WATCHERS = grichenk
