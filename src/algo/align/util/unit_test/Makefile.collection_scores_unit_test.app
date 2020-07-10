APP = collection_scores_unit_test
SRC = collection_scores_unit_test

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalgoalignutil xalgoseq \
          $(BLAST_FORMATTER_MINIMAL_LIBS) \
	  xqueryparse tables xregexp $(PCRE_LIB) $(BLAST_LIBS) \
	  test_boost $(OBJMGR_LIBS)

LIBS = $(NETWORK_LIBS) $(PCRE_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included objects

CHECK_CMD = collection_scores_unit_test -input-binary -seq-entry data/collection_score_entries.asnb -aligns data/collection_score_blast.asnb
CHECK_COPY = data

WATCHERS = kotliaro
