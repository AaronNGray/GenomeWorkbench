@echo off
REM
REM $Id: make-all-x64.bat 23131 2011-02-10 19:25:24Z kuznets $
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

%~dp0%script_path%\do-make --do-all --x64 --unicode %*

endlocal