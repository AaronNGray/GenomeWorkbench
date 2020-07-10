# $Id: Makefile.test_ncbi_url.app 528153 2017-02-21 14:02:18Z grichenk $

APP = test_ncbi_url
SRC = test_ncbi_url
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =

WATCHERS = grichenk
