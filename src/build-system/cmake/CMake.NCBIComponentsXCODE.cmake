#############################################################################
# $Id: CMake.NCBIComponentsXCODE.cmake 609378 2020-06-01 14:15:02Z ivanov $
#############################################################################

##
## NCBI CMake components description - XCODE
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX
##  HAVE_XXX


set(NCBI_REQUIRE_unix_FOUND YES)
if(XCODE)
set(NCBI_REQUIRE_XCODE_FOUND YES)
endif()
#to debug
#set(NCBI_TRACE_COMPONENT_JPEG ON)
#############################################################################
# common settings
set(NCBI_TOOLS_ROOT $ENV{NCBI})
set(NCBI_OPT_ROOT  /opt/X11)
set(NCBI_PlatformBits 64)

include(CheckLibraryExists)
include(${NCBI_TREE_CMAKECFG}/FindExternalLibrary.cmake)

check_library_exists(dl dlopen "" HAVE_LIBDL)
if(HAVE_LIBDL)
    set(DL_LIBS -ldl)
else(HAVE_LIBDL)
    message(FATAL_ERROR "dl library not found")
endif(HAVE_LIBDL)

set(THREAD_LIBS   ${CMAKE_THREAD_LIBS_INIT})
find_library(CRYPT_LIBS NAMES crypt)
find_library(MATH_LIBS NAMES m)

if (APPLE)
    find_library(NETWORK_LIBS c)
    find_library(RT_LIBS c)
else (APPLE)
    find_library(NETWORK_LIBS nsl)
    find_library(RT_LIBS        rt)
endif (APPLE)
set(ORIG_LIBS   ${DL_LIBS} ${RT_LIBS} ${MATH_LIBS} ${CMAKE_THREAD_LIBS_INIT})

set(FOUNDATION_LIBS "-framework foundation")

#############################################################################
# Kerberos 5
set(KRB5_LIBS "-framework Kerberos" -liconv)

############################################################################
set(NCBI_ThirdPartyBasePath ${NCBI_TOOLS_ROOT})

set(NCBI_ThirdParty_BACKWARD   ${NCBI_ThirdPartyBasePath}/backward-cpp-1.3.20180206-44ae960)
set(NCBI_ThirdParty_TLS        ${NCBI_ThirdPartyBasePath}/gnutls-3.4.0)
#set(NCBI_ThirdParty_FASTCGI 
set(NCBI_ThirdParty_Boost      ${NCBI_ThirdPartyBasePath}/boost-1.62.0-ncbi1)
#set(NCBI_ThirdParty_PCRE
#set(NCBI_ThirdParty_Z
#set(NCBI_ThirdParty_BZ2
set(NCBI_ThirdParty_LZO        ${NCBI_ThirdPartyBasePath}/lzo-2.05)
set(NCBI_ThirdParty_BerkeleyDB ${NCBI_ThirdPartyBasePath}/BerkeleyDB)
set(NCBI_ThirdParty_LMDB       ${NCBI_ThirdPartyBasePath}/lmdb-0.9.18)
set(NCBI_ThirdParty_JPEG       ${NCBI_ThirdPartyBasePath}/safe-sw)
set(NCBI_ThirdParty_PNG        ${NCBI_OPT_ROOT})
#set(NCBI_ThirdParty_GIF
set(NCBI_ThirdParty_TIFF       ${NCBI_ThirdPartyBasePath}/safe-sw)
set(NCBI_ThirdParty_XML        ${NCBI_ThirdPartyBasePath}/libxml-2.7.8)
set(NCBI_ThirdParty_XSLT       ${NCBI_ThirdPartyBasePath}/libxml-2.7.8)
set(NCBI_ThirdParty_EXSLT      ${NCBI_ThirdParty_XSLT})
set(NCBI_ThirdParty_SQLITE3    ${NCBI_ThirdPartyBasePath}/sqlite-3.26.0-ncbi1)
#set(NCBI_ThirdParty_Sybase
set(NCBI_ThirdParty_VDB        "/net/snowman/vol/projects/trace_software/vdb/vdb-versions/2.10.6")
set(NCBI_ThirdParty_VDB_ARCH x86_64)
set(NCBI_ThirdParty_wxWidgets ${NCBI_ThirdPartyBasePath}/wxWidgets-3.1.3-ncbi1)
set(NCBI_ThirdParty_GLEW      ${NCBI_ThirdPartyBasePath}/glew-1.5.8)
set(NCBI_ThirdParty_FTGL      ${NCBI_ThirdPartyBasePath}/ftgl-2.1.3-rc5)
set(NCBI_ThirdParty_FreeType  ${NCBI_OPT_ROOT})
set(NCBI_ThirdParty_NGHTTP2   ${NCBI_ThirdPartyBasePath}/nghttp2-1.40.0)
set(NCBI_ThirdParty_UV        ${NCBI_ThirdPartyBasePath}/libuv-1.35.0)
set(NCBI_ThirdParty_GL2PS     ${NCBI_ThirdPartyBasePath}/gl2ps-1.4.0)
set(NCBI_ThirdParty_Nettle    ${NCBI_ThirdPartyBasePath}/nettle-3.1.1)
set(NCBI_ThirdParty_GMP       ${NCBI_ThirdPartyBasePath}/gmp-6.0.0a)

