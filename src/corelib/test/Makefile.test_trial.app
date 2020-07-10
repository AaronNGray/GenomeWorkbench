#################################
# $Id: Makefile.test_trial.app 488963 2016-01-07 00:44:21Z fukanchi $
# Author:  Eugene Vasilchenko
#################################

APP = test_trial
SRC = test_trial
LIB = xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = vasilche
