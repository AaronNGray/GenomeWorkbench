#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(
  unit_test_autodef unit_test_entry_edit unit_test_field_handler
  unit_test_cds_fix unit_test_loc_edit unit_test_mail_report
  unit_test_source_edit unit_test_rna_edit unit_test_parse_text
  unit_test_pub_edit unit_test_gap_trim unit_test_feature_propagate
  unit_test_seq_edit
)