#############################################################################
#############################################################################

function(NCBI_define_component _name)

    if(NCBI_COMPONENT_${_name}_DISABLED)
        message("DISABLED ${_name}")
        return()
    endif()
# root
    set(_root "")
    if (DEFINED NCBI_ThirdParty_${_name})
        set(_root ${NCBI_ThirdParty_${_name}})
    else()
        string(FIND ${_name} "." dotfound)
        string(SUBSTRING ${_name} 0 ${dotfound} _dotname)
        if (DEFINED NCBI_ThirdParty_${_dotname})
            set(_root ${NCBI_ThirdParty_${_dotname}})
        else()
            message("NOT FOUND ${_name}: NCBI_ThirdParty_${_name} not found")
            return()
        endif()
    endif()

    if (EXISTS ${_root}/include)
        set(NCBI_COMPONENT_${_name}_INCLUDE ${_root}/include PARENT_SCOPE)
    else()
        message("NOT FOUND ${_name}: ${_root}/include not found")
        return()
    endif()

# libraries
    set(_args ${ARGN})
    if(BUILD_SHARED_LIBS)
        set(_suffixes .dylib .a)
    else()
        set(_suffixes .a .dylib)
    endif()
    set(_roots ${_root})
    set(_subdirs Release${NCBI_PlatformBits}/lib lib64 lib)
#    set(_subdirs Release${NCBI_PlatformBits}/lib lib64 ${_XCODE_EXTRA_LIBS})
    if (BUILD_SHARED_LIBS AND DEFINED NCBI_ThirdParty_${_name}_SHLIB)
        set(_roots ${NCBI_ThirdParty_${_name}_SHLIB} ${_roots})
        set(_subdirs shlib64 shlib lib64 lib)
    endif()

    set(_all_found YES)
    set(_all_libs "")
    foreach(_root IN LISTS _roots)
        foreach(_libdir IN LISTS _subdirs)
            set(_all_found YES)
            set(_all_libs "")
            foreach(_lib IN LISTS _args)
                set(_this_found NO)
                if(NCBI_TRACE_COMPONENT_${_name})
                    message("${_name}: checking ${_root}/${_libdir}/lib${_lib}")
                endif()
                foreach(_sfx IN LISTS _suffixes)
                    if(EXISTS ${_root}/${_libdir}/lib${_lib}${_sfx})
                        list(APPEND _all_libs ${_root}/${_libdir}/lib${_lib}${_sfx})
                        set(_this_found YES)
                        break()
                    else()
                        if(NCBI_TRACE_COMPONENT_${_name})
                            message("${_name}: ${_root}/${_libdir}/lib${_lib}${_sfx} not found")
                        endif()
                    endif()
                endforeach()
                if(NOT _this_found)
                    set(_all_found NO)
                    break()
                endif()
            endforeach()
            if(_all_found)
                break()
            endif()
        endforeach()
        if(_all_found)
            break()
        endif()
    endforeach()

    if(_all_found)
        message(STATUS "Found ${_name}: ${_root}")
        set(NCBI_COMPONENT_${_name}_FOUND YES PARENT_SCOPE)
#        set(NCBI_COMPONENT_${_name}_INCLUDE ${_root}/include)
        set(NCBI_COMPONENT_${_name}_LIBS ${_all_libs} PARENT_SCOPE)

        string(TOUPPER ${_name} _upname)
        set(HAVE_LIB${_upname} 1 PARENT_SCOPE)
        string(REPLACE "." "_" _altname ${_upname})
        set(HAVE_${_altname} 1 PARENT_SCOPE)

        list(APPEND NCBI_ALL_COMPONENTS ${_name})
        set(NCBI_ALL_COMPONENTS ${NCBI_ALL_COMPONENTS} PARENT_SCOPE)
    else()
        set(NCBI_COMPONENT_${_name}_FOUND NO)
        message("NOT FOUND ${_name}: some libraries not found at ${_root}")
    endif()

