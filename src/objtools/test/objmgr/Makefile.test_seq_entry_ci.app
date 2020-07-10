# $Id: Makefile.test_seq_entry_ci.app 505858 2016-06-29 16:55:21Z elisovdn $

APP = test_seq_entry_ci
SRC = test_seq_entry_ci

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost $(OBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = vasilche grichenk
