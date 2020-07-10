# $Id: Makefile.demo_wx_app.app 41919 2018-11-13 23:35:47Z rudnev $

APP = demo_wx_app
PROJ_TAG = demo

SRC = wx_main_frame about_dlg wx_ncbi_app_demo test_table test_client

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB  = w_wx w_seq gui_opengl gui_objutils \
       gencoll_client genome_collection variation gui_utils xobjutil \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       ximage $(COMPRESS_LIBS) xregexp eutils_client xmlwrapp xconnect $(SOBJMGR_LIBS)

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS)  $(GLEW_LIBS) $(IMAGE_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects OpenGL wxWidgets WinMain

