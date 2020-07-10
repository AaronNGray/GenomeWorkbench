#############################################################################
# $Id: CMakeLists.txt 42608 2019-03-26 14:01:31Z gouriano $
#############################################################################

NCBI_add_library(gui_glmesa)
if(NCBI_EXPERIMENTAL_CFG)
NCBI_add_subdirectory(demo)
endif()

# Include projects from this directory
#include(CMakeLists.gui_glmesa.lib.txt)

# Recurse subdirectories
#add_subdirectory(demo EXCLUDE_FROM_ALL)
