# $Id: Makefile.unit_test_assembly_info.app 44941 2020-04-22 18:54:59Z vakatov $

APP = unit_test_assembly_info

SRC = unit_test_assembly_info

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = gui_objects \
      ncbi_xcache_netcache xconnserv xthrserv \
      $(OBJREAD_LIBS) $(XFORMAT_LIBS) \
      xalnmgr xobjutil tables test_boost xobjsimple xid_mapper \
      ncbi_xloader_csra ncbi_xloader_snp ncbi_xloader_vdbgraph \
      dbsnp_ptis grpc_integration $(CONNEXT) connect \
      ncbi_xloader_wgs $(SRAREAD_LIBS) ncbi_xreader_pubseqos2 \
      gui_objutils gui_utils xdiscrepancy xalgoalignutil xalgoseq \
      xobjwrite variation_utils snputil xvalidate xobjreadex \
      macro valerr gbproj entrezgene biotree \
      xqueryparse eutils_client xmlwrapp $(OBJEDIT_LIBS) $(BLAST_LIBS) \
      gencoll_client sqlitewrapp $(COMPRESS_LIBS) taxon1 xcgi xregexp \
      $(PCRE_LIB) $(DBAPI_CTLIB) $(OBJMGR_LIBS)

LIBS = $(VDB_STATIC_LIBS) $(GRPC_LIBS) $(CMPRS_LIBS) $(LIBXSLT_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(PCRE_LIBS) \
       $(LIBXML_LIBS) $(SQLITE3_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) \
       $(SYBASE_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD  = unit_test_assembly_info
CHECK_TIMEOUT = 1800

WATCHERS = shkeda gencoll-dev
