# $Id: Makefile.seqfeatdata_unit_test.app 586496 2019-05-16 20:34:11Z xiangcha $

APP = seqfeatdata_unit_test
SRC = seqfeatdata_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects

CHECK_CMD =

WATCHERS = grichenk foleyjp
