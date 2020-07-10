# $Id: Makefile.test_ncbi_lbos.app 512629 2016-09-01 18:48:54Z elisovdn $

###  BASIC PROJECT SETTINGS
APP = test_ncbi_lbos
SRC = test_ncbi_lbos
# OBJ = 

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xconnect test_boost xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included
CHECK_REQUIRES = in-house-resources

# Comment out if you do not want it to run automatically as part of
# "make check".
# CHECK_CMD = test_ncbi_lbos lbosdev01.be-md.ncbi.nlm.nih.gov:8080 /CHECK_NAME=test_ncbi_lbos
CHECK_COPY = test_ncbi_lbos.ini
CHECK_TIMEOUT = 600

WATCHERS = elisovdn
