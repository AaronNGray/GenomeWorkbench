#############################################################################
# $Id: CMakeChecks.compiler.cmake 609369 2020-06-01 14:12:55Z ivanov $
#############################################################################
#
# This config is designed to capture all compiler and linker definitions and search parameters
#

set(NCBI_DEFAULT_PCH "ncbi_pch.hpp")
set(NCBI_DEFAULT_HEADERS "*.h*;*impl/*.h*;*.inl;*impl/*.inl")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if(ChaosMonkey IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    add_definitions(-DNCBI_MONKEY)
endif()

if(Int8GI IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    add_definitions(-DNCBI_INT8_GI)
endif()

if(StrictGI IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    add_definitions(-DNCBI_STRICT_GI)
endif()

if("$ENV{DEBUG_TRACE_FATAL_SIGNALS}" OR BackwardSig IN_LIST NCBI_PTBCFG_PROJECT_FEATURES )
    set(USE_LIBBACKWARD_SIG_HANDLING 1)
endif()

if (NCBI_EXPERIMENTAL_CFG)

    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    if (BUILD_SHARED_LIBS)
        set(NCBI_DLL_BUILD 1)
        set(NCBI_DLL_SUPPORT 1)
    endif()
 
#----------------------------------------------------------------------------
if (WIN32)

    set(NCBI_COMPILER_MSVC 1)
    set(NCBI_COMPILER ${CMAKE_C_COMPILER_ID})
    set(NCBI_COMPILER_VERSION ${MSVC_VERSION})
    set(NCBI_COMPILER_ALT "VS_")
    if ("${NCBI_COMPILER_VERSION}" LESS "1900")
        set(NCBI_COMPILER_ALT ${NCBI_COMPILER_ALT}2015)
    elseif ("${NCBI_COMPILER_VERSION}" LESS "1924")
        set(NCBI_COMPILER_ALT ${NCBI_COMPILER_ALT}2017)
    else()
        set(NCBI_COMPILER_ALT ${NCBI_COMPILER_ALT}2019)
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        if (BUILD_SHARED_LIBS)
            set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL)
        else()
            if(CfgMT IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
                set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL DebugMT ReleaseMT)
            else()
                set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL)
            endif()
        endif()
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations" FORCE)
        set(NCBI_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
        set(NCBI_CONFIGURATION_RUNTIMELIB "")

        foreach(_cfg ${NCBI_CONFIGURATION_TYPES})
            string(TOUPPER ${_cfg} _cfg)
            set(CMAKE_CXX_FLAGS_${_cfg} "")
            set(CMAKE_C_FLAGS_${_cfg} "")
            set(CMAKE_EXE_LINKER_FLAGS_${_cfg} "")
            set(CMAKE_SHARED_LINKER_FLAGS_${_cfg} "")
        endforeach()

        if(CfgProps IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)

            if (NOT DEFINED NCBI_DEFAULT_CFGPROPS)
                set(NCBI_DEFAULT_CFGPROPS "ncbi_cpp.cfg.props")
                if (EXISTS "${NCBI_TREE_CMAKECFG}/${NCBI_DEFAULT_CFGPROPS}")
                    if (NOT EXISTS "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/${NCBI_DEFAULT_CFGPROPS}")
                        file(COPY "${NCBI_TREE_CMAKECFG}/${NCBI_DEFAULT_CFGPROPS}" DESTINATION "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}")
                    endif()
                    set(NCBI_DEFAULT_CFGPROPS "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/${NCBI_DEFAULT_CFGPROPS}")
                else()
                    message(WARNING "File not found:  ${NCBI_TREE_CMAKECFG}/${NCBI_DEFAULT_CFGPROPS}")
                    unset(NCBI_DEFAULT_CFGPROPS)
                endif()
            endif()

            set(CMAKE_CXX_FLAGS   "/DWIN32 /D_WINDOWS /EHsc")
            set(CMAKE_C_FLAGS   "/DWIN32 /D_WINDOWS")

            set(CMAKE_CXX_FLAGS_DEBUGDLL   "/MDd /Zi")
            set(CMAKE_CXX_FLAGS_DEBUGMT    "/MTd /Zi")
            set(CMAKE_CXX_FLAGS_RELEASEDLL "/MD /Zi")
            set(CMAKE_CXX_FLAGS_RELEASEMT  "/MT /Zi")

            set(CMAKE_C_FLAGS_DEBUGDLL   "/MDd /Zi")
            set(CMAKE_C_FLAGS_DEBUGMT    "/MTd /Zi")
            set(CMAKE_C_FLAGS_RELEASEDLL "/MD /Zi")
            set(CMAKE_C_FLAGS_RELEASEMT  "/MT /Zi")

            set(CMAKE_EXE_LINKER_FLAGS_DEBUGDLL   "/DEBUG")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUGMT    "/DEBUG")

            set(CMAKE_SHARED_LINKER_FLAGS_DEBUGDLL   "/DEBUG")
            set(CMAKE_SHARED_LINKER_FLAGS_DEBUGMT    "/DEBUG")

        else()
            set(CMAKE_CXX_FLAGS_DEBUGDLL   "/MP /MDd /Zi /Od /RTC1 /D_DEBUG")
            set(CMAKE_CXX_FLAGS_DEBUGMT    "/MP /MTd /Zi /Od /RTC1 /D_DEBUG")
            set(CMAKE_CXX_FLAGS_RELEASEDLL "/MP /MD  /Zi /O2 /Ob1 /DNDEBUG")
            set(CMAKE_CXX_FLAGS_RELEASEMT  "/MP /MT  /Zi /O2 /Ob1 /DNDEBUG")

            set(CMAKE_C_FLAGS_DEBUGDLL   "/MP /MDd /Zi /Od /RTC1 /D_DEBUG")
            set(CMAKE_C_FLAGS_DEBUGMT    "/MP /MTd /Zi /Od /RTC1 /D_DEBUG")
            set(CMAKE_C_FLAGS_RELEASEDLL "/MP /MD  /Zi /O2 /Ob1 /DNDEBUG")
            set(CMAKE_C_FLAGS_RELEASEMT  "/MP /MT  /Zi /O2 /Ob1 /DNDEBUG")

            set(CMAKE_EXE_LINKER_FLAGS_DEBUGDLL   "/DEBUG /INCREMENTAL:NO")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUGMT    "/DEBUG /INCREMENTAL:NO")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASEDLL "/INCREMENTAL:NO")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASEMT  "/INCREMENTAL:NO")

            set(CMAKE_SHARED_LINKER_FLAGS_DEBUGDLL   "/DEBUG /INCREMENTAL:NO")
            set(CMAKE_SHARED_LINKER_FLAGS_DEBUGMT    "/DEBUG /INCREMENTAL:NO")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASEDLL "/INCREMENTAL:NO")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASEMT  "/INCREMENTAL:NO")
        endif()
        if(Symbols IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
            set(CMAKE_EXE_LINKER_FLAGS "/DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
            set(CMAKE_SHARED_LINKER_FLAGS "/DEBUG ${CMAKE_SHARED_LINKER_FLAGS}")
        endif()

    else()
        set(NCBI_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")
        if ("${CMAKE_MSVC_RUNTIME_LIBRARY}" STREQUAL "MultiThreaded" OR "${CMAKE_MSVC_RUNTIME_LIBRARY}" STREQUAL "MultiThreadedDebug")
            set(NCBI_CONFIGURATION_RUNTIMELIB "MT")
        else()
            set(NCBI_CONFIGURATION_RUNTIMELIB "DLL")
        endif()
		set(NCBI_DEFAULT_USEPCH OFF)
    endif()

    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE)
    if(BUILD_SHARED_LIBS)
        add_definitions(-DNCBI_DLL_BUILD)
    endif()

    if(NOT DEFINED NCBI_DEFAULT_USEPCH)
		set(NCBI_DEFAULT_USEPCH ON)
        if ($ENV{NCBI_AUTOMATED_BUILD})
			set(NCBI_DEFAULT_USEPCH OFF)
		endif()
    endif()
    set(NCBI_DEFAULT_PCH_DEFINE "NCBI_USE_PCH")
    set(NCBI_DEFAULT_RESOURCES "${NCBI_TREE_CMAKECFG}/ncbi.rc")
    set(NCBI_DEFAULT_DLLENTRY  "${NCBI_TREE_CMAKECFG}/dll_main.cpp")
    set(NCBI_DEFAULT_GUIENTRY  "${NCBI_TREE_CMAKECFG}/winmain.cpp")

    set(ORIG_LIBS ws2_32.lib dbghelp.lib)
    return()

#----------------------------------------------------------------------------
elseif (XCODE)

    set(NCBI_COMPILER ${CMAKE_C_COMPILER_ID})
    string(REPLACE "." "" NCBI_COMPILER_VERSION ${XCODE_VERSION})
    set(NCBI_COMPILER_ALT ${NCBI_COMPILER})

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        if (BUILD_SHARED_LIBS)
            set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL)
        else()
#           set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL DebugMT ReleaseMT)
            set(CMAKE_CONFIGURATION_TYPES DebugDLL ReleaseDLL)
        endif()
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations" FORCE)
        set(NCBI_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES})
        set(NCBI_CONFIGURATION_RUNTIMELIB "")

        set(CMAKE_CXX_FLAGS_DEBUGDLL   "-g -gdwarf -ggdb3 -O0 -D_DEBUG")
        set(CMAKE_CXX_FLAGS_DEBUGMT    "-g -gdwarf -ggdb3 -O0 -D_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASEDLL "-Os -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASEMT  "-Os -DNDEBUG")

        set(CMAKE_C_FLAGS_DEBUGDLL   "-g -gdwarf -ggdb3 -O0 -D_DEBUG")
        set(CMAKE_C_FLAGS_DEBUGMT    "-g -gdwarf -ggdb3 -O0 -D_DEBUG")
        set(CMAKE_C_FLAGS_RELEASEDLL "-Os -DNDEBUG")
        set(CMAKE_C_FLAGS_RELEASEMT  "-Os -DNDEBUG")

        set(CMAKE_EXE_LINKER_FLAGS_DEBUGDLL   "-stdlib=libc++ -framework CoreServices")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUGMT    "-stdlib=libc++ -framework CoreServices")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASEDLL "-stdlib=libc++ -framework CoreServices")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASEMT  "-stdlib=libc++ -framework CoreServices")
    else()
        set(NCBI_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")
        set(NCBI_CONFIGURATION_RUNTIMELIB "DLL")
    endif()

    add_definitions(-DNCBI_XCODE_BUILD -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)

    find_package(Threads REQUIRED)
    if (CMAKE_USE_PTHREADS_INIT)
        add_definitions(-D_MT -D_REENTRANT -D_THREAD_SAFE)
        set(NCBI_POSIX_THREADS 1)
    endif (CMAKE_USE_PTHREADS_INIT)

    set(NCBI_DEFAULT_USEPCH ON)

    return()

#----------------------------------------------------------------------------
else()
endif()

endif()

if (WIN32)
    set(NCBI_COMPILER_MSVC 1)
    set(NCBI_COMPILER ${CMAKE_C_COMPILER_ID})
    set(NCBI_COMPILER_VERSION ${MSVC_VERSION})
else()
    set(NCBI_COMPILER ${CMAKE_C_COMPILER_ID})
    set(_tmp ${CMAKE_CXX_COMPILER_VERSION})
    string(REPLACE "." ";" _tmp "${_tmp}")
    list(GET _tmp 0 _v1)
    list(GET _tmp 1 _v2)
    list(GET _tmp 2 _v3)
    set(NCBI_COMPILER_VERSION ${_v1}${_v2}${_v3})
    set(NCBI_COMPILER_VERSION_DOTTED ${_v1}.${_v2}.${_v3})
endif()

if ("${NCBI_COMPILER}" STREQUAL "GNU")
    set(NCBI_COMPILER_GCC 1)
    set(NCBI_COMPILER "GCC")
elseif ("${NCBI_COMPILER}" STREQUAL "Intel")
    set(NCBI_COMPILER_ICC 1)
    set(NCBI_COMPILER "ICC")
elseif ("${NCBI_COMPILER}" STREQUAL "AppleClang")
    set(NCBI_COMPILER_APPLE_CLANG 1)
    set(NCBI_COMPILER "APPLE_CLANG")
elseif ("${NCBI_COMPILER}" STREQUAL "Clang")
    set(NCBI_COMPILER_LLVM_CLANG 1)
    set(NCBI_COMPILER "LLVM_CLANG")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()
if ("${BUILD_SHARED_LIBS}" STREQUAL "")
    set(BUILD_SHARED_LIBS OFF)
endif()

if (BUILD_SHARED_LIBS)
    set(NCBI_DLL_BUILD 1)
    set(NCBI_DLL_SUPPORT 1)
endif()

if (NOT buildconf)
  set(buildconf "${CMAKE_BUILD_TYPE}MT64")
  set(buildconf0 ${CMAKE_BUILD_TYPE})
  set(NCBI_BUILD_TYPE "${CMAKE_BUILD_TYPE}MT64")
endif (NOT buildconf)
set(NCBI_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")

if(MaxDebug IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    add_definitions(-D_GLIBCXX_DEBUG)
endif()

if(Symbols IN_LIST NCBI_PTBCFG_PROJECT_FEATURES)
    set(CMAKE_DEBUG_SYMBOLS ON)
endif()

message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build shared libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "NCBI Compiler: ${NCBI_COMPILER}")
message(STATUS "NCBI Compiler Version: ${NCBI_COMPILER_VERSION}")
message(STATUS "NCBI Compiler Version Tag: ${NCBI_COMPILER}${NCBI_COMPILER_VERSION}-${CMAKE_BUILD_TYPE}")

# pass these back for ccache to pick up
set(ENV{CCACHE_UMASK} 002)
set(ENV{CCACHE_BASEDIR} ${top_src_dir})

#
# Threading libraries
find_package(Threads REQUIRED)
## message("CMAKE_THREAD_LIBS_INIT: ${CMAKE_THREAD_LIBS_INIT}")
## message("CMAKE_USE_SPROC_INIT: ${CMAKE_USE_SPROC_INIT}")
## message("CMAKE_USE_WIN32_THREADS_INIT: ${CMAKE_USE_WIN32_THREADS_INIT}")
## message("CMAKE_USE_PTHREADS_INIT: ${CMAKE_USE_PTHREADS_INIT}")
## message("CMAKE_HP_PTHREADS_INIT: ${CMAKE_HP_PTHREADS_INIT}")
if (CMAKE_USE_PTHREADS_INIT)
    add_definitions(-D_MT -D_REENTRANT -D_THREAD_SAFE)
    set(NCBI_POSIX_THREADS 1)
endif (CMAKE_USE_PTHREADS_INIT)

#
# OpenMP
if (NOT XCODE AND NOT NCBI_COMPILER_LLVM_CLANG)
find_package(OpenMP)
## message("OPENMP_FOUND: ${OPENMP_FOUND}")
## message("OpenMP_CXX_SPEC_DATE: ${OpenMP_CXX_SPEC_DATE}")
if (OPENMP_FOUND)
    add_definitions(${OpenMP_CXX_FLAGS})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif (OPENMP_FOUND)
endif()

#
# See:
# http://stackoverflow.com/questions/32752446/using-compiler-prefix-commands-with-cmake-distcc-ccache
#
# There are better ways to do this
option(CMAKE_USE_CCACHE "Use 'ccache' as a preprocessor" OFF)
option(CMAKE_USE_DISTCC "Use 'distcc' as a preprocessor" OFF)

#----------------------------------------------------------------------------
# flags may be set by configuration scripts
if(NOT "$ENV{NCBI_COMPILER_C_FLAGS}" STREQUAL "")
    set(NCBI_COMPILER_C_FLAGS "$ENV{NCBI_COMPILER_C_FLAGS}" CACHE STRING "Cache preset C_FLAGS" FORCE)
endif()
if(NOT "$ENV{NCBI_COMPILER_CXX_FLAGS}" STREQUAL "")
    set(NCBI_COMPILER_CXX_FLAGS "$ENV{NCBI_COMPILER_CXX_FLAGS}" CACHE STRING "Cache preset CXX_FLAGS" FORCE)
endif()
if(NOT "$ENV{NCBI_COMPILER_EXE_LINKER_FLAGS}" STREQUAL "")
    set(NCBI_COMPILER_EXE_LINKER_FLAGS "$ENV{NCBI_COMPILER_EXE_LINKER_FLAGS}" CACHE STRING "Cache preset EXE_LINKER_FLAGS" FORCE)
endif()
if(NOT "$ENV{NCBI_COMPILER_SHARED_LINKER_FLAGS}" STREQUAL "")
    set(NCBI_COMPILER_SHARED_LINKER_FLAGS "$ENV{NCBI_COMPILER_SHARED_LINKER_FLAGS}" CACHE STRING "Cache preset SHARED_LINKER_FLAGS" FORCE)
endif()

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${NCBI_COMPILER_C_FLAGS}")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${NCBI_COMPILER_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${NCBI_COMPILER_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${NCBI_COMPILER_SHARED_LINKER_FLAGS}")
#----------------------------------------------------------------------------

if (NOT WIN32)
    set(_ggdb "-ggdb3")
    set(_ggdb1 "-ggdb1")
    if(NCBI_COMPILER_GCC)

        if("${NCBI_COMPILER_VERSION}" LESS "730")
            add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
        endif()

    elseif(NCBI_COMPILER_ICC)

        if("${NCBI_COMPILER_VERSION}" STREQUAL "1900")
            set(NCBI_COMPILER_COMPONENTS ICC1903)
        endif()
        set(_ggdb "-g")
        set(_ggdb1 "")
# Defining _GCC_NEXT_LIMITS_H ensures that <limits.h> chaining doesn't
# stop short, as can otherwise happen.
        add_definitions(-D_GCC_NEXT_LIMITS_H)
# -we70: "incomplete type is not allowed" should be an error, not a warning!
# -wd2651: Suppress spurious "attribute does not apply to any entity"
#          when deprecating enum values (via NCBI_STD_DEPRECATED).
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -we70 -wd2651")
        set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -Kc++ -static-intel -diag-disable 10237")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Kc++ -static-intel -diag-disable 10237")

    elseif(NCBI_COMPILER_LLVM_CLANG)

        if("${NCBI_COMPILER_VERSION}" STREQUAL "700")
            set(NCBI_COMPILER_COMPONENTS GCC730)
        endif()

    endif()

    if ("${CMAKE_DEBUG_SYMBOLS}" STREQUAL "")
        set(CMAKE_CXX_FLAGS_RELEASE "-gdwarf-4 ${_ggdb1} -O3 -DNDEBUG" CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELEASE   "-gdwarf-4 ${_ggdb1} -O3 -DNDEBUG" CACHE STRING "" FORCE)
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-gdwarf-4 ${_ggdb} -O3 -DNDEBUG" CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELEASE   "-gdwarf-4 ${_ggdb} -O3 -DNDEBUG" CACHE STRING "" FORCE)
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4 ${_ggdb} -O0 -D_DEBUG" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG   "-gdwarf-4 ${_ggdb} -O0 -D_DEBUG" CACHE STRING "" FORCE)
endif() 

