#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################


NCBI_add_library(dbapi_driver)
NCBI_add_subdirectory(
  util ctlib ftds95 ftds100 ftds-default mysql odbc samples
)

