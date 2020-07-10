# $Id: Makefile.unit_test_location_constraint.app 539735 2017-06-27 11:11:02Z bollin $
APP = unit_test_location_constraint
SRC = unit_test_location_constraint

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = macro $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_CMD =

WATCHERS = bollin
