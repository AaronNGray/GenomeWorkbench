#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_add_app(
  test_objmgr_data test_objmgr_data_mt
  test_objmgr_mem test_seqvector_ci seqvec_bench test_annot_ci
  test_edit_saver id_unit_test id_unit_test_bad
  test_feat_tree feat_unit_test test_seq_entry_ci
)
NCBI_add_subdirectory(util)

