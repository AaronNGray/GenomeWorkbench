# $Id: Makefile.phylo_tree_demo_wx.app 43836 2019-09-09 20:29:51Z evgeniev $

ASN_DEP = biotree

APP = demo_phylo_tree_wx

PROJ_TAG = demo

SRC = wx_phylo_tree_demo_app phylo_tree_demo_dlg

LIB =  w_phylo_tree xalgophytree fastme w_gl w_data w_wx gui_graph gui_opengl \
       gui_print gui_config gui_objutils gui_utils gbproj ximage \
       gencoll_client genome_collection sqlitewrapp xalgoalignutil xalgoseq \
       snputil eutils_client xmlwrapp xvalidate valid valerr \
       gbproj taxon3 taxon1 entrezgene biotree $(XFORMAT_LIBS) \
       $(BLAST_LIBS) xconnect xqueryparse \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xregexp tables $(COMPRESS_LIBS) $(PCRE_LIB) $(SOBJMGR_LIBS)


LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(FTGL_LIBS) $(OPENGL_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(IMAGE_LIBS) $(CMPRS_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(PCRE_LIBS) $(DL_LIBS) \
       $(SQLITE3_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects OpenGL wxWidgets WinMain

