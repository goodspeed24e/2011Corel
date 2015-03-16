!IF EXIST("V:\bin\makeSDKDef")
!INCLUDE V:\bin\makeSDKDef
!ENDIF

#// for Trexe
RUNTR=do_nothing
BACKUPBINARY=do_nothing
RUNTR1XENV=do_nothing
COPYRGN=0

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA NoISMP_TRSDK Release target
VideoH264VDecSDK(QA)-NoISMP_TRSDK_v1x(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-NoISMP_TRSDK_v1x(Release)
    set RUNTR=RunTR
    set COPYRGN=1
    set RUNTR1XENV=EnvMakeAutoSet
    set BACKUPBINARY=BackupBinary
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA NoISMP_TR Release target
VideoH264VDecSDK(QA)-NoISMP_TR(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-NoISMP_TR(Release)
    set RUNTR=RunTR
    set BACKUPBINARY=BackupBinary
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA NoISMP_NOTR Release target
VideoH264VDecSDK(QA)-NoISMP_NOTR(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-NoISMP_NOTR(Release)
    set RUNTR=RunTRForNoTRMacro
    set COPYPDB_MAP=1
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA ISMP_TRSDK Release target
VideoH264VDecSDK(QA)-ISMP_TRSDK_v1x(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-ISMP_TRSDK_v1x(Release)
    set RUNTR=RunTR
    set COPYRGN=1
    set RUNTR1XENV=EnvMakeAutoSet
    set BACKUPBINARY=BackupBinary
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA ISMP_TR Release target
VideoH264VDecSDK(QA)-ISMP_TR(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-ISMP_TR(Release)
    set RUNTR=RunTR
    set BACKUPBINARY=BackupBinary
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA ISMP_NOTR Release target
VideoH264VDecSDK(QA)-ISMP_NOTR(Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-ISMP_NOTR(Release)
    set RUNTR=RunTRForNoTRMacro
    set COPYPDB_MAP=1
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA NOTR Release PDB target
VideoH264VDecSDK(QA)-NOTR(PDB-Release)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-NOTR(PDB-Release)
    set RUNTR=RunTRForNoTRMacro
    set COPYPDB_MAP=1
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit

#//////////////////////////////////////////////////////////////////////////////////////
# VideoH264VDec QA ISMP_NoTR Debug target
VideoH264VDecSDK(QA)-ISMP_NOTR(Debug)_all:
    set IVI_MEDIA=VideoH264VDecSDK(QA)-ISMP_NOTR(Debug)
    set RUNTR=RunTRForNoTRMacro
    set COPYPDB_MAP=1
    set BUILDCONFIG=Debug
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit


#//////////////////////////////////////////////////////////////////////////////////////
# BuildBinary:
#  %Build Binaries
BuildBinary:
    @echo [BuildBinary]:
    cd "$(SSROOTDIR)"
    $(MYMAKEUTIL)   EnvDefine \
    				$(RUNTR1XENV) \
                    AdditionalCL \
                    BuildVC8VideoH264VDec \
                    $(BACKUPBINARY) \
                    $(RUNTR)

#//////////////////////////////////////////////////////////////////////////////////////
# BuildVC8VideoH264VDec:
#  BuildVC8VideoH264VDec
BuildVC8VideoH264VDec:
    @echo [VC8VideoH264VDec]:
    @echo Compiler flags: %%CL%%
    @echo Linker flags: %%LINK%%
    cd $(SSROOTDIR)
    $(BUILDVS2005) "H264VDec.sln" $(USEDXENV) /REBUILD "$(BUILDCONFIG)" /project "H264VDec" /projectconfig "$(BUILDCONFIG)|Win32"
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
# BackupBinary:
#  %Backup Binary
BackupBinary:
    @echo [BackupBinary]:
    cd "$(SSROOTDIR)"
    -mkdir .\Binary
    -mkdir .\Binary\Bin
    -mkdir .\Binary\Bin\TRMacro
    -copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.dll" .\Binary\bin\TRMacro\*.dll
    -copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.ax" .\Binary\bin\TRMacro\*.ax
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
# RunTR:
RunTR:
    @echo [RunTR]:
    cd "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)"
#   copy "$(ROOTVDRIVE)\bin\trexe.exe"                         "$(SSROOTDIR)\Exports\bin"
    perl.exe "$(ROOTVDRIVE)\bin\batchTR.pl"                    "."
#//////////////////////////////////////////////////////////////////////////////////////
    
#//////////////////////////////////////////////////////////////////////////////////////
# RunTRForNoTRMacro:
RunTRForNoTRMacro:
    @echo [RunTRForNoTRMacro]:
    cd "$(SSROOTDIR)"
    -mkdir .\Binary\Bin\TrexeNoTRMacro
    -copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.*"        .\Binary\bin\TrexeNoTRMacro\*.*
    cd "$(SSROOTDIR)\Binary\Bin\TrexeNoTRMacro"
    perl.exe "$(ROOTVDRIVE)\bin\batchTR.pl"                    "."
    -del "$(SSROOTDIR)\Binary\Bin\TrexeNoTRMacro\trexe.exe"
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
# do_nothing: is a dummy action
do_nothing:
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
#// AdditionalCL:
AdditionalCL:
    @echo [AdditionalCL]:
#// Use GenAppConfigHeader.vbs to generate SDKConfigDef.h for build target
#// It need to input two arguments: original SDKConfigDef.h path and target name
    $(CSCRIPT) $(GENSDKCONFIGVBS) "$(SSROOTDIR)\SDKConfigDef.h" "$(IVI_MEDIA)"
    set CL=%%CL%%
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
# Deposit:
#  %deposit to ...
Deposit:
    @echo [Deposit]:
    cd "$(SSROOTDIR)"
    @echo _BUILD_NAME_=$(IVI_MEDIA)
    -mkdir .\Binary
    -mkdir .\Binary\Bin
    -mkdir .\Binary\Doc
    -mkdir .\Binary\Inc
    -mkdir .\Binary\Lib
    copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.dll" .\Binary\bin\*.dll
    -xcopy "$(SSROOTDIR)\Exports\Doc" "$(SSROOTDIR)\Binary\Doc" /S /E /Y
    -xcopy "$(SSROOTDIR)\Exports\Inc" "$(SSROOTDIR)\Binary\Inc" /S /E /Y
    -xcopy "$(SSROOTDIR)\Exports\Lib" "$(SSROOTDIR)\Binary\Lib" /S /E /Y
!   IF $(COPYRGN)==1
        copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.rgn" .\Binary\bin\*.rgn
!   ENDIF
!   IF $(COPYPDB_MAP)==1
        copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.map" .\Binary\bin\*.map
        copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.pdb" .\Binary\bin\*.pdb
!   ENDIF
    @echo NMAKE_COMPLETED
#//////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////
#// EnvDefine:
#  %set CL compiler options
EnvDefine:
    @echo [EnvDefine]:
    set VSINSTALLDIR=$(MVS2005ROOTDIR)
    set VCINSTALLDIR=$(MVC2005ROOTDIR)
    set FrameworkDir=$(FRAMEWORKDIR2005)
    set FrameworkVersion=$(FRAMEWORKVER2005)
    set FrameworkSDKDir=$(MVS2005FWROOTDIR)
    set INCLUDE=$(BOOST1350INC1);$(WSDK6INC1);$(WSDK6INC2);$(WSDK6INC3);$(DXSDKFEB2006INC1);$(MVS2005INC1);$(MVS2005INC2);$(MVS2005INC3);$(MVS2005INC4);$(MVS2005INC5);$(MVS2005INC6);%%INCLUDE%%
    set LIB=$(BOOST1350LIB1);$(WSDK6LIB1);$(DXSDKFEB2006LIB1);$(MVS2005LIB1);$(MVS2005LIB2);$(MVS2005LIB3);$(MVS2005LIB4);$(MVS2005LIB5);$(MVS2005LIB6);$(MVS2005LIB7);%%LIB%%
    set PATH=$(WSDK6BIN1);$(MVS2005ROOTDIR)\Common7\IDE;$(MVC2005ROOTDIR)\BIN;$(MVS2005ROOTDIR)\Common7\Tools;$(MVS2005ROOTDIR)\Common7\Tools\bin;$(MVC2005ROOTDIR)\PlatformSDK\bin;$(MVS2005FWROOTDIR)\bin;$(FRAMEWORKDIR2005)\$(FRAMEWORKVER2005);$(MVC2005ROOTDIR)\VCPackages;%%PATH%%
    set LIBPATH=$(FRAMEWORKDIR2005)\$(FRAMEWORKVER2005);$(MVC2005ROOTDIR)\ATLMFC\LIB;%%LIBPATH%%

#//////////////////////////////////////////////////////////////////////////////////////
#// EnvMakeAutoSet:
#  %set TRSDK Env.
!IF EXIST("$(SSROOTDIR)\Imports\Inc\TRSDK_V1\TRSDKv1makefile")
!INCLUDE $(SSROOTDIR)\Imports\Inc\TRSDK_V1\TRSDKv1makefile
!ENDIF