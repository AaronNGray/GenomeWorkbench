#############################################################################
# $Id: CMakeLists.txt 594841 2019-10-10 13:44:35Z ludwigf $
#############################################################################

NCBI_project_tags(test)
NCBI_add_app(
  unit_test_gtfwriter
  unit_test_gff3writer
  unit_test_gff3flybasewriter
  unit_test_vcfwriter
  unit_test_wigwriter
  unit_test_bedwriter
  unit_test_bedgraphwriter
  unit_test_ucscwriter
  unit_test_srcwriter
  unit_test_alnwriter
  unit_test_pslwriter
)

