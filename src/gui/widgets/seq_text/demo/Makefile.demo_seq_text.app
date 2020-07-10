# $Id: Makefile.demo_seq_text.app 44918 2020-04-17 22:29:32Z ucko $

APP = demo_seq_text

PROJ_TAG = demo


SRC = seq_text_demo seq_text_demo_dlg

LIB = w_seq_text w_gl w_wx gui_config gui_objutils $(OBJEDIT_LIBS) \
      gencoll_client genome_collection sqlitewrapp variation \
      gui_opengl gui_print \
      gui_utils$(DLL) xalgoalignutil xalgoseq snputil eutils_client xmlwrapp \
      variation valerr taxon1 entrezgene biotree submit ximage gbproj \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil $(PCRE_LIB) xncbi \
      $(BLAST_LIBS) $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL))

LIBS = $(SQLITE3_LIBS) $(GENBANK_THIRD_PARTY_LIBS) $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) \
       $(GLEW_LIBS) $(OPENGL_LIBS) \
       $(IMAGE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) \
       $(FTGL_LIBS) $(GLEW_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(FTGL_INCLUDE) $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

REQUIRES = objects OpenGL wxWidgets WinMain -LimitedLinker
