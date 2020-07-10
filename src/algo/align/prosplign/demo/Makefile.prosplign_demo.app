#################################
# $Id: Makefile.prosplign_demo.app 415609 2013-10-18 18:00:06Z chetvern $
#################################

WATCHERS = chetvern

APP = prosplign_demo
SRC = prosplign_demo

LIB = prosplign xalgoalignutil xalnmgr xobjutil \
      tables \
      $(OBJMGR_LIBS:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) \
       $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)


REQUIRES = objects
