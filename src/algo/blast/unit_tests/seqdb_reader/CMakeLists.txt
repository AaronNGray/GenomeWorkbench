#############################################################################
# $Id: CMakeLists.txt 593585 2019-09-20 13:57:47Z gouriano $
#############################################################################

NCBI_requires(Boost.Test.Included)
NCBI_add_app(seqdb_unit_test seqdb_lmdb_unit_test)

