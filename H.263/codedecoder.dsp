# Microsoft Developer Studio Project File - Name="codedecoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=codedecoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "codedecoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "codedecoder.mak" CFG="codedecoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "codedecoder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "codedecoder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "codedecoder - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "codedecoder - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "codedecoder - Win32 Release"
# Name "codedecoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CodeInter.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeOneIntra.cpp
# End Source File
# Begin Source File

SOURCE=.\Countbit.cpp
# End Source File
# Begin Source File

SOURCE=.\Dct.cpp
# End Source File
# Begin Source File

SOURCE=.\DecodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\EncodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Getbits.cpp
# End Source File
# Begin Source File

SOURCE=.\Getblk.cpp
# End Source File
# Begin Source File

SOURCE=.\Gethdr.cpp
# End Source File
# Begin Source File

SOURCE=.\Getpic.cpp
# End Source File
# Begin Source File

SOURCE=.\Getvlc.cpp
# End Source File
# Begin Source File

SOURCE=.\Idct.cpp
# End Source File
# Begin Source File

SOURCE=.\IO.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Mot_est.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputVlc.cpp
# End Source File
# Begin Source File

SOURCE=.\Pred.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RateControl.cpp
# End Source File
# Begin Source File

SOURCE=.\RateControlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Recon.cpp
# End Source File
# Begin Source File

SOURCE=.\Sarcode.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Store.cpp
# End Source File
# Begin Source File

SOURCE=".\codedecoder.cpp"
# End Source File
# Begin Source File

SOURCE=".\codedecoder.rc"
# End Source File
# Begin Source File

SOURCE=".\codedecoderDoc.cpp"
# End Source File
# Begin Source File

SOURCE=".\codedecoderView.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DecodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Display.h
# End Source File
# Begin Source File

SOURCE=.\EncodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Getbits.h
# End Source File
# Begin Source File

SOURCE=.\Getblk.h
# End Source File
# Begin Source File

SOURCE=.\Gethdr.h
# End Source File
# Begin Source File

SOURCE=.\Getpic.h
# End Source File
# Begin Source File

SOURCE=.\Getvlc.h
# End Source File
# Begin Source File

SOURCE=.\Global.h
# End Source File
# Begin Source File

SOURCE=.\Idct.h
# End Source File
# Begin Source File

SOURCE=.\Indices.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MotionDlg.h
# End Source File
# Begin Source File

SOURCE=.\OpenFiles.h
# End Source File
# Begin Source File

SOURCE=.\OutputVlc.h
# End Source File
# Begin Source File

SOURCE=.\ProgressBar.h
# End Source File
# Begin Source File

SOURCE=.\PropertyDlg.h
# End Source File
# Begin Source File

SOURCE=.\RateControlDlg.h
# End Source File
# Begin Source File

SOURCE=.\Recon.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Sactbls.h
# End Source File
# Begin Source File

SOURCE=.\Sarcode.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Store.h
# End Source File
# Begin Source File

SOURCE=".\codedecoder.h"
# End Source File
# Begin Source File

SOURCE=".\codedecoderDoc.h"
# End Source File
# Begin Source File

SOURCE=".\codedecoderView.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\animal01.ico
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browsex1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browsex2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browsex3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\car.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=".\res\codedecoder.ico"
# End Source File
# Begin Source File

SOURCE=".\res\codedecoder.rc2"
# End Source File
# Begin Source File

SOURCE=".\res\codedecoderDoc.ico"
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
