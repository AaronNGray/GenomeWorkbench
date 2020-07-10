#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_requires(LMDB)
NCBI_add_library(writedb)
NCBI_add_subdirectory(unit_test)