if (APPLE)
  add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
elseif (UNIX)
  add_definitions(-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
endif (APPLE)

if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall -Wno-format-y2k )
endif()

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

macro(set_cxx_compiler_flag_optional)
    foreach (var ${ARGN})
        CHECK_CXX_COMPILER_FLAG("${var}" _COMPILER_SUPPORTS_FLAG)
        if (_COMPILER_SUPPORTS_FLAG)
            message(STATUS "The compiler ${CMAKE_CXX_COMPILER} supports ${var}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${var}")
            break()
        endif()
    endforeach()
    if (_COMPILER_SUPPORTS_FLAG)
    else()
        message(WARNING "The compiler ${CMAKE_CXX_COMPILER} has no support for any of ${ARGN}")
    endif()
endmacro()

macro(set_c_compiler_flag_optional)
    foreach (var ${ARGN})
        CHECK_C_COMPILER_FLAG(${var} _COMPILER_SUPPORTS_FLAG)
        if (_COMPILER_SUPPORTS_FLAG)
            message(STATUS "The compiler ${CMAKE_C_COMPILER} supports ${var}")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${var}")
            break()
        endif()
    endforeach()
    if (_COMPILER_SUPPORTS_FLAG)
    else()
        message(WARNING "The compiler ${CMAKE_C_COMPILER} has no support for any of ${ARGN}")
    endif()
