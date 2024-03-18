@REM echo off
setlocal EnableDelayedExpansion
@REM get current dir
set SCRIPTDIR=%~dp0
set CONFIG_FN=config.bat
IF EXIST %CONFIG_FN% (
	echo Found %CONFIG_FN%, proceeding...
) ELSE (
    echo: 
    echo:
    echo FATAL ERROR================
    echo   %CONFIG_FN% file is missing
    echo   Please create it from %CONFIG_FN%.template and edit to match your environment
    echo   For more details take a look at docs/building-on-win.md 
    echo:
    exit 1
)
cd ..\..
set /p VERSION=<version.txt
cd %SCRIPTDIR%
call %CONFIG_FN%
call "%QTPATH%\qtenv2.bat"
@REM call vcvarsall.bat
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
echo %VCVARSALL%
call "%VCVARSALL%" x64
cd %SCRIPTDIR%
rmdir /s /q ..\build
mkdir ..\build
cd ..\build
echo %cd%
qmake ../../projects/JovIva/JovIva.pro CONFIG+=release
%JOM%
rmdir /s /q %SCRIPTDIR%\dist
mkdir %SCRIPTDIR%\dist
@REM copy the executable to the dist folder
echo Copying the executable to the dist folder
copy ..\..\out\JovIva.exe %SCRIPTDIR%\dist
cd %SCRIPTDIR%\dist
echo Running windeployqt
windeployqt --no-translations JovIva.exe
echo Running inno Setup
cd %SCRIPTDIR%
"%INNOPATH%\ISCC.exe" /dMyAppVersion="%VERSION%" joviva.iss
rmdir /s /q ..\build
rmdir /s /q dist