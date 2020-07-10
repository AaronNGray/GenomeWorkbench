#############################################################################
# $Id: CMakeLists.txt 563304 2018-05-07 16:19:23Z gouriano $
#############################################################################

NCBI_project_tags(core)
NCBI_add_library(corelib test_mt test_boost)
NCBI_add_subdirectory(test)
