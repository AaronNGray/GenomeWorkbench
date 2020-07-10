# $Id: Makefile.test_ncbidiag_mt.app 464565 2015-04-10 17:53:06Z grichenk $

APP = test_ncbidiag_mt
SRC = test_ncbidiag_mt
LIB = test_mt xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) -DNCBI_SHOW_FUNCTION_NAME

CHECK_COPY = test_ncbidiag_mt.sh
CHECK_CMD = test_ncbidiag_mt.sh -format old /CHECK_NAME=test_ncbidiag_mt_old_fmt
CHECK_CMD = test_ncbidiag_mt.sh -format new /CHECK_NAME=test_ncbidiag_mt_new_fmt

WATCHERS = grichenk
