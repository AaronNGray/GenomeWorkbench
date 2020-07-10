#############################################################################
# $Id: CMakeChecks.cmake 609374 2020-06-01 14:13:44Z ivanov $
#############################################################################
#
# Note:
# This file is included before everything else
# Anything related to the initial state should go early in this file!


if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    if(NOT DEFINED XCODE)
        set(XCODE ON)
    endif()
endif()

string(TIMESTAMP NCBI_TIMESTAMP_START "%s")
string(TIMESTAMP _start)
message("Started: ${_start}")

#############################################################################
# Source tree description
#
set(NCBI_DIRNAME_RUNTIME bin)
set(NCBI_DIRNAME_ARCHIVE lib)
if (WIN32)
    set(NCBI_DIRNAME_SHARED ${NCBI_DIRNAME_RUNTIME})
else()
    set(NCBI_DIRNAME_SHARED ${NCBI_DIRNAME_ARCHIVE})
endif()
set(NCBI_DIRNAME_SRC             src)
set(NCBI_DIRNAME_INCLUDE         include)
set(NCBI_DIRNAME_COMMON_INCLUDE  common)
set(NCBI_DIRNAME_CFGINC          inc)
set(NCBI_DIRNAME_INTERNAL        internal)
set(NCBI_DIRNAME_EXPORT          cmake)
set(NCBI_DIRNAME_TESTING         testing)
set(NCBI_DIRNAME_SCRIPTS         scripts)
set(NCBI_DIRNAME_COMMON_SCRIPTS  scripts/common)
set(NCBI_DIRNAME_BUILDCFG ${NCBI_DIRNAME_SRC}/build-system)
set(NCBI_DIRNAME_CMAKECFG ${NCBI_DIRNAME_SRC}/build-system/cmake)


if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build-system/cmake/CMake.NCBIptb.cmake")
    set(_this_root     ${CMAKE_CURRENT_SOURCE_DIR}/..)
else()
    set(_this_root     ${CMAKE_SOURCE_DIR})
endif()
get_filename_component(_this_root  "${_this_root}"     ABSOLUTE)
get_filename_component(top_src_dir "${CMAKE_CURRENT_LIST_DIR}/../../.."   ABSOLUTE)

set(NCBI_TREE_ROOT    ${_this_root})
set(NCBI_SRC_ROOT     ${NCBI_TREE_ROOT}/${NCBI_DIRNAME_SRC})
set(NCBI_INC_ROOT     ${NCBI_TREE_ROOT}/${NCBI_DIRNAME_INCLUDE})
set(NCBITK_TREE_ROOT  ${top_src_dir})
set(NCBITK_SRC_ROOT   ${NCBITK_TREE_ROOT}/${NCBI_DIRNAME_SRC})
set(NCBITK_INC_ROOT   ${NCBITK_TREE_ROOT}/${NCBI_DIRNAME_INCLUDE})
if (NOT EXISTS "${NCBI_SRC_ROOT}")
    set(NCBI_SRC_ROOT   ${NCBI_TREE_ROOT})
endif()
if (NOT EXISTS "${NCBI_INC_ROOT}")
    set(NCBI_INC_ROOT   ${NCBI_TREE_ROOT})
endif()

set(build_root      ${CMAKE_BINARY_DIR})
set(builddir        ${CMAKE_BINARY_DIR})
set(includedir0     ${NCBI_INC_ROOT})
set(includedir      ${NCBI_INC_ROOT})
set(incdir          ${CMAKE_BINARY_DIR}/${NCBI_DIRNAME_CFGINC})
set(incinternal     ${NCBI_INC_ROOT}/${NCBI_DIRNAME_INTERNAL})

set(NCBI_DIRNAME_BUILD  build)
#if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
if (OFF)
    string(FIND ${CMAKE_BINARY_DIR} ${NCBI_TREE_ROOT} _pos_root)
    string(FIND ${CMAKE_BINARY_DIR} ${NCBI_SRC_ROOT}  _pos_src)
    if(NOT "${_pos_root}" LESS "0" AND "${_pos_src}" LESS "0" AND NOT "${CMAKE_BINARY_DIR}" STREQUAL "${NCBI_TREE_ROOT}")
        get_filename_component(NCBI_BUILD_ROOT "${CMAKE_BINARY_DIR}/.." ABSOLUTE)
	    get_filename_component(NCBI_DIRNAME_BUILD ${CMAKE_BINARY_DIR} NAME)
    else()
        get_filename_component(NCBI_BUILD_ROOT "${CMAKE_BINARY_DIR}" ABSOLUTE)
    endif()
