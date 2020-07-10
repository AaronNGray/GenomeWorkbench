#################################
# $Id: Makefile.seqvec_bench.app 590760 2019-08-04 21:12:21Z vakatov $
#################################


APP = seqvec_bench
SRC = seqvec_bench
LIB = test_mt $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = dicuccio
