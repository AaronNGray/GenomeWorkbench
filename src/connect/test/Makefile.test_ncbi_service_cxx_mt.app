# $Id: Makefile.test_ncbi_service_cxx_mt.app 549347 2017-10-24 13:36:00Z mcelhany $

APP = test_ncbi_service_cxx_mt
SRC = test_ncbi_service_cxx_mt

LIB = xconnect test_mt xncbi
LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_ncbi_service_cxx_mt.sh
CHECK_COPY = test_ncbi_service_cxx_mt.sh test_ncbi_service_cxx_mt.ini
CHECK_TIMEOUT = 60

WATCHERS = lavr mcelhany
