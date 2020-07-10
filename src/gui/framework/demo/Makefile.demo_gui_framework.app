# $Id: Makefile.demo_gui_framework.app 44651 2020-02-14 16:53:07Z ucko $

APP = demo_gui_framework

PROJ_TAG = demo

SRC = gui framework_app test_views

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(OPENGL_INCLUDE) $(WXWIDGETS_INCLUDE)

LIB  = gui_framework w_feedback w_gl w_wx \
       gui_objects gui_opengl gui_objutils gui_utils \
       eutils_client xmlwrapp snputil xalgoalignutil $(BLAST_LIBS) \
       gencoll_client genome_collection variation \
       gbproj entrezgene valerr biotree \
       ncbi_xcache_netcache xconnserv xthrserv sqlitewrapp \
       xconnect ximage xregexp $(PCRE_LIB) $(COMPRESS_LIBS) $(SOBJMGR_LIBS)

LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(FTGL_LIBS) $(OPENGL_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(IMAGE_LIBS) $(BLAST_THIRD_PARTY_LIBS) \
       $(PCRE_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(ORIG_LIBS)

REQUIRES = objects OpenGL wxWidgets WinMain

WATCHERS = dicuccio
