# $Id: Makefile.ini2reg.app 41919 2018-11-13 23:35:47Z rudnev $

APP = ini2reg

SRC = ini2reg

LIB  = gui_objutils variation snputil gui_utils \
       submit xalnmgr xobjutil gbproj tables xregexp $(PCRE_LIB) \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
       eutils_client xmlwrapp xconnect $(SOBJMGR_LIBS) $(COMPRESS_LIBS) 

LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) \
	   $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = algo OpenGL
