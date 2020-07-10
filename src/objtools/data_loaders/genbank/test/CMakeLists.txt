#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(
  test_reader_id1 test_reader_pubseq test_reader_gicache
  test_objmgr_gbloader test_objmgr_gbloader_mt
  test_bulkinfo test_bulkinfo_mt
)
