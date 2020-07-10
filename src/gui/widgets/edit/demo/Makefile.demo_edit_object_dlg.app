# $Id: Makefile.demo_edit_object_dlg.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_edit_object_dlg

PROJ_TAG = demo

SRC = demo_edit_object_app select_feature_dlg

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = w_edit w_wx gui_objutils gui_utils w_loaders w_wx \
       gencoll_client genome_collection sqlitewrapp variation snputil \
       general gbproj taxon1 taxon3 ximage $(COMPRESS_LIBS) xregexp \
       xalgoalignutil $(BLAST_LIBS) pubmed valid xformat xalnmgr \
       scoremat tables xobjutil gbseq submit eutils_client xmlwrapp \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xconnect entrezgene biotree valerr $(OBJMGR_LIBS) 

LIBS = $(WXWIDGETS_LIBS) $(IMAGE_LIBS) $(VDB_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(SQLITE3_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects wxWidgets WinMain -LimitedLinker
