#############################################################################
# $Id: CMakeLists.txt 593481 2019-09-19 14:00:01Z gouriano $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_disable_pch()
NCBI_add_library(tables)
NCBI_add_subdirectory(test)

