# $Id: Makefile.unit_test_internal_stops.app 590760 2019-08-04 21:12:21Z vakatov $

APP = unit_test_internal_stops

SRC = unit_test_internal_stops

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalgoseq taxon1 xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost \
      $(OBJMGR_LIBS)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

# Comment this if you do not want it to run automatically as part of
# "make check".
CHECK_CMD = unit_test_internal_stops
CHECK_COPY =

WATCHERS = chetvern
