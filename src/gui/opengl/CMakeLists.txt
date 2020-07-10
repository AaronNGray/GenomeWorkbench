#############################################################################
# $Id: CMakeLists.txt 43370 2019-06-20 16:34:36Z gouriano $
#############################################################################

NCBI_add_library(gui_opengl shaders)
NCBI_add_subdirectory(mesa)

# Include projects from this directory
#include(CMakeLists.gui_opengl.lib.txt)

# Recurse subdirectories
#add_subdirectory(mesa)
