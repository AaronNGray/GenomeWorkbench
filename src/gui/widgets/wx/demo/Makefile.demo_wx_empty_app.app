# $Id: Makefile.demo_wx_empty_app.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_wx_empty_app

PROJ_TAG = demo


SRC = demo_wx_empty_app hello_world_dlg

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(GLEW_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = w_wx gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation \
       snputil gui_utils xobjutil gbproj tables \
	   eutils_client xmlwrapp xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(SOBJMGR_LIBS)

LIBS = $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(IMAGE_LIBS) $(CMPRS_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(SQLITE3_LIBS) $(ORIG_LIBS)

REQUIRES = WinMain


