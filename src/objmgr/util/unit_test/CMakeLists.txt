#############################################################################
# $Id: CMakeLists.txt 599929 2020-01-09 16:28:21Z foleyjp $
#############################################################################


NCBI_project_tags(test)
NCBI_add_app(
  unit_test_seq_translator unit_test_fasta_ostream
  unit_test_mol_wt unit_test_seq_loc_util unit_test_defline
  unit_test_bioseqgaps_ci unit_test_obj_sniff unit_test_get_label 
)

