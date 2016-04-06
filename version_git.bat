@echo off

REM Is Cygwin installed?
if exist C:\Cygwin\bin\bash.exe goto cygwin

REM Is Git for Windows installed?
if exist "C:\Program Files\Git\bin\bash.exe" goto git_for_win

REM No Cygwin or Git for Windows installed
REM If there is a file assume if is propperly generated
if exist version_git.h goto end

REM Generate a dummy file
echo No bash found, generating dummy version_git.h...
echo #pragma once > version_git.h
echo #define VERSION_GIT_STR "UNKNOWN" >> version_git.h
echo #define VERSION_GIT_WSTR L"UNKNOWN" >> version_git.h
goto end

REM Let a Cygwin bash script do all the work
:cygwin
PATH=C:\Cygwin\bin
bash version_git.sh
goto end

REM Let a Git for Windows bash script do all the work
:git_for_win
PATH=C:\Program Files\Git\mingw64\bin;C:\Program Files\Git\bin
bash version_git.sh
goto end

:end
