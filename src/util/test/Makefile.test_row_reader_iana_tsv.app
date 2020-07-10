# $Id: Makefile.test_row_reader_iana_tsv.app 534346 2017-04-26 17:22:17Z satskyse $

APP = test_row_reader_iana_tsv
SRC = test_row_reader_iana_tsv

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY =

WATCHERS = satskyse
