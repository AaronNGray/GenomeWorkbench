# $Id: Makefile.demo_taxtree.app 43500 2019-07-19 01:28:33Z ucko $

# Graphic Sequence Widget lib

SRC = test_taxtree taxtree_testdlg

APP = demo_taxtree

PROJ_TAG = demo


LIB = w_taxtree w_data w_wx gui_opengl gui_objutils \
      gencoll_client genome_collection sqlitewrapp variation gui_utils \
      ximage xalnmgr xobjutil \
      taxon1 valerr tables gbseq submit \
      ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      eutils_client xmlwrapp xconnect $(COMPRESS_LIBS) $(OBJMGR_LIBS)

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIBS = $(IMAGE_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects OpenGL wxWidgets WinMain

