#################################
# $Id: Makefile.test_rangemap.app 173093 2009-10-14 16:24:46Z vakatov $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build serialization test application "serialtest"
#################################

APP = test_rangemap
SRC = test_rangemap
LIB = xutil xncbi

CHECK_CMD = test_rangemap

WATCHERS = vasilche
