#############################################################################
# $Id: CMakeLists.txt 601791 2020-02-13 15:52:34Z ludwigf $
#############################################################################
# 
#

NCBI_add_library(util)
NCBI_add_subdirectory(
  regexp xregexp compress diff image tables
  sequtil bitset qparse lmdb lmdbxx test demo
)

