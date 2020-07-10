# $Id: Makefile.demo_wx_tree.app 41919 2018-11-13 23:35:47Z rudnev $

APP = demo_wx_tree

PROJ_TAG = demo


SRC = demo_wx_tree tree_demo_dlg

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = w_wx gui_objutils \
       gencoll_client genome_collection variation \
       snputil gui_utils xobjutil gbproj tables \
       eutils_client xmlwrapp xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(OBJMGR_LIBS)

LIBS = $(WXWIDGETS_LIBS) $(IMAGE_LIBS) $(CMPRS_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = WinMain


