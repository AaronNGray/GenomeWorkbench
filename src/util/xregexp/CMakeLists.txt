#############################################################################
# $Id: CMakeLists.txt 593481 2019-09-19 14:00:01Z gouriano $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_requires(PCRE)
NCBI_add_library(xregexp xregexp_template_tester)

