#################################
# $Id: Makefile.agp_val_test.app 582359 2019-03-14 10:42:44Z kornbluh $
#################################

REQUIRES = objects

APP = agp_val_test
SRC = agp_val_test

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik drozdov

