# $Id: Makefile.test_doi_lookup.app 45092 2020-05-28 18:08:36Z ivanov $

APP = test_doi_lookup
SRC = test_doi_lookup

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = gui_core gui_objects gui_objutils gui_objutils gui_utils xdiscrepancy eutils_client xalgoalignutil xalgoseq xobjwrite variation_utils snputil xvalidate xobjreadex dbsnp_tooltip_service searchbyrsid macro gencoll_client valerr trackmgr gbproj entrezgene biotree ncbi_xcache_netcache xconnserv xqueryparse $(COMPRESS_LIBS) xmlwrapp $(XFORMAT_LIBS) $(SQLITE3_WRAPPER) $(BLAST_LIBS) taxon1 xcgi xthrserv xconnect test_boost xqueryparse xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(LIBXSLT_LIBS) $(SQLITE3_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(PCRE_LIBS) $(LIBXML_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_doi_lookup-1.asn test_doi_lookup-2.asn test_doi_lookup-3.asn test_doi_lookup-4.asn test_doi_lookup-5.asn test_doi_lookup-6.asn
CHECK_TIMEOUT = 3000
CHECK_REQUIRES = Linux

WATCHERS = gotvyans
