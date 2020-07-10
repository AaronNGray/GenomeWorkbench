# $Id: Makefile.demo_widgets_gl_wx.app 44653 2020-02-14 17:32:47Z ivanov $

APP = demo_widgets_gl_wx

PROJ_TAG = demo

SRC = demo_widgets_gl_wx gl_demo_dlg ruler_demo_panel font_demo_panel \
      font_window gl3d_demo_panel gl3d_window glinfo_demo_panel \
      glpanewidget_demo_panel glpanewidget_child_demo glpanewidget_demo

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE) $(CMPRS_INCLUDE)
CXXFLAGS = $(ORIG_CXXFLAGS) $(OBJCXX_CXXFLAGS)

LIB  = w_gl w_feedback w_wx snputil gui_objutils gui_opengl \
       gencoll_client genome_collection sqlitewrapp variation gui_utils \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xobjutil eutils_client xmlwrapp ximage xconnect xcompress $(COMPRESS_LIBS) $(PCRE_LIB) $(SOBJMGR_LIBS)

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(FTGL_LIBS)  $(GLEW_LIBS) $(OPENGL_LIBS) \
       $(IMAGE_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(SQLITE3_LIBS) \
       $(NETWORK_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(ORIG_LIBS)

REQUIRES = WinMain
