!IF EXIST("V:\bin\makeSDKDef")
!INCLUDE V:\bin\makeSDKDef
!ENDIF

#//////////////////////////////////////////////////////////////////////////////////////
# PerfII QA Release target
PerfIISDK(QA)-(Release)_all:
    set IVI_MEDIA=PerfIISDK(QA)-(Release)
    $(MYMAKEUTIL) BuildBinary
    $(MYMAKEUTIL) Deposit


#//////////////////////////////////////////////////////////////////////////////////////
# BuildBinary:
#  %Build Binaries
BuildBinary:
    @echo [BuildBinary]:
    cd "$(SSROOTDIR)"
    $(MYMAKEUTIL)   EnvDefine \
                    AdditionalCL \
                    BuildVC8Dog

#//////////////////////////////////////////////////////////////////////////////////////
# BuildVC8Dog:
#  BuildVC8Dog      
BuildVC8Dog:
    @echo [VC8Dog]:
    @echo Compiler flags: %%CL%%
    @echo Linker flags: %%LINK%%
    cd $(SSROOTDIR)
    $(BUILDVS2005) "DOG.sln" $(USEDXENV) /REBUILD "$(BUILDCONFIG)" /project "DOG_DLL" /projectconfig "$(BUILDCONFIG)|Win32"
    $(BUILDVS2005) "DOG.sln" $(USEDXENV) /REBUILD "$(BUILDCONFIG)" /project "Perf2" /projectconfig "$(BUILDCONFIG)|Win32"
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
    copy "$(SSROOTDIR)\Exports\bin\$(BUILDCONFIG)\*.*" .\Binary\bin\*.*
    -xcopy "$(SSROOTDIR)\Exports\Doc" "$(SSROOTDIR)\Binary\Doc" /S /E /Y
    -xcopy "$(SSROOTDIR)\Exports\Inc" "$(SSROOTDIR)\Binary\Inc" /S /E /Y
    -xcopy "$(SSROOTDIR)\Exports\Lib" "$(SSROOTDIR)\Binary\Lib" /S /E /Y
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