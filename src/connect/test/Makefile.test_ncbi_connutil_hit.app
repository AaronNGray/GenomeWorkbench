# $Id: Makefile.test_ncbi_connutil_hit.app 530905 2017-03-19 01:19:36Z lavr $

APP = test_ncbi_connutil_hit
SRC = test_ncbi_connutil_hit
LIB = connssl connect $(NCBIATOMIC_LIB)

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_connutil_hit www.ncbi.nlm.nih.gov 443 /Service/bounce.cgi dummy test_ncbi_connutil_hit.dat /CHECK_NAME=test_ncbi_connutil_hit
CHECK_COPY = test_ncbi_connutil_hit.dat

WATCHERS = lavr
