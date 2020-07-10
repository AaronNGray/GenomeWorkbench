#################################
# $Id: Makefile.id_unit_test_bad.app 590760 2019-08-04 21:12:21Z vakatov $
#################################

REQUIRES = dbapi FreeTDS Boost.Test.Included

APP = id_unit_test_bad
SRC = id_unit_test_bad
LIB = test_boost xobjutil ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = all_readers.sh

CHECK_CMD = all_readers.sh id_unit_test_bad
CHECK_TIMEOUT = 800

WATCHERS = vasilche
