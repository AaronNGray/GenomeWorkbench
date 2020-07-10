###############################
# $Id: Makefile.test_basic_cleanup.app 586722 2019-05-22 01:44:11Z ucko $
###############################

APP = test_basic_cleanup
SRC = test_basic_cleanup
LIB = xcleanup $(OBJEDIT_LIBS) xobjutil valid xconnect xregexp $(PCRE_LIB) \
      $(SOBJMGR_LIBS)

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = bollin kans