else()
    get_filename_component(NCBI_BUILD_ROOT "${CMAKE_BINARY_DIR}/.." ABSOLUTE)
    get_filename_component(NCBI_DIRNAME_BUILD ${CMAKE_BINARY_DIR} NAME)
endif()

set(NCBI_CFGINC_ROOT   ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_CFGINC})

if (NCBI_EXPERIMENTAL_CFG)
    get_filename_component(incdir "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_CFGINC}" ABSOLUTE)
if(ON)
    if(WIN32 OR XCODE)
        set(incdir          ${incdir}/$<CONFIG>)
    endif()
else()
    if (WIN32)
        set(incdir          ${incdir}/\$\(Configuration\))
    elseif (XCODE)
        set(incdir          ${incdir}/\$\(CONFIGURATION\))
    endif()
endif()
endif()

if (NCBI_EXPERIMENTAL_CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_RUNTIME}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_SHARED}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_ARCHIVE}")
else()
    get_filename_component(EXECUTABLE_OUTPUT_PATH "${NCBI_BUILD_ROOT}/bin" ABSOLUTE)
    get_filename_component(LIBRARY_OUTPUT_PATH "${NCBI_BUILD_ROOT}/lib" ABSOLUTE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}")
endif()

set(NCBI_TREE_CMAKECFG "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(NCBI_TREE_BUILDCFG "${CMAKE_CURRENT_LIST_DIR}/.."   ABSOLUTE)

if(EXISTS ${NCBI_TREE_ROOT}/CMake.CustomConfig.txt)
	include(${NCBI_TREE_ROOT}/CMake.CustomConfig.txt)
endif()

############################################################################
# OS-specific settings
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.os.cmake)

#############################################################################
# Build configurations and compiler definitions
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.compiler.cmake)

#############################################################################
set(_prebuilt_loc)
if ("${NCBI_COMPILER}" STREQUAL "MSVC")
    set(_prebuilt_loc "CMake-VS")
    if ("${NCBI_COMPILER_VERSION}" LESS "1900")
        set(_prebuilt_loc ${_prebuilt_loc}2015)
    elseif ("${NCBI_COMPILER_VERSION}" LESS "1924")
        set(_prebuilt_loc ${_prebuilt_loc}2017)
    else()
        set(_prebuilt_loc ${_prebuilt_loc}2019)
    endif()
    if (BUILD_SHARED_LIBS)
        set(_prebuilt_loc ${_prebuilt_loc}-DLL)
    else()
        set(_prebuilt_loc ${_prebuilt_loc})
    endif()
elseif(XCODE)
    set(_prebuilt_loc "CMake-Xcode${NCBI_COMPILER_VERSION}")
    if (BUILD_SHARED_LIBS)
        set(_prebuilt_loc ${_prebuilt_loc}-DLL)
    else()
        set(_prebuilt_loc ${_prebuilt_loc})
    endif()
else()
    set(_prebuilt_loc "CMake-${NCBI_COMPILER}${NCBI_COMPILER_VERSION}-${CMAKE_BUILD_TYPE}")
    if (BUILD_SHARED_LIBS)
        set(_prebuilt_loc ${_prebuilt_loc}DLL)
    endif()
endif()
set(NCBI_DIRNAME_PREBUILT  ${_prebuilt_loc})

set(_tk_includedir      ${NCBITK_INC_ROOT})
set(_tk_incinternal     ${NCBITK_INC_ROOT}/${NCBI_DIRNAME_INTERNAL})
set(_inc_dirs)
foreach( _inc IN ITEMS ${includedir} ${incinternal} ${_tk_includedir} ${_tk_incinternal})
    if (IS_DIRECTORY ${_inc})
        list(APPEND _inc_dirs ${_inc})
    endif()
