#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################

NCBI_requires(PYTHON -Cygwin)
NCBI_add_library(python_ncbi_dbapi)
NCBI_add_subdirectory(tests)

