#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################

NCBI_requires(MySQL)
NCBI_add_library(ncbi_xdbapi_mysql)
NCBI_add_subdirectory(samples)

