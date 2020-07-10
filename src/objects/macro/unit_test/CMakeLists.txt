#############################################################################
# $Id: CMakeLists.txt 593488 2019-09-19 14:56:53Z gouriano $
#############################################################################

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(unit_test_string_constraint unit_test_location_constraint)

