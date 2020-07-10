# $Id: Makefile.test_row_reader.app 531960 2017-03-30 16:27:34Z satskyse $

APP = test_row_reader
SRC = test_row_reader

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_row_reader.txt

WATCHERS = satskyse
