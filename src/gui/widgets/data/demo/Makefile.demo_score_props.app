# $Id: Makefile.demo_score_props.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_score_props

PROJ_TAG = demo


SRC = demo_score_props_app score_methods_dlg

CPPFLAGS = $(ORIG_CPPFLAGS) $(WXWIDGETS_INCLUDE)

LIB  = w_data gui_graph gui_opengl w_wx gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation snputil \
       gui_utils ximage aln_multiple \
       xalnmgr scoremat tables xobjutil valerr \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       eutils_client xmlwrapp xconnect $(SOBJMGR_LIBS) $(COMPRESS_LIBS) 

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(IMAGE_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(LIBXML_LIBS)

REQUIRES = objects OpenGL wxWidgets WinMain


