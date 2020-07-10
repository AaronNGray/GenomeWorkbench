# $Id: Makefile.test_cached_taxon.app 501626 2016-05-17 17:32:10Z kornbluh $

APP = test_cached_taxon
SRC = test_cached_taxon

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost $(XFORMAT_LIBS) xalnmgr xobjutil xregexp $(OBJMGR_LIBS) tables

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_cached_taxon.ini
CHECK_TIMEOUT = 3000

WATCHERS = bollin kans
