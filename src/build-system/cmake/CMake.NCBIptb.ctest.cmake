#############################################################################
# $Id: CMake.NCBIptb.ctest.cmake 607660 2020-05-06 12:49:19Z ivanov $
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper extension
##  In NCBI CMake wrapper, adds CMake tests (which use CMake testing framework)
##    Author: Andrei Gourianov, gouriano@ncbi
##


##############################################################################
# Testing
set(NCBITEST_DRIVER "${NCBI_TREE_CMAKECFG}/TestDriver.cmake")
NCBI_define_test_resource(ServiceMapper 8)
enable_testing()

if (WIN32)
    set(_knowndir "C:/Apps/Admin_Installs/Cygwin64/bin;C:/cygwin64/bin;$ENV{PATH}")
    string(REPLACE "\\" "/" _knowndir "${_knowndir}")
    foreach(_dir IN LISTS _knowndir)
        if (NOT "${_dir}" STREQUAL "" AND EXISTS "${_dir}/sh.exe")
            set(NCBI_REQUIRE_CygwinTest_FOUND YES)
            set(NCBI_CygwinTest_PATH "${_dir}")
            break()
        endif()
    endforeach()
endif()

##############################################################################
macro(NCBI_internal_process_cmake_test_requires _test)
    set(NCBITEST_REQUIRE_NOTFOUND "")
    set(_all ${NCBITEST__REQUIRES} ${NCBITEST_${_test}_REQUIRES})
    if (NOT "${_all}" STREQUAL "")
        list(REMOVE_DUPLICATES _all)
    endif()

    foreach(_req IN LISTS _all)
        NCBI_util_parse_sign( ${_req} _value _negate)
        if (${_value} OR NCBI_REQUIRE_${_value}_FOUND OR NCBI_COMPONENT_${_value}_FOUND)
            if (_negate)
                set(NCBITEST_REQUIRE_NOTFOUND ${NCBITEST_REQUIRE_NOTFOUND} ${_req})
            endif()
        else()
            if (NOT _negate)
                set(NCBITEST_REQUIRE_NOTFOUND ${NCBITEST_REQUIRE_NOTFOUND} ${_req})
            endif()
        endif()     
    endforeach()
endmacro()

##############################################################################
macro(NCBI_internal_process_cmake_test_resources _test)
    set(_all ${NCBITEST__RESOURCES} ${NCBITEST_${_test}_RESOURCES})
    if (NOT "${_all}" STREQUAL "")
        list(REMOVE_DUPLICATES _all)
    endif()

    foreach(_res IN LISTS _all)
        if(DEFINED NCBITEST_RESOURCE_${_res}_AMOUNT)
            get_property(_count  GLOBAL PROPERTY NCBITEST_RESOURCE_${_res}_COUNT)
            if(NCBI_VERBOSE_ALLPROJECTS OR NCBI_VERBOSE_PROJECT_${NCBI_PROJECT})
                message("${NCBI_PROJECT} (${NCBI_CURRENT_SOURCE_DIR}): Test ${_test} uses resource ${_res} (${NCBITEST_RESOURCE_${_res}_AMOUNT})")
            endif()
            set_tests_properties(${_test} PROPERTIES RESOURCE_LOCK "NCBITEST_RESOURCE_${_res}_${_count}")
            math(EXPR _count "${_count} + 1")
            if("${_count}" GREATER_EQUAL "${NCBITEST_RESOURCE_${_res}_AMOUNT}")
                set(_count 0)
            endif()
            set_property(GLOBAL PROPERTY NCBITEST_RESOURCE_${_res}_COUNT ${_count})
        else()
            if(NCBI_VERBOSE_ALLPROJECTS OR NCBI_VERBOSE_PROJECT_${NCBI_PROJECT})
                message("${NCBI_PROJECT} (${NCBI_CURRENT_SOURCE_DIR}): Test ${_test} uses resource ${_res} (1)")
            endif()
            set_tests_properties(${_test} PROPERTIES RESOURCE_LOCK "NCBITEST_RESOURCE_${_res}")
        endif()
    endforeach()
endmacro()

