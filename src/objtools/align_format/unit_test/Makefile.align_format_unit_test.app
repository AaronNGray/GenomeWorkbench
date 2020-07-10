# $Id: Makefile.align_format_unit_test.app 590760 2019-08-04 21:12:21Z vakatov $

APP = align_format_unit_test
SRC = showdefline_unit_test showalign_unit_test blast_test_util \
vectorscreen_unit_test tabularinof_unit_test aln_printer_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)
CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS = $(FAST_LDFLAGS)

LIB_ = test_boost $(BLAST_DB_DATA_LOADER_LIBS) align_format taxon1 blastdb_format \
        gene_info xalnmgr xcgi xhtml seqmasks_io seqdb blast_services xobjutil \
	$(OBJREAD_LIBS) xnetblastcli xnetblast blastdb scoremat tables $(OBJMGR_LIBS)

LIB = $(LIB_:%=%$(STATIC)) $(LMDB_LIB)
LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD = align_format_unit_test
CHECK_COPY = data align_format_unit_test.ini
CHECK_REQUIRES = in-house-resources

REQUIRES = Boost.Test.Included

WATCHERS = zaretska jianye madden camacho boratyng fongah2
CHECK_TIMEOUT = 900
