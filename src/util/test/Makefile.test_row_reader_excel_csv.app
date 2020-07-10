# $Id: Makefile.test_row_reader_excel_csv.app 539448 2017-06-22 14:51:54Z satskyse $

APP = test_row_reader_excel_csv
SRC = test_row_reader_excel_csv

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY =

WATCHERS = satskyse
