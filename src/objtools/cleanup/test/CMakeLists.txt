#############################################################################
# $Id: CMakeLists.txt 608332 2020-05-14 16:04:14Z ivanov $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(unit_test_basic_cleanup unit_test_extended_cleanup seq_entry_reassign_ids
  unit_test_capitalization_string test_fix_feature_ids unit_test_cleanup_message
)
