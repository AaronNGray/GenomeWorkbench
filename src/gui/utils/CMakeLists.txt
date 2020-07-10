#############################################################################
# $Id: CMakeLists.txt 44466 2019-12-20 17:32:08Z coulouri $
#############################################################################
##############################################################################
# 
#

NCBI_add_library(gui_utils)
NCBI_add_subdirectory(app_job muparser splines)

if(OFF)
# Include projects from this directory
include(CMakeLists.gui_utils.lib.txt)

# Recurse subdirectories
NCBI_add_subdirectory(app_job)
NCBI_add_subdirectory(muparser)
NCBI_add_subdirectory(splines)


endif()
