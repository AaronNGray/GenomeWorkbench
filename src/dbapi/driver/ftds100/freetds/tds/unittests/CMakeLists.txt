#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################

NCBI_add_definitions(HAVE_CONFIG_H=1)
NCBI_uses_toolkit_libraries(tds_ftds100)
NCBI_project_watchers(ucko satskyse)

NCBI_add_app(
  test-tds100 t0001 t0002 t0003 t0004 t0005 t0006 t0007 t0008
  dynamic1 convert dataread utf8_1 utf8_2 utf8_3 numeric iconv_fread
  toodynamic challenge readconf charconv nulls corrupt collations
)


