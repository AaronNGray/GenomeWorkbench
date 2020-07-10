#############################################################################
# $Id: CMakeLists.txt 42614 2019-03-26 18:22:31Z gouriano $
#############################################################################

NCBI_add_library(gui_objutils)
if (NCBI_EXPERIMENTAL_CFG)
NCBI_add_subdirectory(test test_macro)
endif()

# Include projects from this directory
#include(CMakeLists.gui_objutils.lib.txt)
#add_subdirectory(test_macro)
