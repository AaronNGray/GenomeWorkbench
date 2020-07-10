#################################
# $Id: Makefile.feat_unit_test.app 590760 2019-08-04 21:12:21Z vakatov $
#################################

REQUIRES = dbapi FreeTDS Boost.Test.Included

APP = feat_unit_test
SRC = feat_unit_test
LIB = test_boost xobjutil ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = feat_unit_test

WATCHERS = vasilche
