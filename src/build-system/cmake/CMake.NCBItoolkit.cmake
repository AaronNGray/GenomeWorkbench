#############################################################################
# $Id: CMake.NCBItoolkit.cmake 609379 2020-06-01 14:15:14Z ivanov $
#############################################################################

if(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)
set( NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED ON)

if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    if(NOT DEFINED XCODE)
        set(XCODE ON)
    endif()
endif()

string(REGEX REPLACE "([][^$.\\*+?|()])" "\\\\\\1" _tmp ${CMAKE_CURRENT_SOURCE_DIR})
if (NOT DEFINED NCBI_EXTERNAL_TREE_ROOT AND NOT ${CMAKE_CURRENT_LIST_DIR} MATCHES "^${_tmp}/")
    get_filename_component(NCBI_EXTERNAL_TREE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.."   ABSOLUTE)
    message(STATUS "Found NCBI C++ Toolkit: ${NCBI_EXTERNAL_TREE_ROOT}")
endif()

set(NCBI_PTBCFG_INSTALL_EXPORT ncbi-cpp-toolkit)

###############################################################################

if(NOT DEFINED NCBI_EXPERIMENTAL)
    if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
        set(NCBI_EXPERIMENTAL ON)
    else()
        set(NCBI_EXPERIMENTAL ON)
    endif()
endif()

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

if(NCBI_EXPERIMENTAL)

    set(NCBI_EXPERIMENTAL_CFG              ON)
    set(NCBI_EXPERIMENTAL_SUBDIRS          ON)
    set(NCBI_EXPERIMENTAL_DISABLE_HUNTER   ON)
    set(NCBI_VERBOSE_ALLPROJECTS           OFF)
    if(NCBI_PTBCFG_SKIP_ANALYSIS)
        set(NCBI_PTBCFG_ENABLE_COLLECTOR       OFF)
    else()
        set(NCBI_PTBCFG_ENABLE_COLLECTOR       ON)
    endif()

    if(BUILD_SHARED_LIBS)
        if(WIN32 OR XCODE)
            set(NCBI_PTBCFG_COMPOSITE_DLL dll)
            set(NCBI_PTBCFG_ALLOW_COMPOSITE ON)
        endif()
    endif()

    set(NCBI_PTBCFG_ADDCHECK               ON)
    if (NOT "${NCBI_PTBCFG_INSTALL_PATH}" STREQUAL "")
        set(NCBI_PTBCFG_DOINSTALL              ON)
    endif()

else()

    set(NCBI_EXPERIMENTAL_CFG              OFF)
    set(NCBI_EXPERIMENTAL_SUBDIRS          OFF)
    set(NCBI_EXPERIMENTAL_DISABLE_HUNTER   OFF)
    set(NCBI_VERBOSE_ALLPROJECTS           OFF)
    set(NCBI_PTBCFG_ENABLE_COLLECTOR       OFF)
    set(NCBI_PTBCFG_DOINSTALL              OFF)
    set(NCBI_PTBCFG_ADDCHECK               OFF)
endif()


###############################################################################
## Initialize Hunter
##
if (NOT NCBI_EXPERIMENTAL_DISABLE_HUNTER)
if (WIN32)
    if (NOT HUNTER_ROOT)
        set(HUNTER_ROOT ${CMAKE_SOURCE_DIR}/../HunterPackages)
    endif()
    include(build-system/cmake/HunterGate.cmake)
    HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.18.39.tar.gz"
        SHA1 "a6fbc056c3d9d7acdaa0a07c575c9352951c2f6c"
    )
endif()
endif()

set(_listdir "${CMAKE_CURRENT_LIST_DIR}")
if (NOT EXISTS "${_listdir}/CMake.NCBIptb.cmake")
    message(FATAL_ERROR "Cannot find NCBIptb build system in ${_listdir}")
endif()

include(${_listdir}/CMakeMacros.cmake)
include(${_listdir}/CMakeChecks.cmake)
include(${_listdir}/CMake.NCBIptb.cmake)
include(${_listdir}/CMake.NCBIptb.ncbi.cmake)
include(${_listdir}/CMake.NCBIptb.datatool.cmake)
include(${_listdir}/CMake.NCBIptb.grpc.cmake)
include(${_listdir}/CMake.NCBIptb.ctest.cmake)
if(NCBI_PTBCFG_ADDCHECK)
    include(${_listdir}/CMake.NCBIptb.ntest.cmake)
endif()
if(NCBI_PTBCFG_DOINSTALL)
    include(${_listdir}/CMake.NCBIptb.install.cmake)
endif()
include(${_listdir}/CMake.NCBIptb.legacy.cmake)

if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
    if (EXISTS ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
        include(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
    else()
        message(FATAL_ERROR "${NCBI_PTBCFG_INSTALL_EXPORT} was not found in ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}")
    endif()
    NCBI_import_hostinfo(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.hostinfo)
endif()

include(${_listdir}/CMakeChecks.final-message.cmake)
endif(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)