endmacro()

# Check for appropriate C++11 flags
#if (UNIX)
if(OFF)
	set_cxx_compiler_flag_optional("-std=gnu++14" "-std=gnu++11" "-std=c++11" "-std=c++0x")
	set_c_compiler_flag_optional  ("-std=gnu11" "-std=c11" "-std=gnu99" "-std=c99")
endif()

find_program(CCACHE_EXECUTABLE ccache
             PATHS /usr/local/ccache/3.2.5/bin/)
find_program(DISTCC_EXECUTABLE
             NAMES distcc.sh distcc
             HINTS $ENV{NCBI}/bin )


set(NCBI_COMPILER_WRAPPER "")
if (CMAKE_USE_DISTCC AND DISTCC_EXECUTABLE AND
    CMAKE_USE_CCACHE AND CCACHE_EXECUTABLE)
set(NCBI_COMPILER_WRAPPER "CCACHE_BASEDIR=${top_src_dir} CCACHE_PREFIX=${DISTCC_EXECUTABLE} ${CCACHE_EXECUTABLE} ${NCBI_COMPILER_WRAPPER}")
elseif (CMAKE_USE_DISTCC AND DISTCC_EXECUTABLE)
    set(NCBI_COMPILER_WRAPPER "${DISTCC_EXECUTABLE} ${NCBI_COMPILER_WRAPPER}")
