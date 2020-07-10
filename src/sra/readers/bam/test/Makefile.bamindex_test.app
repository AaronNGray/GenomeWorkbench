#################################
# $Id: Makefile.bamindex_test.app 536810 2017-05-23 16:31:56Z vasilche $
# Author:  Eugene Vasilchenko
#################################

# Build application "bamindex_test"
#################################

APP = bamindex_test
SRC = bamindex_test bam_test_common

LIB =   bamread $(BAM_LIBS) xobjreadex $(OBJREAD_LIBS) xobjutil xobjsimple \
        $(OBJMGR_LIBS)
LIBS =  $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

WATCHERS = vasilche
