@echo off
REM $Id: make_install.bat 33317 2015-07-07 15:42:15Z evgeniev $
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
REM Create installation for Genome Workbench (based on build results).
REM
REM     make_install.bat <install dir> <ReleaseDLL|DebugDLL> <8|9|10>
REM
REM     %1% - directory where GBench will be installed.
REM
REM ===========================================================================

setlocal

set msvc=9
set conf=ReleaseDLL

if not _%2 == _ (set conf=%2)
if not _%3 == _ (set msvc=%3)

if %msvc% == 9 (
	set "project_folder=msvc%msvc%00_prj")
if %msvc% == 10 (
	set "project_folder=msvc%msvc%00_prj")
if %msvc% == 11 (
	set "project_folder=vs2012")
if %msvc% == 12 (
	set "project_folder=vs2013")
set tree_build=compilers\%project_folder%\dll\bin\%conf%\gbench

set tree_root=..\..\..\..

set cmd=%~n0
set script_name=%0
set initial_dir=%CD%

cd %~p0
for /f "delims=" %%a in ('cd') do (set script_dir=%%a)
if "%script_dir:~-1%"=="\" (set "script_dir=%script_dir:~0,-1%")
cd %tree_root%
for /f "delims=" %%a in ('cd') do (set tree_root=%%a)
if "%tree_root:~-1%"=="\" (set "tree_root=%tree_root:~0,-1%")
cd %initial_dir%

set install_dir=%~1
set install_dirname=%~n1

REM ###########################################################################
:DO_INSTALL
 
if not exist "%tree_root%" (
    echo ERROR [install]: "%tree_root%" not found
    goto ABORT
)    

set build=1

:next_bld
set "inst_bld=%install_dir%-%build%"
if exist "%inst_bld%" (
    set /a build += 1
    goto next_bld
)

if exist "%install_dir%" (
    if "%build%"=="1" (
        rename "%install_dir%" "%install_dirname%-1"
        set build=2
        set "install_dir=%install_dir%-2"
    ) else (
        set "install_dir=%inst_bld%"
    )
) else (
    if not "%build%"=="1" (
        set "install_dir=%inst_bld%"
    )
)

mkdir "%install_dir%"
    
if errorlevel 1 (
    echo ERROR [install]: "%install_dir%" cannot be created.
    goto ABORT
) else (
    echo INFO [install]: "%install_dir%" is created for installation.
)

    
echo INFO [install]: Copying Genome Workbench files...
xcopy "%tree_root%\%tree_build%" "%install_dir%" /e /q
xcopy "%tree_root%\compilers\%project_folder%\__configured*.*" "%install_dir%" /q >nul

REM ###########################################################################
:COMPLETE
echo.
echo INFO [install]: Genome Workbench is installed in "%install_dir%"
echo INFO [install]: %cmd% complete. 

:EXIT
cd %initial_dir%
endlocal & set _output=%install_dir%
exit /b %ERRORLEVEL%

:ABORT
echo.
echo INFO: %cmd% failed.
set ERRORLEVEL=1
goto EXIT
