#################################
# $Id: Makefile.coretest.app 373163 2012-08-27 13:52:21Z gouriano $
# Author:  Denis Vakatov (vakatov@ncbi.nlm.nih.gov)
#################################

# Build test CoreLib application "coretest"
#################################

APP = coretest
SRC = coretest
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = vakatov
