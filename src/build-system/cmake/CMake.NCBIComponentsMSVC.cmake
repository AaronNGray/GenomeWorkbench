#############################################################################
# $Id: CMake.NCBIComponentsMSVC.cmake 609371 2020-06-01 14:13:18Z ivanov $
#############################################################################

##
## NCBI CMake components description - MSVC
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX
##  HAVE_XXX


set(NCBI_REQUIRE_MSWin_FOUND YES)
#to debug
#set(NCBI_TRACE_COMPONENT_GRPC ON)
#############################################################################
# common settings
set(NCBI_TOOLS_ROOT $ENV{NCBI})
string(REPLACE "\\" "/" NCBI_TOOLS_ROOT ${NCBI_TOOLS_ROOT})

set(NCBI_PlatformBits 64)
if("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 15 2017 Win64")
    set(NCBI_ThirdPartyCompiler vs2017.64)
elseif("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 15 2017")
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64")
        set(NCBI_ThirdPartyCompiler vs2017.64)
    else()
        set(NCBI_ThirdPartyCompiler vs2017)
        set(NCBI_PlatformBits 32)
        message(FATAL_ERROR "32 bit configurations not supported")
    endif()
else()
#    message(WARNING "Generator ${CMAKE_GENERATOR} not tested")
    set(NCBI_ThirdPartyCompiler vs2017.64)
endif()

