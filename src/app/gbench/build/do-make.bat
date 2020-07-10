@echo off
REM $Id: do-make.bat 44671 2020-02-19 21:16:49Z evgeniev $
REM ===========================================================================
REM
REM                            PUBLIC DOMAIN NOTICE
REM               National Center for Biotechnology Information
REM
REM  This software/database is a "United States Government Work" under the
REM  terms of the United States Copyright Act.  It was written as part of
REM  the author's official duties as a United States Government employee and
REM  thus cannot be copyrighted.  This software/database is freely available
REM  to the public for use. The National Library of Medicine and the U.S.
REM  Government have not placed any restriction on its use or reproduction.
REM
REM  Although all reasonable efforts have been taken to ensure the accuracy
REM  and reliability of the software and data, the NLM and the U.S.
REM  Government do not and cannot warrant the performance or results that
REM  may be obtained by using this software or data. The NLM and the U.S.
REM  Government disclaim all warranties, express or implied, including
REM  warranties of performance, merchantability or fitness for any particular
REM  purpose.
REM
REM  Please cite the author in any work or product based on this material.
REM
REM ===========================================================================
REM
REM Author:  Yury Voronov
REM
REM Main script for configuring, building, installing Genome Workbench on Windows
REM as well as making installators for both 32 and 64 bit platforms.
REM
REM ===========================================================================

setlocal
setlocal enabledelayedexpansion

set installdir=.\install
set msvc=12

set help_req=no
set conf_req=no
set build_req=no
set rebuild_req=no
set install_req=no
set sdk_req=no
set instler_req=no
set all_req=no


set use_x64=no
set use_debug=no
set use_dll=yes
set use_unicode=no
set use_staticstd=no
set use_internal=no
set ignore_unknown=no


:SETUP
set script_path=src\app\gbench\build
set script_path_len=20

set "cmd=%~n0"
set "script_name=%0"
set "initial_dir=%CD%"
set "tree_root="

cd %~p0
for /f "delims=" %%a in ('cd') do (set "script_dir=%%a")
if "%script_dir:~-1%"=="\" (set "script_dir=%script_dir:~0,-1%")

call set "_subpath=%%script_dir:~-%script_path_len%%%"
if "%_subpath%" == "%script_path%" (
    set "tree_root=..\..\..\.."

) else if exist "%script_path%" (
    set "tree_root=."
)

if "%tree_root%" == "" goto TREE_ROOT_SET

    cd %tree_root%
    for /f "delims=" %%a in ('cd') do (set "tree_root=%%a")
    if "%tree_root:~-1%"=="\" (set "tree_root=%tree_root:~0,-1%")

:TREE_ROOT_SET

cd "%initial_dir%"


REM ###########################################################################
REM parse arguments

set unknown=
set ignore_unknown=no
set dest=

:PARSEARGS
if "%1"=="" goto ENDPARSEARGS
if "%dest%"=="src"                      (set rootdir=%~1&     set dest=& goto CONTINUEPARSEARGS)
if "%dest%"=="ins"                      (set installdir=%~1& set dest=& goto CONTINUEPARSEARGS)
if "%dest%"=="dst"                      (set dstdir=%~1&     set dest=& goto CONTINUEPARSEARGS)
if "%dest%"=="ver"                      (set msvc=%1&        set dest=& goto CONTINUEPARSEARGS)

if "%1"=="--rootdir"                    (set dest=src&           goto CONTINUEPARSEARGS)
if "%1"=="--installdir"                 (set dest=ins&           goto CONTINUEPARSEARGS)
if "%1"=="--dstdir"                     (set dest=dst&           goto CONTINUEPARSEARGS)
if "%1"=="--msvc"                       (set dest=ver&           goto CONTINUEPARSEARGS)

