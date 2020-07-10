# $Id: Makefile.gbproject.app 43615 2019-08-09 14:49:33Z vakatov $

APP = gbproject

SRC = gbproject_tool fileloader projectfilewriter

LIB_ = gui_objects gui_objutils xobjwrite variation_utils \
       gencoll_client genome_collection sqlitewrapp variation snputil \
	   gui_utils xalgoalignutil $(XFORMAT_LIBS) xobjread \
       biotree pubmed gbseq xobjsimple snputil \
       entrezgene submit valerr xalnmgr \
       xregexp $(PCRE_LIB) xqueryparse $(BLAST_LIBS) \
	   xhtml eutils_client xmlwrapp gbproj \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   $(OBJMGR_LIBS)

LIB = $(LIB_:%=%$(STATIC))

LIBS = $(GENBANK_THIRD_PARTY_LIBS) \
       $(CMPRS_LIBS) $(NETWORK_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) \
       $(BLAST_THIRD_PARTY_LIBS) $(SQLITE3_LIBS) $(PCRE_LIBS) $(DL_LIBS) \
       $(ORIG_LIBS)

LDFLAGS = $(FAST_LDFLAGS)

REQUIRES = objects algo OpenGL -LimitedLinker
