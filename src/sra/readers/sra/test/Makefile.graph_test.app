#################################
# $Id: Makefile.graph_test.app 469130 2015-06-01 18:44:14Z ucko $
# Author:  Eugene Vasilchenko
#################################

# Build application "graph_test"
#################################

APP = graph_test
SRC = graph_test

LIB =   $(SRAREAD_LIBS) $(SOBJMGR_LIBS) $(CMPRS_LIB)
LIBS =  $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

CHECK_REQUIRES = in-house-resources -MSWin -Solaris
CHECK_CMD = graph_test -q .

WATCHERS = vasilche ucko
