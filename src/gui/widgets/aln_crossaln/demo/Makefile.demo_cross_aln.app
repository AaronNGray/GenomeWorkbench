# $Id: Makefile.demo_cross_aln.app 43615 2019-08-09 14:49:33Z vakatov $

APP = demo_crossaln_wx

PROJ_TAG = demo

SRC = wx_cross_aln_demo_app cross_aln_demo_dlg

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB = w_aln_crossaln w_hit_matrix w_data w_gl w_wx gui_graph gui_opengl \
      gui_print gui_objutils $(OBJEDIT_LIBS) gencoll_client genome_collection sqlitewrapp \
      variation snputil gui_utils gbproj xalgoalignutil xalnmgr valerr \
      entrezgene biotree submit $(BLAST_LIBS) \
      xqueryparse ximage $(COMPRESS_LIBS) xregexp $(PCRE_LIB) \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      eutils_client xmlwrapp $(OBJMGR_LIBS)


LIBS = $(GENBANK_THIRD_PARTY_LIBS) \
       $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(OPENGL_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(IMAGE_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(FTGL_LIBS) $(DL_LIBS) \
       $(SQLITE3_LIBS) $(ORIG_LIBS)


#POST_LINK = $(WX_POST_LINK)

REQUIRES = objects OpenGL wxWidgets WinMain
