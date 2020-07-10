@echo off
REM
REM $Id: make-all-x64-15.bat 42562 2019-03-20 19:56:44Z katargir $
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

set script_path=src\app\gbench\build

%~dp0%script_path%\do-make --do-all --msvc 15 --x64 --unicode %*

endlocal
