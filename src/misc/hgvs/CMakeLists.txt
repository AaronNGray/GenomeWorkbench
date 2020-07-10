#############################################################################
# $Id: CMakeLists.txt 593581 2019-09-20 13:30:24Z gouriano $
#############################################################################


NCBI_requires(Boost.Spirit)
NCBI_add_definitions(BOOST_ERROR_CODE_HEADER_ONLY BOOST_SYSTEM_NO_DEPRECATED)
NCBI_add_library(hgvs_parser)
NCBI_add_subdirectory(test)
