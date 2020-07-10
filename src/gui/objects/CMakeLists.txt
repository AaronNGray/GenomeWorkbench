#############################################################################
# $Id: CMakeLists.txt 42608 2019-03-26 14:01:31Z gouriano $
#############################################################################

NCBI_add_library(gui_objects)
NCBI_add_subdirectory(test)

# Include projects from this directory
#include(CMakeLists.gui_objects.asn.txt)
# Recurse subdirectories
#add_subdirectory(test EXCLUDE_FROM_ALL)
