# Microsoft Developer Studio Project File - Name="DeusEx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DeusEx - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DeusEx.mak" CFG="DeusEx - Win32 Release"
!MESSAGE 
!MESSAGE Possible configuration:
!MESSAGE 
!MESSAGE "DeusEx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DeusEx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\DX_DeusExBuild"
# PROP Intermediate_Dir "C:\DX_DeusExBuild\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# ADD CPP /nologo /Zp4 /MD /W4 /WX /vd0 /GX /O2 /Ob2 /I ".\Inc" /I "..\Inc" /I ".\Src" /I "..\Src" /I ".\Classes" /I "..\Classes" /I ".\SDK" /I "..\SDK" /I ".\SDK\Core\Inc" /I "..\SDK\Core\Inc" /I ".\SDK\Core\Src" /I "..\SDK\Core\Src" /I ".\SDK\Engine\Inc" /I "..\SDK\Engine\Inc" /I ".\SDK\Engine\Src" /I "..\SDK\Engine\Src" /I ".\SDK\Extension\Inc" /I "..\SDK\Extension\Inc" /I ".\SDK\ConSys\Inc" /I "..\SDK\ConSys\Inc" /D "NDEBUG" /D ThisPackage=DeusEx /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "DEUSEX_EXPORTS" /D DEUSEX_API=DLL_EXPORT /D CONSYS_API=DLL_IMPORT /D EXTENSION_API=DLL_IMPORT /FR"C:\DX_DeusExBuild\Obj\\" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"C:\DX_DeusExBuild\DeusEx.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib /nologo /dll /machine:I386
# ADD LINK32 .\SDK\Core\Lib\Core.lib .\SDK\Engine\Lib\Engine.lib .\SDK\ConSys\Lib\ConSys.lib .\SDK\Extension\Lib\Extension.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib /libpath:"Z:\DeusEx_RTX\System" /libpath:".\SDK\Core\Lib" /libpath:".\SDK\Engine\Lib" /libpath:".\SDK\ConSys\Lib" /libpath:".\SDK\Extension\Lib" /nologo /dll /incremental:no /machine:I386 /out:"C:\DX_DeusExBuild\DeusEx.dll" /implib:"C:\DX_DeusExBuild\DeusEx.lib" /pdb:"C:\DX_DeusExBuild\DeusEx.pdb"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Installing DeusEx.dll to Z:\DeusEx_RTX\System
PostBuild_Cmds=.\_PostBuildInstall_DeusEx.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DeusEx - Win32 Release"
# Begin Group "Classes"

# PROP Default_Filter "uc"
# Begin Source File

SOURCE=.\Classes\Augmentation.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\AugmentationManager.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\BallisticArmor.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DataVaultImageNote.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DeusExDecoration.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DeusExLevelInfo.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DeusExLog.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DeusExPlayer.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DeusExSaveInfo.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\DumpLocation.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\GameDirectory.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\LaserIterator.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\ParticleIterator.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\ScriptedPawn.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\Skill.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\SkillManager.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\WeaponNPCMelee.uc

# End Source File
# Begin Source File

SOURCE=.\Classes\WeaponNPCRanged.uc

# End Source File

# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\ADeusExDecoration_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\ADeusExPlayer_MiscNatives_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\ADeusExPlayer_SaveNatives_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\AScriptedPawn_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\DeusExSaveTravel_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\DeusEx_ClassBoilerplate_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\DeusEx_Registration_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\UDeusExSaveInfo_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\UDumpLocation_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\ULaserIterator_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\UParticleIterator_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\Src\XGameDirectory_Reconstructed.cpp

# End Source File
# Begin Source File

SOURCE=.\DeusEx.def

# End Source File
# Begin Source File

SOURCE=.\_PostBuildInstall_DeusEx.bat

# End Source File
# Begin Source File

SOURCE=.\_PrepareBuildDir_DeusEx.bat

# End Source File
# Begin Source File

SOURCE=.\_Build_DeusEx_VC98.bat

# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Inc\AAugmentation.h

# End Source File
# Begin Source File

SOURCE=.\Inc\AAugmentationManager.h

# End Source File
# Begin Source File

SOURCE=.\Inc\ADeusExDecoration.h

# End Source File
# Begin Source File

SOURCE=.\Inc\ADeusExLevelInfo.h

# End Source File
# Begin Source File

SOURCE=.\Inc\ADeusExPlayer.h

# End Source File
# Begin Source File

SOURCE=.\Inc\AScriptedPawn.h

# End Source File
# Begin Source File

SOURCE=.\Inc\DeusEx.h

# End Source File
# Begin Source File

SOURCE=.\Inc\DeusExClasses.h

# End Source File
# Begin Source File

SOURCE=.\Inc\DeusExGameEngine.h

# End Source File
# Begin Source File

SOURCE=.\Inc\DeusExReconstructedPrivate.h

# End Source File
# Begin Source File

SOURCE=.\Inc\ExtGameDirectory.h

# End Source File
# Begin Source File

SOURCE=.\Inc\UDataVaultImageNote.h

# End Source File
# Begin Source File

SOURCE=.\Inc\UDeusExLog.h

# End Source File
# Begin Source File

SOURCE=.\Inc\UDeusExSaveInfo.h

# End Source File
# Begin Source File

SOURCE=.\Inc\UDumpLocation.h

# End Source File
# Begin Source File

SOURCE=.\Inc\ULaserIterator.h

# End Source File
# Begin Source File

SOURCE=.\Inc\UParticleIterator.h

# End Source File
# Begin Source File

SOURCE=.\Inc\uparticle.h

# End Source File

# End Group
# End Target
# End Project