endforeach()
list(REMOVE_DUPLICATES _inc_dirs)
include_directories(${incdir} ${_inc_dirs})
include_regular_expression("^.*[.](h|hpp|c|cpp|inl|inc)$")
if(OFF)
message("CMAKE_SOURCE_DIR    = ${CMAKE_SOURCE_DIR}")
message("NCBI_TREE_ROOT      = ${NCBI_TREE_ROOT}")
message("NCBI_SRC_ROOT       = ${NCBI_SRC_ROOT}")
message("NCBI_INC_ROOT       = ${NCBI_INC_ROOT}")
message("NCBITK_TREE_ROOT    = ${NCBITK_TREE_ROOT}")
message("NCBITK_SRC_ROOT     = ${NCBITK_SRC_ROOT}")
message("NCBITK_INC_ROOT     = ${NCBITK_INC_ROOT}")
message("NCBI_BUILD_ROOT     = ${NCBI_BUILD_ROOT}")
message("NCBI_CFGINC_ROOT    = ${NCBI_CFGINC_ROOT}")
message("NCBI_TREE_BUILDCFG  = ${NCBI_TREE_BUILDCFG}")
message("NCBI_TREE_CMAKECFG  = ${NCBI_TREE_CMAKECFG}")
message("include_directories(${incdir} ${_inc_dirs})")
endif()

if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
    set(NCBI_EXTERNAL_BUILD_ROOT  ${NCBI_EXTERNAL_TREE_ROOT}/${NCBI_DIRNAME_PREBUILT})
    if (NOT EXISTS ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
        message(FATAL_ERROR "${NCBI_PTBCFG_INSTALL_EXPORT} was not found in ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}")
    endif()
endif()

#set(CMAKE_MODULE_PATH "${NCBI_SRC_ROOT}/build-system/cmake/" ${CMAKE_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${NCBI_TREE_CMAKECFG}")


#############################################################################
# Basic checks
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.basic-checks.cmake)

#############################################################################
# Hunter packages for Windows

if (NOT NCBI_EXPERIMENTAL_DISABLE_HUNTER)
if (WIN32)
    #set(HUNTER_STATUS_DEBUG TRUE)
    hunter_add_package(wxWidgets)
    hunter_add_package(Boost COMPONENTS filesystem regex system test)
    hunter_add_package(ZLIB)
    hunter_add_package(BZip2)
    hunter_add_package(Jpeg)
    hunter_add_package(PNG)
    hunter_add_package(TIFF)
    #hunter_add_package(freetype)
endif()
endif()

#############################################################################
# External libraries
include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponents.cmake)

#############################################################################
# Generation of configuration files

# Stable components
# This sets a version to be used throughout our config process
# NOTE: Adjust as needed
#
set(NCBI_CPP_TOOLKIT_VERSION_MAJOR 24)
set(NCBI_CPP_TOOLKIT_VERSION_MINOR 0)
set(NCBI_CPP_TOOLKIT_VERSION_PATCH 0)
set(NCBI_CPP_TOOLKIT_VERSION_EXTRA "")
set(NCBI_CPP_TOOLKIT_VERSION
    ${NCBI_CPP_TOOLKIT_VERSION_MAJOR}.${NCBI_CPP_TOOLKIT_VERSION_MINOR}.${NCBI_CPP_TOOLKIT_VERSION_PATCH}${NCBI_CPP_TOOLKIT_VERSION_EXTRA})

#############################################################################
# Subversion
# This is needed for some use cases

include(FindSubversion)
if (Subversion_FOUND AND EXISTS ${top_src_dir}/.svn)
    Subversion_WC_INFO(${top_src_dir} TOOLKIT)
elseif(NOT "$ENV{SVNREV}" STREQUAL "")
    set(TOOLKIT_WC_REVISION "$ENV{SVNREV}")
    set(TOOLKIT_WC_URL "$ENV{SVNURL}")
else()
    set(TOOLKIT_WC_REVISION 0)
    set(TOOLKIT_WC_URL "")
endif()
if(NOT "$ENV{NCBI_SUBVERSION_REVISION}" STREQUAL "")
    set(TOOLKIT_WC_REVISION "$ENV{NCBI_SUBVERSION_REVISION}")
endif()
set(NCBI_SUBVERSION_REVISION ${TOOLKIT_WC_REVISION})
message(STATUS "SVN revision = ${TOOLKIT_WC_REVISION}")
message(STATUS "SVN URL = ${TOOLKIT_WC_URL}")

if (Subversion_FOUND AND EXISTS ${top_src_dir}/src/corelib/.svn)
    Subversion_WC_INFO(${top_src_dir}/src/corelib CORELIB)
else()
    set(CORELIB_WC_REVISION 0)
    set(CORELIB_WC_URL "")
endif()

if(NOT "$ENV{NCBI_SC_VERSION}" STREQUAL "")
    set(NCBI_SC_VERSION $ENV{NCBI_SC_VERSION})