endfunction()

#############################################################################
macro(NCBI_find_library _name)
    if(NOT NCBI_COMPONENT_${_name}_DISABLED)
        set(_args ${ARGN})
        find_library(NCBI_COMPONENT_${_name}_LIBS ${_args})
        if(NCBI_COMPONENT_${_name}_LIBS)
            set(NCBI_COMPONENT_${_name}_FOUND YES)
            list(APPEND NCBI_ALL_COMPONENTS ${_name})
            message(STATUS "Found ${_name}: ${NCBI_COMPONENT_${_name}_LIBS}")

            string(TOUPPER ${_name} _upname)
            set(HAVE_LIB${_upname} 1)
            set(HAVE_${_upname} 1)
        else()
            set(NCBI_COMPONENT_${_name}_FOUND NO)
            message("NOT FOUND ${_name}")
        endif()
    else()
        message("DISABLED ${_name}")
    endif()
endmacro()

#############################################################################
# NCBI_C
set(NCBI_COMPONENT_NCBI_C_FOUND NO)

#############################################################################
# BACKWARD, UNWIND
if(NOT NCBI_COMPONENT_BACKWARD_DISABLED)
    if(EXISTS ${NCBI_ThirdParty_BACKWARD}/include)
        set(LIBBACKWARD_INCLUDE ${NCBI_ThirdParty_BACKWARD}/include)
        set(HAVE_LIBBACKWARD_CPP YES)
        set(NCBI_COMPONENT_BACKWARD_FOUND YES)
        set(NCBI_COMPONENT_BACKWARD_INCLUDE ${LIBBACKWARD_INCLUDE})
        list(APPEND NCBI_ALL_COMPONENTS BACKWARD)
    else()
        message("NOT FOUND BACKWARD")
    endif()
else(NOT NCBI_COMPONENT_BACKWARD_DISABLED)
    message("DISABLED BACKWARD")
endif(NOT NCBI_COMPONENT_BACKWARD_DISABLED)

#############################################################################
#LMDB
NCBI_define_component(LMDB lmdb)
if(NOT NCBI_COMPONENT_LMDB_FOUND)
  set(NCBI_COMPONENT_LMDB_FOUND ${NCBI_COMPONENT_LocalLMDB_FOUND})
  set(NCBI_COMPONENT_LMDB_INCLUDE ${NCBI_COMPONENT_LocalLMDB_INCLUDE})
  set(NCBI_COMPONENT_LMDB_NCBILIB ${NCBI_COMPONENT_LocalLMDB_NCBILIB})
endif()

#############################################################################
# PCRE
if(NOT NCBI_COMPONENT_PCRE_FOUND)
  set(NCBI_COMPONENT_PCRE_FOUND ${NCBI_COMPONENT_LocalPCRE_FOUND})
  set(NCBI_COMPONENT_PCRE_INCLUDE ${NCBI_COMPONENT_LocalPCRE_INCLUDE})
  set(NCBI_COMPONENT_PCRE_NCBILIB ${NCBI_COMPONENT_LocalPCRE_NCBILIB})
  list(APPEND NCBI_ALL_COMPONENTS PCRE)
endif()

#############################################################################
# Z
set(NCBI_COMPONENT_Z_FOUND YES)
set(NCBI_COMPONENT_Z_LIBS -lz)
list(APPEND NCBI_ALL_COMPONENTS Z)

#############################################################################
#BZ2
set(NCBI_COMPONENT_BZ2_FOUND YES)
set(NCBI_COMPONENT_BZ2_LIBS -lbz2)
list(APPEND NCBI_ALL_COMPONENTS BZ2)

#############################################################################
# LZO
NCBI_define_component(LZO lzo2)

#############################################################################
# Boost.Test.Included
if (EXISTS ${NCBI_ThirdParty_Boost}/include)
  message(STATUS "Found Boost.Test.Included: ${NCBI_ThirdParty_Boost}")
  set(NCBI_COMPONENT_Boost.Test.Included_FOUND YES)
  set(NCBI_COMPONENT_Boost.Test.Included_INCLUDE ${NCBI_ThirdParty_Boost}/include)
  list(APPEND NCBI_ALL_COMPONENTS Boost.Test.Included)
