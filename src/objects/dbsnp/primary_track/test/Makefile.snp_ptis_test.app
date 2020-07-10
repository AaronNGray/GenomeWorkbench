# $Id: Makefile.snp_ptis_test.app 602843 2020-03-03 17:47:26Z vakatov $
# Author:  Eugene Vasilchenko


APP = snp_ptis_test
SRC = snp_ptis_test

REQUIRES = $(GRPC_OPT)

LIB = dbsnp_ptis grpc_integration $(SEQ_LIBS) pub medline biblio general \
      xser $(XCONNEXT) xconnect xutil xncbi
LIBS = $(GRPC_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(GRPC_INCLUDE)

CHECK_CMD = snp_ptis_test
CHECK_REQUIRES = in-house-resources GRPC

WATCHERS = vasilche