set(NCBI_ThirdPartyBasePath ${NCBI_TOOLS_ROOT}/Lib/ThirdParty)
set(NCBI_ThirdPartyAppsPath ${NCBI_TOOLS_ROOT}/App/ThirdParty)
set(NCBI_ThirdParty_NCBI_C  ${NCBI_TOOLS_ROOT}/Lib/Ncbi/C/${NCBI_ThirdPartyCompiler}/c.current)
set(NCBI_ThirdParty_VDBROOT //snowman/trace_software/vdb)


if (${NCBI_ThirdPartyCompiler} STREQUAL "vs2017.64" OR ${NCBI_ThirdPartyCompiler} STREQUAL "vs2017")
    set(NCBI_ThirdParty_TLS        ${NCBI_ThirdPartyBasePath}/gnutls/${NCBI_ThirdPartyCompiler}/3.4.9)
    set(NCBI_ThirdParty_FASTCGI    ${NCBI_ThirdPartyBasePath}/fastcgi/${NCBI_ThirdPartyCompiler}/2.4.1)
    set(NCBI_ThirdParty_Boost      ${NCBI_ThirdPartyBasePath}/boost/${NCBI_ThirdPartyCompiler}/1.66.0)
    set(NCBI_ThirdParty_PCRE       ${NCBI_ThirdPartyBasePath}/pcre/${NCBI_ThirdPartyCompiler}/7.9)
    set(NCBI_ThirdParty_Z          ${NCBI_ThirdPartyBasePath}/z/${NCBI_ThirdPartyCompiler}/1.2.8)
    set(NCBI_ThirdParty_BZ2        ${NCBI_ThirdPartyBasePath}/bzip2/${NCBI_ThirdPartyCompiler}/1.0.6)
    set(NCBI_ThirdParty_LZO        ${NCBI_ThirdPartyBasePath}/lzo/${NCBI_ThirdPartyCompiler}/2.10)
    set(NCBI_ThirdParty_BerkeleyDB ${NCBI_ThirdPartyBasePath}/berkeleydb/${NCBI_ThirdPartyCompiler}/4.6.21.NC)
    set(NCBI_ThirdParty_LMDB       ${NCBI_ThirdPartyBasePath}/lmdb/${NCBI_ThirdPartyCompiler}/0.9.24)
    set(NCBI_ThirdParty_JPEG       ${NCBI_ThirdPartyBasePath}/jpeg/${NCBI_ThirdPartyCompiler}/9c)
    set(NCBI_ThirdParty_PNG        ${NCBI_ThirdPartyBasePath}/png/${NCBI_ThirdPartyCompiler}/1.6.34)
    set(NCBI_ThirdParty_GIF        ${NCBI_ThirdPartyBasePath}/gif/${NCBI_ThirdPartyCompiler}/4.1.3)
    set(NCBI_ThirdParty_TIFF       ${NCBI_ThirdPartyBasePath}/tiff/${NCBI_ThirdPartyCompiler}/3.6.1)
    set(NCBI_ThirdParty_XML        ${NCBI_ThirdPartyBasePath}/xml/${NCBI_ThirdPartyCompiler}/2.7.8)
    set(NCBI_ThirdParty_XSLT       ${NCBI_ThirdPartyBasePath}/xslt/${NCBI_ThirdPartyCompiler}/1.1.26)
    set(NCBI_ThirdParty_EXSLT      ${NCBI_ThirdParty_XSLT})
    set(NCBI_ThirdParty_SQLITE3    ${NCBI_ThirdPartyBasePath}/sqlite/${NCBI_ThirdPartyCompiler}/3.26.0)
    set(NCBI_ThirdParty_Sybase     ${NCBI_ThirdPartyBasePath}/sybase/${NCBI_ThirdPartyCompiler}/15.5)
    set(NCBI_ThirdParty_VDB        ${NCBI_ThirdParty_VDBROOT}/vdb-versions/cxx_toolkit/2.10)
        set(NCBI_ThirdParty_VDB_ARCH_INC x86_64)
        set(NCBI_ThirdParty_VDB_ARCH     x86_64/v141)

    set(NCBI_ThirdParty_PYTHON     ${NCBI_ThirdPartyAppsPath}/Python38)
    set(NCBI_ThirdParty_PROTOBUF   ${NCBI_ThirdPartyBasePath}/grpc/${NCBI_ThirdPartyCompiler}/1.28.1)
    set(NCBI_ThirdParty_GRPC       ${NCBI_ThirdPartyBasePath}/grpc/${NCBI_ThirdPartyCompiler}/1.28.1)
endif()

set(NCBI_ThirdParty_XALAN       ${NCBI_ThirdPartyBasePath}/xalan/${NCBI_ThirdPartyCompiler}/1.10.0-20080814)
set(NCBI_ThirdParty_XERCES      ${NCBI_ThirdPartyBasePath}/xerces/${NCBI_ThirdPartyCompiler}/2.8.0)
set(NCBI_ThirdParty_FTGL        ${NCBI_ThirdPartyBasePath}/ftgl/${NCBI_ThirdPartyCompiler}/2.1.3-rc5)
set(NCBI_ThirdParty_GLEW        ${NCBI_ThirdPartyBasePath}/glew/${NCBI_ThirdPartyCompiler}/1.5.8)
set(NCBI_ThirdParty_FreeType    ${NCBI_ThirdPartyBasePath}/freetype/${NCBI_ThirdPartyCompiler}/2.4.10)
set(NCBI_ThirdParty_wxWidgets   ${NCBI_ThirdPartyBasePath}/wxwidgets/${NCBI_ThirdPartyCompiler}/3.1.3-ncbi1)
set(NCBI_ThirdParty_UV          ${NCBI_ThirdPartyBasePath}/uv/${NCBI_ThirdPartyCompiler}/1.35.0)
set(NCBI_ThirdParty_NGHTTP2     ${NCBI_ThirdPartyBasePath}/nghttp2/${NCBI_ThirdPartyCompiler}/1.40.0)
set(NCBI_ThirdParty_GL2PS       ${NCBI_ThirdPartyBasePath}/gl2ps/${NCBI_ThirdPartyCompiler}/1.4.0)

#############################################################################
#############################################################################

function(NCBI_define_component _name)

    if(NCBI_COMPONENT_${_name}_DISABLED)
        message("DISABLED ${_name}")
        return()
    endif()
# root
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
# include dir
    if (EXISTS ${_root}/include)
        set(_found YES)
    else()
        message("NOT FOUND ${_name}: ${_root}/include not found")
        set(_found NO)
    endif()
# libraries
    set(_args ${ARGN})
    if (_found)
        if(BUILD_SHARED_LIBS)
            set(_locations lib_dll lib_static lib)
        else()
            set(_locations lib_static lib_dll lib)
        endif()
        set(_rt ${NCBI_CONFIGURATION_RUNTIMELIB})

        foreach(_libdir IN LISTS _locations)
            set(_found YES)
            foreach(_cfg ${NCBI_CONFIGURATION_TYPES})
                if(NCBI_TRACE_COMPONENT_${_name})
                    message("${_name}: checking ${_root}/${_libdir}/${_cfg}${_rt}")
                endif()
                foreach(_lib IN LISTS _args)
                    if(NOT EXISTS ${_root}/${_libdir}/${_cfg}${_rt}/${_lib})
                        if(NCBI_TRACE_COMPONENT_${_name})
                            message("${_name}: ${_root}/${_libdir}/${_cfg}${_rt}/${_lib} not found")
                        endif()
                        set(_found NO)
                    endif()
                endforeach()
            endforeach()
            if (_found)
#                set(_libtype ${_libdir}/\$\(Configuration\))
                set(_libtype ${_libdir}/$<CONFIG>${_rt})
                break()
            endif()
        endforeach()

        if (NOT _found)
            set(_found YES)
            foreach(_cfg ${NCBI_CONFIGURATION_TYPES})
                if(NCBI_TRACE_COMPONENT_${_name})
                    message("${_name}: checking ${_root}/${_cfg}${_rt}")
                endif()
                foreach(_lib IN LISTS _args)
                    if(NOT EXISTS ${_root}/${_cfg}${_rt}/${_lib})
                        if(NCBI_TRACE_COMPONENT_${_name})
                            message("${_name}: ${_root}/${_cfg}${_rt}/${_lib} not found")
                        endif()
                        set(_found NO)
                    endif()
                endforeach()
            endforeach()
            if (_found)
#                set(_libtype \$\(Configuration\))
                set(_libtype $<CONFIG>${_rt})
            endif()
        endif()

        if (NOT _found)
            set(_locations lib libs)
            foreach(_libdir IN LISTS _locations)
                set(_found YES)
                if(NCBI_TRACE_COMPONENT_${_name})
                    message("${_name}: checking ${_root}/${_libdir}")
                endif()
                foreach(_lib IN LISTS _args)
                    if(NOT EXISTS ${_root}/${_libdir}/${_lib})
                        if(NCBI_TRACE_COMPONENT_${_name})
                            message("${_name}: ${_root}/${_libdir}/${_lib} not found")
                        endif()
                        set(_found NO)
                    endif()
                endforeach()
                if (_found)
                    set(_libtype ${_libdir})
                    break()
                endif()
            endforeach()
        endif()

        if (NOT _found)
            message("NOT FOUND ${_name}: some libraries not found at ${_root}")
        endif()
    endif()

    if (_found)
        message(STATUS "Found ${_name}: ${_root}")
        set(NCBI_COMPONENT_${_name}_FOUND YES PARENT_SCOPE)
        set(NCBI_COMPONENT_${_name}_INCLUDE ${_root}/include PARENT_SCOPE)
        foreach(_lib IN LISTS _args)
            set(NCBI_COMPONENT_${_name}_LIBS ${NCBI_COMPONENT_${_name}_LIBS} ${_root}/${_libtype}/${_lib})
        endforeach()
        set(NCBI_COMPONENT_${_name}_LIBS ${NCBI_COMPONENT_${_name}_LIBS} PARENT_SCOPE)
        if (EXISTS ${_root}/bin)
            set(NCBI_COMPONENT_${_name}_BINPATH ${_root}/bin PARENT_SCOPE)
        endif()

        string(TOUPPER ${_name} _upname)
        set(HAVE_LIB${_upname} 1 PARENT_SCOPE)
        string(REPLACE "." "_" _altname ${_upname})
        set(HAVE_${_altname} 1 PARENT_SCOPE)

        list(APPEND NCBI_ALL_COMPONENTS ${_name})
        set(NCBI_ALL_COMPONENTS ${NCBI_ALL_COMPONENTS} PARENT_SCOPE)
    else()
        set(NCBI_COMPONENT_${_name}_FOUND NO)
    endif()
endfunction()

#############################################################################
#############################################################################

#############################################################################
# NCBI_C
if(OFF)
  set(_c_libs  blast ddvlib medarch ncbi ncbiacc ncbicdr
               ncbicn3d ncbicn3d_ogl ncbidesk ncbiid1
               ncbimain ncbimmdb ncbinacc ncbiobj ncbispel
               ncbitool ncbitxc2 netblast netcli netentr
               regexp smartnet vibgif vibnet vibrant
               vibrant_ogl)
endif()
NCBI_define_component(NCBI_C ncbiobj.lib ncbimmdb.lib ncbi.lib)

##############################################################################
# UUID
set(NCBI_COMPONENT_UUID_FOUND YES)
set(NCBI_COMPONENT_UUID_LIBS uuid.lib rpcrt4.lib)

#############################################################################
# MySQL
set(NCBI_COMPONENT_MySQL_FOUND NO)

#############################################################################
# ODBC
if(NOT NCBI_COMPONENT_ODBC_DISABLED)
    set(NCBI_COMPONENT_ODBC_FOUND YES)
    set(NCBI_COMPONENT_ODBC_LIBS odbc32.lib odbccp32.lib odbcbcp.lib)
    set(HAVE_ODBC 1)
    set(HAVE_ODBCSS_H 1)
    list(APPEND NCBI_ALL_COMPONENTS ODBC)
else()
    message("DISABLED ODBC")
endif()

##############################################################################
# OpenGL
if(NOT NCBI_COMPONENT_OpenGL_DISABLED)
    set(NCBI_COMPONENT_OpenGL_FOUND YES)
    set(NCBI_COMPONENT_OpenGL_LIBS opengl32.lib glu32.lib)
    set(HAVE_OPENGL 1)
    list(APPEND NCBI_ALL_COMPONENTS OpenGL)
else()
    message("DISABLED OpenGL")
endif()

#############################################################################
# LMDB
NCBI_define_component(LMDB liblmdb.lib)
if(NOT NCBI_COMPONENT_LMDB_FOUND)
    set(NCBI_COMPONENT_LMDB_FOUND ${NCBI_COMPONENT_LocalLMDB_FOUND})
    set(NCBI_COMPONENT_LMDB_INCLUDE ${NCBI_COMPONENT_LocalLMDB_INCLUDE})
    set(NCBI_COMPONENT_LMDB_NCBILIB ${NCBI_COMPONENT_LocalLMDB_NCBILIB})
endif()

#############################################################################
# PCRE
NCBI_define_component(PCRE libpcre.lib)
if(NOT NCBI_COMPONENT_PCRE_FOUND)
    set(NCBI_COMPONENT_PCRE_FOUND ${NCBI_COMPONENT_LocalPCRE_FOUND})
    set(NCBI_COMPONENT_PCRE_INCLUDE ${NCBI_COMPONENT_LocalPCRE_INCLUDE})
    set(NCBI_COMPONENT_PCRE_NCBILIB ${NCBI_COMPONENT_LocalPCRE_NCBILIB})
endif()

#############################################################################
# Z
NCBI_define_component(Z libz.lib)
if(NOT NCBI_COMPONENT_Z_FOUND)
    set(NCBI_COMPONENT_Z_FOUND ${NCBI_COMPONENT_LocalZ_FOUND})
    set(NCBI_COMPONENT_Z_INCLUDE ${NCBI_COMPONENT_LocalZ_INCLUDE})
    set(NCBI_COMPONENT_Z_NCBILIB ${NCBI_COMPONENT_LocalZ_NCBILIB})
endif()

#############################################################################
# BZ2
NCBI_define_component(BZ2 libbzip2.lib)
if(NOT NCBI_COMPONENT_BZ2_FOUND)
    set(NCBI_COMPONENT_BZ2_FOUND ${NCBI_COMPONENT_LocalBZ2_FOUND})
    set(NCBI_COMPONENT_BZ2_INCLUDE ${NCBI_COMPONENT_LocalBZ2_INCLUDE})
    set(NCBI_COMPONENT_BZ2_NCBILIB ${NCBI_COMPONENT_LocalBZ2_NCBILIB})
endif()

#############################################################################
# LZO
NCBI_define_component(LZO liblzo.lib)

#############################################################################
# Boost.Test.Included
NCBI_define_component(Boost.Test.Included)
if(NCBI_COMPONENT_Boost.Test.Included_FOUND)
    set(NCBI_COMPONENT_Boost.Test.Included_DEFINES BOOST_TEST_NO_LIB)
endif()

#############################################################################
# Boost.Test
NCBI_define_component(Boost.Test libboost_unit_test_framework.lib)
if(NCBI_COMPONENT_Boost.Test_FOUND)
    set(NCBI_COMPONENT_Boost.Test_DEFINES BOOST_AUTO_LINK_NOMANGLE)
endif()

#############################################################################
# Boost.Spirit
NCBI_define_component(Boost.Spirit libboost_thread.lib boost_thread.lib boost_system.lib boost_date_time.lib boost_chrono.lib)
if(NCBI_COMPONENT_Boost.Spirit_FOUND)
    set(NCBI_COMPONENT_Boost.Spirit_DEFINES BOOST_AUTO_LINK_NOMANGLE)
endif()

#############################################################################
# Boost
NCBI_define_component(Boost boost_filesystem.lib boost_iostreams.lib boost_date_time.lib boost_regex.lib  boost_system.lib)

#############################################################################
# JPEG
NCBI_define_component(JPEG libjpeg.lib)

#############################################################################
# PNG
NCBI_define_component(PNG libpng.lib)

#############################################################################
# GIF
NCBI_define_component(GIF libgif.lib)

#############################################################################
# TIFF
NCBI_define_component(TIFF libtiff.lib)

#############################################################################
# TLS
NCBI_define_component(TLS)

#############################################################################
# FASTCGI
NCBI_define_component(FASTCGI libfcgi.lib)

#############################################################################
# BerkeleyDB
NCBI_define_component(BerkeleyDB libdb.lib)
if(NCBI_COMPONENT_BerkeleyDB_FOUND)
    set(HAVE_BERKELEY_DB 1)
    set(HAVE_BDB         1)
    set(HAVE_BDB_CACHE   1)
endif()

#############################################################################
# XML
NCBI_define_component(XML libxml2.lib)
if (NCBI_COMPONENT_XML_FOUND)
    if(NOT BUILD_SHARED_LIBS)
        set (NCBI_COMPONENT_XML_DEFINES LIBXML_STATIC)
    endif()
endif()

#############################################################################
# XSLT
NCBI_define_component(XSLT libexslt.lib libxslt.lib)

#############################################################################
# EXSLT
NCBI_define_component(EXSLT libexslt.lib)

#############################################################################
# SQLITE3
NCBI_define_component(SQLITE3 sqlite3.lib)

#############################################################################
# LAPACK
set(NCBI_COMPONENT_LAPACK_FOUND NO)

#############################################################################
# Sybase
NCBI_define_component(Sybase libsybdb.lib libsybct.lib libsybblk.lib libsybcs.lib)
if (NCBI_COMPONENT_Sybase_FOUND)
    set(SYBASE_PATH ${NCBI_ThirdParty_Sybase}/Sybase)
    set(SYBASE_LCL_PATH "")
endif()

#############################################################################
# VDB
if(NOT NCBI_COMPONENT_VDB_DISABLED)
set(NCBI_COMPONENT_VDB_INCLUDE
    ${NCBI_ThirdParty_VDB}/interfaces
    ${NCBI_ThirdParty_VDB}/interfaces/cc/vc++/${NCBI_ThirdParty_VDB_ARCH_INC}
    ${NCBI_ThirdParty_VDB}/interfaces/cc/vc++
    ${NCBI_ThirdParty_VDB}/interfaces/os/win)
set(NCBI_COMPONENT_VDB_BINPATH
    ${NCBI_ThirdParty_VDB}/win/release/${NCBI_ThirdParty_VDB_ARCH}/bin)
set(NCBI_COMPONENT_VDB_LIBS
    ${NCBI_COMPONENT_VDB_BINPATH}/ncbi-vdb-md.lib)

set(_found YES)
foreach(_inc IN LISTS NCBI_COMPONENT_VDB_INCLUDE NCBI_COMPONENT_VDB_LIBS)
    if(NOT EXISTS ${_inc})
        message("NOT FOUND VDB: ${_inc} not found")
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
# PYTHON
NCBI_define_component(PYTHON python38.lib python3.lib)
if(NCBI_COMPONENT_PYTHON_FOUND)
    set(NCBI_COMPONENT_PYTHON_BINPATH ${NCBI_ThirdParty_PYTHON})
endif()

##############################################################################
# GRPC/PROTOBUF
set(NCBI_PROTOC_APP "${NCBI_ThirdParty_GRPC}/bin/ReleaseDLL/protoc.exe")
set(NCBI_GRPC_PLUGIN "${NCBI_ThirdParty_GRPC}/bin/ReleaseDLL/grpc_cpp_plugin.exe")
if(NOT EXISTS "${NCBI_PROTOC_APP}")
    message("NOT FOUND: ${NCBI_PROTOC_APP}")
else()
NCBI_define_component(PROTOBUF libprotobuf.lib)
NCBI_define_component(GRPC grpc++.lib grpc.lib gpr.lib address_sorting.lib cares.lib libprotobuf.lib
                           upb.lib crypto.lib ssl.lib absl_throw_delegate.lib absl_strings.lib
                           absl_bad_optional_access.lib  absl_str_format_internal.lib
                           absl_raw_logging_internal.lib absl_int128.lib)
if(NCBI_COMPONENT_GRPC_FOUND)
    set(NCBI_COMPONENT_GRPC_DEFINES _WIN32_WINNT=0x0600)
endif()
endif()

##############################################################################
# XALAN
NCBI_define_component(XALAN xalan-c.lib XalanMessages.lib)

##############################################################################
# XERCES
NCBI_define_component(XERCES xerces-c.lib)
if(NCBI_COMPONENT_XERCES_FOUND)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_XERCES_DEFINES XERCES_DLL)
    else()
        set(NCBI_COMPONENT_XERCES_DEFINES XML_LIBRARY)
    endif()
