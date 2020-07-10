# $Id: Makefile.test_ncbi_linkerd_cxx.app 568530 2018-08-07 13:50:06Z mcelhany $

# Temporarily disable on Windows due to missing devops support.
REQUIRES = -MSWin

APP = test_ncbi_linkerd_cxx
SRC = test_ncbi_linkerd_cxx

LIB = xregexp $(PCRE_LIB) xconnect xncbi
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD =
CHECK_TIMEOUT = 600

WATCHERS = mcelhany
