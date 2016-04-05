@echo off

REM Is Cygwin installed?
if exist C:\Cygwin\bin\bash.exe goto bash

REM No Cygwin installed
REM If there is a file assume if is propperly generated
if exist version_git.h goto end

REM Generate a dummy file
echo Generating dummy version_git.h...
echo #pragma once > version_git.h
echo #define VERSION_GIT_STR "UNKNOWN" >> version_git.h
echo #define VERSION_GIT_WSTR L"UNKNOWN" >> version_git.h
goto end

REM Let a bash script do all the work
:bash
C:\Cygwin\bin\bash version_git.sh
goto end

:end
