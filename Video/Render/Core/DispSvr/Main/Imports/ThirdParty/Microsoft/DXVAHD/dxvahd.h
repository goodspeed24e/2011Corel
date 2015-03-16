

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0542 */
/* Compiler settings for dxvahd.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __dxvahd_h__
#define __dxvahd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDXVAHD_Device_FWD_DEFINED__
#define __IDXVAHD_Device_FWD_DEFINED__
typedef interface IDXVAHD_Device IDXVAHD_Device;
#endif 	/* __IDXVAHD_Device_FWD_DEFINED__ */


#ifndef __IDXVAHD_VideoProcessor_FWD_DEFINED__
#define __IDXVAHD_VideoProcessor_FWD_DEFINED__
typedef interface IDXVAHD_VideoProcessor IDXVAHD_VideoProcessor;
#endif 	/* __IDXVAHD_VideoProcessor_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_dxvahd_0000_0000 */
/* [local] */ 

#if 0
typedef DWORD IDirect3DDevice9Ex;

typedef DWORD IDirect3DSurface9;

typedef DWORD D3DCOLOR;

typedef DWORD D3DFORMAT;

typedef DWORD D3DPOOL;

#endif // 0
#if defined(_WIN32) && !defined(_NO_COM)




DEFINE_GUID(IID_IDXVAHD_Device,         0x95f12dfd,0xd77e,0x49be,0x81,0x5f,0x57,0xd5,0x79,0x63,0x4d,0x6d);

DEFINE_GUID(IID_IDXVAHD_VideoProcessor, 0x95f4edf4,0x6e03,0x4cd7,0xbe,0x1b,0x30,0x75,0xd6,0x65,0xaa,0x52);




#endif


typedef 
enum _DXVAHD_FRAME_FORMAT
    {	DXVAHD_FRAME_FORMAT_PROGRESSIVE	= 0,
	DXVAHD_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST	= 1,
	DXVAHD_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST	= 2
    } 	DXVAHD_FRAME_FORMAT;

typedef 
enum _DXVAHD_DEVICE_USAGE
    {	DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL	= 0,
	DXVAHD_DEVICE_USAGE_OPTIMAL_SPEED	= 1,
	DXVAHD_DEVICE_USAGE_OPTIMAL_QUALITY	= 2
    } 	DXVAHD_DEVICE_USAGE;

typedef 
enum _DXVAHD_SURFACE_TYPE
    {	DXVAHD_SURFACE_TYPE_VIDEO_INPUT	= 0,
	DXVAHD_SURFACE_TYPE_VIDEO_INPUT_PRIVATE	= 1,
	DXVAHD_SURFACE_TYPE_VIDEO_OUTPUT	= 2
    } 	DXVAHD_SURFACE_TYPE;

typedef 
enum _DXVAHD_DEVICE_TYPE
    {	DXVAHD_DEVICE_TYPE_HARDWARE	= 0,
	DXVAHD_DEVICE_TYPE_SOFTWARE	= 1,
	DXVAHD_DEVICE_TYPE_REFERENCE	= 2,
	DXVAHD_DEVICE_TYPE_OTHER	= 3
    } 	DXVAHD_DEVICE_TYPE;

typedef 
enum _DXVAHD_DEVICE_CAPS
    {	DXVAHD_DEVICE_CAPS_LINEAR_SPACE	= 0x1,
	DXVAHD_DEVICE_CAPS_xvYCC	= 0x2,
	DXVAHD_DEVICE_CAPS_RGB_RANGE_CONVERSION	= 0x4,
	DXVAHD_DEVICE_CAPS_YCbCr_MATRIX_CONVERSION	= 0x8
    } 	DXVAHD_DEVICE_CAPS;

typedef 
enum _DXVAHD_FEATURE_CAPS
    {	DXVAHD_FEATURE_CAPS_ALPHA_FILL	= 0x1,
	DXVAHD_FEATURE_CAPS_CONSTRICTION	= 0x2,
	DXVAHD_FEATURE_CAPS_LUMA_KEY	= 0x4
    } 	DXVAHD_FEATURE_CAPS;

