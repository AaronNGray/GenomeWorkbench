#################################
# $Id: Makefile.agp_count.app 582359 2019-03-14 10:42:44Z kornbluh $
#################################

APP = agp_count
SRC = agp_count

LIB = $(OBJREAD_LIBS) seqset $(SEQ_LIBS) pub medline biblio general \
      xser xutil xncbi

WATCHERS = sapojnik drozdov

