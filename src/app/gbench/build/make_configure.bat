@echo off
REM $Id: make_configure.bat 42560 2019-03-20 19:48:10Z katargir $
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
REM Configures Genome Workbench solution.
REM
REM     make_configure.bat <MSVC version> [arch] 
REM
REM     %1% - 
REM     %2% - 
REM
REM ===========================================================================

setlocal

set solution=ncbi_gbench.sln
set prj_to_sln_path=dll\build\gbench

set tree_root=..\..\..\..
set msvc_vars=msvcvars.bat
set clean_vars=msvcvars-clean.bat
set log=cofigure.log

:SETUP

set cmd=%~n0
set script_name=%0
set initial_dir=%CD%

cd %~p0
for /f "delims=" %%a in ('cd') do (set script_dir=%%a)
if "%script_dir:~-1%"=="\" (set "script_dir=%script_dir:~0,-1%")

cd %tree_root%
for /f "delims=" %%a in ('cd') do (set tree_root=%%a)
if "%tree_root:~-1%"=="\" (set "tree_root=%tree_root:~0,-1%")

if not exist "%tree_root%" (
    echo ERROR: "%tree_root%" not found
    goto ABORT
)    

cd %initial_dir%

:ARGS_CHECK

set msvc=12
if "%1" == "" (goto VERSION_CHECK_PASSED)
set "msvc=%1"

if %1 == 12 (
	set "project_folder=vs2013"
		set "conf_prj=_CONFIGURE_")
if %1 == 14 (
	set "project_folder=vs2015"
		set "conf_prj=_CONFIGURE_")        
if %1 == 15 (
	set "project_folder=vs2017"
		set "conf_prj=_CONFIGURE_")        

if defined project_folder goto VERSION_CHECK_PASSED

    echo MSVC version %1 is not supported
    goto ABORT

:VERSION_CHECK_PASSED
shift

REM if "%1" == "" (goto CONFIG_PASSED)
REM
REM if /i "%1" == "release" (set config=ReleaseDLL & goto CONFIG_PASSED)
REM if /i "%1" == "debug" (set config=DebugDLL & goto CONFIG_PASSED)
REM 
REM     echo Configuration %1 is not supported
REM     call :usage
REM     goto ABORT
REM 
REM :CONFIG_PASSED
REM shift

set arch=32
if "%1" == "" (goto ARCH_CHECK_PASSED)
set "arch=%1"

:ARCH_CHECK_PASSED
shift

REM ###########################################################################
:CHECK_MSVC

set "project_dir=%tree_root%\compilers\%project_folder%"

if not exist "%project_dir%" (
    echo ERROR: MSVC %msvc% is not set up
    goto ABORT
)

if  %msvc% LEQ 7 (
    echo ERROR: MSVC %msvc% is not supported
    goto ABORT
)

REM ###########################################################################
:SET_MSVC_VARS

if not "%VSINSTALLDIR%" == "" (call "%script_dir%\%clean_vars%")

call "%script_dir%\%msvc_vars%" %msvc% %arch%
if errorlevel 1 goto ABORT

REM ###########################################################################
:DO_CONFIGURE

set "solution_path=%project_dir%\%prj_to_sln_path%\%solution%"
 
set "archw=Win32" 
if "%arch:~-2%" == "64" (set "archw=x64")

if exist "%initial_dir%\%log%" (del "%initial_dir%\%log%")

echo.
echo INFO: Configure "%prj_to_sln_path%\%solution% [%arch%]" on MSVC %msvc%
%DEVENV% %solution_path% /build "ReleaseDLL|%archw%" /project "%conf_prj%" /out "%initial_dir%\%log%"
if errorlevel 1 goto ABORT

REM ###########################################################################
:COMPLETE
echo.
echo INFO: %cmd% complete.

:EXIT
cd %initial_dir%
endlocal
exit /b %ERRORLEVEL%

:ABORT
echo.
echo INFO: %cmd% failed.
echo INFO: Make sure file %project_dir%\__configure.lock is removed.
echo.
call :usage
set ERRORLEVEL=1
goto EXIT

:usage
echo Usage: %~n0 [msvc version] [arch]
echo        This program should run from src\app\gbench\build directory from inside The Tree.
goto :eof
