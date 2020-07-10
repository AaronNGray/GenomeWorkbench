@echo off
REM
REM $Id: do-make.bat 21870 2010-08-04 20:02:17Z voronov $
REM
REM ===========================================================================
REM
REM Author:  Yury Voronov
REM
REM Calls do-make.bat from inside source tree.
REM
REM     do-make.bat 
REM
REM ===========================================================================

setlocal

set script_path=src\app\gbench\build

%~dp0%script_path%\do-make %*

endlocal
