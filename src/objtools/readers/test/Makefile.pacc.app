#################################
# $Id: Makefile.pacc.app 386637 2013-01-22 16:35:54Z ucko $
#################################

APP = pacc
SRC = pacc

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik
