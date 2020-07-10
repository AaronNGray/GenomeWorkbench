# $Id: Makefile.demo_flatfile_widget.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_flatfile_widget

PROJ_TAG = demo


SRC = flatfile_demo_app flat_file_demo_dlg

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB = w_seq w_text_widget w_data w_wx gui_objutils gui_opengl gui_print \
      gencoll_client genome_collection sqlitewrapp variation \
      gui_utils$(DLL) xalgoalignutil xalgoseq snputil eutils_client xmlwrapp \
      $(XFORMAT_LIBS) valerr taxon1 entrezgene biotree gbseq gbproj \
      submit $(BLAST_LIBS) xqueryparse ximage xregexp $(PCRE_LIB) \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
      dbapi_driver$(DLL) $(OBJMGR_LIBS:ncbi_%=ncbi_%$(DLL))

LIBS = $(WXWIDGETS_LIBS) $(OPENGL_LIBS) $(IMAGE_LIBS) $(SQLITE3_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects wxWidgets WinMain

