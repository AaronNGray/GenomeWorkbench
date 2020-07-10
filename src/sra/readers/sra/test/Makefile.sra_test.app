#################################
# $Id: Makefile.sra_test.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Eugene Vasilchenko
#################################

# Build application "sra_test"
#################################

APP = sra_test
SRC = sra_test

LIB =   $(SRAREAD_LIBS) $(OBJMGR_LIBS)
LIBS =  $(GENBANK_THIRD_PARTY_LIBS) $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

CHECK_CMD = sra_test
CHECK_CMD = sra_test -sra SRR000001.1
CHECK_CMD = sra_test -sra_all SRR000000.1.2 -no_sra /CHECK_NAME=sra_test_none
CHECK_CMD = sra_test -sra SRR000000.1 -no_sra  /CHECK_NAME=sra_test_none
CHECK_REQUIRES = in-house-resources

WATCHERS = vasilche ucko
