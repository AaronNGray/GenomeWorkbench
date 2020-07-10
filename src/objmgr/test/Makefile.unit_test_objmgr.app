#################################
# $Id: Makefile.unit_test_objmgr.app 536474 2017-05-18 19:16:48Z vasilche $
#################################

REQUIRES = Boost.Test.Included

APP = unit_test_objmgr
SRC = unit_test_objmgr
LIB = test_boost $(SOBJMGR_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

CHECK_CMD = unit_test_objmgr

WATCHERS = vasilche
