# $Id: Makefile.demo_align_table.app 43500 2019-07-19 01:28:33Z ucko $

APP = demo_align_table


PROJ_TAG = demo

SRC = app_demo_align_table demo_align_table_frame


CPPFLAGS = $(ORIG_CPPFLAGS) $(WXWIDGETS_INCLUDE)

LIB  = w_aln_table w_aln_multi w_data w_wx \
       gui_objutils \
       gencoll_client genome_collection sqlitewrapp variation gui_utils \
	   xalgoalignutil xnetblast xblast xalnmgr \
	   general ximage xregexp $(PCRE_LIB) \
       scoremat tables xobjutil valerr \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   eutils_client xmlwrapp xconnect $(OBJMGR_LIBS) $(COMPRESS_LIBS)

LIBS = $(WXWIDGETS_LIBS) $(IMAGE_LIBS) $(VDB_LIBS) \
       $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(SQLITE3_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects wxWidgets WinMain


