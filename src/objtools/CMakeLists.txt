#############################################################################
# $Id: CMakeLists.txt 593577 2019-09-20 12:22:42Z gouriano $
#############################################################################

NCBI_add_subdirectory(
  unit_test_util readers blast lds2 data_loaders simple
  alnmgr cddalignview test manip cleanup format edit validator
  asniotest align seqmasks_io eutils
  align_format snputil uudutil variation writers pubseq_gateway
  logging import
)
