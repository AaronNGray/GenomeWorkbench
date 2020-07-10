#############################################################################
# $Id: CMakeLists.txt 593493 2019-09-19 15:37:14Z gouriano $
#############################################################################


NCBI_project_tags(test)
NCBI_add_app(
  test_objmgr_basic
  test_objmgr
  test_objmgr_mt
  test_objmgr_sv
  test_seqmap_switch
  unit_test_objmgr
)