##############################################################################
function(NCBI_internal_add_cmake_test _test)
    if( NOT DEFINED NCBITEST_${_test}_CMD)
        set(NCBITEST_${_test}_CMD ${NCBI_${NCBI_PROJECT}_OUTPUT})
    endif()
    if (XCODE)
        set(_extra -DXCODE=TRUE)
    endif()
    get_filename_component(_ext ${NCBITEST_${_test}_CMD} EXT)
    if("${_ext}" STREQUAL ".sh" OR "${_ext}" STREQUAL ".bash")
        if (WIN32)
            set(NCBITEST_${_test}_REQUIRES ${NCBITEST_${_test}_REQUIRES} CygwinTest)
            if(NCBI_REQUIRE_CygwinTest_FOUND)
                set(_extra ${_extra} -DNCBITEST_CYGWIN=${NCBI_CygwinTest_PATH})
            endif()
        endif()
        if(EXISTS ${NCBI_CURRENT_SOURCE_DIR}/${NCBITEST_${_test}_CMD})
            set(NCBITEST_${_test}_ASSETS   ${NCBITEST_${_test}_ASSETS}   ${NCBITEST_${_test}_CMD})
        endif()
    endif()
    set(_assets ${NCBITEST__ASSETS} ${NCBITEST_${_test}_ASSETS})
    if (DEFINED NCBITEST_${_test}_TIMEOUT)
        set(_timeout ${NCBITEST_${_test}_TIMEOUT})
    elseif(DEFINED NCBITEST__TIMEOUT)
        set(_timeout ${NCBITEST__TIMEOUT})
    else()
        set(_timeout 1800)
    endif()
    string(REPLACE ";" " " _args    "${NCBITEST_${_test}_ARG}")
    string(REPLACE ";" " " _assets   "${_assets}")

    NCBI_internal_process_cmake_test_requires(${_test})
    if ( NOT "${NCBITEST_REQUIRE_NOTFOUND}" STREQUAL "")
        if(NCBI_VERBOSE_ALLPROJECTS OR NCBI_VERBOSE_PROJECT_${NCBI_PROJECT})
            message("${NCBI_PROJECT} (${NCBI_CURRENT_SOURCE_DIR}): Test ${_test} is excluded because of unmet requirements: ${NCBITEST_REQUIRE_NOTFOUND}")
        endif()
        return()
    endif()

    file(RELATIVE_PATH _xoutdir "${NCBI_SRC_ROOT}" "${NCBI_CURRENT_SOURCE_DIR}")
    if (WIN32 OR XCODE)
        set(_outdir ../${NCBI_DIRNAME_TESTING}/$<CONFIG>/${_xoutdir})
    else()
        set(_outdir ../${NCBI_DIRNAME_TESTING}/${_xoutdir})
    endif()

    add_test(NAME ${_test} COMMAND ${CMAKE_COMMAND}
        -DNCBITEST_NAME=${_test}
        -DNCBITEST_CONFIG=$<CONFIG>
        -DNCBITEST_COMMAND=${NCBITEST_${_test}_CMD}
        -DNCBITEST_ARGS=${_args}
        -DNCBITEST_TIMEOUT=${_timeout}
        -DNCBITEST_BINDIR=../${NCBI_DIRNAME_RUNTIME}
        -DNCBITEST_SOURCEDIR=../../${NCBI_DIRNAME_SRC}/${_xoutdir}
        -DNCBITEST_OUTDIR=${_outdir}
        -DNCBITEST_ASSETS=${_assets}
        ${_extra}
        -P "../../${NCBI_DIRNAME_CMAKECFG}/TestDriver.cmake"
        WORKING_DIRECTORY .
    )

    NCBI_internal_process_cmake_test_resources(${_test})
endfunction()

##############################################################################
function(NCBI_internal_AddCMakeTest _variable _access)
    if("${_access}" STREQUAL "MODIFIED_ACCESS" AND DEFINED NCBI_${NCBI_PROJECT}_ALLTESTS)
        foreach(_test IN LISTS NCBI_${NCBI_PROJECT}_ALLTESTS)
            NCBI_internal_add_cmake_test(${_test})
        endforeach()
    endif()
endfunction()

#############################################################################
function(NCBI_internal_FinalizeCMakeTest)
    file(MAKE_DIRECTORY ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_TESTING})
    file(MAKE_DIRECTORY ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/Testing/Temporary)
endfunction()

#############################################################################
NCBI_register_hook(TARGET_ADDED NCBI_internal_AddCMakeTest)
NCBI_register_hook(ALL_ADDED    NCBI_internal_FinalizeCMakeTest)