if "%1"=="--help"                       (set help_req=yes&       goto CONTINUEPARSEARGS)
if "%1"=="--configure"                  (set conf_req=yes&       goto CONTINUEPARSEARGS)
if "%1"=="--build"                      (set build_req=yes&      goto CONTINUEPARSEARGS)
if "%1"=="--rebuild"                    (set rebuild_req=yes&    goto CONTINUEPARSEARGS)
if "%1"=="--install"                    (set install_req=yes&    goto CONTINUEPARSEARGS)
if "%1"=="--with-installer"             (set instler_req=yes&    goto CONTINUEPARSEARGS)
if "%1"=="--with-sdk"                   (set sdk_req=yes&        goto CONTINUEPARSEARGS)
if "%1"=="--do-all"                     (set all_req=yes&        goto CONTINUEPARSEARGS)

if "%1"=="--x64"                        (set use_x64=yes&        goto CONTINUEPARSEARGS)
if "%1"=="--with-64"                    (set use_x64=yes&        goto CONTINUEPARSEARGS)
if "%1"=="--release"                    (set use_debug=no&       goto CONTINUEPARSEARGS)
if "%1"=="--without-debug"              (set use_debug=no&       goto CONTINUEPARSEARGS)
if "%1"=="--debug"                      (set use_debug=yes&      goto CONTINUEPARSEARGS)
if "%1"=="--with-debug"                 (set use_debug=yes&      goto CONTINUEPARSEARGS)
if "%1"=="--unicode"                    (set use_unicode=yes&      goto CONTINUEPARSEARGS)
if "%1"=="--with-unicode"               (set use_unicode=yes&      goto CONTINUEPARSEARGS)
if "%1"=="--static"                     (set use_dll=no&         goto CONTINUEPARSEARGS)
if "%1"=="--without-dll"                (set use_dll=no&         goto CONTINUEPARSEARGS)
if "%1"=="--dll"                        (set use_dll=yes&        goto CONTINUEPARSEARGS)
if "%1"=="--with-dll"                   (set use_dll=yes&        goto CONTINUEPARSEARGS)
if "%1"=="--with-static-exe"            (set use_staticstd=yes&  goto CONTINUEPARSEARGS)
if "%1"=="--with-internal"              (set use_internal=yes&   goto CONTINUEPARSEARGS)

if "%1"=="--ignore-unsupported"         (set ignore_unknown=yes& goto CONTINUEPARSEARGS)
set "unknown=%unknown% %1"

:CONTINUEPARSEARGS
shift
goto PARSEARGS
:ENDPARSEARGS

REM ###########################################################################
REM [TBD] check and report unknown options

REM ###########################################################################
REM help
:HELP

if "%help_req%"=="yes" (

    call :usage
    goto EXIT
)

REM ###########################################################################
REM check MSVC version

IF "%msvc%"=="12" set "project_folder=vs2013"
IF "%msvc%"=="14" set "project_folder=vs2015"
IF "%msvc%"=="15" set "project_folder=vs2017"

if defined project_folder goto VERSION_CHECK_PASSED
    echo ERROR: MSVC version %msvc% is not supported
    call :usage
    goto ABORT

:VERSION_CHECK_PASSED

REM ###########################################################################
REM check directories

if defined rootdir (set "tree_root=%rootdir%")
if not exist "%tree_root%" (
    echo ERROR: Root directry "%tree_root%" is not found
    goto ABORT
)    

cd %tree_root%\
for /f "delims=" %%a in ('cd') do (set "tree_root=%%a")
if "%tree_root:~-1%"=="\" (set "tree_root=%tree_root:~0,-1%")
cd "%initial_dir%"

if not defined dstdir (set "dstdir=%installdir%")

set "script_dir=%tree_root%\%script_path%"
if not exist "%script_dir%" (
    echo ERROR: Script directory "%script_dir%" is not found
    goto ABORT
)    

set "project_dir=%tree_root%\compilers\%project_folder%"
if not exist "%project_dir%" (
    echo ERROR: Project for MSVC %msvc% is not set up
    goto ABORT
)

