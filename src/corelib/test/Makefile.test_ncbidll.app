# $Id: Makefile.test_ncbidll.app 170864 2009-09-17 16:09:35Z vakatov $

APP = test_ncbidll
SRC = test_ncbidll
LIB = xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = DLL
CHECK_CMD =

WATCHERS = ivanov
