# $Id: Makefile.test_ncbi_monkey.app 502584 2016-05-25 18:43:45Z elisovdn $

###  BASIC PROJECT SETTINGS
APP = test_ncbi_monkey
SRC = test_ncbi_monkey
# OBJ = 

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xconnect test_boost xncbi

LIBS = $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included
CHECK_REQUIRES = in-house-resources

# Comment out if you do not want it to run automatically as part of
# "make check".
CHECK_CMD =
CHECK_COPY = test_ncbi_monkey.ini

WATCHERS = elisovdn