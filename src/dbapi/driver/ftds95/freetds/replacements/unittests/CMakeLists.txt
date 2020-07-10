#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################

NCBI_add_definitions(HAVE_CONFIG_H=1)
NCBI_uses_toolkit_libraries(tds_ftds95)
NCBI_project_watchers(ucko satskyse)

NCBI_add_app(condition mutex1 strings passarg)

