#############################################################################
# $Id: CMakeLists.txt 593499 2019-09-19 15:56:50Z gouriano $
#############################################################################

NCBI_project_tags(test grid)
NCBI_add_app(
  test_nsstorage test_ic_client test_netcache_api 
  test_json_over_uttp test_compound_id test_netservice_params
)
