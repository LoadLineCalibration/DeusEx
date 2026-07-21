@echo off
rem Copies the VM-built DeusEx.dll from the temporary build directory to the host tree.
set BUILD_DIR=C:\DX_DeusExBuild
set HOST_SYSTEM=Z:\DeusEx_RTX\System

call .\_PrepareBuildDir_DeusEx.bat

if not exist "%HOST_SYSTEM%" (
    echo Host system directory not found: %HOST_SYSTEM%
    exit /B 1
)

if exist "%BUILD_DIR%\DeusEx.dll" copy /Y "%BUILD_DIR%\DeusEx.dll" "%HOST_SYSTEM%\DeusEx.dll"
if exist "%BUILD_DIR%\DeusEx.lib" copy /Y "%BUILD_DIR%\DeusEx.lib" "%HOST_SYSTEM%\DeusEx.lib"
if exist "%BUILD_DIR%\DeusEx.pdb" copy /Y "%BUILD_DIR%\DeusEx.pdb" "%HOST_SYSTEM%\DeusEx.pdb"
if exist "%BUILD_DIR%\DeusEx.exp" copy /Y "%BUILD_DIR%\DeusEx.exp" "%HOST_SYSTEM%\DeusEx.exp"

exit /B 0
