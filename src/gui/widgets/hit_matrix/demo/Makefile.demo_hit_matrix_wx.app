# $Id: Makefile.demo_hit_matrix_wx.app 44918 2020-04-17 22:29:32Z ucko $

APP = demo_hit_matrix_wx

PROJ_TAG = demo

SRC = wx_hit_matrix_demo_app hit_matrix_demo_dlg

LIB  = w_hit_matrix w_data w_gl gui_graph w_wx gui_print gui_opengl gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation snputil \
       gui_utils$(DLL) $(OBJEDIT_LIBS) gbproj ximage $(COMPRESS_LIBS) xregexp \
       xalgoalignutil $(BLAST_LIBS) xalnmgr scoremat tables xobjutil valerr \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   eutils_client xmlwrapp xconnect entrezgene biotree $(OBJMGR_LIBS)

LIBS = $(SQLITE3_LIBS) $(GENBANK_THIRD_PARTY_LIBS) \
       $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) \
       $(GLEW_LIBS) $(OPENGL_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(IMAGE_LIBS) $(PCRE_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(FTGL_LIBS) \
       $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(FTGL_LIBS) $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

REQUIRES = objects OpenGL wxWidgets WinMain -LimitedLinker
