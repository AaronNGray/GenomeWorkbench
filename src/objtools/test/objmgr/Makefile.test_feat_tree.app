# $Id: Makefile.test_feat_tree.app 505858 2016-06-29 16:55:21Z elisovdn $

APP = test_feat_tree
SRC = test_feat_tree

CPPFLAGS = $(ORIG_CPPFLAGS)

LIB = xobjutil $(OBJMGR_LIBS)
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = test_feat_tree.sh
CHECK_CMD = test_feat_tree.sh
CHECK_REQUIRES = unix in-house-resources -Cygwin

WATCHERS = vasilche
