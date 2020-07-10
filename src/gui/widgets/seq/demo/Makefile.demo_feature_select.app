# $Id: Makefile.demo_feature_select.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_feature_select

PROJ_TAG = demo

SRC = featureselect_demo_app

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = w_seq w_data w_wx gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation \
       snputil gui_utils ximage xregexp $(PCRE_LIB) \
       $(XFORMAT_LIBS) xalnmgr scoremat tables gbseq submit \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   xobjutil eutils_client xmlwrapp xconnect $(SOBJMGR_LIBS) $(COMPRESS_LIBS) 

LIBS = $(WXWIDGETS_LIBS) $(IMAGE_LIBS) $(BLAST_THIRD_PARTY_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(SQLITE3_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects wxWidgets WinMain

