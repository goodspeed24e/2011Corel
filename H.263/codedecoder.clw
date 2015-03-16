; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMyView
LastTemplate=CPropertySheet
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "codedecoder.h"
LastPage=0

ClassCount=11
Class1=CMyApp
Class2=CMyDoc
Class3=CMyView
Class4=CMainFrame

ResourceCount=6
Resource1=IDD_EncodeDlg
Resource2=IDD_RateControl
Resource3=IDD_ABOUTBOX
Class5=CAboutDlg
Class6=CDecodeDlg
Class7=CMotionDlg
Class8=CProgressBar
Resource4=IDD_DecodeDlg
Class9=CPropertyDlg
Class10=CRateControlDlg
Resource5=IDR_MAINFRAME
Class11=CEncodeDlg
Resource6=IDD_Motion

[CLS:CMyApp]
Type=0
HeaderFile=codedecoder.h
ImplementationFile=codedecoder.cpp
Filter=N

[CLS:CMyDoc]
Type=0
HeaderFile=codedecoderDoc.h
ImplementationFile=codedecoderDoc.cpp
Filter=N

[CLS:CMyView]
Type=0
HeaderFile=codedecoderView.h
ImplementationFile=codedecoderView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=CMyView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=codedecoder.cpp
ImplementationFile=codedecoder.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_Encode
Command11=ID_Decode
Command12=ID_VIEW_TOOLBAR
Command13=ID_VIEW_STATUS_BAR
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_EDIT_COPY
Command2=ID_Encode
Command3=ID_FILE_NEW
Command4=ID_FILE_OPEN
Command5=ID_FILE_PRINT
Command6=ID_FILE_SAVE
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_NEXT_PANE
Command11=ID_PREV_PANE
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_EDIT_CUT
Command15=ID_EDIT_UNDO
CommandCount=15

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_APP_ABOUT
Command6=ID_Encode
Command7=ID_Decode
CommandCount=7

[CLS:CDecodeDlg]
Type=0
HeaderFile=DecodeDlg.h
ImplementationFile=DecodeDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CDecodeDlg

[DLG:IDD_DecodeDlg]
Type=1
Class=CDecodeDlg
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[DLG:IDD_EncodeDlg]
Type=1
Class=CEncodeDlg
ControlCount=16
Control1=IDC_STATIC,static,1342308352
Control2=IDC_InType,combobox,1344340226
Control3=IDC_STATIC,static,1342308352
Control4=IDC_InitDir,edit,1350631552
Control5=IDC_Browse,button,1342242827
Control6=IDC_STATIC,static,1342308352
Control7=IDC_MaxFrame,edit,1350639744
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_QPI,edit,1350639746
Control11=IDC_QP,edit,1350639746
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308364
Control14=IDC_STATIC,button,1342177287
Control15=IDC_CHECK1,button,1342242819
Control16=IDC_Pnum,edit,1350639746

[CLS:CProgressBar]
Type=0
HeaderFile=ProgressBar.h
ImplementationFile=ProgressBar.cpp
BaseClass=CProgressCtrl
Filter=W
LastObject=CProgressBar
VirtualFilter=NWC

[DLG:IDD_RateControl]
Type=1
Class=CRateControlDlg
ControlCount=8
Control1=IDC_IfRateControl,button,1342242819
Control2=IDC_STATIC,static,1342308352
Control3=IDC_Bitrate,edit,1350631554
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,button,1342177287
Control6=IDC_frame25,button,1342308361
Control7=IDC_frame30,button,1342177289
Control8=IDC_frame10,button,1342177289

[CLS:CPropertyDlg]
Type=0
HeaderFile=PropertyDlg.h
ImplementationFile=PropertyDlg.cpp
BaseClass=CPropertySheet
Filter=W
LastObject=CPropertyDlg

[CLS:CRateControlDlg]
Type=0
HeaderFile=RateControlDlg.h
ImplementationFile=RateControlDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CRateControlDlg

[DLG:IDD_Motion]
Type=1
Class=CMotionDlg
ControlCount=2
Control1=IDC_RADIO1,button,1342308361
Control2=IDC_RADIO2,button,1342177289

[CLS:CMotionDlg]
Type=0
HeaderFile=MotionDlg.h
ImplementationFile=MotionDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC

[CLS:CEncodeDlg]
Type=0
HeaderFile=EncodeDlg.h
ImplementationFile=EncodeDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=IDC_Browse

