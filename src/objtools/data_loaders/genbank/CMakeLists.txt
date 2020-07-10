#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_add_library(ncbi_xreader ncbi_xloader_genbank)
NCBI_add_subdirectory(
  cache pubseq id2 id1 pubseq2 gicache test
)