else()
    set(NCBI_SC_VERSION 0)
    if (NOT "${CORELIB_WC_URL}" STREQUAL "")
        string(REGEX REPLACE ".*/production/components/core/([0-9]+)\\.[0-9]+/.*" "\\1" _SC_VER "${CORELIB_WC_URL}")
        string(LENGTH "${_SC_VER}" _SC_VER_LEN)
        if (${_SC_VER_LEN} LESS 10 AND NOT "${_SC_VER}" STREQUAL "")
            set(NCBI_SC_VERSION ${_SC_VER})
            message(STATUS "Stable Components Number = ${NCBI_SC_VERSION}")
        endif()
    endif()
endif()

set(NCBI_TEAMCITY_BUILD_NUMBER 0)
if (NOT "$ENV{TEAMCITY_VERSION}" STREQUAL "")
    set(NCBI_TEAMCITY_BUILD_NUMBER   $ENV{BUILD_NUMBER})
    set(NCBI_TEAMCITY_PROJECT_NAME   $ENV{TEAMCITY_PROJECT_NAME})
    set(NCBI_TEAMCITY_BUILDCONF_NAME $ENV{TEAMCITY_BUILDCONF_NAME})
    if(EXISTS "$ENV{TEAMCITY_BUILD_PROPERTIES_FILE}")
        file(STRINGS "$ENV{TEAMCITY_BUILD_PROPERTIES_FILE}" _list)
        foreach( _item IN LISTS _list)
            if ("${_item}" MATCHES "teamcity.build.id")
                string(REPLACE "teamcity.build.id" "" _item ${_item})
                string(REPLACE " " "" _item ${_item})
                string(REPLACE "=" "" _item ${_item})
                set(NCBI_TEAMCITY_BUILD_ID ${_item})
                break()
            endif()
        endforeach()
    else()
        message("$ENV{TEAMCITY_BUILD_PROPERTIES_FILE} DOES NOT EXIST")
    endif()
    if ("${NCBI_TEAMCITY_BUILD_ID}" STREQUAL "")
        string(RANDOM _name)
        string(UUID NCBI_TEAMCITY_BUILD_ID NAMESPACE "73203eb4-80d3-4957-a110-8aae92c7e615" NAME ${_name} TYPE SHA1)
    endif()
    message(STATUS "TeamCity build number = ${NCBI_TEAMCITY_BUILD_NUMBER}")
    message(STATUS "TeamCity project name = ${NCBI_TEAMCITY_PROJECT_NAME}")
    message(STATUS "TeamCity build conf   = ${NCBI_TEAMCITY_BUILDCONF_NAME}")
    message(STATUS "TeamCity build ID     = ${NCBI_TEAMCITY_BUILD_ID}")
endif()

#############################################################################
cmake_host_system_information(RESULT _local_host_name  QUERY HOSTNAME)
if (WIN32 OR XCODE)
    set(HOST "${HOST_CPU}-${HOST_OS}")
else()
#    set(HOST "${HOST_CPU}-unknown-${HOST_OS}")
    set(HOST "${HOST_CPU}-${HOST_OS}")
endif()
string(REPLACE ";" " " FEATURES "${NCBI_ALL_COMPONENTS}")

if (NCBI_EXPERIMENTAL_CFG)

    set(_tk_common_include "${NCBITK_INC_ROOT}/common")
    if (WIN32 OR XCODE)
        foreach(_cfg ${NCBI_CONFIGURATION_TYPES})

            set(_file "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_cfg}")
            if (WIN32)
                string(REPLACE "/" "\\\\" _file ${_file})
            endif()
            set(c_ncbi_runpath "${_file}")
            if (WIN32)
                string(REPLACE "/" "\\\\" SYBASE_PATH "${SYBASE_PATH}")
            endif()

            set(NCBI_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${_cfg}--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
            set(NCBI_SIGNATURE_CFG "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-\$<CONFIG>--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
            set(NCBI_SIGNATURE_${_cfg} "${NCBI_SIGNATURE}")
if(OFF)
            if (WIN32)
                configure_file(${NCBI_TREE_CMAKECFG}/ncbiconf_msvc_site.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_msvc_site.h)
            elseif (XCODE)
                configure_file(${NCBI_TREE_CMAKECFG}/ncbiconf_msvc_site.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_xcode_site.h)
            endif()
else()
            if (WIN32)
                configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_msvc.h)
            elseif (XCODE)
                configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_xcode.h)
            endif()
