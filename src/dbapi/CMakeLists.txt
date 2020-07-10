#############################################################################
# $Id: CMakeLists.txt 593523 2019-09-19 18:41:09Z gouriano $
#############################################################################
# 
#
if (APPLE AND NOT XCODE)
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-framework,ApplicationServices -liconv")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-framework,ApplicationServices -liconv")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-framework,ApplicationServices -liconv")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-framework,ApplicationServices -liconv -undefined dynamic_lookup")
endif ()

NCBI_add_library(dbapi)
NCBI_add_subdirectory(driver simple cache lang_bind test)

