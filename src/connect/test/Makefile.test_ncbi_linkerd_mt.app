# $Id: Makefile.test_ncbi_linkerd_mt.app 567726 2018-07-23 19:12:39Z mcelhany $

# Temporarily disable on Windows due to missing devops support.
REQUIRES = -MSWin

APP = test_ncbi_linkerd_mt
SRC = test_ncbi_linkerd_mt

LIB = test_mt xconnect xregexp $(PCRE_LIB) xncbi
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_ncbi_linkerd_mt.sh
CHECK_COPY = test_ncbi_linkerd_mt.sh test_ncbi_linkerd_mt.ini
CHECK_TIMEOUT = 600

WATCHERS = lavr mcelhany
