@echo off
REM
REM $Id: make-all-x64-internal-15-up.bat 42575 2019-03-21 19:24:51Z filippov $
REM
REM ===========================================================================
REM
REM Author:  Yury Voronov
REM
REM Calls do-make.bat from inside source tree.
REM
REM     make-all-x64.bat 
REM
REM ===========================================================================

setlocal

set version_path=include\gui\objects\gbench_version.hpp
@COPY /B %version_path%+,, %version_path%

set script_path=src\app\gbench\build

%~dp0%script_path%\do-make --do-all --msvc 15 --with-internal --x64 --unicode %*

endlocal