endif()
            if (EXISTS ${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in)
                configure_file(${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbicfg.cfg.c)
            endif()
            configure_file(${_tk_common_include}/ncbi_build_ver.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/ncbi_build_ver.h)
            if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
                configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_INC_ROOT}/common/ncbi_revision.h)
            else()
                configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBITK_INC_ROOT}/common/ncbi_revision.h)
            endif()
        endforeach()
        if(NOT EXISTS ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c)
            file(WRITE ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c "#include <common/config/ncbicfg.cfg.c>\n")
        endif()
        if (WIN32)
            if (BUILD_SHARED_LIBS)
                set(NCBITEST_SIGNATURE "${NCBI_COMPILER_ALT}-\$<CONFIG>MTdll64--${HOST_CPU}-win64-${_local_host_name}")
            else()
                set(NCBITEST_SIGNATURE "${NCBI_COMPILER_ALT}-\$<CONFIG>MTstatic64--${HOST_CPU}-win64-${_local_host_name}")
            endif()
        else()
            set(NCBITEST_SIGNATURE "${NCBI_SIGNATURE_CFG}")
        endif()
    else()
#Linux
        set(c_ncbi_runpath ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
        set(NCBI_TLS_VAR "__thread")

        set(NCBI_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${NCBI_BUILD_TYPE}--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
        configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/ncbiconf_unix.h)
        if (EXISTS ${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in)
            configure_file(${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c)
        endif()

        configure_file(${_tk_common_include}/ncbi_build_ver.h.in ${NCBI_CFGINC_ROOT}/common/ncbi_build_ver.h)
        configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_INC_ROOT}/common/ncbi_revision.h)
        if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
            configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_INC_ROOT}/common/ncbi_revision.h)
        else()
            configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBITK_INC_ROOT}/common/ncbi_revision.h)
        endif()
    endif()

else (NCBI_EXPERIMENTAL_CFG)

set(HOST "${HOST_CPU}-unknown-${HOST_OS}")
set(NCBI_SIGNATURE "${CMAKE_C_COMPILER_ID}_${MSVC_VERSION}-${HOST}-${_local_host_name}")

if (WIN32)
    set(HOST "${HOST_CPU}-${HOST_OS}")
    set(NCBI_SIGNATURE "${CMAKE_C_COMPILER_ID}_${MSVC_VERSION}-${HOST}-${_local_host_name}")
endif()

# This file holds information about the build version
message(STATUS "Generating ${includedir}/common/ncbi_build_ver.h")
configure_file(${includedir}/common/ncbi_build_ver.h.in ${includedir}/common/ncbi_build_ver.h)
message(STATUS "Generating ${includedir}/common/ncbi_revision.h")
configure_file(${includedir}/common/ncbi_revision.h.in ${includedir}/common/ncbi_revision.h)

# OS-specific generated header configs
if (UNIX)
    message(STATUS "Generating ${build_root}/inc/ncbiconf_unix.h...")
    configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${build_root}/inc/ncbiconf_unix.h)
    set(_os_specific_config ${build_root}/inc/ncbiconf_unix.h)
endif(UNIX)

if (WIN32)
    message(STATUS "Generating ${build_root}/inc/ncbiconf_msvc.h...")
    configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${build_root}/inc/ncbiconf_msvc.h)
    message(STATUS "Generating ${includedir}/common/config/ncbiconf_msvc_site.h...")
    configure_file(${NCBI_TREE_CMAKECFG}/ncbiconf_msvc_site.h.in ${includedir}/common/config/ncbiconf_msvc_site.h)
    set(_os_specific_config ${build_root}/inc/ncbiconf_msvc.h ${includedir}/common/config/ncbiconf_msvc_site.h)
endif (WIN32)

if (APPLE AND NOT UNIX) #XXX 
    message(STATUS "Generating ${build_root}/inc/ncbiconf_xcode.h...")
    configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${build_root}/inc/ncbiconf_xcode.h)
    set(_os_specific_config ${build_root}/inc/ncbiconf_xcode.h)
endif (APPLE AND NOT UNIX)

#
# write ncbicfg.c.in
#
# FIXME:
# We need to set these variables to get them into the cfg file:
#  - c_ncbi_runpath
#  - SYBASE_LCL_PATH
#  - SYBASE_PATH
#  - FEATURES
set(c_ncbi_runpath "$ORIGIN/../lib")
set(SYBASE_LCL_PATH ${SYBASE_LIBRARIES})
set(SYBASE_PATH "")
set(FEATURES "")
configure_file(${NCBI_SRC_ROOT}/corelib/ncbicfg.c.in ${CMAKE_BINARY_DIR}/corelib/ncbicfg.c)

endif (NCBI_EXPERIMENTAL_CFG)

