# $Id: Makefile.demo_seq_desktop.app 41919 2018-11-13 23:35:47Z rudnev $

APP = demo_seq_desktop

PROJ_TAG = demo


SRC = seq_tree_demo_app seq_tree_demo_dlg

CPPFLAGS = $(GLEW_INCLUDE) $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB = w_seq_desktop w_data w_gl w_wx gui_objutils variation gui_opengl gui_print \
      gui_utils$(DLL) xalgoalignutil xalgoseq snputil eutils_client xmlwrapp \
      $(XFORMAT_LIBS) valerr taxon1 entrezgene biotree gbseq gbproj \
      submit $(BLAST_LIBS) xqueryparse ximage xregexp $(PCRE_LIB) \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      dbapi_driver$(DLL) $(OBJMGR_LIBS:ncbi_%=ncbi_%$(DLL))

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(OPENGL_LIBS) $(FTGL_LIBS) $(IMAGE_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects wxWidgets WinMain

