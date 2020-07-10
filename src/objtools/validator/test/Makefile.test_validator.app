###############################
# $Id: Makefile.test_validator.app 590760 2019-08-04 21:12:21Z vakatov $
###############################

APP = test_validator
SRC = test_validator
LIB = xvalidate taxon1 $(OBJEDIT_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil valerr \
      tables xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS =  $(GENBANK_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_CMD  = test_validator.sh
CHECK_COPY = current.prt test_validator.sh

REQUIRES = -Cygwin objects

WATCHERS = bollin kans foleyjp asztalos gotvyans
