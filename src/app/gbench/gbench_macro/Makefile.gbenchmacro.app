
APP = gbenchmacro
SRC = gbench_macro


LIB  = gui_objutils xdiscrepancy gui_utils xalgoalignutil xalgoseq xid_mapper xobjwrite variation_utils snputil \
       eutils_client xvalidate macro gencoll_client sqlitewrapp valerr psg_client gbproj entrezgene biotree xmlwrapp \
       $(OBJEDIT_LIBS) $(XFORMAT_LIBS) $(BLAST_LIBS) taxon1 xhtml xcgi xqueryparse \
       ncbi_xcache_netcache xconnserv xthrserv xutil xncbi \
       xregexp $(PCRE_LIB) ncbi_xloader_wgs ncbi_xloader_vdbgraph ncbi_xloader_snp dbsnp_ptis grpc_integration \
       $(CONNEXT) $(SRAREAD_LIBS) xconnect $(OBJMGR_LIBS)


LIBS = $(VDB_STATIC_LIBS) $(GRPC_LIBS) $(LIBXSLT_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(LIBXML_LIBS) $(PSG_CLIENT_LIBS) $(NETWORK_LIBS) \
       $(SQLITE3_LIBS) $(ORIG_LIBS) 


