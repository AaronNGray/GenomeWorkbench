# $Id: Makefile.demo_align_multiple_wx.app 44653 2020-02-14 17:32:47Z ivanov $

APP = demo_align_multiple_wx

PROJ_TAG = demo

SRC = align_multiple_demo_app align_multiple_demo_dlg

LIB = w_aln_multi w_seq_graphic splines w_seq w_gl w_data w_aln_score w_wx \
      w_loaders gui_objects gui_objutils \
      $(ncbi_xloader_bam) $(ncbi_xloader_csra) $(bamread) $(sraread) \
      gencoll_client genome_collection sqlitewrapp xobjreadex variation \
      gui_graph gui_opengl \
      gui_print gui_utils$(DLL) gbproj xalgoalignutil xalgoseq \
      snputil eutils_client xmlwrapp $(OBJEDIT_LIBS) \
      dbsnp_tooltip_service searchbyrsid trackmgr \
      variation valerr taxon1 entrezgene biotree submit genesbyloc \
      $(BLAST_LIBS) xqueryparse ximage xregexp $(PCRE_LIB) xhtml \
      ncbi_xcache_netcache xconnserv xthrserv \
      $(OBJMGR_LIBS:ncbi_%=ncbi_%$(DLL))

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(OPENGL_LIBS) $(VDB_LIBS) \
	   $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(IMAGE_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
	   $(FTGL_LIBS) $(BLAST_THIRD_PARTY_LIBS) \
           $(SQLITE3_LIBS) $(GENBANK_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

CPPFLAGS = $(FTGL_INCLUDE) $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

REQUIRES = objects OpenGL wxWidgets WinMain -LimitedLinker
