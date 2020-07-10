#################################
# $Id: Makefile.test_histogram_binning.app 571790 2018-10-02 18:14:56Z kornbluh $

APP = test_histogram_binning
SRC = test_histogram_binning
LIB = xutil test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = kachalos