REM ###########################################################################
REM adjust commands

if "%rebuild_req%" == "yes" (set build_req=yes)
if "%all_req%" == "yes" (
    set conf_req=yes
    set build_req=yes
    set install_req=yes
    set sdk_req=yes
    set instler_req=yes
)

REM ###########################################################################
REM target architecture, solution path, configuration and flags

if "%use_x64%"=="yes" (
  set arch=64
) else (
  set arch=32
)
if "%use_dll%"=="yes" (
  if "%use_debug%"=="yes" (
    set cfg=DebugDLL
  ) else (
    set cfg=ReleaseDLL
  )
) else (
  if "%use_debug%"=="yes" (
    if "%use_staticstd%"=="yes" (
      set cfg=DebugMT
    ) else (
      set cfg=DebugDLL
    )
  ) else (
    if "%use_staticstd%"=="yes" (
      set cfg=ReleaseMT
    ) else (
      set cfg=ReleaseDLL
    )
  )
)
if "%use_unicode%"=="yes" (
  set cfg=Unicode_%cfg%
)

echo MAKE: Configuration to be built is %cfg%

REM ###########################################################################
REM configure
:DO_CONFIGURE

if not "%conf_req%" == "yes" goto DO_BUILD

    call %script_dir%\make_configure %msvc% %arch%

REM ###########################################################################
REM build
:DO_BUILD

if not "%build_req%" == "yes" goto DO_INSTALL

    if "%rebuild_req%" == "yes" (
        REM [TBD] clean
    )
    
    set solution=ncbi_gbench.sln
    set project=gbench-install

    set prj_to_sln_path=dll\build\gbench

    set msvc_vars=msvcvars.bat
    set clean_vars=msvcvars-clean.bat
    set log=build.log
    
    
    REM ###########################################################################
    :SET_MSVC_VARS

    if not "%VSINSTALLDIR%" == "" (call "%script_dir%\%clean_vars%")

    call "%script_dir%\%msvc_vars%" %msvc% %arch%
    if errorlevel 1 goto ABORT

    REM ###########################################################################
    :DO_DO_BUILD

    set "solution_path=%project_dir%\%prj_to_sln_path%\%solution%"
     
    set "archw=Win32" 
    if "%arch:~-2%" == "64" (set "archw=x64")

    if exist "%initial_dir%\%log%" (del "%initial_dir%\%log%")

    echo.
    echo INFO [build]: Build "%prj_to_sln_path%\%solution% [%cfg%|%arch%]" on MSVC %msvc%
    if "%use_internal%" == "yes" (
        set "project=%project%-internal"
    )
    if defined MSBUILD (
       set project=%project:-=_%
       echo !project!
       echo %MSBUILD% %solution_path% /t:"!project!:Rebuild" /p:Configuration="%cfg%" /fl /flp:logfile="%initial_dir%\%log%" /m
       %MSBUILD% %solution_path% /t:"!project!:Rebuild" /p:Configuration="%cfg%" /fl /flp:logfile="%initial_dir%\%log%" /m
    ) else (
       %DEVENV% %solution_path% /build "%cfg%|%archw%" /project "%project%" /out "%initial_dir%\%log%"
    )
    
    if errorlevel 1 goto ABORT

REM ###########################################################################
REM install
:DO_INSTALL

if not "%install_req%" == "yes" goto DO_INSTALL_SDK

    call %script_dir%\make_install "%installdir%" %cfg% %msvc%
    if defined _output (set "installdir=%_output%")

REM ###########################################################################
REM install sdk
:DO_INSTALL_SDK

