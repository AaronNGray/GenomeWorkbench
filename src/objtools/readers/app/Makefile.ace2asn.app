#################################
# $Id: Makefile.ace2asn.app 398472 2013-05-06 19:02:03Z kornbluh $
#################################

APP = ace2asn
SRC = ace2asn

LIB = $(OBJREAD_LIBS) xobjutil $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = grichenk
