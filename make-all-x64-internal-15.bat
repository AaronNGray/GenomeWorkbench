@echo off
REM
REM $Id: make-all-x64-internal-15.bat 42567 2019-03-20 21:24:51Z katargir $
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

%~dp0%script_path%\do-make --do-all --msvc 15 --with-internal --x64 --unicode %*

endlocal
