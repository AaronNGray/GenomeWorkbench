# $Id: Makefile.seq_id_unit_test.app 574429 2018-11-13 14:03:45Z ivanov $
APP = seq_id_unit_test
SRC = seq_id_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =

WATCHERS = ucko
