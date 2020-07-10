# $Id: Makefile.gbench_new.app 43918 2019-09-18 20:36:22Z katargir $

ASN_DEP = seq seqset pubmed

APP = gbench-bin

SRC = gui gbench_app app_extensions app_services browser_config commands \
      network_options_page \
      aboutdlg init \
      read_pipe_thread wakeup_thread \
      open_files_task \
      net_conn_problem_dlg testing_ncbi_conn_dlg \
      new_version_dlg view_options_page \
      app_updater_task

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(WXWIDGETS_INCLUDE) $(SRA_INCLUDE)

LIB  = gui_core gui_framework gui_objects w_object_list w_aln_multi w_seq_graphic \
       w_aln_score w_macro_edit w_gl w_wx w_data w_feedback w_loaders\
       gui_opengl snputil gui_objutils variation gui_utils \
       gbproj xcleanup xalnmgr xalgoalignutil tables pubmed xobjutil \
       entrezgene valerr eutils_client xmlwrapp \
       ximage xregexp xconnserv xconnect connssl \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       dbapi_driver ncbi_xcache_sqlite3 \
       $(BLAST_LIBS) $(PCRE_LIB) $(COMPRESS_LIBS) $(XCONNEXT) $(SRAREAD_LIBS) $(OBJMGR_LIBS)

LIBS = $(SRA_SDK_SYSLIBS) $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(SQLITE3_LIBS) \
       $(IMAGE_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = gbench wxWidgets OpenGL WinMain
