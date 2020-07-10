# $Id: Makefile.test_ncbi_lbos_mt.app 512629 2016-09-01 18:48:54Z elisovdn $

###  BASIC PROJECT SETTINGS
APP = test_ncbi_lbos_mt
SRC = test_ncbi_lbos_mt
# OBJ =

CPPFLAGS =  -DLBOS_TEST_MT $(ORIG_CPPFLAGS)

LIB = test_mt xconnect xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = in-house-resources

# Comment out if you do not want it to run automatically as part of
# "make check".
# CHECK_CMD = test_ncbi_lbos_mt  -repeats=2 -threads=10 lbosdev01.be-md.ncbi.nlm.nih.gov:8080   /CHECK_NAME=test_ncbi_lbos_mt
CHECK_COPY = test_ncbi_lbos_mt.ini
CHECK_TIMEOUT = 600

WATCHERS = elisovdn
