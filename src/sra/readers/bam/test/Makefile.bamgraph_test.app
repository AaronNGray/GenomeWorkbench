#################################
# $Id: Makefile.bamgraph_test.app 469130 2015-06-01 18:44:14Z ucko $
# Author:  Eugene Vasilchenko
#################################

# Build application "sra_test"
#################################

APP = bamgraph_test
SRC = bamgraph_test

LIB =   bamread $(BAM_LIBS) xobjreadex $(OBJREAD_LIBS) xobjutil xobjsimple \
        $(OBJMGR_LIBS)
LIBS =  $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

WATCHERS = vasilche ucko
