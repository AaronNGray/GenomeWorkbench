#################################
# $Id: Makefile.test_source_mod_parser.app 501626 2016-05-17 17:32:10Z kornbluh $
#################################

APP = test_source_mod_parser
SRC = test_source_mod_parser

LIB = xobjreadex $(OBJREAD_LIBS) xobjutil $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = ucko gotvyans
