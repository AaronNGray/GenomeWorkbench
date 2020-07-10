# $Id: Makefile.unit_test_title_parser.app 578890 2019-01-28 16:40:58Z foleyjp $

APP = unit_test_title_parser
SRC = unit_test_title_parser

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = title_parser_test_cases

WATCHERS = foleyjp
