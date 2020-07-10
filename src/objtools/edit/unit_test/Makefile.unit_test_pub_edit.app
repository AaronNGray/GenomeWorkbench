# $Id: Makefile.unit_test_pub_edit.app 574504 2018-11-14 15:53:49Z foleyjp $

APP = unit_test_pub_edit
SRC = unit_test_pub_edit

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = $(OBJEDIT_LIBS) xunittestutil xobjutil \
       $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD = unit_test_pub_edit
CHECK_TIMEOUT = 3000

WATCHERS = asztalos
