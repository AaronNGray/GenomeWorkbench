#############################################################################
# $Id: CMakeLists.txt 593585 2019-09-20 13:57:47Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(
  unit_test_gene_model unit_test_polya unit_test_gap_analysis
  unit_test_internal_stops unit_test_align_cleanup unit_test_orf
)