typedef 
enum _DXVAHD_FILTER_CAPS
    {	DXVAHD_FILTER_CAPS_BRIGHTNESS	= 0x1,
	DXVAHD_FILTER_CAPS_CONTRAST	= 0x2,
	DXVAHD_FILTER_CAPS_HUE	= 0x4,
	DXVAHD_FILTER_CAPS_SATURATION	= 0x8,
	DXVAHD_FILTER_CAPS_NOISE_REDUCTION	= 0x10,
	DXVAHD_FILTER_CAPS_EDGE_ENHANCEMENT	= 0x20,
	DXVAHD_FILTER_CAPS_ANAMORPHIC_SCALING	= 0x40
    } 	DXVAHD_FILTER_CAPS;

typedef 
enum _DXVAHD_INPUT_FORMAT_CAPS
    {	DXVAHD_INPUT_FORMAT_CAPS_RGB_INTERLACED	= 0x1,
	DXVAHD_INPUT_FORMAT_CAPS_RGB_PROCAMP	= 0x2,
	DXVAHD_INPUT_FORMAT_CAPS_RGB_LUMA_KEY	= 0x4
    } 	DXVAHD_INPUT_FORMAT_CAPS;

typedef 
enum _DXVAHD_PROCESSOR_CAPS
    {	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BLEND	= 0x1,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_BOB	= 0x2,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE	= 0x4,
	DXVAHD_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION	= 0x8,
	DXVAHD_PROCESSOR_CAPS_INVERSE_TELECINE	= 0x10,
	DXVAHD_PROCESSOR_CAPS_FRAME_RATE_CONVERSION	= 0x20
    } 	DXVAHD_PROCESSOR_CAPS;

typedef 
enum _DXVAHD_ITELECINE_CAPS
    {	DXVAHD_ITELECINE_CAPS_32	= 0x1,
	DXVAHD_ITELECINE_CAPS_22	= 0x2,
	DXVAHD_ITELECINE_CAPS_2224	= 0x4,
	DXVAHD_ITELECINE_CAPS_2332	= 0x8,
	DXVAHD_ITELECINE_CAPS_32322	= 0x10,
	DXVAHD_ITELECINE_CAPS_55	= 0x20,
	DXVAHD_ITELECINE_CAPS_64	= 0x40,
	DXVAHD_ITELECINE_CAPS_87	= 0x80,
	DXVAHD_ITELECINE_CAPS_222222222223	= 0x100,
	DXVAHD_ITELECINE_CAPS_OTHER	= 0x80000000
    } 	DXVAHD_ITELECINE_CAPS;

typedef 
enum _DXVAHD_FILTER
    {	DXVAHD_FILTER_BRIGHTNESS	= 0,
	DXVAHD_FILTER_CONTRAST	= 1,
	DXVAHD_FILTER_HUE	= 2,
	DXVAHD_FILTER_SATURATION	= 3,
	DXVAHD_FILTER_NOISE_REDUCTION	= 4,
	DXVAHD_FILTER_EDGE_ENHANCEMENT	= 5,
	DXVAHD_FILTER_ANAMORPHIC_SCALING	= 6
    } 	DXVAHD_FILTER;

typedef 
enum _DXVAHD_BLT_STATE
    {	DXVAHD_BLT_STATE_TARGET_RECT	= 0,
	DXVAHD_BLT_STATE_BACKGROUND_COLOR	= 1,
	DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE	= 2,
	DXVAHD_BLT_STATE_ALPHA_FILL	= 3,
	DXVAHD_BLT_STATE_CONSTRICTION	= 4,
	DXVAHD_BLT_STATE_PRIVATE	= 1000
    } 	DXVAHD_BLT_STATE;

typedef 
enum _DXVAHD_ALPHA_FILL_MODE
    {	DXVAHD_ALPHA_FILL_MODE_OPAQUE	= 0,
	DXVAHD_ALPHA_FILL_MODE_BACKGROUND	= 1,
	DXVAHD_ALPHA_FILL_MODE_DESTINATION	= 2,
	DXVAHD_ALPHA_FILL_MODE_SOURCE_STREAM	= 3
    } 	DXVAHD_ALPHA_FILL_MODE;