else()
  message("Component Boost.Test.Included ERROR: ${NCBI_ThirdParty_Boost}/include not found")
  set(NCBI_COMPONENT_Boost.Test.Included_FOUND NO)
endif()

#############################################################################
# Boost.Test
NCBI_define_component(Boost.Test boost_unit_test_framework)

#############################################################################
# Boost.Spirit
NCBI_define_component(Boost.Spirit boost_thread-mt)

#############################################################################
# JPEG
NCBI_define_component(JPEG jpeg)

#############################################################################
# PNG
NCBI_define_component(PNG png)

#############################################################################
# GIF
set(NCBI_COMPONENT_GIF_FOUND YES)
list(APPEND NCBI_ALL_COMPONENTS GIF)

#############################################################################
# TIFF
NCBI_define_component(TIFF tiff)

#############################################################################
# TLS
if (EXISTS ${NCBI_ThirdParty_TLS}/include)
  message(STATUS "Found TLS: ${NCBI_ThirdParty_TLS}")
  set(NCBI_COMPONENT_TLS_FOUND YES)
  set(NCBI_COMPONENT_TLS_INCLUDE ${NCBI_ThirdParty_TLS}/include)
  list(APPEND NCBI_ALL_COMPONENTS TLS)
else()
  message("Component TLS ERROR: ${NCBI_ThirdParty_TLS}/include not found")
  set(NCBI_COMPONENT_TLS_FOUND NO)
endif()

#############################################################################
# FASTCGI
set(NCBI_COMPONENT_FASTCGI_FOUND NO)

#############################################################################
# SQLITE3
NCBI_define_component(SQLITE3 sqlite3)

#############################################################################
#BerkeleyDB
NCBI_define_component(BerkeleyDB db)
if(NCBI_COMPONENT_BerkeleyDB_FOUND)
  set(HAVE_BERKELEY_DB 1)
  set(HAVE_BDB         1)
  set(HAVE_BDB_CACHE   1)
endif()

#############################################################################
# ODBC
set(NCBI_COMPONENT_ODBC_FOUND NO)
set(ODBC_INCLUDE  ${NCBI_INC_ROOT}/dbapi/driver/odbc/unix_odbc 
                  ${NCBI_INC_ROOT}/dbapi/driver/odbc/unix_odbc)
set(NCBI_COMPONENT_ODBC_INCLUDE ${ODBC_INCLUDE})
set(HAVE_ODBC 0)
set(HAVE_ODBCSS_H 0)

#############################################################################
# MySQL
set(NCBI_COMPONENT_MySQL_FOUND NO)

#############################################################################
# Sybase
set(NCBI_COMPONENT_Sybase_FOUND NO)

#############################################################################
# PYTHON
set(NCBI_COMPONENT_PYTHON_FOUND NO)

#############################################################################
# VDB
if(NOT NCBI_COMPONENT_VDB_DISABLED)
set(NCBI_COMPONENT_VDB_INCLUDE
  ${NCBI_ThirdParty_VDB}/interfaces
  ${NCBI_ThirdParty_VDB}/interfaces/cc/gcc/${NCBI_ThirdParty_VDB_ARCH}
  ${NCBI_ThirdParty_VDB}/interfaces/cc/gcc
  ${NCBI_ThirdParty_VDB}/interfaces/os/mac
  ${NCBI_ThirdParty_VDB}/interfaces/os/unix)
set(NCBI_COMPONENT_VDB_LIBS
  ${NCBI_ThirdParty_VDB}/mac/release/${NCBI_ThirdParty_VDB_ARCH}/lib/libncbi-vdb.a)

set(_found YES)
foreach(_inc IN LISTS NCBI_COMPONENT_VDB_INCLUDE NCBI_COMPONENT_VDB_LIBS)
  if(NOT EXISTS ${_inc})
    message("Component VDB ERROR: ${_inc} not found")
    set(_found NO)
  endif()
endforeach()
if(_found)
  message(STATUS "Found VDB: ${NCBI_ThirdParty_VDB}")
  set(NCBI_COMPONENT_VDB_FOUND YES)
  set(HAVE_NCBI_VDB 1)
  list(APPEND NCBI_ALL_COMPONENTS VDB)
