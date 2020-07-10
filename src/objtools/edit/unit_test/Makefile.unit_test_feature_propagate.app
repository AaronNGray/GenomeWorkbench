# $Id: Makefile.unit_test_feature_propagate.app 574504 2018-11-14 15:53:49Z foleyjp $

APP = unit_test_feature_propagate
SRC = unit_test_feature_propagate

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = $(OBJEDIT_LIBS) xunittestutil xalnmgr xobjutil \
       tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = 
CHECK_TIMEOUT = 3000

WATCHERS = bollin asztalos
