#############################################################################
# $Id: CMakeLists.txt 564071 2018-05-21 13:10:07Z gouriano $
#############################################################################

NCBI_disable_pch()
NCBI_add_library(lmdb)
NCBI_add_app(lmdb_test1 lmdb_test2 lmdb_test3 lmdb_test4 lmdb_test5)