typedef 
enum _DXVAHD_STREAM_STATE
    {	DXVAHD_STREAM_STATE_D3DFORMAT	= 0,
	DXVAHD_STREAM_STATE_FRAME_FORMAT	= 1,
	DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE	= 2,
	DXVAHD_STREAM_STATE_OUTPUT_RATE	= 3,
	DXVAHD_STREAM_STATE_SOURCE_RECT	= 4,
	DXVAHD_STREAM_STATE_DESTINATION_RECT	= 5,
	DXVAHD_STREAM_STATE_ALPHA	= 6,
	DXVAHD_STREAM_STATE_PALETTE	= 7,
	DXVAHD_STREAM_STATE_LUMA_KEY	= 8,
	DXVAHD_STREAM_STATE_ASPECT_RATIO	= 9,
	DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS	= 100,
	DXVAHD_STREAM_STATE_FILTER_CONTRAST	= 101,
	DXVAHD_STREAM_STATE_FILTER_HUE	= 102,
	DXVAHD_STREAM_STATE_FILTER_SATURATION	= 103,
	DXVAHD_STREAM_STATE_FILTER_NOISE_REDUCTION	= 104,
	DXVAHD_STREAM_STATE_FILTER_EDGE_ENHANCEMENT	= 105,
	DXVAHD_STREAM_STATE_FILTER_ANAMORPHIC_SCALING	= 106,
	DXVAHD_STREAM_STATE_PRIVATE	= 1000
    } 	DXVAHD_STREAM_STATE;

typedef 
enum _DXVAHD_OUTPUT_RATE
    {	DXVAHD_OUTPUT_RATE_NORMAL	= 0,
	DXVAHD_OUTPUT_RATE_HALF	= 1,
	DXVAHD_OUTPUT_RATE_CUSTOM	= 2
    } 	DXVAHD_OUTPUT_RATE;

typedef struct _DXVAHD_RATIONAL
    {
    UINT Numerator;
    UINT Denominator;
    } 	DXVAHD_RATIONAL;

typedef struct _DXVAHD_COLOR_RGBA
    {
    FLOAT R;
    FLOAT G;
    FLOAT B;
    FLOAT A;
    } 	DXVAHD_COLOR_RGBA;

typedef struct _DXVAHD_COLOR_YCbCrA
    {
    FLOAT Y;
    FLOAT Cb;
    FLOAT Cr;
    FLOAT A;
    } 	DXVAHD_COLOR_YCbCrA;

typedef union _DXVAHD_COLOR
    {
    DXVAHD_COLOR_RGBA RGB;
    DXVAHD_COLOR_YCbCrA YCbCr;
    } 	DXVAHD_COLOR;

typedef struct _DXVAHD_CONTENT_DESC
    {
    DXVAHD_FRAME_FORMAT InputFrameFormat;
    DXVAHD_RATIONAL InputFrameRate;
    UINT InputWidth;
    UINT InputHeight;
    DXVAHD_RATIONAL OutputFrameRate;
    UINT OutputWidth;
    UINT OutputHeight;
    } 	DXVAHD_CONTENT_DESC;

typedef struct _DXVAHD_VPDEVCAPS
    {
    DXVAHD_DEVICE_TYPE DeviceType;
    UINT DeviceCaps;
    UINT FeatureCaps;
    UINT FilterCaps;
    UINT InputFormatCaps;
    D3DPOOL InputPool;
    UINT OutputFormatCount;
    UINT InputFormatCount;
    UINT VideoProcessorCount;
    UINT MaxInputStreams;
    UINT MaxStreamStates;
    } 	DXVAHD_VPDEVCAPS;

typedef struct _DXVAHD_VPCAPS
    {
    GUID VPGuid;
    UINT PastFrames;
    UINT FutureFrames;
    UINT ProcessorCaps;
    UINT ITelecineCaps;
    UINT CustomRateCount;
    } 	DXVAHD_VPCAPS;

typedef struct _DXVAHD_CUSTOM_RATE_DATA
    {
    DXVAHD_RATIONAL CustomRate;
    UINT OutputFrames;
    BOOL InputInterlaced;
    UINT InputFramesOrFields;
    } 	DXVAHD_CUSTOM_RATE_DATA;

typedef struct _DXVAHD_FILTER_RANGE_DATA
    {
    INT Minimum;
    INT Maximum;
    INT Default;
    FLOAT Multiplier;
    } 	DXVAHD_FILTER_RANGE_DATA;

typedef struct _DXVAHD_BLT_STATE_TARGET_RECT_DATA
    {
    BOOL Enable;
    RECT TargetRect;
    } 	DXVAHD_BLT_STATE_TARGET_RECT_DATA;

