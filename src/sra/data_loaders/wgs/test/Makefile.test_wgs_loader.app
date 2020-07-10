# $Id: Makefile.test_wgs_loader.app 590760 2019-08-04 21:12:21Z vakatov $

PROJ_TAG = test

APP = test_wgs_loader
SRC = test_wgs_loader

REQUIRES = Boost.Test.Included

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE) $(BOOST_INCLUDE)

LIB = ncbi_xloader_wgs $(SRAREAD_LIBS) xobjreadex $(OBJREAD_LIBS) xobjutil \
      test_boost $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

CHECK_COPY     = test_wgs_loader.ini
CHECK_CMD      = test_wgs_loader
CHECK_CMD      = test_wgs_loader --run_test="*StateTest"
CHECK_TIMEOUT  = 300
CHECK_REQUIRES = in-house-resources -Solaris

WATCHERS = vasilche ucko
