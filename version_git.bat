@echo off

REM Is Git for Windows x64 installed?
if exist "%ProgramW6432%\Git\bin\bash.exe" (

	echo Using bash from Git for Windows x64
	setlocal
	PATH=%ProgramW6432%\Git\mingw64\bin;%ProgramW6432%\Git\bin
	bash version_git.sh
	exit /b 0
)

REM Is Cygwin installed?
if exist C:\Cygwin\bin\bash.exe (

	echo Using bash from Cygwin
	setlocal
	PATH=C:\Cygwin\bin
	bash version_git.sh
	exit /b 0
)

REM Is Cygwin64 installed?
if exist C:\Cygwin64\bin\bash.exe (

	echo Using bash from Cygwin64
	setlocal
	PATH=C:\Cygwin64\bin
	bash version_git.sh
	exit /b 0
)

REM No Cygwin or Git for Windows installed
REM If there is a file assume if is properly generated
if exist version_git.h (
	echo No bash found, but version_git.h exists
	exit /b 0
)

echo No bash found
exit /b 1