typedef struct _DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA
    {
    BOOL YCbCr;
    DXVAHD_COLOR BackgroundColor;
    } 	DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA;

typedef struct _DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA
    {
    UINT Usage	: 1;
    UINT RGB_Range	: 1;
    UINT YCbCr_Matrix	: 1;
    UINT YCbCr_xvYCC	: 1;
    } 	DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA;

typedef struct _DXVAHD_BLT_STATE_ALPHA_FILL_DATA
    {
    DXVAHD_ALPHA_FILL_MODE Mode;
    UINT StreamNumber;
    } 	DXVAHD_BLT_STATE_ALPHA_FILL_DATA;

typedef struct _DXVAHD_BLT_STATE_CONSTRICTION_DATA
    {
    BOOL Enable;
    SIZE Size;
    } 	DXVAHD_BLT_STATE_CONSTRICTION_DATA;

typedef struct _DXVAHD_BLT_STATE_PRIVATE_DATA
    {
    GUID Guid;
    UINT DataSize;
    void *pData;
    } 	DXVAHD_BLT_STATE_PRIVATE_DATA;

typedef struct _DXVAHD_STREAM_STATE_D3DFORMAT_DATA
    {
    D3DFORMAT Format;
    } 	DXVAHD_STREAM_STATE_D3DFORMAT_DATA;

typedef struct _DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA
    {
    DXVAHD_FRAME_FORMAT FrameFormat;
    } 	DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA;

typedef struct _DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA
    {
    UINT Type	: 1;
    UINT RGB_Range	: 1;
    UINT YCbCr_Matrix	: 1;
    UINT YCbCr_xvYCC	: 1;
    } 	DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA;

typedef struct _DXVAHD_STREAM_STATE_OUTPUT_RATE_DATA
    {
    BOOL RepeatFrame;
    DXVAHD_OUTPUT_RATE OutputRate;
    DXVAHD_RATIONAL CustomRate;
    } 	DXVAHD_STREAM_STATE_OUTPUT_RATE_DATA;

typedef struct _DXVAHD_STREAM_STATE_SOURCE_RECT_DATA
    {
    BOOL Enable;
    RECT SourceRect;
    } 	DXVAHD_STREAM_STATE_SOURCE_RECT_DATA;

typedef struct _DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA
    {
    BOOL Enable;
    RECT DestinationRect;
    } 	DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA;

typedef struct _DXVAHD_STREAM_STATE_ALPHA_DATA
    {
    BOOL Enable;
    FLOAT Alpha;
    } 	DXVAHD_STREAM_STATE_ALPHA_DATA;

typedef struct _DXVAHD_STREAM_STATE_PALETTE_DATA
    {
    UINT Count;
    D3DCOLOR *pEntries;
    } 	DXVAHD_STREAM_STATE_PALETTE_DATA;

typedef struct _DXVAHD_STREAM_STATE_LUMA_KEY_DATA
    {
    BOOL Enable;
    FLOAT Lower;
    FLOAT Upper;
    } 	DXVAHD_STREAM_STATE_LUMA_KEY_DATA;

typedef struct _DXVAHD_STREAM_STATE_ASPECT_RATIO_DATA
    {
    BOOL Enable;
    DXVAHD_RATIONAL SourceAspectRatio;
    DXVAHD_RATIONAL DestinationAspectRatio;
    } 	DXVAHD_STREAM_STATE_ASPECT_RATIO_DATA;

typedef struct _DXVAHD_STREAM_STATE_FILTER_DATA
    {
    BOOL Enable;
    INT Level;
    } 	DXVAHD_STREAM_STATE_FILTER_DATA;

typedef struct _DXVAHD_STREAM_STATE_PRIVATE_DATA
    {
    GUID Guid;
    UINT DataSize;
    void *pData;
    } 	DXVAHD_STREAM_STATE_PRIVATE_DATA;

typedef struct _DXVAHD_STREAM_DATA
    {
    BOOL Enable;
    UINT OutputIndex;
    UINT InputFrameOrField;
    UINT PastFrames;
    UINT FutureFrames;
    IDirect3DSurface9 **ppPastSurfaces;
    IDirect3DSurface9 *pInputSurface;
    IDirect3DSurface9 **ppFutureSurfaces;
    } 	DXVAHD_STREAM_DATA;





