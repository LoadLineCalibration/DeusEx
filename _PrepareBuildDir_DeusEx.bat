@echo off
rem Prepares the VM-local temporary build directories used by the VC98 project.
set BUILD_DIR=C:\DX_DeusExBuild

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%BUILD_DIR%\Obj" mkdir "%BUILD_DIR%\Obj"

exit /B 0
