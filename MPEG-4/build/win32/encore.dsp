# Microsoft Developer Studio Project File - Name="win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "encore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "encore.mak" CFG="win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "win32 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_MMX_" /D "_RC_" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\debug\encore.lib"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_MMX_" /D "_RC_" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\debug\encore.lib"

!ENDIF 

# Begin Target

# Name "win32 - Win32 Release"
# Name "win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\bitstream.c
# End Source File
# Begin Source File

SOURCE=..\..\src\encore.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mom_access.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mom_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_code.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_est_comp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_est_mb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\putvlc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rate_ctl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\text_bits.c
# End Source File
# Begin Source File

SOURCE=..\..\src\text_code.c
# End Source File
# Begin Source File

SOURCE=..\..\src\text_code_mb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\text_dct.c
# End Source File
# Begin Source File

SOURCE=..\..\src\vop_code.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\bitstream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\encore.h
# End Source File
# Begin Source File

SOURCE=..\..\src\max_level.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mom_access.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mom_structs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mom_util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\momusys.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_code.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_est_comp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_est_mb.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mot_util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\non_unix.h
# End Source File
# Begin Source File

SOURCE=..\..\src\putvlc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rate_ctl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\text_bits.h
# End Source File
# Begin Source File

SOURCE=..\..\src\text_code.h
# End Source File
# Begin Source File

SOURCE=..\..\src\text_code_mb.h
# End Source File
# Begin Source File

SOURCE=..\..\src\text_dct.h
# End Source File
# Begin Source File

SOURCE=..\..\src\text_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\vlc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\vm_common_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\vm_enc_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\vop_code.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zigzag.h
# End Source File
# End Group
# Begin Group "MMX Sources"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=..\..\src\intel_mmx\text_fdct_mmx.c
# End Source File
# Begin Source File

SOURCE=..\..\src\intel_mmx\text_idct_mmx.c
# End Source File
# End Group
# End Target
# End Project