DEFINE_GUID(DXVAHD_STREAM_STATE_PRIVATE_IVTC, 0x9c601e3c,0x0f33,0x414c,0xa7,0x39,0x99,0x54,0x0e,0xe4,0x2d,0xa5);




typedef struct _DXVAHD_STREAM_STATE_PRIVATE_IVTC_DATA
    {
    BOOL Enable;
    UINT ITelecineFlags;
    UINT Frames;
    UINT InputField;
    } 	DXVAHD_STREAM_STATE_PRIVATE_IVTC_DATA;



extern RPC_IF_HANDLE __MIDL_itf_dxvahd_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxvahd_0000_0000_v0_0_s_ifspec;

#ifndef __IDXVAHD_Device_INTERFACE_DEFINED__
#define __IDXVAHD_Device_INTERFACE_DEFINED__

/* interface IDXVAHD_Device */
/* [local][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDXVAHD_Device;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95f12dfd-d77e-49be-815f-57d579634d6d")
    IDXVAHD_Device : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateVideoSurface( 
            /* [annotation][in] */ 
            __in  UINT Width,
            /* [annotation][in] */ 
            __in  UINT Height,
            /* [annotation][in] */ 
            __in  D3DFORMAT Format,
            /* [annotation][in] */ 
            __in  D3DPOOL Pool,
            /* [annotation][in] */ 
            __in  DWORD Usage,
            /* [annotation][in] */ 
            __in  DXVAHD_SURFACE_TYPE Type,
            /* [annotation][in] */ 
            __in  UINT NumSurfaces,
            /* [annotation][size_is][out] */ 
            __out_ecount(NumSurfaces)  IDirect3DSurface9 **ppSurfaces,
            /* [annotation][out][in] */ 
            __inout_opt  HANDLE *pSharedHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorDeviceCaps( 
            /* [annotation][out] */ 
            __out  DXVAHD_VPDEVCAPS *pCaps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorOutputFormats( 
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  D3DFORMAT *pFormats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorInputFormats( 
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  D3DFORMAT *pFormats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCaps( 
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  DXVAHD_VPCAPS *pCaps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCustomRates( 
            /* [annotation][in] */ 
            __in  const GUID *pVPGuid,
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  DXVAHD_CUSTOM_RATE_DATA *pRates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorFilterRange( 
            /* [annotation][in] */ 
            __in  DXVAHD_FILTER Filter,
            /* [annotation][out] */ 
            __out  DXVAHD_FILTER_RANGE_DATA *pRange) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateVideoProcessor( 
            /* [annotation][in] */ 
            __in  const GUID *pVPGuid,
            /* [annotation][out] */ 
            __deref_out  IDXVAHD_VideoProcessor **ppVideoProcessor) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDXVAHD_DeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXVAHD_Device * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXVAHD_Device * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXVAHD_Device * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoSurface )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  UINT Width,
            /* [annotation][in] */ 
            __in  UINT Height,
            /* [annotation][in] */ 
            __in  D3DFORMAT Format,
            /* [annotation][in] */ 
            __in  D3DPOOL Pool,
            /* [annotation][in] */ 
            __in  DWORD Usage,
            /* [annotation][in] */ 
            __in  DXVAHD_SURFACE_TYPE Type,
            /* [annotation][in] */ 
            __in  UINT NumSurfaces,
            /* [annotation][size_is][out] */ 
            __out_ecount(NumSurfaces)  IDirect3DSurface9 **ppSurfaces,
            /* [annotation][out][in] */ 
            __inout_opt  HANDLE *pSharedHandle);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorDeviceCaps )( 
            IDXVAHD_Device * This,
            /* [annotation][out] */ 
            __out  DXVAHD_VPDEVCAPS *pCaps);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorOutputFormats )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  D3DFORMAT *pFormats);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorInputFormats )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  D3DFORMAT *pFormats);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorCaps )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  DXVAHD_VPCAPS *pCaps);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorCustomRates )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  const GUID *pVPGuid,
            /* [annotation][in] */ 
            __in  UINT Count,
            /* [annotation][size_is][out] */ 
            __out_ecount(Count)  DXVAHD_CUSTOM_RATE_DATA *pRates);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessorFilterRange )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  DXVAHD_FILTER Filter,
            /* [annotation][out] */ 
            __out  DXVAHD_FILTER_RANGE_DATA *pRange);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoProcessor )( 
            IDXVAHD_Device * This,
            /* [annotation][in] */ 
            __in  const GUID *pVPGuid,
            /* [annotation][out] */ 
            __deref_out  IDXVAHD_VideoProcessor **ppVideoProcessor);
        
        END_INTERFACE
    } IDXVAHD_DeviceVtbl;

    interface IDXVAHD_Device
    {
        CONST_VTBL struct IDXVAHD_DeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXVAHD_Device_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDXVAHD_Device_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDXVAHD_Device_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDXVAHD_Device_CreateVideoSurface(This,Width,Height,Format,Pool,Usage,Type,NumSurfaces,ppSurfaces,pSharedHandle)	\
    ( (This)->lpVtbl -> CreateVideoSurface(This,Width,Height,Format,Pool,Usage,Type,NumSurfaces,ppSurfaces,pSharedHandle) ) 

#define IDXVAHD_Device_GetVideoProcessorDeviceCaps(This,pCaps)	\
    ( (This)->lpVtbl -> GetVideoProcessorDeviceCaps(This,pCaps) ) 

#define IDXVAHD_Device_GetVideoProcessorOutputFormats(This,Count,pFormats)	\
    ( (This)->lpVtbl -> GetVideoProcessorOutputFormats(This,Count,pFormats) ) 

#define IDXVAHD_Device_GetVideoProcessorInputFormats(This,Count,pFormats)	\
    ( (This)->lpVtbl -> GetVideoProcessorInputFormats(This,Count,pFormats) ) 

#define IDXVAHD_Device_GetVideoProcessorCaps(This,Count,pCaps)	\
    ( (This)->lpVtbl -> GetVideoProcessorCaps(This,Count,pCaps) ) 

#define IDXVAHD_Device_GetVideoProcessorCustomRates(This,pVPGuid,Count,pRates)	\
    ( (This)->lpVtbl -> GetVideoProcessorCustomRates(This,pVPGuid,Count,pRates) ) 

#define IDXVAHD_Device_GetVideoProcessorFilterRange(This,Filter,pRange)	\
    ( (This)->lpVtbl -> GetVideoProcessorFilterRange(This,Filter,pRange) ) 

#define IDXVAHD_Device_CreateVideoProcessor(This,pVPGuid,ppVideoProcessor)	\
    ( (This)->lpVtbl -> CreateVideoProcessor(This,pVPGuid,ppVideoProcessor) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDXVAHD_Device_INTERFACE_DEFINED__ */


#ifndef __IDXVAHD_VideoProcessor_INTERFACE_DEFINED__
#define __IDXVAHD_VideoProcessor_INTERFACE_DEFINED__

/* interface IDXVAHD_VideoProcessor */
/* [local][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDXVAHD_VideoProcessor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95f4edf4-6e03-4cd7-be1b-3075d665aa52")
    IDXVAHD_VideoProcessor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetVideoProcessBltState( 
            /* [annotation][in] */ 
            __in  DXVAHD_BLT_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][in] */ 
            __in_bcount(DataSize)  const void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessBltState( 
            /* [annotation][in] */ 
            __in  DXVAHD_BLT_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][out] */ 
            __inout_bcount(DataSize)  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVideoProcessStreamState( 
            /* [annotation][in] */ 
            __in  UINT StreamNumber,
            /* [annotation][in] */ 
            __in  DXVAHD_STREAM_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][in] */ 
            __in_bcount(DataSize)  const void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoProcessStreamState( 
            /* [annotation][in] */ 
            __in  UINT StreamNumber,
            /* [annotation][in] */ 
            __in  DXVAHD_STREAM_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][out] */ 
            __inout_bcount(DataSize)  void *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VideoProcessBltHD( 
            /* [annotation][in] */ 
            __in  IDirect3DSurface9 *pOutputSurface,
            /* [annotation][in] */ 
            __in  UINT OutputFrame,
            /* [annotation][in] */ 
            __in  UINT StreamCount,
            /* [annotation][size_is][in] */ 
            __in_ecount(StreamCount)  const DXVAHD_STREAM_DATA *pStreams) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDXVAHD_VideoProcessorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXVAHD_VideoProcessor * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXVAHD_VideoProcessor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXVAHD_VideoProcessor * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetVideoProcessBltState )( 
            IDXVAHD_VideoProcessor * This,
            /* [annotation][in] */ 
            __in  DXVAHD_BLT_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][in] */ 
            __in_bcount(DataSize)  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessBltState )( 
            IDXVAHD_VideoProcessor * This,
            /* [annotation][in] */ 
            __in  DXVAHD_BLT_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][out] */ 
            __inout_bcount(DataSize)  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetVideoProcessStreamState )( 
            IDXVAHD_VideoProcessor * This,
            /* [annotation][in] */ 
            __in  UINT StreamNumber,
            /* [annotation][in] */ 
            __in  DXVAHD_STREAM_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][in] */ 
            __in_bcount(DataSize)  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoProcessStreamState )( 
            IDXVAHD_VideoProcessor * This,
            /* [annotation][in] */ 
            __in  UINT StreamNumber,
            /* [annotation][in] */ 
            __in  DXVAHD_STREAM_STATE State,
            /* [annotation][in] */ 
            __in  UINT DataSize,
            /* [annotation][out] */ 
            __inout_bcount(DataSize)  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *VideoProcessBltHD )( 
            IDXVAHD_VideoProcessor * This,
            /* [annotation][in] */ 
            __in  IDirect3DSurface9 *pOutputSurface,
            /* [annotation][in] */ 
            __in  UINT OutputFrame,
            /* [annotation][in] */ 
            __in  UINT StreamCount,
            /* [annotation][size_is][in] */ 
            __in_ecount(StreamCount)  const DXVAHD_STREAM_DATA *pStreams);
        
        END_INTERFACE
    } IDXVAHD_VideoProcessorVtbl;

    interface IDXVAHD_VideoProcessor
    {
        CONST_VTBL struct IDXVAHD_VideoProcessorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXVAHD_VideoProcessor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDXVAHD_VideoProcessor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDXVAHD_VideoProcessor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDXVAHD_VideoProcessor_SetVideoProcessBltState(This,State,DataSize,pData)	\
    ( (This)->lpVtbl -> SetVideoProcessBltState(This,State,DataSize,pData) ) 

#define IDXVAHD_VideoProcessor_GetVideoProcessBltState(This,State,DataSize,pData)	\
    ( (This)->lpVtbl -> GetVideoProcessBltState(This,State,DataSize,pData) ) 

#define IDXVAHD_VideoProcessor_SetVideoProcessStreamState(This,StreamNumber,State,DataSize,pData)	\
    ( (This)->lpVtbl -> SetVideoProcessStreamState(This,StreamNumber,State,DataSize,pData) ) 

#define IDXVAHD_VideoProcessor_GetVideoProcessStreamState(This,StreamNumber,State,DataSize,pData)	\
    ( (This)->lpVtbl -> GetVideoProcessStreamState(This,StreamNumber,State,DataSize,pData) ) 

#define IDXVAHD_VideoProcessor_VideoProcessBltHD(This,pOutputSurface,OutputFrame,StreamCount,pStreams)	\
    ( (This)->lpVtbl -> VideoProcessBltHD(This,pOutputSurface,OutputFrame,StreamCount,pStreams) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDXVAHD_VideoProcessor_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_dxvahd_0000_0002 */
/* [local] */ 





typedef HRESULT (CALLBACK* PDXVAHDSW_CreateDevice)(

    __in IDirect3DDevice9Ex* pD3DDevice,

    __out HANDLE* phDevice

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_ProposeVideoPrivateFormat)(

    __in HANDLE hDevice,

    __inout D3DFORMAT* pFormat

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorDeviceCaps)(

    __in HANDLE hDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __out DXVAHD_VPDEVCAPS* pCaps

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorOutputFormats)(

    __in HANDLE hDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __in UINT Count,

    __out_ecount(Count) D3DFORMAT* pFormats

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorInputFormats)(

    __in HANDLE hDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __in UINT Count,

    __out_ecount(Count) D3DFORMAT* pFormats

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorCaps)(

    __in HANDLE hDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __in UINT Count,

    __out_ecount(Count) DXVAHD_VPCAPS* pCaps

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorCustomRates)(

    __in HANDLE hDevice,

    __in const GUID* pVPGuid,

    __in UINT Count,

    __out_ecount(Count) DXVAHD_CUSTOM_RATE_DATA* pRates

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessorFilterRange)(

    __in HANDLE hDevice,

    __in DXVAHD_FILTER Filter,

    __out DXVAHD_FILTER_RANGE_DATA* pRange

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_DestroyDevice)(

    __in HANDLE hDevice

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_CreateVideoProcessor)(

    __in HANDLE hDevice,

    __in const GUID* pVPGuid,

    __out HANDLE* phVideoProcessor

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_SetVideoProcessBltState)(

    __in HANDLE hVideoProcessor,

    __in DXVAHD_BLT_STATE State,

    __in UINT DataSize,

    __in_bcount(DataSize) const void* pData

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessBltStatePrivate)(

    __in HANDLE hVideoProcessor,

    __inout DXVAHD_BLT_STATE_PRIVATE_DATA* pData

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_SetVideoProcessStreamState)(

    __in HANDLE hVideoProcessor,

    __in UINT StreamNumber,

    __in DXVAHD_STREAM_STATE State,

    __in UINT DataSize,

    __in_bcount(DataSize) const void* pData

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_GetVideoProcessStreamStatePrivate)(

    __in HANDLE hVideoProcessor,

    __in UINT StreamNumber,

    __inout DXVAHD_STREAM_STATE_PRIVATE_DATA* pData

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_VideoProcessBltHD)(

    __in HANDLE hVideoProcessor,

    __in IDirect3DSurface9* pOutputSurface,

    __in UINT OutputFrame,

    __in UINT StreamCount,

    __in_ecount(StreamCount) const DXVAHD_STREAM_DATA* pStreams

    );



typedef HRESULT (CALLBACK* PDXVAHDSW_DestroyVideoProcessor)(

    __in HANDLE hVideoProcessor

    );



typedef struct _DXVAHDSW_CALLBACKS

{

    PDXVAHDSW_CreateDevice                      CreateDevice;

    PDXVAHDSW_ProposeVideoPrivateFormat         ProposeVideoPrivateFormat;

    PDXVAHDSW_GetVideoProcessorDeviceCaps       GetVideoProcessorDeviceCaps;

    PDXVAHDSW_GetVideoProcessorOutputFormats    GetVideoProcessorOutputFormats;

    PDXVAHDSW_GetVideoProcessorInputFormats     GetVideoProcessorInputFormats;

    PDXVAHDSW_GetVideoProcessorCaps             GetVideoProcessorCaps;

    PDXVAHDSW_GetVideoProcessorCustomRates      GetVideoProcessorCustomRates;

    PDXVAHDSW_GetVideoProcessorFilterRange      GetVideoProcessorFilterRange;

    PDXVAHDSW_DestroyDevice                     DestroyDevice;

    PDXVAHDSW_CreateVideoProcessor              CreateVideoProcessor;

    PDXVAHDSW_SetVideoProcessBltState           SetVideoProcessBltState;

    PDXVAHDSW_GetVideoProcessBltStatePrivate    GetVideoProcessBltStatePrivate;

    PDXVAHDSW_SetVideoProcessStreamState        SetVideoProcessStreamState;

    PDXVAHDSW_GetVideoProcessStreamStatePrivate GetVideoProcessStreamStatePrivate;

    PDXVAHDSW_VideoProcessBltHD                 VideoProcessBltHD;

    PDXVAHDSW_DestroyVideoProcessor             DestroyVideoProcessor;

} DXVAHDSW_CALLBACKS;



typedef HRESULT (CALLBACK* PDXVAHDSW_Plugin)(

    __in UINT Size,

    __out_bcount(Size) void* pCallbacks

    );








HRESULT WINAPI

DXVAHD_CreateDevice(

    __in IDirect3DDevice9Ex* pD3DDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __in_opt PDXVAHDSW_Plugin pPlugin,

    __deref_out IDXVAHD_Device** ppDevice

    );



typedef HRESULT (WINAPI* PDXVAHD_CreateDevice)(

    __in IDirect3DDevice9Ex* pD3DDevice,

    __in const DXVAHD_CONTENT_DESC* pContentDesc,

    __in DXVAHD_DEVICE_USAGE Usage,

    __in_opt PDXVAHDSW_Plugin pPlugin,

    __deref_out IDXVAHD_Device** ppDevice

    );






extern RPC_IF_HANDLE __MIDL_itf_dxvahd_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxvahd_0000_0002_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


