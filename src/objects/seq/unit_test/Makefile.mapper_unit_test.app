# $Id: Makefile.mapper_unit_test.app 573835 2018-11-02 12:41:47Z ivanov $
APP = mapper_unit_test
SRC = mapper_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = $(SEQ_LIBS) pub medline biblio general xser xutil test_boost xncbi

CHECK_COPY = mapper_unit_test.ini mapper_test_data
CHECK_CMD =

WATCHERS = grichenk
