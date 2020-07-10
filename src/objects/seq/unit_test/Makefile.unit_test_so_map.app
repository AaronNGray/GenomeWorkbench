# $Id: Makefile.unit_test_so_map.app 573835 2018-11-02 12:41:47Z ivanov $

APP = unit_test_so_map
SRC = unit_test_so_map

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

LIBS = $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS)

CHECK_CMD =

WATCHERS = ludwigf
