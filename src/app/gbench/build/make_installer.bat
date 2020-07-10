@set echsw=off
@echo %echsw%
REM $Id: make_installer.bat 44780 2020-03-11 02:50:59Z evgeniev $
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
REM Create installator for Genome Workbench.
REM
REM     make_installer.bat <install dir> <32|64>
REM
REM     %1% - directory where GBench is installed. Installer will be put here.          
REM     %2% - 32/64-bits architecture (optional; default is 32 bit).
REM
REM ===========================================================================

setlocal
setlocal disabledelayedexpansion

set version_file=include\gui\objects\gbench_version.hpp

REM ATTENTION: extra_files_dir may be actually %script_dir%
set extra_files=license.txt gbench-res1.ini gbench-res2.ini
set extra_files_dir=src\gui\scripts\install\win32

set make_nsis=makensis.exe
set nsis_dir=\\snowman\win-coremake\App\ThirdParty\NSIS

set tree_root=..\..\..\..

set cmd=%~n0
set script_name=%0
set initial_dir=%CD%

cd %~p0
for /f "delims=" %%a in ('cd') do (set script_dir=%%a)
cd %tree_root%
for /f "delims=" %%a in ('cd') do (set tree_root=%%a)
cd %initial_dir%

set "extra_files_dir=%tree_root%\%extra_files_dir%"

set install_dir=%~1
cd %install_dir%
for /f "delims=" %%a in ('cd') do (set "install_dir=%%a")

cd %initial_dir%

set ver_arch=%2

REM ###########################################################################
:DO_INSTALLER

if not exist "%tree_root%" (
    echo ERROR: "%tree_root%" not found
    goto ABORT
)    

if not exist "%install_dir%" (
    echo ERROR: "%install_dir%" not found
    goto ABORT
)    

set "ver_file=%tree_root%\%version_file%"
if not exist "%ver_file%" (
    echo WARNING: GBench version cannot be detected. Using defaults.

    set ver_major=0
    set ver_minor=0
    set ver_revision=0

    goto START_INSTALL
)    

for /f "tokens=3 delims=, " %%k in ('find "eMajor" "%ver_file%"') do (set ver_major=%%k)
for /f "tokens=3 delims=, " %%k in ('find "eMinor" "%ver_file%"') do (set ver_minor=%%k)
for /f "tokens=3 delims=, " %%k in ('find "ePatch" "%ver_file%"') do (set ver_revision=%%k)

REM ATTENTION: Country code should be 001 (USA) - see http://ss64.com/nt/date.html

set build_date=%DATE:~10,4%%DATE:~4,2%%DATE:~7,2%

rem Check platform file to obtain platform built.


REM ###########################################################################
:PREPARE_FILES

cd %extra_files_dir%
for %%f in (%extra_files%) do (xcopy %%f %install_dir%\ /y /q)

cd %script_dir%

set nsis_file=gbench.nsi
set base_nsis_file=%nsis_file%.in

if exist "%install_dir%\%nsis_file%" (del "%install_dir%\%nsis_file%")

@echo off
REM ============================================
REM Replace version related macros in nsi file.
REM It can be done instead via environment variables.
REM set INPUT_PATH=%install_dir%
REM set VERSION_MAJOR=%ver_major%
REM set VERSION_MINOR=%ver_minor%
REM set VERSION_REVISION=%ver_revision%
REM set VERSION_BUILD_DATE=%build_date%
REM set VERSION_ARCH=%ver_arch%

for /f "eol=¬ delims=" %%s in (%base_nsis_file%) do (
    set "line=%%s"
    call :procline
)
goto next

:procline
    set "line=%line:"=`%"

    call set "line=%%line:@INPUT_PATH@=%install_dir%%%"
    call set "line=%%line:@VERSION_MAJOR@=%ver_major%%%"
    call set "line=%%line:@VERSION_MINOR@=%ver_minor%%%"
    call set "line=%%line:@VERSION_REVISION@=%ver_revision%%%"
    call set "line=%%line:@VERSION_BUILD_DATE@=%build_date%%%"
    call set "line=%%line:@VERSION_ARCH@=%ver_arch%%%"

    set "line=%line:|=^|%"
    set "line=%line:&=^&%"
    set "line=%line:>=^>%"
    set "line=%line:<=^<%"
    
    set "nql=%line:"=`%"
    set "ntl=%nql:	=%"
    set "nsl=%nql: =%"
    
    if "%ntl%"=="" goto :eof
    if "%nsl%"=="" goto :eof
    
    set "line=%line:`="%"
    echo %line% >> %install_dir%\%nsis_file%

goto :eof

:next
@echo %echsw%

REM ###########################################################################
:RUN_CREATOR
%nsis_dir%\%make_nsis% %install_dir%\%nsis_file%


REM ###########################################################################
:CLEANUP
del "%install_dir%\%nsis_file%"
for %%f in (%extra_files%) do (del "%install_dir%\%%f")

REM ###########################################################################
:COMPLETE
echo.
echo INFO: Genome Workbench installer is created in "%install_dir%"
echo INFO: %cmd% complete.

:EXIT
cd %initial_dir%
endlocal
exit /b %ERRORLEVEL%

:ABORT
echo.
echo INFO: %cmd% failed.
set ERRORLEVEL=1
goto EXIT
