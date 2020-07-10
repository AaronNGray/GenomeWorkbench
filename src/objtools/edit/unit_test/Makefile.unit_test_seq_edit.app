# $Id: Makefile.unit_test_seq_edit.app 589491 2019-07-11 16:43:44Z foleyjp $

APP = unit_test_seq_edit
SRC = unit_test_seq_edit

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = $(OBJEDIT_LIBS) xunittestutil xalnmgr xobjutil \
       tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = seq_edit_test_cases
CHECK_TIMEOUT = 3000

WATCHERS = foleyjp
