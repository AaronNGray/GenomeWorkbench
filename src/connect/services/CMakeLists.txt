#############################################################################
# $Id: CMakeLists.txt 593499 2019-09-19 15:56:50Z gouriano $
#############################################################################

NCBI_project_tags(grid)
NCBI_add_library(xconnserv ncbi_xcache_netcache ncbi_xblobstorage_netcache)
NCBI_add_subdirectory(test)

