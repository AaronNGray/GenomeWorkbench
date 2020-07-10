# $Id: Makefile.seq_loc_unit_test.app 459281 2015-02-17 17:34:26Z vasilche $
APP = seq_loc_unit_test
SRC = seq_loc_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =

WATCHERS = vasilche
