#################################
# $Id: Makefile.id_unit_test.app 598983 2019-12-17 21:19:31Z vasilche $
#################################

REQUIRES = dbapi FreeTDS Boost.Test.Included

APP = id_unit_test
SRC = id_unit_test
LIB = test_boost xobjsimple xobjutil ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = all_readers.sh id_unit_test.ini

CHECK_CMD = all_readers.sh id_unit_test
CHECK_TIMEOUT = 800

WATCHERS = vasilche
