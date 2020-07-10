# $Id: Makefile.ecnum_unit_test.app 173093 2009-10-14 16:24:46Z vakatov $
APP = ecnum_unit_test
SRC = ecnum_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =

WATCHERS = ucko
