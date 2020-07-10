# $Id: Makefile.unit_test_field.app 481991 2015-10-19 17:53:13Z vasilche $
APP = unit_test_field
SRC = unit_test_field

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = general xser xutil test_boost xncbi

CHECK_CMD = unit_test_field

WATCHERS = vasilche
