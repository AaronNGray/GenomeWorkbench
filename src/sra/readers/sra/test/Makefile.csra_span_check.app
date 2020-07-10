#################################
# $Id: Makefile.csra_span_check.app 593430 2019-09-18 17:27:33Z vasilche $
# Author:  Eugene Vasilchenko
#################################

# Build application "csra_span_check"
#################################

APP = csra_span_check
SRC = csra_span_check

LIB =   $(SRAREAD_LIBS) $(SOBJMGR_LIBS) $(CMPRS_LIB)
LIBS =  $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

WATCHERS = vasilche
