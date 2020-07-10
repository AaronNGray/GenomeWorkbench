# $Id: Makefile.seqfeat_unit_test.app 573835 2018-11-02 12:41:47Z ivanov $

APP = seqfeat_unit_test
SRC = seqfeat_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

CHECK_CMD =

WATCHERS = grichenk
