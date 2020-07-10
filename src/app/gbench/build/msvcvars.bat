@echo off
REM
REM $Id: msvcvars.bat 44657 2020-02-14 19:05:46Z evgeniev $
REM
REM This file is copied from 
REM    compilers/msvc800_prj/msvcvars.bat 
REM    rev.103491 2007-05-04 17:18:18Z kazimird
REM
REM ===========================================================================
REM
REM Author:  Yury Voronov
REM
REM Calls appropriate version of vcvarsall.bat script.
REM
REM     msvcvars.bat <msvc version> [arch]
REM
REM     %1% - MSVC version (12|14)
REM     %2% - Windows platform (32|64|32_64)
REM
REM REMEMBER: this script sets global environment variables. In order to 
REM clean it one should call msvcvars-clean.bat in the same directory.
REM ===========================================================================

if "%1" == "" (
    echo MSVC version should be specified
    call :usage
    exit /b 1
)

if %1 == 12 goto VERSION_CHECK_PASSED
if %1 == 14 goto VERSION_CHECK_PASSED
if %1 == 15 goto VERSION_CHECK_PASSED

    echo ERROR: MSVC version %1 is unsupported
    call :usage
    exit /b 1

:VERSION_CHECK_PASSED
set "__MSVC=%1"

set "__arch="
if "%2" == "" goto endif

    if /i %2 == 86 set "__arch=x86" & goto endif
    if /i %2 == 32 set "__arch=x86" & goto endif
    if /i %2 == x32 set "__arch=x86" & goto endif
    if /i %2 == 64 set "__arch=amd64" & goto endif
    if /i %2 == x64 set "__arch=amd64" & goto endif
    if /i %2 == 32_64 set "__arch=x86_amd64" & goto endif
    if /i %2 == 86_64 set "__arch=x86_amd64" & goto endif
    
    set "__arch=%2"
     
:endif

:PLATFORM_CHECK_PASSED

if %1 == 15 goto devenv15

call set VSCOMNTOOLS=%%VS%__MSVC%0COMNTOOLS%%

if not "%VSINSTALLDIR%"=="" goto devenv
call "%VSCOMNTOOLS%..\..\VC\vcvarsall.bat" %__arch%
if errorlevel 1 ( 
    echo.
    echo INFO: Make sure MSVC version %__MSVC% is installed on this computer.
    exit /b 1
)

:devenv

if exist "%VSCOMNTOOLS%..\IDE\VCExpress.*" set DEVENV="%VSCOMNTOOLS%..\IDE\VCExpress"
if exist "%VSCOMNTOOLS%..\IDE\devenv.*" set DEVENV="%VSCOMNTOOLS%..\IDE\devenv"

:end
exit /b 0

:devenv15
for /f "tokens=* USEBACKQ" %%i IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -version 15.0 -property productPath`) do (
    set DEVENV="%%~dpni"
)
if not defined VSINSTALLDIR (
   for /f "tokens=* USEBACKQ" %%i IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -version 15.0 -property installationPath`) do (
      set VSINSTALLDIR=%%~dpni
   )
)
if exist "%VSINSTALLDIR%\MSBuild\15.0\Bin\amd64\MSBuild.exe" set MSBUILD="%VSINSTALLDIR%\MSBuild\15.0\Bin\amd64\MSBuild.exe"
exit /b 0

:usage
echo Usage: %~n0 ^<msvc version^> [arch]
goto :eof
