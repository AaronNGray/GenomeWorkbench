#################################
# $Id: Makefile.test_annot_ci.app 590760 2019-08-04 21:12:21Z vakatov $
# Author:  Aleksey Grichenko (grichenk@ncbi.nlm.nih.gov)
#################################

# Build seq-annot iterators test application "test_annot_ci"
#################################


APP = test_annot_ci
SRC = test_annot_ci
LIB = $(OBJMGR_LIBS)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_CMD  = test_annot_ci
CHECK_COPY = test_annot_ci.ini test_annot_entries.asn test_annot_res.asn

WATCHERS = grichenk