if not "%sdk_req%" == "yes" goto DO_INSTALLER

	setlocal

    cd "%installdir%"
    for /f "delims=" %%a in ('cd') do (set "installdir=%%a")
    if "%installdir:~-1%"=="\" (set "installdir=%installdir:~0,-1%")
	cd "%initial_dir%"

	set "sdk_dir=%installdir%\SDK"
	
    if not exist "%sdk_dir%\include" (md "%sdk_dir%\include")
	echo INFO [install-sdk]: Copying include files to SDK...
	
	xcopy "%tree_root%\include" "%sdk_dir%\include" /e /q
	rd /s /q "%sdk_dir%\include\internal"
	rd /s /q "%sdk_dir%\include\build-system"

	set "sdk_dll_dir=%sdk_dir%\lib\dll"

    if not exist "%sdk_dll_dir%" (md "%sdk_dll_dir")
	echo INFO [install-sdk]: Copying library files to SDK...

	for %%g in (Release Debug Unicode_Release Unicode_Debug) do (
		if not exist "%sdk_dll_dir%\%%gDLL" (md "%sdk_dll_dir%\%%gDLL")

		if exist "%project_dir%\dll\lib\%%gDLL" (
		
		    cd "%project_dir%\dll\lib\%%gDLL"
			xcopy *.lib "%sdk_dll_dir%\%%gDLL" /q
		)

		if exist "%project_dir%\dll\bin\%%gDLL" (
		
			cd "%project_dir%\dll\bin\%%gDLL"
			xcopy *.lib "%sdk_dll_dir%\%%gDLL" /q
			xcopy *.dll "%sdk_dll_dir%\%%gDLL" /q
			xcopy *.exp "%sdk_dll_dir%\%%gDLL" /q
		)
	)
	cd "%initial_dir%"
	
	endlocal

REM ###########################################################################
REM installer
:DO_INSTALLER

if not "%instler_req%" == "yes" goto COMPLETE

    call %script_dir%\make_installer "%installdir%" %arch%

REM ###########################################################################
:COMPLETE
echo.
echo INFO: %cmd% complete.

:EXIT
cd "%initial_dir%"
endlocal
exit /b %ERRORLEVEL%

:ABORT
echo.
echo INFO: %cmd% failed.
echo INFO: Make sure file %project_dir%\__configure.lock is removed.
set ERRORLEVEL=1
goto EXIT

:usage
REM --------------------------------------------------------------------------------
REM print usage

echo  USAGE:
echo    %cmd% [OPTIONS]...
echo  SYNOPSIS:
echo    Configure/build/install NCBI Genome Workbench for MSVC build system.

echo  OPTIONS:
echo    --help                 -- print Usage
echo.
echo    --configure            -- configure GBench solution
echo    --build                -- build GBench solution
echo    --rebuild              -- rebuild GBench solution
echo    --install              -- install GBench solution to ^<installdir^>
echo    --with-installer       -- make GBench installer in ^<dstdir^>
echo    --with-sdk             -- install Genome Workbench SDK in ^<installdir^>\SDK
echo    --do-all               -- configure, build, install and make installer
echo.
echo    --rootdir=^<path^>       -- source tree root (. or relative to script itself)
echo    --installdir=^<path^>    -- directory for installation (.\install)
echo    --dstdir=^<path^>        -- directory for installation (^<installdir^>)
echo    --msvc=^<version^>       -- build using specified version of MSVC (9)
echo.
echo    --x64
echo    --with-64              -- build x64 version (cross build for 32bit host)
echo    --release
echo    --without-debug        -- build non-debug versions of libs and apps
echo    --debug
echo    --with-debug           -- build debug versions of libs and apps
echo    --unicode
echo    --with-unicode         -- build unicode versions of libs and apps
echo    --static
echo    --without-dll          -- build all libraries as static ones
echo    --dll
echo    --with-dll             -- assemble libraries into DLLs where requested
echo    --with-static-exe      -- use static C++ standard libraries
echo    --with-internal        -- include NCBI internal packages
echo.
echo    --ignore-unsupported   -- ignore unsupported options

goto :eof
