#################################
# $Id: Makefile.test_ncbi_fast.app 549297 2017-10-23 18:33:41Z gouriano $
#################################


APP = test_ncbi_fast
SRC = test_ncbi_fast
LIB = test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = gouriano vasilche
