#pragma once

#define DXVA1_TEST_ITEM_NUM								5
#define DXVA2_TEST_ITEM_NUM								2
#define CODEC_TEST_ITEM_NUM								4

#define TEST_ITEM_DXVA1_OVERLAY							0x00000001
#define TEST_ITEM_DXVA1_VMR7							0x00000002
#define TEST_ITEM_DXVA1_VMR9WINDOWLESS					0x00000004
#define TEST_ITEM_DXVA1_VMR9CUSTOM						0x00000008
#define TEST_ITEM_DXVA1_EXTERNAL						0x00000010
#define TEST_ITEM_DXVA1									(TEST_ITEM_DXVA1_OVERLAY|TEST_ITEM_DXVA1_VMR7|TEST_ITEM_DXVA1_VMR9WINDOWLESS|TEST_ITEM_DXVA1_VMR9CUSTOM|TEST_ITEM_DXVA1_EXTERNAL)

#define TEST_ITEM_DXVA2_D3D								0x00000100
#define TEST_ITEM_DXVA2_DSHOW							0x00000200
#define TEST_ITEM_DXVA2									(TEST_ITEM_DXVA2_D3D|TEST_ITEM_DXVA2_DSHOW)

#define TEST_CODEC_MPEG2								0x00000001
#define TEST_CODEC_H264									0x00000002
#define TEST_CODEC_VC1									0x00000004
#define TEST_CODEC_MPEG4								0x00000008

#define DEFAULT_CONFIG_FILE								_T("HVDTest.ini")

#define WM_APP_HVDTEST_IN_PROGRESS						WM_APP+1
#define WM_APP_HVDTEST_FINISH							WM_APP+2
#define WM_APP_HVDTEST_CLOSE							WM_APP+3

// For Control Panel
#define ID_CTRLPANEL									100
#define ID_DXVA1_GROUP									101
#define ID_DXVA2_GROUP									102

#define ID_CODEC_GROUP									106

#define ID_DXVA1_OVERLAY								111
#define ID_DXVA1_VMR7									112
#define ID_DXVA1_VMR9WINDOWLESS							113
#define ID_DXVA1_VMR9CUSTOM								114
#define ID_DXVA1_EXTERNAL								115

#define ID_DXVA2_D3D									121
#define ID_DXVA2_DSHOW									122

#define ID_CODEC_MPEG2									151
#define ID_CODEC_H264									152
#define ID_CODEC_VC1									153
#define ID_CODEC_MPEG4									154

#define ID_BTN_TEST										1001

// INI Section
#define SECTION_NAME_GENERAL							_T("General")
//Key in general section
// ini file
#define KEY_NAME_MPEG2INI								_T("MPEG2INI")
#define KEY_NAME_H264INI								_T("H264INI")
#define KEY_NAME_VC1									_T("VC1INI")
#define KEY_NAME_MPEG4INI								_T("MPEG4INI")
//Disable item
#define KEY_NAME_DISABLE_DXVA1_OVERLAY					_T("DisableDXVA1Overlay")
#define KEY_NAME_DISABLE_DXVA1_VMR7						_T("DisableDXVA1VMR7")
#define KEY_NAME_DISABLE_DXVA1_VMR9WINDOWLESS			_T("DisableDXVA1VMR9Windowless")
#define KEY_NAME_DISABLE_DXVA1_VMR9CUSTOM				_T("DisableDXVA1VMR9Custom")
#define KEY_NAME_DISABLE_DXVA1_EXTERNAL					_T("DisableDXVA1External")
//DXVA2
#define KEY_NAME_DISABLE_DXVA2_DIRECT3D					_T("DisableDXVA2Direct3D")
#define KEY_NAME_DISABLE_DXVA2_DIRECTSHOW				_T("DisableDXVA2DirectShow")
//Codec
#define KEY_NAME_DISABLE_CODEC_MPEG2					_T("DisableCodecMPEG2")
#define KEY_NAME_DISABLE_CODEC_H264						_T("DisableCodecH264")
#define KEY_NAME_DISABLE_CODEC_VC1						_T("DisableCodecVC1")
#define KEY_NAME_DISABLE_CODEC_MPEG4					_T("DisableCodecMPEG4")

#define SECTION_NAME_COMMAND							_T("Command")
//Key in Command section
//General command
#define KEY_NAME_AUTOLAUNCH								_T("AutoLaunch")
#define KEY_NAME_AUTOCLOSE								_T("AutoClose")
//DXVA1
#define KEY_NAME_DXVA1_OVERLAY							_T("TestDXVA1Overlay")
#define KEY_NAME_DXVA1_VMR7								_T("TestDXVA1VMR7")
#define KEY_NAME_DXVA1_VMR9WINDOWLESS					_T("TestDXVA1VMR9Windowless")
#define KEY_NAME_DXVA1_VMR9CUSTOM						_T("TestDXVA1VMR9Custom")
#define KEY_NAME_DXVA1_EXTERNAL							_T("TestDXVA1External")
//DXVA2
#define KEY_NAME_DXVA2_DIRECT3D							_T("TestDXVA2Direct3D")
#define KEY_NAME_DXVA2_DIRECTSHOW						_T("TestDXVA2DirectShow")
//Codec
#define KEY_NAME_CODEC_MPEG2							_T("TestCodecMPEG2")
#define KEY_NAME_CODEC_H264								_T("TestCodecH264")
#define KEY_NAME_CODEC_VC1								_T("TestCodecVC1")