endif()

##############################################################################
# FTGL
NCBI_define_component(FTGL ftgl.lib)
if(NCBI_COMPONENT_FTGL_FOUND)
    set(NCBI_COMPONENT_FTGL_DEFINES FTGL_LIBRARY_STATIC)
endif()

##############################################################################
# FreeType
NCBI_define_component(FreeType freetype.lib)

##############################################################################
# GLEW
NCBI_define_component(GLEW glew32mx.lib)
if(NCBI_COMPONENT_GLEW_FOUND)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_GLEW_DEFINES GLEW_MX)
    else()
        set(NCBI_COMPONENT_GLEW_DEFINES GLEW_MX GLEW_STATIC)
    endif()
endif()

##############################################################################
# wxWidgets
NCBI_define_component( wxWidgets
        wxbase.lib wxbase_net.lib wxbase_xml.lib wxmsw_core.lib wxmsw_gl.lib
        wxmsw_html.lib wxmsw_aui.lib wxmsw_adv.lib wxmsw_richtext.lib wxmsw_propgrid.lib
        wxmsw_xrc.lib wxexpat.lib wxjpeg.lib wxpng.lib wxregex.lib wxtiff.lib wxzlib.lib)
if(NCBI_COMPONENT_wxWidgets_FOUND)
    set(NCBI_COMPONENT_wxWidgets_INCLUDE ${NCBI_COMPONENT_wxWidgets_INCLUDE} ${NCBI_COMPONENT_wxWidgets_INCLUDE}/msvc)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXMSW__ NCBI_WXWIN_USE_PCH WXUSINGDLL=1)
    else()
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXMSW__ NCBI_WXWIN_USE_PCH)
    endif()
endif()

##############################################################################
# UV
NCBI_define_component(UV libuv.lib)
if(NCBI_COMPONENT_UV_FOUND)
    set(NCBI_COMPONENT_UV_LIBS ${NCBI_COMPONENT_UV_LIBS} psapi.lib Iphlpapi.lib userenv.lib)
endif()

##############################################################################
# NGHTTP2
NCBI_define_component(NGHTTP2 nghttp2.lib)

##############################################################################
# GL2PS
NCBI_define_component(GL2PS gl2ps.lib)
