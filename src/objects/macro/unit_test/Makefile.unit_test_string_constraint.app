# $Id: Makefile.unit_test_string_constraint.app 449420 2014-10-16 17:22:30Z bollin $
APP = unit_test_string_constraint
SRC = unit_test_string_constraint

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = macro $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =

WATCHERS = bollin
