# Microsoft Developer Studio Project File - Name="LIBTR" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LIBTR - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LIBTR.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LIBTR.mak" CFG="LIBTR - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LIBTR - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "LIBTR - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "LIBTR - Win32 Release DLL" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LIBTR - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\LIBTR.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=echo copy iviTR_VC6.lib iviTR.lib	del iviTR.lib /F	copy iviTR_VC6.lib iviTR.lib	echo copy Scramble_VC6.lib Scramble.lib	del Scramble.lib /F	copy Scramble_VC6.lib Scramble.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "LIBTR - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "STRICT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\LibD\LIBTR.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=echo copy iviTR_VC6D.lib iviTR.lib	del iviTR.lib /F	copy iviTR_VC6D.lib iviTR.lib	echo copy Scramble_VC6.lib Scramble.lib	del Scramble.lib /F	copy Scramble_VC6.lib Scramble.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "LIBTR - Win32 Release DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "LIBTR___Win32_Release_DLL"
# PROP BASE Intermediate_Dir "LIBTR___Win32_Release_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /Ow /Og /Oi /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\LIBTR.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\LIBTR.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=echo copy iviTR_VC6.lib iviTR.lib	del iviTR.lib /F	copy iviTR_VC6.lib iviTR.lib	echo copy Scramble_VC6.lib Scramble.lib	del Scramble.lib /F	copy Scramble_VC6.lib Scramble.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "LIBTR - Win32 Release"
# Name "LIBTR - Win32 Debug"
# Name "LIBTR - Win32 Release DLL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\IVIScramble.cpp
# End Source File
# Begin Source File

SOURCE=.\tr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AntiDebugOff.h
# End Source File
# Begin Source File

SOURCE=.\BaseTR.h
# End Source File
# Begin Source File

SOURCE=.\BaseTROff.h
# End Source File
# Begin Source File

SOURCE=.\DetectTREnableOff.h
# End Source File
# Begin Source File

SOURCE=.\DetectTRExeOff.h
# End Source File
# Begin Source File

SOURCE=.\IVIScramble.h
# End Source File
# Begin Source File

SOURCE=.\IVITR_version.h
# End Source File
# Begin Source File

SOURCE=.\LIBTR.H
# End Source File
# Begin Source File

SOURCE=.\TimingMacrosOff.h
# End Source File
# Begin Source File

SOURCE=.\tr.h
# End Source File
# Begin Source File

SOURCE=.\trcfg.h
# End Source File
# Begin Source File

SOURCE=.\trcfgdbg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\iviTR.lib
# End Source File
# Begin Source File

SOURCE=.\Scramble.lib
# End Source File
# End Target
# End Project
