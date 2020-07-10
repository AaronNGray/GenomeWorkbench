#############################################################################
# $Id: CMakeLists.txt 597226 2019-11-19 02:19:39Z ucko $
#############################################################################
# 
#

NCBI_project_tags(core)
NCBI_add_library(serial cserial)
NCBI_add_subdirectory(datatool test soap grpc_integration)


