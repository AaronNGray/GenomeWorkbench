# $Id: Makefile.test_row_reader_ncbi_tsv.app 538614 2017-06-12 18:42:40Z satskyse $

APP = test_row_reader_ncbi_tsv
SRC = test_row_reader_ncbi_tsv

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xncbi

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY =

WATCHERS = satskyse
