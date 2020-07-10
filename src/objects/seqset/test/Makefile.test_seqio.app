# $Id: Makefile.test_seqio.app 398092 2013-05-02 18:16:55Z vakatov $

APP = test_seqio
SRC = test_seqio

REQUIRES = Boost.Test.Included

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = test_boost seqset $(SEQ_LIBS) pub medline biblio general xser xutil xncbi

CHECK_CMD =
CHECK_REQUIRES = in-house-resources
CHECK_TIMEOUT = 400

WATCHERS = vasilche gouriano
