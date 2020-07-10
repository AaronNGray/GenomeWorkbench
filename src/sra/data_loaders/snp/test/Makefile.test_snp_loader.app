# $Id: Makefile.test_snp_loader.app 602760 2020-03-02 18:08:49Z vakatov $


APP = test_snp_loader
SRC = test_snp_loader

REQUIRES = Boost.Test.Included $(GRPC_OPT)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = ncbi_xloader_snp $(SRAREAD_LIBS) dbsnp_ptis grpc_integration test_boost $(OBJMGR_LIBS) $(CMPRS_LIB)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(GRPC_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

CHECK_COPY = test_snp_loader.ini
CHECK_CMD = test_snp_loader
CHECK_REQUIRES = in-house-resources

WATCHERS = vasilche
