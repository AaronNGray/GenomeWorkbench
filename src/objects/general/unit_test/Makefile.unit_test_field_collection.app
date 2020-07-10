# $Id: Makefile.unit_test_field_collection.app 210327 2010-11-03 14:39:51Z whlavina $
APP = unit_test_field_collection
SRC = unit_test_field_collection

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = general xser xutil test_boost xncbi

CHECK_CMD = unit_test_field_collection -data-in uo.asnt
CHECK_COPY = uo.asnt

WATCHERS = whlavina
