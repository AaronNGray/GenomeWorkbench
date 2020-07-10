# $Id: Makefile.demo_seqgraphic_wx.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_seqgraphic_wx

PROJ_TAG = demo


SRC = wx_seqgraphic_demo_app seqgraphic_demo_dlg

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB =  w_seq_graphic w_seq w_data w_gl w_wx \
       gui_graph gui_config gui_opengl gui_print gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation gui_utils \
       ximage $(XFORMAT_LIBS) xobjutil xregexp \
       splines xmlwrapp eutils_client xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   submit gbseq xalnmgr tables valerr $(OBJMGR_LIBS) 

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(IMAGE_LIBS) \
       $(VDB_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(SQLITE3_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects OpenGL wxWidgets WinMain

