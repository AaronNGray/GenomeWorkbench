#################################
# $Id: Makefile.multireader.app 571066 2018-09-19 18:06:07Z foleyjp $
# Author:  Frank Ludwig
#################################

# Build application "multireader"
#################################

APP =  multireader
SRC =  multireader multifile_source multifile_destination
LIB =  xobjreadex $(OBJEDIT_LIBS) $(XFORMAT_LIBS) \
       xalgophytree biotree fastme xalnmgr tables \
       xobjutil xconnect xregexp $(PCRE_LIB) $(SOBJMGR_LIBS) 

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects algo -Cygwin

# proj_tag gbench allows to include multireader into sviwer build tree 
PROJ_TAG = gbench

WATCHERS = ludwigf gotvyans
