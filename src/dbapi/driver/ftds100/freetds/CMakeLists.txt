#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################

NCBI_disable_pch()
NCBI_add_subdirectory(
  tds ctlib dblib odbc #replacements
)

