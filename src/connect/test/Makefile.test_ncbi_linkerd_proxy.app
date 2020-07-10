# $Id: Makefile.test_ncbi_linkerd_proxy.app 570085 2018-09-04 14:52:03Z mcelhany $

# Temporarily disable on Windows due to missing devops support.
REQUIRES = -MSWin

APP = test_ncbi_linkerd_proxy
SRC = test_ncbi_linkerd_proxy

LIB = xregexp $(PCRE_LIB) xconnect xncbi
LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources
CHECK_CMD =
CHECK_TIMEOUT = 600

WATCHERS = mcelhany
