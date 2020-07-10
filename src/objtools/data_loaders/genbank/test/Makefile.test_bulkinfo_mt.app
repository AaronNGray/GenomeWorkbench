#################################
# $Id: Makefile.test_bulkinfo_mt.app 590760 2019-08-04 21:12:21Z vakatov $
#################################

APP = test_bulkinfo_mt
SRC = test_bulkinfo_mt bulkinfo_tester
LIB = xobjutil test_mt ncbi_xdbapi_ftds $(OBJMGR_LIBS) $(FTDS_LIB)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = bad_len.ids wgs.ids all_readers.sh

CHECK_CMD = all_readers.sh test_bulkinfo_mt -type gi -reference ref/0.gi.txt /CHECK_NAME=test_bulkinfo_mt_gi
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type acc -reference ref/0.acc.txt /CHECK_NAME=test_bulkinfo_mt_acc
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type label -reference ref/0.label.txt /CHECK_NAME=test_bulkinfo_mt_label
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type taxid -reference ref/0.taxid.txt /CHECK_NAME=test_bulkinfo_mt_taxid
CHECK_CMD = all_readers.sh -id2 test_bulkinfo_mt -type length -threads 8 -reference ref/0.length.txt /CHECK_NAME=test_bulkinfo_mt_length
CHECK_CMD = all_readers.sh -id2 test_bulkinfo_mt -type type -threads 8 -reference ref/0.type.txt /CHECK_NAME=test_bulkinfo_mt_type
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type state -reference ref/0.state.txt /CHECK_NAME=test_bulkinfo_mt_state
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type hash -no-recalc -reference ref/0.hash.txt /CHECK_NAME=test_bulkinfo_mt_hash

CHECK_CMD = all_readers.sh test_bulkinfo_mt -type gi -idlist wgs.ids -reference ref/wgs.gi.txt /CHECK_NAME=test_bulkinfo_mt_wgs_gi
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type acc -idlist wgs.ids -reference ref/wgs.acc.txt /CHECK_NAME=test_bulkinfo_mt_wgs_acc
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type label -idlist wgs.ids -reference ref/wgs.label.txt /CHECK_NAME=test_bulkinfo_mt_wgs_label
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type taxid -idlist wgs.ids -reference ref/wgs.taxid.txt /CHECK_NAME=test_bulkinfo_mt_wgs_taxid
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type length -idlist wgs.ids -reference ref/wgs.length.txt /CHECK_NAME=test_bulkinfo_mt_wgs_length
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type type -idlist wgs.ids -reference ref/wgs.type.txt /CHECK_NAME=test_bulkinfo_mt_wgs_types
#CHECK_CMD = all_readers.sh -xid2 test_bulkinfo_mt -type state -idlist wgs.ids -reference ref/wgs.state.txt /CHECK_NAME=test_bulkinfo_mt_wgs_state
CHECK_CMD = all_readers.sh -id2 test_bulkinfo_mt -type state -idlist wgs.ids /CHECK_NAME=test_bulkinfo_mt_wgs_state_id2
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type hash -idlist wgs.ids -no-recalc -reference ref/wgs.hash.txt /CHECK_NAME=test_bulkinfo_mt_wgs_hash

CHECK_CMD = all_readers.sh test_bulkinfo_mt -type gi -idlist bad_len.ids -reference ref/bad_len.gi.txt /CHECK_NAME=test_bulkinfo_mt_bad_gi
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type acc -idlist bad_len.ids -reference ref/bad_len.acc.txt /CHECK_NAME=test_bulkinfo_mt_bad_acc
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type label -idlist bad_len.ids -reference ref/bad_len.label.txt /CHECK_NAME=test_bulkinfo_mt_bad_label
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type taxid -idlist bad_len.ids -reference ref/bad_len.taxid.txt /CHECK_NAME=test_bulkinfo_mt_bad_taxid
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type length -idlist bad_len.ids -reference ref/bad_len.length.txt /CHECK_NAME=test_bulkinfo_mt_bad_length
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type type -idlist bad_len.ids -reference ref/bad_len.type.txt /CHECK_NAME=test_bulkinfo_mt_bad_types
CHECK_CMD = all_readers.sh -xid2 test_bulkinfo_mt -type state -idlist bad_len.ids -reference ref/bad_len.state.txt /CHECK_NAME=test_bulkinfo_mt_bad_state
CHECK_CMD = all_readers.sh -id2 test_bulkinfo_mt -type state -idlist bad_len.ids /CHECK_NAME=test_bulkinfo_mt_bad_state_id2
CHECK_CMD = all_readers.sh test_bulkinfo_mt -type hash -idlist bad_len.ids -reference ref/bad_len.hash.txt /CHECK_NAME=test_bulkinfo_mt_bad_hash

CHECK_TIMEOUT = 400

WATCHERS = vasilche
