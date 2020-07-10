# $Id: Makefile.test_fstream_pushback.app 336970 2011-09-07 19:00:21Z lavr $

APP = test_fstream_pushback
SRC = test_fstream_pushback
LIB = xpbacktest test_mt xncbi

#LINK = purify $(ORIG_LINK)

CHECK_CMD  = test_fstream_pushback.sh
CHECK_COPY = test_fstream_pushback.sh

WATCHERS = lavr