else()
  set(NCBI_COMPONENT_VDB_FOUND NO)
  unset(NCBI_COMPONENT_VDB_INCLUDE)
  unset(NCBI_COMPONENT_VDB_LIBS)
endif()
else(NOT NCBI_COMPONENT_VDB_DISABLED)
    message("DISABLED VDB")
endif(NOT NCBI_COMPONENT_VDB_DISABLED)

#############################################################################
# XML
NCBI_define_component(XML xml2)
if(NCBI_COMPONENT_XML_FOUND)
  set(NCBI_COMPONENT_XML_INCLUDE ${NCBI_ThirdParty_XML}/include/libxml2)
  set(NCBI_COMPONENT_XML_LIBS ${NCBI_COMPONENT_XML_LIBS} -liconv)
endif()

#############################################################################
# XSLT
NCBI_define_component(XSLT exslt xslt)

#############################################################################
# EXSLT
NCBI_define_component(EXSLT exslt)

#############################################################################
# LAPACK
if(NOT NCBI_COMPONENT_LAPACK_DISABLED)
set(NCBI_COMPONENT_LAPACK_FOUND YES)
set(NCBI_COMPONENT_LAPACK_LIBS -llapack)
list(APPEND NCBI_ALL_COMPONENTS LAPACK)
else(NOT NCBI_COMPONENT_LAPACK_DISABLED)
    message("DISABLED LAPACK")
endif(NOT NCBI_COMPONENT_LAPACK_DISABLED)

#############################################################################
# wxWidgets
NCBI_define_component(wxWidgets
    wx_osx_cocoa_gl-3.1
    wx_osx_cocoa_richtext-3.1
    wx_osx_cocoa_aui-3.1
    wx_osx_cocoa_propgrid-3.1
    wx_osx_cocoa_xrc-3.1
    wx_osx_cocoa_qa-3.1
    wx_osx_cocoa_html-3.1
    wx_osx_cocoa_adv-3.1
    wx_osx_cocoa_core-3.1
    wx_base_xml-3.1
    wx_base_net-3.1
    wx_base-3.1
)
if(NCBI_COMPONENT_wxWidgets_FOUND)
    list(GET NCBI_COMPONENT_wxWidgets_LIBS 0 _lib)
    get_filename_component(_libdir ${_lib} DIRECTORY)
    set(NCBI_COMPONENT_wxWidgets_INCLUDE ${NCBI_COMPONENT_wxWidgets_INCLUDE}/wx-3.1 ${_libdir}/wx/include/osx_cocoa-ansi-3.1)
    set(NCBI_COMPONENT_wxWidgets_LIBS    ${NCBI_COMPONENT_wxWidgets_LIBS}  "-framework Cocoa")
    set(NCBI_COMPONENT_wxWidgets_DEFINES __WXMAC__ __WXOSX__ __WXOSX_COCOA__ wxDEBUG_LEVEL=0)
endif()

#############################################################################
# GLEW
NCBI_define_component(GLEW GLEW)

#############################################################################
# OpenGL
if(NOT NCBI_COMPONENT_OpenGL_DISABLED)
set(NCBI_COMPONENT_OpenGL_FOUND YES)
set(NCBI_COMPONENT_OpenGL_LIBS "-framework AGL -framework OpenGL -framework Metal -framework MetalKit")
list(APPEND NCBI_ALL_COMPONENTS OpenGL)
else(NOT NCBI_COMPONENT_OpenGL_DISABLED)
    message("DISABLED OpenGL")
endif(NOT NCBI_COMPONENT_OpenGL_DISABLED)

#############################################################################
# FTGL
NCBI_define_component(FTGL ftgl)

#############################################################################
# FreeType
NCBI_define_component(FreeType freetype)
if(NCBI_COMPONENT_FreeType_FOUND)
    set(NCBI_COMPONENT_FreeType_INCLUDE ${NCBI_COMPONENT_FreeType_INCLUDE} ${NCBI_COMPONENT_FreeType_INCLUDE}/freetype2)
endif()

#############################################################################
# NGHTTP2
NCBI_define_component(NGHTTP2 nghttp2)

#############################################################################
# UV
NCBI_define_component(UV uv)

#############################################################################
# GL2PS
NCBI_define_component(GL2PS gl2ps)

#############################################################################
# Nettle
NCBI_define_component(Nettle nettle hogweed)

#############################################################################
# GMP
#NCBI_define_component(GMP gmp)
