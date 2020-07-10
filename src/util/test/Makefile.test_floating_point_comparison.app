# $Id: Makefile.test_floating_point_comparison.app 173093 2009-10-14 16:24:46Z vakatov $
# Author:  Sergey Satskiy (satskyse@ncbi.nlm.nih.gov)

# Build floating point comparison test application "test_floating_point_comparison"
#################################

APP = test_floating_point_comparison
SRC = test_floating_point_comparison
LIB = xutil test_boost xncbi
REQUIRES = Boost.Test.Included

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

CHECK_CMD =

WATCHERS = satskyse
