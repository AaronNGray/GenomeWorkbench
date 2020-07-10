#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(blast_services_unit_test)

