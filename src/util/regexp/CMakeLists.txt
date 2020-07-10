#############################################################################
# $Id: CMakeLists.txt 593481 2019-09-19 14:00:01Z gouriano $
#############################################################################
# 
#

# Include projects from this directory
NCBI_disable_pch()
NCBI_add_library(regexp)
NCBI_add_app(test_pcre)

