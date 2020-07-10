# $Id: Makefile.test_table_reader.app 43500 2019-07-19 01:28:33Z ucko $

APP = test_table_reader

SRC = test_table_reader

LIB_ = gui_core gui_framework gui_opengl w_loaders w_wx w_object_list w_data gui_objects \
       gui_objutils searchbyrsid trackmgr variation gui_utils gui_config \
       snputil gencoll_client sqlitewrapp \
       genome_collection xobjreadex hgvs entrez2cli entrez2 gbproj $(OBJREAD_LIBS) eutils_client xmlwrapp \
       xalgoalignutil $(XFORMAT_LIBS) valerr taxon1 entrezgene biotree \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       $(BLAST_LIBS) $(BLAST_DB_DATA_LOADER_LIBS) $(OBJMGR_LIBS)

LIB = $(LIB_:%=%$(STATIC))

LIBS = $(VDB_LIBS) $(WXWIDGETS_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(DL_LIBS) \
       $(LIBXML_LIBS) $(LIBXSLT_LIBS) $(SQLITE3_LIBS) $(CMPRS_LIBS) \
       $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

CPPFLAGS = $(WXWIDGETS_INCLUDE) $(ORIG_CPPFLAGS)

REQUIRES = objects

PROJ_TAG = gbench
