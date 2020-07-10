# $Id: Makefile.test_ncbicntr.app 474413 2015-07-28 18:17:51Z vakatov $

APP = test_ncbicntr
SRC = test_ncbicntr
LIB = test_mt xncbi


CHECK_CMD =
CHECK_REQUIRES = MT -Valgrind

WATCHERS = vakatov
