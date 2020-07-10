#############################################################################
# $Id: CMakeLists.txt 594086 2019-09-30 12:19:30Z gouriano $
#############################################################################

NCBI_project_tags(web)
NCBI_add_library(cgi fcgi)
NCBI_add_subdirectory(test)
#NCBI_add_subdirectory(demo)

