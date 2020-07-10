@echo off
if "%1" == "" goto EXIT

set SOL_BASE=%1
set project=%2
set mode=%3
set action=%4

SET DEVENV="e:/Program Files/Microsoft Visual Studio .net 2003/Common7/IDE/devenv"
SET SOL_STATIC=%SOL_BASE%\compilers\msvc710_prj\static\build\ncbi_cpp.sln
SET SOL=%SOL_BASE%\compilers\msvc710_prj\dll\build\gbench\ncbi_gbench.sln


if "%action%" == "rebuild" goto CLEAN
goto BUILD

:CLEAN

@echo cleaning projects...
%DEVENV% "%SOL%" /clean DebugDLL
%DEVENV% "%SOL%" /clean ReleaseDLL

:BUILD

@echo building projects...

%DEVENV% "%SOL%" /build "%mode%" /project "%project%"

:EXIT

