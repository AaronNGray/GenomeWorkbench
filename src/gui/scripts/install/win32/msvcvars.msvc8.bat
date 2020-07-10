@echo off
REM
REM $Id: msvcvars.msvc8.bat 21824 2010-07-23 23:52:56Z voronov $
REM
REM This file is copied from 
REM    compilers/msvc800_prj/msvcvars.bat 
REM    rev.103491 2007-05-04 17:18:18Z kazimird
REM

@if not "%VSINSTALLDIR%"=="" goto devenv
@call "%VS80COMNTOOLS%vsvars32.bat"

:devenv

if exist "%VS80COMNTOOLS%..\IDE\VCExpress.*" set DEVENV="%VS80COMNTOOLS%..\IDE\VCExpress"
if exist "%VS80COMNTOOLS%..\IDE\devenv.*" set DEVENV="%VS80COMNTOOLS%..\IDE\devenv"

:end