elseif(CMAKE_USE_CCACHE AND CCACHE_EXECUTABLE)
    set(NCBI_COMPILER_WRAPPER "CCACHE_BASEDIR=${top_src_dir} ${CCACHE_EXECUTABLE} ${NCBI_COMPILER_WRAPPER}")
    # pass these back for ccache to pick up
    set(ENV{CCACHE_BASEDIR} ${top_src_dir})
    message(STATUS "Enabling ccache: ${CCACHE_EXECUTABLE}")
    message(STATUS "  ccache basedir: ${top_src_dir}")
endif()

set(CMAKE_CXX_COMPILE_OBJECT
    "${NCBI_COMPILER_WRAPPER} ${CMAKE_CXX_COMPILE_OBJECT}")

set(CMAKE_C_COMPILE_OBJECT
    "${NCBI_COMPILER_WRAPPER} ${CMAKE_C_COMPILE_OBJECT}")

message(STATUS "NCBI_COMPILER_WRAPPER = ${NCBI_COMPILER_WRAPPER}")


#
# NOTE:
# uncomment this for strict mode for library compilation
#
#set(CMAKE_SHARED_LINKER_FLAGS_DYNAMIC "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic")

set(CMAKE_SHARED_LINKER_FLAGS_RDYNAMIC "${CMAKE_SHARED_LINKER_FLAGS}") # for smooth transition, please don't use
set(CMAKE_SHARED_LINKER_FLAGS_ALLOW_UNDEFINED "${CMAKE_SHARED_LINKER_FLAGS}")
if (NOT APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif ()


# Establishing sane RPATH definitions
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, use the install RPATH already
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

SET(CMAKE_INSTALL_RPATH "/$ORIGIN/../lib")

#this add RUNPATH to binaries (RPATH is already there anyway), which makes it more like binaries built by C++ Toolkit
if (NOT WIN32 AND NOT APPLE)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")
endif()

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
