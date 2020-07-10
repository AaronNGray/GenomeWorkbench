# $Id: Makefile.demo_feature_table.app 44918 2020-04-17 22:29:32Z ucko $

APP = demo_feature_table

PROJ_TAG = demo

SRC = app_demo_feat_table demo_feat_table_frame

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = w_feat_table w_wx w_data gui_config gui_objutils \
       gencoll_client genome_collection sqlitewrapp gbproj \
       variation snputil gui_utils xalgoalignutil \
       xalnmgr $(BLAST_LIBS) scoremat tables \
       $(OBJEDIT_LIBS) valerr entrezgene biotree \
       general ximage xregexp $(PCRE_LIB) \
	   eutils_client xmlwrapp xconnect \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       xobjutil $(OBJMGR_LIBS) $(COMPRESS_LIBS) 

LIBS = $(SQLITE3_LIBS) $(GENBANK_THIRD_PARTY_LIBS) \
       $(WXWIDGETS_LIBS) $(IMAGE_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects wxWidgets WinMain -LimitedLinker
