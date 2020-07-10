#############################################################################
# $Id: CMakeLists.txt 595385 2019-10-21 11:15:30Z mcelhany $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(dbapi_unit_test dbapi_context_test dbapi_svc_test)
NCBI_add_subdirectory(driver)

