# $Id: Makefile.test_ncbi_ipv6.app 574668 2018-11-16 13:43:21Z lavr $

APP = test_ncbi_ipv6
SRC = test_ncbi_ipv6
LIB = connect

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_ipv6 2607:F220:041E:4000::/52 /CHECK_NAME=test_ncbi_ipv6

WATCHERS = lavr
