

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Fri Sep 19 10:13:22 2008
 */
/* Compiler settings for .\DispSvr.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __DispSvr_h__
#define __DispSvr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDisplayVideoSourceManager_FWD_DEFINED__
#define __IDisplayVideoSourceManager_FWD_DEFINED__
typedef interface IDisplayVideoSourceManager IDisplayVideoSourceManager;
#endif 	/* __IDisplayVideoSourceManager_FWD_DEFINED__ */


#ifndef __IDisplayServer_FWD_DEFINED__
#define __IDisplayServer_FWD_DEFINED__
typedef interface IDisplayServer IDisplayServer;
#endif 	/* __IDisplayServer_FWD_DEFINED__ */


#ifndef __IDisplayVideoSource_FWD_DEFINED__
#define __IDisplayVideoSource_FWD_DEFINED__
typedef interface IDisplayVideoSource IDisplayVideoSource;
#endif 	/* __IDisplayVideoSource_FWD_DEFINED__ */


#ifndef __IDisplayLock_FWD_DEFINED__
#define __IDisplayLock_FWD_DEFINED__
typedef interface IDisplayLock IDisplayLock;
#endif 	/* __IDisplayLock_FWD_DEFINED__ */


#ifndef __IDisplayRenderEngine_FWD_DEFINED__
#define __IDisplayRenderEngine_FWD_DEFINED__
typedef interface IDisplayRenderEngine IDisplayRenderEngine;
#endif 	/* __IDisplayRenderEngine_FWD_DEFINED__ */


#ifndef __IDisplayEventHandler_FWD_DEFINED__
#define __IDisplayEventHandler_FWD_DEFINED__
typedef interface IDisplayEventHandler IDisplayEventHandler;
#endif 	/* __IDisplayEventHandler_FWD_DEFINED__ */


#ifndef __IDisplayEventHost_FWD_DEFINED__
#define __IDisplayEventHost_FWD_DEFINED__
typedef interface IDisplayEventHost IDisplayEventHost;
#endif 	/* __IDisplayEventHost_FWD_DEFINED__ */


#ifndef __IDisplayObject_FWD_DEFINED__
#define __IDisplayObject_FWD_DEFINED__
typedef interface IDisplayObject IDisplayObject;
#endif 	/* __IDisplayObject_FWD_DEFINED__ */


#ifndef __IParentDisplayObject_FWD_DEFINED__
#define __IParentDisplayObject_FWD_DEFINED__
typedef interface IParentDisplayObject IParentDisplayObject;
#endif 	/* __IParentDisplayObject_FWD_DEFINED__ */


#ifndef __IDisplayVideoSink_FWD_DEFINED__
#define __IDisplayVideoSink_FWD_DEFINED__
typedef interface IDisplayVideoSink IDisplayVideoSink;
#endif 	/* __IDisplayVideoSink_FWD_DEFINED__ */


#ifndef __IDisplayVideoMixer_FWD_DEFINED__
#define __IDisplayVideoMixer_FWD_DEFINED__
typedef interface IDisplayVideoMixer IDisplayVideoMixer;
#endif 	/* __IDisplayVideoMixer_FWD_DEFINED__ */


#ifndef __IDisplayProperties_FWD_DEFINED__
#define __IDisplayProperties_FWD_DEFINED__
typedef interface IDisplayProperties IDisplayProperties;
#endif 	/* __IDisplayProperties_FWD_DEFINED__ */


#ifndef __IDisplayOptimizedRender_FWD_DEFINED__
#define __IDisplayOptimizedRender_FWD_DEFINED__
typedef interface IDisplayOptimizedRender IDisplayOptimizedRender;
#endif 	/* __IDisplayOptimizedRender_FWD_DEFINED__ */


#ifndef __IDisplayServerStateEventSink_FWD_DEFINED__
#define __IDisplayServerStateEventSink_FWD_DEFINED__
typedef interface IDisplayServerStateEventSink IDisplayServerStateEventSink;
#endif 	/* __IDisplayServerStateEventSink_FWD_DEFINED__ */


#ifndef __IDispSvrRenderEngineNotify_FWD_DEFINED__
#define __IDispSvrRenderEngineNotify_FWD_DEFINED__
typedef interface IDispSvrRenderEngineNotify IDispSvrRenderEngineNotify;
#endif 	/* __IDispSvrRenderEngineNotify_FWD_DEFINED__ */


#ifndef __DisplayServer_FWD_DEFINED__
#define __DisplayServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class DisplayServer DisplayServer;
#else
typedef struct DisplayServer DisplayServer;
#endif /* __cplusplus */

#endif 	/* __DisplayServer_FWD_DEFINED__ */


#ifndef __DisplayRenderEngine_FWD_DEFINED__
#define __DisplayRenderEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class DisplayRenderEngine DisplayRenderEngine;
#else
typedef struct DisplayRenderEngine DisplayRenderEngine;
#endif /* __cplusplus */

#endif 	/* __DisplayRenderEngine_FWD_DEFINED__ */


#ifndef __CompositeDisplayObject_FWD_DEFINED__
#define __CompositeDisplayObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class CompositeDisplayObject CompositeDisplayObject;
#else
typedef struct CompositeDisplayObject CompositeDisplayObject;
#endif /* __cplusplus */

#endif 	/* __CompositeDisplayObject_FWD_DEFINED__ */


#ifndef __VideoRootDisplayObject_FWD_DEFINED__
#define __VideoRootDisplayObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class VideoRootDisplayObject VideoRootDisplayObject;
#else
typedef struct VideoRootDisplayObject VideoRootDisplayObject;
#endif /* __cplusplus */

#endif 	/* __VideoRootDisplayObject_FWD_DEFINED__ */


#ifndef __ServerStateDisplayObject_FWD_DEFINED__
#define __ServerStateDisplayObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class ServerStateDisplayObject ServerStateDisplayObject;
#else
typedef struct ServerStateDisplayObject ServerStateDisplayObject;
#endif /* __cplusplus */

#endif 	/* __ServerStateDisplayObject_FWD_DEFINED__ */


#ifndef __ShineDisplayObject_FWD_DEFINED__
#define __ShineDisplayObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShineDisplayObject ShineDisplayObject;
#else
typedef struct ShineDisplayObject ShineDisplayObject;
#endif /* __cplusplus */

#endif 	/* __ShineDisplayObject_FWD_DEFINED__ */


#ifndef __DisplayVideoMixer_FWD_DEFINED__
#define __DisplayVideoMixer_FWD_DEFINED__

#ifdef __cplusplus
typedef class DisplayVideoMixer DisplayVideoMixer;
#else
typedef struct DisplayVideoMixer DisplayVideoMixer;
#endif /* __cplusplus */

#endif 	/* __DisplayVideoMixer_FWD_DEFINED__ */


#ifndef __VideoSourceDisplayObject_FWD_DEFINED__
#define __VideoSourceDisplayObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class VideoSourceDisplayObject VideoSourceDisplayObject;
#else
typedef struct VideoSourceDisplayObject VideoSourceDisplayObject;
#endif /* __cplusplus */

#endif 	/* __VideoSourceDisplayObject_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_DispSvr_0000_0000 */
/* [local] */ 

#if 0
typedef LPVOID *IBaseFilter;

typedef LPVOID *IFilterGraph;

#endif
#include <d3d9.h>
#include <dshow.h>
#include <ddraw.h>
#ifndef VIDEO_SOURCE_TAG
#define VIDEO_SOURCE_TAG 0x12345
#endif
#if 0
typedef struct _NORMALIZEDRECT
    {
    float left;
    float top;
    float right;
    float bottom;
    } 	NORMALIZEDRECT;

typedef struct _NORMALIZEDRECT *PNORMALIZEDRECT;

#endif








extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0000_v0_0_s_ifspec;

#ifndef __IDisplayVideoSourceManager_INTERFACE_DEFINED__
#define __IDisplayVideoSourceManager_INTERFACE_DEFINED__

/* interface IDisplayVideoSourceManager */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayVideoSourceManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56A32D0E-5649-4b5a-A746-D94E56E9CAF8")
    IDisplayVideoSourceManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddVideoSource( 
            /* [in] */ IBaseFilter *pVMR,
            /* [out] */ IDisplayVideoSource **ppVidSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveVideoSource( 
            /* [in] */ IDisplayVideoSource *pVidSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoSourceCount( 
            /* [out] */ LONG *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoSourceByIndex( 
            /* [in] */ LONG lIndex,
            /* [out] */ IDisplayVideoSource **ppVideoSource) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayVideoSourceManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayVideoSourceManager * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayVideoSourceManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayVideoSourceManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddVideoSource )( 
            IDisplayVideoSourceManager * This,
            /* [in] */ IBaseFilter *pVMR,
            /* [out] */ IDisplayVideoSource **ppVidSrc);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveVideoSource )( 
            IDisplayVideoSourceManager * This,
            /* [in] */ IDisplayVideoSource *pVidSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSourceCount )( 
            IDisplayVideoSourceManager * This,
            /* [out] */ LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSourceByIndex )( 
            IDisplayVideoSourceManager * This,
            /* [in] */ LONG lIndex,
            /* [out] */ IDisplayVideoSource **ppVideoSource);
        
        END_INTERFACE
    } IDisplayVideoSourceManagerVtbl;

    interface IDisplayVideoSourceManager
    {
        CONST_VTBL struct IDisplayVideoSourceManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayVideoSourceManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayVideoSourceManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayVideoSourceManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayVideoSourceManager_AddVideoSource(This,pVMR,ppVidSrc)	\
    ( (This)->lpVtbl -> AddVideoSource(This,pVMR,ppVidSrc) ) 

#define IDisplayVideoSourceManager_RemoveVideoSource(This,pVidSrc)	\
    ( (This)->lpVtbl -> RemoveVideoSource(This,pVidSrc) ) 

#define IDisplayVideoSourceManager_GetVideoSourceCount(This,plCount)	\
    ( (This)->lpVtbl -> GetVideoSourceCount(This,plCount) ) 

#define IDisplayVideoSourceManager_GetVideoSourceByIndex(This,lIndex,ppVideoSource)	\
    ( (This)->lpVtbl -> GetVideoSourceByIndex(This,lIndex,ppVideoSource) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayVideoSourceManager_INTERFACE_DEFINED__ */


#ifndef __IDisplayServer_INTERFACE_DEFINED__
#define __IDisplayServer_INTERFACE_DEFINED__

/* interface IDisplayServer */
/* [unique][helpstring][uuid][object][local] */ 


enum DisplayServerInitFlags
    {	DISPSVR_NO_RENDER_THREAD	= 0x1,
	DISPSVR_SUSPENDED	= 0x2,
	DISPSVR_NO_WINDOW_OUTPUT	= 0x4,
	DISPSVR_USE_CUSTOMIZED_OUTPUT	= 0x20,
	DISPSVR_WAITING_FOR_VSYNC	= 0x40,
	DISPSVR_DEVICE_LOST_NOTIFY	= 0x80,
	DISPSVR_DETECT_D3D_HIJACK	= 0x100,
	DISPSVR_FPU_PRESERVE	= 0x200,
	DISPSVR_USE_D3D9EX	= 0x400,
	DISPSVR_USE_RT_VIRTUALIZATION	= 0x800,
	DISPSVR_USE_STENCIL_BUFFER	= 0x1000,
	DISPSVR_USE_EXCLUSIVE_MODE	= 0x2000,
	DISPSVR_USE_MESSAGE_THREAD	= 0x4000
    } ;

EXTERN_C const IID IID_IDisplayServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("32A4CD3D-47C3-476b-B2FF-0A0200080CA9")
    IDisplayServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ HWND hWnd,
            /* [in] */ IDisplayRenderEngine *pRenderEngine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginDeviceLoss( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndDeviceLoss( 
            /* [in] */ IUnknown *pDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRenderEngine( 
            /* [out] */ IDisplayRenderEngine **ppRenderEngine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageWindow( 
            /* [out] */ HWND *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMessageWindow( 
            /* [in] */ HWND hwnd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayServer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDisplayServer * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ HWND hWnd,
            /* [in] */ IDisplayRenderEngine *pRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IDisplayServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginDeviceLoss )( 
            IDisplayServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndDeviceLoss )( 
            IDisplayServer * This,
            /* [in] */ IUnknown *pDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetRenderEngine )( 
            IDisplayServer * This,
            /* [out] */ IDisplayRenderEngine **ppRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageWindow )( 
            IDisplayServer * This,
            /* [out] */ HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMessageWindow )( 
            IDisplayServer * This,
            /* [in] */ HWND hwnd);
        
        END_INTERFACE
    } IDisplayServerVtbl;

    interface IDisplayServer
    {
        CONST_VTBL struct IDisplayServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayServer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayServer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayServer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayServer_Initialize(This,dwFlags,hWnd,pRenderEngine)	\
    ( (This)->lpVtbl -> Initialize(This,dwFlags,hWnd,pRenderEngine) ) 

#define IDisplayServer_Terminate(This)	\
    ( (This)->lpVtbl -> Terminate(This) ) 

#define IDisplayServer_BeginDeviceLoss(This)	\
    ( (This)->lpVtbl -> BeginDeviceLoss(This) ) 

#define IDisplayServer_EndDeviceLoss(This,pDevice)	\
    ( (This)->lpVtbl -> EndDeviceLoss(This,pDevice) ) 

#define IDisplayServer_GetRenderEngine(This,ppRenderEngine)	\
    ( (This)->lpVtbl -> GetRenderEngine(This,ppRenderEngine) ) 

#define IDisplayServer_GetMessageWindow(This,phwnd)	\
    ( (This)->lpVtbl -> GetMessageWindow(This,phwnd) ) 

#define IDisplayServer_SetMessageWindow(This,hwnd)	\
    ( (This)->lpVtbl -> SetMessageWindow(This,hwnd) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayServer_INTERFACE_DEFINED__ */


#ifndef __IDisplayVideoSource_INTERFACE_DEFINED__
#define __IDisplayVideoSource_INTERFACE_DEFINED__

/* interface IDisplayVideoSource */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayVideoSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3604A4CC-2610-4ef2-97DB-9FBE7D638E55")
    IDisplayVideoSource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGraph( 
            /* [out] */ IFilterGraph **ppGraph) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTexture( 
            /* [out] */ IUnknown **ppTexture,
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVideoSize( 
            /* [out] */ LONG *plWidth,
            /* [out] */ LONG *plHeight,
            /* [out] */ float *pfAspectRatio) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            IBaseFilter *pVMRFilter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginDraw( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndDraw( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsValid( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearImage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginDeviceLoss( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndDeviceLoss( 
            /* [in] */ IUnknown *pDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableInitiativeDisplay( 
            /* [in] */ BOOL bEnable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayVideoSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayVideoSource * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayVideoSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGraph )( 
            IDisplayVideoSource * This,
            /* [out] */ IFilterGraph **ppGraph);
        
        HRESULT ( STDMETHODCALLTYPE *GetTexture )( 
            IDisplayVideoSource * This,
            /* [out] */ IUnknown **ppTexture,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSize )( 
            IDisplayVideoSource * This,
            /* [out] */ LONG *plWidth,
            /* [out] */ LONG *plHeight,
            /* [out] */ float *pfAspectRatio);
        
        HRESULT ( STDMETHODCALLTYPE *Attach )( 
            IDisplayVideoSource * This,
            IBaseFilter *pVMRFilter);
        
        HRESULT ( STDMETHODCALLTYPE *Detach )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginDraw )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndDraw )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsValid )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *ClearImage )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginDeviceLoss )( 
            IDisplayVideoSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndDeviceLoss )( 
            IDisplayVideoSource * This,
            /* [in] */ IUnknown *pDevice);
        
        HRESULT ( STDMETHODCALLTYPE *EnableInitiativeDisplay )( 
            IDisplayVideoSource * This,
            /* [in] */ BOOL bEnable);
        
        END_INTERFACE
    } IDisplayVideoSourceVtbl;

    interface IDisplayVideoSource
    {
        CONST_VTBL struct IDisplayVideoSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayVideoSource_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayVideoSource_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayVideoSource_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayVideoSource_GetGraph(This,ppGraph)	\
    ( (This)->lpVtbl -> GetGraph(This,ppGraph) ) 

#define IDisplayVideoSource_GetTexture(This,ppTexture,lpNormRect)	\
    ( (This)->lpVtbl -> GetTexture(This,ppTexture,lpNormRect) ) 

#define IDisplayVideoSource_GetVideoSize(This,plWidth,plHeight,pfAspectRatio)	\
    ( (This)->lpVtbl -> GetVideoSize(This,plWidth,plHeight,pfAspectRatio) ) 

#define IDisplayVideoSource_Attach(This,pVMRFilter)	\
    ( (This)->lpVtbl -> Attach(This,pVMRFilter) ) 

#define IDisplayVideoSource_Detach(This)	\
    ( (This)->lpVtbl -> Detach(This) ) 

#define IDisplayVideoSource_BeginDraw(This)	\
    ( (This)->lpVtbl -> BeginDraw(This) ) 

#define IDisplayVideoSource_EndDraw(This)	\
    ( (This)->lpVtbl -> EndDraw(This) ) 

#define IDisplayVideoSource_IsValid(This)	\
    ( (This)->lpVtbl -> IsValid(This) ) 

#define IDisplayVideoSource_ClearImage(This)	\
    ( (This)->lpVtbl -> ClearImage(This) ) 

#define IDisplayVideoSource_BeginDeviceLoss(This)	\
    ( (This)->lpVtbl -> BeginDeviceLoss(This) ) 

#define IDisplayVideoSource_EndDeviceLoss(This,pDevice)	\
    ( (This)->lpVtbl -> EndDeviceLoss(This,pDevice) ) 

#define IDisplayVideoSource_EnableInitiativeDisplay(This,bEnable)	\
    ( (This)->lpVtbl -> EnableInitiativeDisplay(This,bEnable) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayVideoSource_INTERFACE_DEFINED__ */


#ifndef __IDisplayLock_INTERFACE_DEFINED__
#define __IDisplayLock_INTERFACE_DEFINED__

/* interface IDisplayLock */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("87A59958-7DF9-4e67-9860-8CFD5054CF1E")
    IDisplayLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Lock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unlock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TryLock( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayLock * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *Lock )( 
            IDisplayLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unlock )( 
            IDisplayLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *TryLock )( 
            IDisplayLock * This);
        
        END_INTERFACE
    } IDisplayLockVtbl;

    interface IDisplayLock
    {
        CONST_VTBL struct IDisplayLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayLock_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayLock_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayLock_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayLock_Lock(This)	\
    ( (This)->lpVtbl -> Lock(This) ) 

#define IDisplayLock_Unlock(This)	\
    ( (This)->lpVtbl -> Unlock(This) ) 

#define IDisplayLock_TryLock(This)	\
    ( (This)->lpVtbl -> TryLock(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayLock_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_DispSvr_0000_0004 */
/* [local] */ 

class CAutoDisplayLock 
{
    CAutoDisplayLock(const CAutoDisplayLock &refAutoLock);
    CAutoDisplayLock &operator=(const CAutoDisplayLock &refAutoLock);
protected:
    IDisplayLock* m_pLock;
public:
    CAutoDisplayLock(IDisplayLock* pLock)
    {
        m_pLock = pLock;
        if (m_pLock)
        {
            m_pLock->AddRef();
            m_pLock->Lock();
        }
    }
    ~CAutoDisplayLock() 
    {
        if (m_pLock)
        {
            m_pLock->Unlock();
            m_pLock->Release();
            m_pLock = NULL;
        }
    }
};


extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0004_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0004_v0_0_s_ifspec;

#ifndef __IDisplayRenderEngine_INTERFACE_DEFINED__
#define __IDisplayRenderEngine_INTERFACE_DEFINED__

/* interface IDisplayRenderEngine */
/* [unique][helpstring][uuid][object][local] */ 


enum DisplayRequest
    {	DISPLAY_REQUEST_Render	= 0,
	DISPLAY_REQUEST_Present	= ( DISPLAY_REQUEST_Render + 1 ) ,
	DISPLAY_REQUEST_FreezeState	= ( DISPLAY_REQUEST_Present + 1 ) ,
	DISPLAY_REQUEST_FrameProperty	= ( DISPLAY_REQUEST_FreezeState + 1 ) ,
	DISPLAY_REQUEST_AutoRender	= ( DISPLAY_REQUEST_FrameProperty + 1 ) ,
	DISPLAY_REQUEST_ICT	= ( DISPLAY_REQUEST_AutoRender + 1 ) ,
	DISPLAY_REQUEST_DISPMIXERCAPS	= ( DISPLAY_REQUEST_ICT + 1 ) ,
	DISPLAY_REQUEST_CustomizedOutput	= ( DISPLAY_REQUEST_DISPMIXERCAPS + 1 ) ,
	DISPLAY_REQUEST_ResetQueue	= ( DISPLAY_REQUEST_CustomizedOutput + 1 ) ,
	DISPLAY_REQUEST_ScreenCaptureDefense	= ( DISPLAY_REQUEST_ResetQueue + 1 ) ,
	DISPLAY_REQUEST_ClearRenderTarget	= ( DISPLAY_REQUEST_ScreenCaptureDefense + 1 ) ,
	DISPLAY_REQUEST_ProcessLostDevice	= ( DISPLAY_REQUEST_ClearRenderTarget + 1 ) ,
	DISPLAY_REQUEST_CheckDisplayModeChange	= ( DISPLAY_REQUEST_ProcessLostDevice + 1 ) ,
	DISPLAY_REQUEST_ExclusiveMode	= ( DISPLAY_REQUEST_CheckDisplayModeChange + 1 ) ,
	DISPLAY_REQUEST_WaitEngineReady	= ( DISPLAY_REQUEST_ExclusiveMode + 1 ) ,
	DISPLAY_REQUEST_TestCooperativeLevel	= ( DISPLAY_REQUEST_WaitEngineReady + 1 ) 
    } ;

enum OR_FRAME_PROPERTY
    {	OR_FRAME_TOP_FIELD_FIRST	= 0x1,
	OR_FRAME_BOTTOM_FIELD_FIRST	= 0x2,
	OR_FRAME_PROGRESSIVE	= 0x4,
	OR_FRAME_REPEAT_FIRST_FIELD	= 0x8
    } ;

EXTERN_C const IID IID_IDisplayRenderEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D89A29FF-E2C3-4e61-A1D1-C51D177D1200")
    IDisplayRenderEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ HWND hWnd,
            /* [in] */ UINT BackBufferWidth,
            /* [in] */ UINT BackBufferHeight,
            /* [in] */ IDisplayLock *pLock,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Render( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateMainVideoTime( 
            /* [in] */ void *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessLostDevice( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootObject( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRootObject( 
            /* [in] */ IDisplayObject *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameRate( 
            /* [in] */ int nFramesPerSecBy100) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameRate( 
            /* [out] */ int *pnFramesPerSecBy100) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameRateAvg( 
            /* [out] */ int *pnFramesPerSecBy100) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMixingPrefs( 
            /* [out] */ DWORD *pdwPrefs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDisplayServer( 
            /* [in] */ IDisplayServer *pDisplayServer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayServer( 
            /* [out] */ IDisplayServer **ppDisplayServer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get3DDevice( 
            /* [out] */ IUnknown **ppDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set3DDevice( 
            /* [in] */ IUnknown *pDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayWindow( 
            /* [out] */ HWND *phwnd,
            /* [out] */ RECT *pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDisplayWindow( 
            /* [in] */ HWND hwnd,
            /* [in] */ RECT *pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBackBufferSize( 
            /* [in] */ UINT BackBufferWidth,
            /* [in] */ UINT BackBufferHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBackBufferSize( 
            /* [out] */ UINT *BackBufferWidth,
            /* [out] */ UINT *BackBufferHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor( 
            /* [out] */ COLORREF *pColor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor( 
            /* [in] */ COLORREF Color) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLock( 
            /* [out] */ IDisplayLock **ppLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableRendering( 
            /* [in] */ BOOL bEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartRenderingThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopRenderingThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NodeRequest( 
            /* [in] */ DWORD request,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ IDisplayObject *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetColorKey( 
            /* [in] */ DWORD dwColorKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetColorKey( 
            /* [out] */ DWORD *pdwColorKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageWindow( 
            /* [out] */ HWND *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMessageWindow( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecomFPS( 
            /* [out] */ DWORD *pdwRecomFPS) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecomVideoPixelArea( 
            /* [out] */ DWORD *pdwRecomVideoPixelArea) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdviseEventNotify( 
            /* [in] */ IDispSvrRenderEngineNotify *pEventNotify) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnAdviseEventNotify( 
            /* [in] */ IDispSvrRenderEngineNotify *pEventNotify) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessWindowMessage( 
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessRequest( 
            /* [in] */ DWORD request,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayRenderEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayRenderEngine * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayRenderEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDisplayRenderEngine * This,
            /* [in] */ HWND hWnd,
            /* [in] */ UINT BackBufferWidth,
            /* [in] */ UINT BackBufferHeight,
            /* [in] */ IDisplayLock *pLock,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateMainVideoTime )( 
            IDisplayRenderEngine * This,
            /* [in] */ void *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessLostDevice )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootObject )( 
            IDisplayRenderEngine * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetRootObject )( 
            IDisplayRenderEngine * This,
            /* [in] */ IDisplayObject *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameRate )( 
            IDisplayRenderEngine * This,
            /* [in] */ int nFramesPerSecBy100);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameRate )( 
            IDisplayRenderEngine * This,
            /* [out] */ int *pnFramesPerSecBy100);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameRateAvg )( 
            IDisplayRenderEngine * This,
            /* [out] */ int *pnFramesPerSecBy100);
        
        HRESULT ( STDMETHODCALLTYPE *GetMixingPrefs )( 
            IDisplayRenderEngine * This,
            /* [out] */ DWORD *pdwPrefs);
        
        HRESULT ( STDMETHODCALLTYPE *SetDisplayServer )( 
            IDisplayRenderEngine * This,
            /* [in] */ IDisplayServer *pDisplayServer);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayServer )( 
            IDisplayRenderEngine * This,
            /* [out] */ IDisplayServer **ppDisplayServer);
        
        HRESULT ( STDMETHODCALLTYPE *Get3DDevice )( 
            IDisplayRenderEngine * This,
            /* [out] */ IUnknown **ppDevice);
        
        HRESULT ( STDMETHODCALLTYPE *Set3DDevice )( 
            IDisplayRenderEngine * This,
            /* [in] */ IUnknown *pDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayWindow )( 
            IDisplayRenderEngine * This,
            /* [out] */ HWND *phwnd,
            /* [out] */ RECT *pRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetDisplayWindow )( 
            IDisplayRenderEngine * This,
            /* [in] */ HWND hwnd,
            /* [in] */ RECT *pRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetBackBufferSize )( 
            IDisplayRenderEngine * This,
            /* [in] */ UINT BackBufferWidth,
            /* [in] */ UINT BackBufferHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetBackBufferSize )( 
            IDisplayRenderEngine * This,
            /* [out] */ UINT *BackBufferWidth,
            /* [out] */ UINT *BackBufferHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetBackgroundColor )( 
            IDisplayRenderEngine * This,
            /* [out] */ COLORREF *pColor);
        
        HRESULT ( STDMETHODCALLTYPE *SetBackgroundColor )( 
            IDisplayRenderEngine * This,
            /* [in] */ COLORREF Color);
        
        HRESULT ( STDMETHODCALLTYPE *GetLock )( 
            IDisplayRenderEngine * This,
            /* [out] */ IDisplayLock **ppLock);
        
        HRESULT ( STDMETHODCALLTYPE *EnableRendering )( 
            IDisplayRenderEngine * This,
            /* [in] */ BOOL bEnable);
        
        HRESULT ( STDMETHODCALLTYPE *StartRenderingThread )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *StopRenderingThread )( 
            IDisplayRenderEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *NodeRequest )( 
            IDisplayRenderEngine * This,
            /* [in] */ DWORD request,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ IDisplayObject *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetColorKey )( 
            IDisplayRenderEngine * This,
            /* [in] */ DWORD dwColorKey);
        
        HRESULT ( STDMETHODCALLTYPE *GetColorKey )( 
            IDisplayRenderEngine * This,
            /* [out] */ DWORD *pdwColorKey);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageWindow )( 
            IDisplayRenderEngine * This,
            /* [out] */ HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMessageWindow )( 
            IDisplayRenderEngine * This,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecomFPS )( 
            IDisplayRenderEngine * This,
            /* [out] */ DWORD *pdwRecomFPS);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecomVideoPixelArea )( 
            IDisplayRenderEngine * This,
            /* [out] */ DWORD *pdwRecomVideoPixelArea);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseEventNotify )( 
            IDisplayRenderEngine * This,
            /* [in] */ IDispSvrRenderEngineNotify *pEventNotify);
        
        HRESULT ( STDMETHODCALLTYPE *UnAdviseEventNotify )( 
            IDisplayRenderEngine * This,
            /* [in] */ IDispSvrRenderEngineNotify *pEventNotify);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessWindowMessage )( 
            IDisplayRenderEngine * This,
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessRequest )( 
            IDisplayRenderEngine * This,
            /* [in] */ DWORD request,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2);
        
        END_INTERFACE
    } IDisplayRenderEngineVtbl;

    interface IDisplayRenderEngine
    {
        CONST_VTBL struct IDisplayRenderEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayRenderEngine_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayRenderEngine_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayRenderEngine_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayRenderEngine_Initialize(This,hWnd,BackBufferWidth,BackBufferHeight,pLock,dwFlags)	\
    ( (This)->lpVtbl -> Initialize(This,hWnd,BackBufferWidth,BackBufferHeight,pLock,dwFlags) ) 

#define IDisplayRenderEngine_Terminate(This)	\
    ( (This)->lpVtbl -> Terminate(This) ) 

#define IDisplayRenderEngine_Render(This)	\
    ( (This)->lpVtbl -> Render(This) ) 

#define IDisplayRenderEngine_UpdateMainVideoTime(This,pObject)	\
    ( (This)->lpVtbl -> UpdateMainVideoTime(This,pObject) ) 

#define IDisplayRenderEngine_ProcessLostDevice(This)	\
    ( (This)->lpVtbl -> ProcessLostDevice(This) ) 

#define IDisplayRenderEngine_GetRootObject(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> GetRootObject(This,riid,ppvObject) ) 

#define IDisplayRenderEngine_SetRootObject(This,pObject)	\
    ( (This)->lpVtbl -> SetRootObject(This,pObject) ) 

#define IDisplayRenderEngine_SetFrameRate(This,nFramesPerSecBy100)	\
    ( (This)->lpVtbl -> SetFrameRate(This,nFramesPerSecBy100) ) 

#define IDisplayRenderEngine_GetFrameRate(This,pnFramesPerSecBy100)	\
    ( (This)->lpVtbl -> GetFrameRate(This,pnFramesPerSecBy100) ) 

#define IDisplayRenderEngine_GetFrameRateAvg(This,pnFramesPerSecBy100)	\
    ( (This)->lpVtbl -> GetFrameRateAvg(This,pnFramesPerSecBy100) ) 

#define IDisplayRenderEngine_GetMixingPrefs(This,pdwPrefs)	\
    ( (This)->lpVtbl -> GetMixingPrefs(This,pdwPrefs) ) 

#define IDisplayRenderEngine_SetDisplayServer(This,pDisplayServer)	\
    ( (This)->lpVtbl -> SetDisplayServer(This,pDisplayServer) ) 

#define IDisplayRenderEngine_GetDisplayServer(This,ppDisplayServer)	\
    ( (This)->lpVtbl -> GetDisplayServer(This,ppDisplayServer) ) 

#define IDisplayRenderEngine_Get3DDevice(This,ppDevice)	\
    ( (This)->lpVtbl -> Get3DDevice(This,ppDevice) ) 

#define IDisplayRenderEngine_Set3DDevice(This,pDevice)	\
    ( (This)->lpVtbl -> Set3DDevice(This,pDevice) ) 

#define IDisplayRenderEngine_GetDisplayWindow(This,phwnd,pRect)	\
    ( (This)->lpVtbl -> GetDisplayWindow(This,phwnd,pRect) ) 

#define IDisplayRenderEngine_SetDisplayWindow(This,hwnd,pRect)	\
    ( (This)->lpVtbl -> SetDisplayWindow(This,hwnd,pRect) ) 

#define IDisplayRenderEngine_SetBackBufferSize(This,BackBufferWidth,BackBufferHeight)	\
    ( (This)->lpVtbl -> SetBackBufferSize(This,BackBufferWidth,BackBufferHeight) ) 

#define IDisplayRenderEngine_GetBackBufferSize(This,BackBufferWidth,BackBufferHeight)	\
    ( (This)->lpVtbl -> GetBackBufferSize(This,BackBufferWidth,BackBufferHeight) ) 

#define IDisplayRenderEngine_GetBackgroundColor(This,pColor)	\
    ( (This)->lpVtbl -> GetBackgroundColor(This,pColor) ) 

#define IDisplayRenderEngine_SetBackgroundColor(This,Color)	\
    ( (This)->lpVtbl -> SetBackgroundColor(This,Color) ) 

#define IDisplayRenderEngine_GetLock(This,ppLock)	\
    ( (This)->lpVtbl -> GetLock(This,ppLock) ) 

#define IDisplayRenderEngine_EnableRendering(This,bEnable)	\
    ( (This)->lpVtbl -> EnableRendering(This,bEnable) ) 

#define IDisplayRenderEngine_StartRenderingThread(This)	\
    ( (This)->lpVtbl -> StartRenderingThread(This) ) 

#define IDisplayRenderEngine_StopRenderingThread(This)	\
    ( (This)->lpVtbl -> StopRenderingThread(This) ) 

#define IDisplayRenderEngine_NodeRequest(This,request,param1,param2,pObject)	\
    ( (This)->lpVtbl -> NodeRequest(This,request,param1,param2,pObject) ) 

#define IDisplayRenderEngine_SetColorKey(This,dwColorKey)	\
    ( (This)->lpVtbl -> SetColorKey(This,dwColorKey) ) 

#define IDisplayRenderEngine_GetColorKey(This,pdwColorKey)	\
    ( (This)->lpVtbl -> GetColorKey(This,pdwColorKey) ) 

#define IDisplayRenderEngine_GetMessageWindow(This,phwnd)	\
    ( (This)->lpVtbl -> GetMessageWindow(This,phwnd) ) 

#define IDisplayRenderEngine_SetMessageWindow(This,hwnd)	\
    ( (This)->lpVtbl -> SetMessageWindow(This,hwnd) ) 

#define IDisplayRenderEngine_GetRecomFPS(This,pdwRecomFPS)	\
    ( (This)->lpVtbl -> GetRecomFPS(This,pdwRecomFPS) ) 

#define IDisplayRenderEngine_GetRecomVideoPixelArea(This,pdwRecomVideoPixelArea)	\
    ( (This)->lpVtbl -> GetRecomVideoPixelArea(This,pdwRecomVideoPixelArea) ) 

#define IDisplayRenderEngine_AdviseEventNotify(This,pEventNotify)	\
    ( (This)->lpVtbl -> AdviseEventNotify(This,pEventNotify) ) 

#define IDisplayRenderEngine_UnAdviseEventNotify(This,pEventNotify)	\
    ( (This)->lpVtbl -> UnAdviseEventNotify(This,pEventNotify) ) 

#define IDisplayRenderEngine_ProcessWindowMessage(This,uMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessWindowMessage(This,uMsg,wParam,lParam) ) 

#define IDisplayRenderEngine_ProcessRequest(This,request,param1,param2)	\
    ( (This)->lpVtbl -> ProcessRequest(This,request,param1,param2) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayRenderEngine_INTERFACE_DEFINED__ */


#ifndef __IDisplayEventHandler_INTERFACE_DEFINED__
#define __IDisplayEventHandler_INTERFACE_DEFINED__

/* interface IDisplayEventHandler */
/* [unique][helpstring][uuid][object][local] */ 


enum DisplayEvent
    {	DISPLAY_EVENT_MainVideoDecoded	= 0,
	DISPLAY_EVENT_EnableRendering	= ( DISPLAY_EVENT_MainVideoDecoded + 1 ) ,
	DISPLAY_EVENT_DispSvrMixing	= ( DISPLAY_EVENT_EnableRendering + 1 ) ,
	DISPLAY_EVENT_VideoSourceRender	= ( DISPLAY_EVENT_DispSvrMixing + 1 ) ,
	DISPLAY_EVENT_VideoSourcePresent	= ( DISPLAY_EVENT_VideoSourceRender + 1 ) 
    } ;

EXTERN_C const IID IID_IDisplayEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6CA13811-C666-4ba2-B9BE-0109B4C121AF")
    IDisplayEventHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyEvent( 
            /* [in] */ DWORD event,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ LPVOID pInstance) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayEventHandler * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayEventHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyEvent )( 
            IDisplayEventHandler * This,
            /* [in] */ DWORD event,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ LPVOID pInstance);
        
        END_INTERFACE
    } IDisplayEventHandlerVtbl;

    interface IDisplayEventHandler
    {
        CONST_VTBL struct IDisplayEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayEventHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayEventHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayEventHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayEventHandler_NotifyEvent(This,event,param1,param2,pInstance)	\
    ( (This)->lpVtbl -> NotifyEvent(This,event,param1,param2,pInstance) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayEventHandler_INTERFACE_DEFINED__ */


#ifndef __IDisplayEventHost_INTERFACE_DEFINED__
#define __IDisplayEventHost_INTERFACE_DEFINED__

/* interface IDisplayEventHost */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayEventHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ECEBF734-BDF3-487d-B84B-DB9EC4E1D514")
    IDisplayEventHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
            /* [in] */ IDisplayEventHandler *pHandler,
            /* [in] */ LPVOID pInstance) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unregister( 
            /* [in] */ IDisplayEventHandler *pHandler) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayEventHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayEventHost * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayEventHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayEventHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IDisplayEventHost * This,
            /* [in] */ IDisplayEventHandler *pHandler,
            /* [in] */ LPVOID pInstance);
        
        HRESULT ( STDMETHODCALLTYPE *Unregister )( 
            IDisplayEventHost * This,
            /* [in] */ IDisplayEventHandler *pHandler);
        
        END_INTERFACE
    } IDisplayEventHostVtbl;

    interface IDisplayEventHost
    {
        CONST_VTBL struct IDisplayEventHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayEventHost_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayEventHost_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayEventHost_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayEventHost_Register(This,pHandler,pInstance)	\
    ( (This)->lpVtbl -> Register(This,pHandler,pInstance) ) 

#define IDisplayEventHost_Unregister(This,pHandler)	\
    ( (This)->lpVtbl -> Unregister(This,pHandler) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayEventHost_INTERFACE_DEFINED__ */


#ifndef __IDisplayObject_INTERFACE_DEFINED__
#define __IDisplayObject_INTERFACE_DEFINED__

/* interface IDisplayObject */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4B500E2-BD7F-4e2f-863A-1294639D90CE")
    IDisplayObject : public IDisplayEventHandler
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ IDisplayRenderEngine *pRenderEngine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRenderEngineOwner( 
            /* [out] */ IDisplayRenderEngine **ppRenderEngine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCLSID( 
            /* [out] */ CLSID *pClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessMessage( 
            /* [in] */ HWND hWnd,
            /* [in] */ UINT msg,
            /* [in] */ UINT wParam,
            /* [in] */ LONG lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Render( 
            /* [in] */ IUnknown *pDevice,
            /* [in] */ const NORMALIZEDRECT *lpParentRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginDeviceLoss( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndDeviceLoss( 
            /* [in] */ IUnknown *pDevice) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayObject * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyEvent )( 
            IDisplayObject * This,
            /* [in] */ DWORD event,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ LPVOID pInstance);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IDisplayObject * This,
            /* [in] */ IDisplayRenderEngine *pRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRenderEngineOwner )( 
            IDisplayObject * This,
            /* [out] */ IDisplayRenderEngine **ppRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *GetCLSID )( 
            IDisplayObject * This,
            /* [out] */ CLSID *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMessage )( 
            IDisplayObject * This,
            /* [in] */ HWND hWnd,
            /* [in] */ UINT msg,
            /* [in] */ UINT wParam,
            /* [in] */ LONG lParam);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IDisplayObject * This,
            /* [in] */ IUnknown *pDevice,
            /* [in] */ const NORMALIZEDRECT *lpParentRect);
        
        HRESULT ( STDMETHODCALLTYPE *BeginDeviceLoss )( 
            IDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndDeviceLoss )( 
            IDisplayObject * This,
            /* [in] */ IUnknown *pDevice);
        
        END_INTERFACE
    } IDisplayObjectVtbl;

    interface IDisplayObject
    {
        CONST_VTBL struct IDisplayObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayObject_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayObject_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayObject_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayObject_NotifyEvent(This,event,param1,param2,pInstance)	\
    ( (This)->lpVtbl -> NotifyEvent(This,event,param1,param2,pInstance) ) 


#define IDisplayObject_Initialize(This,pRenderEngine)	\
    ( (This)->lpVtbl -> Initialize(This,pRenderEngine) ) 

#define IDisplayObject_Terminate(This)	\
    ( (This)->lpVtbl -> Terminate(This) ) 

#define IDisplayObject_GetRenderEngineOwner(This,ppRenderEngine)	\
    ( (This)->lpVtbl -> GetRenderEngineOwner(This,ppRenderEngine) ) 

#define IDisplayObject_GetCLSID(This,pClsid)	\
    ( (This)->lpVtbl -> GetCLSID(This,pClsid) ) 

#define IDisplayObject_ProcessMessage(This,hWnd,msg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessMessage(This,hWnd,msg,wParam,lParam) ) 

#define IDisplayObject_Render(This,pDevice,lpParentRect)	\
    ( (This)->lpVtbl -> Render(This,pDevice,lpParentRect) ) 

#define IDisplayObject_BeginDeviceLoss(This)	\
    ( (This)->lpVtbl -> BeginDeviceLoss(This) ) 

#define IDisplayObject_EndDeviceLoss(This,pDevice)	\
    ( (This)->lpVtbl -> EndDeviceLoss(This,pDevice) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayObject_INTERFACE_DEFINED__ */


#ifndef __IParentDisplayObject_INTERFACE_DEFINED__
#define __IParentDisplayObject_INTERFACE_DEFINED__

/* interface IParentDisplayObject */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IParentDisplayObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B26B321-8B17-435c-8390-A3F8378472C1")
    IParentDisplayObject : public IDisplayObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetChildCount( 
            /* [out] */ LONG *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetChild( 
            /* [in] */ LONG lIndex,
            /* [out] */ IDisplayObject **ppObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IndexOf( 
            /* [in] */ IUnknown *pObject,
            /* [out] */ LONG *plIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddChild( 
            /* [in] */ LONG lZOrder,
            /* [in] */ IUnknown *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveChild( 
            /* [in] */ IUnknown *pObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZOrder( 
            /* [in] */ IUnknown *pObject,
            /* [in] */ LONG lZOrder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZOrder( 
            /* [in] */ IUnknown *pObject,
            /* [out] */ LONG *plZOrder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
            /* [in] */ BOOL bShow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IParentDisplayObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IParentDisplayObject * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IParentDisplayObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IParentDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyEvent )( 
            IParentDisplayObject * This,
            /* [in] */ DWORD event,
            /* [in] */ DWORD param1,
            /* [in] */ DWORD param2,
            /* [in] */ LPVOID pInstance);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IParentDisplayObject * This,
            /* [in] */ IDisplayRenderEngine *pRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IParentDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRenderEngineOwner )( 
            IParentDisplayObject * This,
            /* [out] */ IDisplayRenderEngine **ppRenderEngine);
        
        HRESULT ( STDMETHODCALLTYPE *GetCLSID )( 
            IParentDisplayObject * This,
            /* [out] */ CLSID *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMessage )( 
            IParentDisplayObject * This,
            /* [in] */ HWND hWnd,
            /* [in] */ UINT msg,
            /* [in] */ UINT wParam,
            /* [in] */ LONG lParam);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pDevice,
            /* [in] */ const NORMALIZEDRECT *lpParentRect);
        
        HRESULT ( STDMETHODCALLTYPE *BeginDeviceLoss )( 
            IParentDisplayObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndDeviceLoss )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetChildCount )( 
            IParentDisplayObject * This,
            /* [out] */ LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetChild )( 
            IParentDisplayObject * This,
            /* [in] */ LONG lIndex,
            /* [out] */ IDisplayObject **ppObject);
        
        HRESULT ( STDMETHODCALLTYPE *IndexOf )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pObject,
            /* [out] */ LONG *plIndex);
        
        HRESULT ( STDMETHODCALLTYPE *AddChild )( 
            IParentDisplayObject * This,
            /* [in] */ LONG lZOrder,
            /* [in] */ IUnknown *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveChild )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetZOrder )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pObject,
            /* [in] */ LONG lZOrder);
        
        HRESULT ( STDMETHODCALLTYPE *GetZOrder )( 
            IParentDisplayObject * This,
            /* [in] */ IUnknown *pObject,
            /* [out] */ LONG *plZOrder);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IParentDisplayObject * This,
            /* [in] */ BOOL bShow);
        
        END_INTERFACE
    } IParentDisplayObjectVtbl;

    interface IParentDisplayObject
    {
        CONST_VTBL struct IParentDisplayObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParentDisplayObject_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IParentDisplayObject_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IParentDisplayObject_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IParentDisplayObject_NotifyEvent(This,event,param1,param2,pInstance)	\
    ( (This)->lpVtbl -> NotifyEvent(This,event,param1,param2,pInstance) ) 


#define IParentDisplayObject_Initialize(This,pRenderEngine)	\
    ( (This)->lpVtbl -> Initialize(This,pRenderEngine) ) 

#define IParentDisplayObject_Terminate(This)	\
    ( (This)->lpVtbl -> Terminate(This) ) 

#define IParentDisplayObject_GetRenderEngineOwner(This,ppRenderEngine)	\
    ( (This)->lpVtbl -> GetRenderEngineOwner(This,ppRenderEngine) ) 

#define IParentDisplayObject_GetCLSID(This,pClsid)	\
    ( (This)->lpVtbl -> GetCLSID(This,pClsid) ) 

#define IParentDisplayObject_ProcessMessage(This,hWnd,msg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessMessage(This,hWnd,msg,wParam,lParam) ) 

#define IParentDisplayObject_Render(This,pDevice,lpParentRect)	\
    ( (This)->lpVtbl -> Render(This,pDevice,lpParentRect) ) 

#define IParentDisplayObject_BeginDeviceLoss(This)	\
    ( (This)->lpVtbl -> BeginDeviceLoss(This) ) 

#define IParentDisplayObject_EndDeviceLoss(This,pDevice)	\
    ( (This)->lpVtbl -> EndDeviceLoss(This,pDevice) ) 


#define IParentDisplayObject_GetChildCount(This,plCount)	\
    ( (This)->lpVtbl -> GetChildCount(This,plCount) ) 

#define IParentDisplayObject_GetChild(This,lIndex,ppObject)	\
    ( (This)->lpVtbl -> GetChild(This,lIndex,ppObject) ) 

#define IParentDisplayObject_IndexOf(This,pObject,plIndex)	\
    ( (This)->lpVtbl -> IndexOf(This,pObject,plIndex) ) 

#define IParentDisplayObject_AddChild(This,lZOrder,pObject)	\
    ( (This)->lpVtbl -> AddChild(This,lZOrder,pObject) ) 

#define IParentDisplayObject_RemoveChild(This,pObject)	\
    ( (This)->lpVtbl -> RemoveChild(This,pObject) ) 

#define IParentDisplayObject_SetZOrder(This,pObject,lZOrder)	\
    ( (This)->lpVtbl -> SetZOrder(This,pObject,lZOrder) ) 

#define IParentDisplayObject_GetZOrder(This,pObject,plZOrder)	\
    ( (This)->lpVtbl -> GetZOrder(This,pObject,plZOrder) ) 

#define IParentDisplayObject_Show(This,bShow)	\
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IParentDisplayObject_INTERFACE_DEFINED__ */


#ifndef __IDisplayVideoSink_INTERFACE_DEFINED__
#define __IDisplayVideoSink_INTERFACE_DEFINED__

/* interface IDisplayVideoSink */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayVideoSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1EE35D5C-4DAB-4356-808C-ECC5BA2A19A5")
    IDisplayVideoSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnVideoSourceAdded( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ FLOAT fAspectRatio) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnVideoSourceRemoved( 
            /* [in] */ IDisplayVideoSource *pVidSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get3DDevice( 
            /* [out] */ IUnknown **ppDevice) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayVideoSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayVideoSink * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayVideoSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayVideoSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnVideoSourceAdded )( 
            IDisplayVideoSink * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ FLOAT fAspectRatio);
        
        HRESULT ( STDMETHODCALLTYPE *OnVideoSourceRemoved )( 
            IDisplayVideoSink * This,
            /* [in] */ IDisplayVideoSource *pVidSrc);
        
        HRESULT ( STDMETHODCALLTYPE *Get3DDevice )( 
            IDisplayVideoSink * This,
            /* [out] */ IUnknown **ppDevice);
        
        END_INTERFACE
    } IDisplayVideoSinkVtbl;

    interface IDisplayVideoSink
    {
        CONST_VTBL struct IDisplayVideoSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayVideoSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayVideoSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayVideoSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayVideoSink_OnVideoSourceAdded(This,pVidSrc,fAspectRatio)	\
    ( (This)->lpVtbl -> OnVideoSourceAdded(This,pVidSrc,fAspectRatio) ) 

#define IDisplayVideoSink_OnVideoSourceRemoved(This,pVidSrc)	\
    ( (This)->lpVtbl -> OnVideoSourceRemoved(This,pVidSrc) ) 

#define IDisplayVideoSink_Get3DDevice(This,ppDevice)	\
    ( (This)->lpVtbl -> Get3DDevice(This,ppDevice) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayVideoSink_INTERFACE_DEFINED__ */


#ifndef __IDisplayVideoMixer_INTERFACE_DEFINED__
#define __IDisplayVideoMixer_INTERFACE_DEFINED__

/* interface IDisplayVideoMixer */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayVideoMixer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8ADD1492-CA9D-42c5-9F0E-4A5D6D9DEFF7")
    IDisplayVideoMixer : public IDisplayVideoSourceManager
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOutputRect( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutputRect( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIdealOutputRect( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ DWORD dwWidth,
            /* [in] */ DWORD dwHeight,
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZOrder( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ DWORD *pdwZOrder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZOrder( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ DWORD dwZOrder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAlpha( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ float *pAlpha) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAlpha( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ float Alpha) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE KeepAspectRatio( 
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ BOOL bKeepAR) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayVideoMixerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayVideoMixer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayVideoMixer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayVideoMixer * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddVideoSource )( 
            IDisplayVideoMixer * This,
            /* [in] */ IBaseFilter *pVMR,
            /* [out] */ IDisplayVideoSource **ppVidSrc);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveVideoSource )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSourceCount )( 
            IDisplayVideoMixer * This,
            /* [out] */ LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetVideoSourceByIndex )( 
            IDisplayVideoMixer * This,
            /* [in] */ LONG lIndex,
            /* [out] */ IDisplayVideoSource **ppVideoSource);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputRect )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutputRect )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetIdealOutputRect )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ DWORD dwWidth,
            /* [in] */ DWORD dwHeight,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetZOrder )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ DWORD *pdwZOrder);
        
        HRESULT ( STDMETHODCALLTYPE *SetZOrder )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ DWORD dwZOrder);
        
        HRESULT ( STDMETHODCALLTYPE *GetAlpha )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [out] */ float *pAlpha);
        
        HRESULT ( STDMETHODCALLTYPE *SetAlpha )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ float Alpha);
        
        HRESULT ( STDMETHODCALLTYPE *KeepAspectRatio )( 
            IDisplayVideoMixer * This,
            /* [in] */ IDisplayVideoSource *pVidSrc,
            /* [in] */ BOOL bKeepAR);
        
        END_INTERFACE
    } IDisplayVideoMixerVtbl;

    interface IDisplayVideoMixer
    {
        CONST_VTBL struct IDisplayVideoMixerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayVideoMixer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayVideoMixer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayVideoMixer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayVideoMixer_AddVideoSource(This,pVMR,ppVidSrc)	\
    ( (This)->lpVtbl -> AddVideoSource(This,pVMR,ppVidSrc) ) 

#define IDisplayVideoMixer_RemoveVideoSource(This,pVidSrc)	\
    ( (This)->lpVtbl -> RemoveVideoSource(This,pVidSrc) ) 

#define IDisplayVideoMixer_GetVideoSourceCount(This,plCount)	\
    ( (This)->lpVtbl -> GetVideoSourceCount(This,plCount) ) 

#define IDisplayVideoMixer_GetVideoSourceByIndex(This,lIndex,ppVideoSource)	\
    ( (This)->lpVtbl -> GetVideoSourceByIndex(This,lIndex,ppVideoSource) ) 


#define IDisplayVideoMixer_GetOutputRect(This,pVidSrc,lpNormRect)	\
    ( (This)->lpVtbl -> GetOutputRect(This,pVidSrc,lpNormRect) ) 

#define IDisplayVideoMixer_SetOutputRect(This,pVidSrc,lpNormRect)	\
    ( (This)->lpVtbl -> SetOutputRect(This,pVidSrc,lpNormRect) ) 

#define IDisplayVideoMixer_GetIdealOutputRect(This,pVidSrc,dwWidth,dwHeight,lpNormRect)	\
    ( (This)->lpVtbl -> GetIdealOutputRect(This,pVidSrc,dwWidth,dwHeight,lpNormRect) ) 

#define IDisplayVideoMixer_GetZOrder(This,pVidSrc,pdwZOrder)	\
    ( (This)->lpVtbl -> GetZOrder(This,pVidSrc,pdwZOrder) ) 

#define IDisplayVideoMixer_SetZOrder(This,pVidSrc,dwZOrder)	\
    ( (This)->lpVtbl -> SetZOrder(This,pVidSrc,dwZOrder) ) 

#define IDisplayVideoMixer_GetAlpha(This,pVidSrc,pAlpha)	\
    ( (This)->lpVtbl -> GetAlpha(This,pVidSrc,pAlpha) ) 

#define IDisplayVideoMixer_SetAlpha(This,pVidSrc,Alpha)	\
    ( (This)->lpVtbl -> SetAlpha(This,pVidSrc,Alpha) ) 

#define IDisplayVideoMixer_KeepAspectRatio(This,pVidSrc,bKeepAR)	\
    ( (This)->lpVtbl -> KeepAspectRatio(This,pVidSrc,bKeepAR) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayVideoMixer_INTERFACE_DEFINED__ */


#ifndef __IDisplayProperties_INTERFACE_DEFINED__
#define __IDisplayProperties_INTERFACE_DEFINED__

/* interface IDisplayProperties */
/* [unique][helpstring][uuid][object][local] */ 


enum DisplayPropertiesCaptureFormat
    {	DISPPROP_CAPTUREFMT_CVF	= 0,
	DISPPROP_CAPTUREFMT_JPG	= ( DISPPROP_CAPTUREFMT_CVF + 1 ) ,
	DISPPROP_CAPTUREFMT_PNG	= ( DISPPROP_CAPTUREFMT_JPG + 1 ) 
    } ;

EXTERN_C const IID IID_IDisplayProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("403A5DE1-A8DA-4b8a-84F9-38A2693E191F")
    IDisplayProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOutputRect( 
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOutputRect( 
            /* [in] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCropRect( 
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCropRect( 
            /* [in] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVertexBufferRect( 
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameColor( 
            /* [out] */ COLORREF *pColor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameColor( 
            /* [in] */ COLORREF Color) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CaptureFrame( 
            /* [in] */ DWORD dwFormat,
            /* [out] */ BYTE **ppFrame,
            /* [out] */ UINT *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoom( 
            /* [in] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoom( 
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShow( 
            /* [in] */ BOOL bShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetShow( 
            /* [out] */ BOOL *bShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearFrame( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClientToDO( 
            /* [out][in] */ POINT *pPt) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayProperties * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputRect )( 
            IDisplayProperties * This,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutputRect )( 
            IDisplayProperties * This,
            /* [in] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetCropRect )( 
            IDisplayProperties * This,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetCropRect )( 
            IDisplayProperties * This,
            /* [in] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetVertexBufferRect )( 
            IDisplayProperties * This,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameColor )( 
            IDisplayProperties * This,
            /* [out] */ COLORREF *pColor);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameColor )( 
            IDisplayProperties * This,
            /* [in] */ COLORREF Color);
        
        HRESULT ( STDMETHODCALLTYPE *CaptureFrame )( 
            IDisplayProperties * This,
            /* [in] */ DWORD dwFormat,
            /* [out] */ BYTE **ppFrame,
            /* [out] */ UINT *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetZoom )( 
            IDisplayProperties * This,
            /* [in] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetZoom )( 
            IDisplayProperties * This,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        HRESULT ( STDMETHODCALLTYPE *SetShow )( 
            IDisplayProperties * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetShow )( 
            IDisplayProperties * This,
            /* [out] */ BOOL *bShow);
        
        HRESULT ( STDMETHODCALLTYPE *ClearFrame )( 
            IDisplayProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *ClientToDO )( 
            IDisplayProperties * This,
            /* [out][in] */ POINT *pPt);
        
        END_INTERFACE
    } IDisplayPropertiesVtbl;

    interface IDisplayProperties
    {
        CONST_VTBL struct IDisplayPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayProperties_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayProperties_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayProperties_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayProperties_GetOutputRect(This,lpNormRect)	\
    ( (This)->lpVtbl -> GetOutputRect(This,lpNormRect) ) 

#define IDisplayProperties_SetOutputRect(This,lpNormRect)	\
    ( (This)->lpVtbl -> SetOutputRect(This,lpNormRect) ) 

#define IDisplayProperties_GetCropRect(This,lpNormRect)	\
    ( (This)->lpVtbl -> GetCropRect(This,lpNormRect) ) 

#define IDisplayProperties_SetCropRect(This,lpNormRect)	\
    ( (This)->lpVtbl -> SetCropRect(This,lpNormRect) ) 

#define IDisplayProperties_GetVertexBufferRect(This,lpNormRect)	\
    ( (This)->lpVtbl -> GetVertexBufferRect(This,lpNormRect) ) 

#define IDisplayProperties_GetFrameColor(This,pColor)	\
    ( (This)->lpVtbl -> GetFrameColor(This,pColor) ) 

#define IDisplayProperties_SetFrameColor(This,Color)	\
    ( (This)->lpVtbl -> SetFrameColor(This,Color) ) 

#define IDisplayProperties_CaptureFrame(This,dwFormat,ppFrame,pSize)	\
    ( (This)->lpVtbl -> CaptureFrame(This,dwFormat,ppFrame,pSize) ) 

#define IDisplayProperties_SetZoom(This,lpNormRect)	\
    ( (This)->lpVtbl -> SetZoom(This,lpNormRect) ) 

#define IDisplayProperties_GetZoom(This,lpNormRect)	\
    ( (This)->lpVtbl -> GetZoom(This,lpNormRect) ) 

#define IDisplayProperties_SetShow(This,bShow)	\
    ( (This)->lpVtbl -> SetShow(This,bShow) ) 

#define IDisplayProperties_GetShow(This,bShow)	\
    ( (This)->lpVtbl -> GetShow(This,bShow) ) 

#define IDisplayProperties_ClearFrame(This)	\
    ( (This)->lpVtbl -> ClearFrame(This) ) 

#define IDisplayProperties_ClientToDO(This,pPt)	\
    ( (This)->lpVtbl -> ClientToDO(This,pPt) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayProperties_INTERFACE_DEFINED__ */


#ifndef __IDisplayOptimizedRender_INTERFACE_DEFINED__
#define __IDisplayOptimizedRender_INTERFACE_DEFINED__

/* interface IDisplayOptimizedRender */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayOptimizedRender;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CEF8B340-07B7-4568-A270-4DB5E6F97099")
    IDisplayOptimizedRender : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MultiTextureRender( 
            /* [in] */ IUnknown *pDevice,
            /* [in] */ LONG lStageIdx,
            /* [in] */ LONG lTexIdx,
            /* [in] */ const NORMALIZEDRECT *lpParentRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTextureCount( 
            /* [out] */ LONG *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTextureCoord( 
            /* [in] */ LONG lIndex,
            /* [out] */ NORMALIZEDRECT *lpNormRect) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayOptimizedRenderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayOptimizedRender * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayOptimizedRender * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayOptimizedRender * This);
        
        HRESULT ( STDMETHODCALLTYPE *MultiTextureRender )( 
            IDisplayOptimizedRender * This,
            /* [in] */ IUnknown *pDevice,
            /* [in] */ LONG lStageIdx,
            /* [in] */ LONG lTexIdx,
            /* [in] */ const NORMALIZEDRECT *lpParentRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetTextureCount )( 
            IDisplayOptimizedRender * This,
            /* [out] */ LONG *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetTextureCoord )( 
            IDisplayOptimizedRender * This,
            /* [in] */ LONG lIndex,
            /* [out] */ NORMALIZEDRECT *lpNormRect);
        
        END_INTERFACE
    } IDisplayOptimizedRenderVtbl;

    interface IDisplayOptimizedRender
    {
        CONST_VTBL struct IDisplayOptimizedRenderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayOptimizedRender_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayOptimizedRender_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayOptimizedRender_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayOptimizedRender_MultiTextureRender(This,pDevice,lStageIdx,lTexIdx,lpParentRect)	\
    ( (This)->lpVtbl -> MultiTextureRender(This,pDevice,lStageIdx,lTexIdx,lpParentRect) ) 

#define IDisplayOptimizedRender_GetTextureCount(This,plCount)	\
    ( (This)->lpVtbl -> GetTextureCount(This,plCount) ) 

#define IDisplayOptimizedRender_GetTextureCoord(This,lIndex,lpNormRect)	\
    ( (This)->lpVtbl -> GetTextureCoord(This,lIndex,lpNormRect) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayOptimizedRender_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_DispSvr_0000_0013 */
/* [local] */ 


enum SERVER_STATE_EVENT
    {	SERVER_STATE_RESET	= 0,
	SERVER_STATE_PRESENT	= ( SERVER_STATE_RESET + 1 ) ,
	SERVER_STATE_RENDER	= ( SERVER_STATE_PRESENT + 1 ) ,
	SERVER_STATE_VIDEO_SOURCE_ADD	= ( SERVER_STATE_RENDER + 1 ) ,
	SERVER_STATE_VIDEO_SOURCE_REMOVE	= ( SERVER_STATE_VIDEO_SOURCE_ADD + 1 ) ,
	SERVER_STATE_VIDEO_SOURCE_PROCESS_INPUT	= ( SERVER_STATE_VIDEO_SOURCE_REMOVE + 1 ) ,
	SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT	= ( SERVER_STATE_VIDEO_SOURCE_PROCESS_INPUT + 1 ) ,
	SERVER_STATE_VIDEO_SOURCE_DROP_INPUT	= ( SERVER_STATE_VIDEO_SOURCE_PROCESS_OUTPUT + 1 ) 
    } ;


extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0013_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DispSvr_0000_0013_v0_0_s_ifspec;

#ifndef __IDisplayServerStateEventSink_INTERFACE_DEFINED__
#define __IDisplayServerStateEventSink_INTERFACE_DEFINED__

/* interface IDisplayServerStateEventSink */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IDisplayServerStateEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3311AD7C-6816-4e83-9DFE-014693C23EED")
    IDisplayServerStateEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Notify( 
            /* [in] */ DWORD eEvent,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStateText( 
            /* [out][in] */ LPTSTR pText,
            /* [in] */ UINT uLength) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDisplayServerStateEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDisplayServerStateEventSink * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDisplayServerStateEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDisplayServerStateEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Notify )( 
            IDisplayServerStateEventSink * This,
            /* [in] */ DWORD eEvent,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2);
        
        HRESULT ( STDMETHODCALLTYPE *GetStateText )( 
            IDisplayServerStateEventSink * This,
            /* [out][in] */ LPTSTR pText,
            /* [in] */ UINT uLength);
        
        END_INTERFACE
    } IDisplayServerStateEventSinkVtbl;

    interface IDisplayServerStateEventSink
    {
        CONST_VTBL struct IDisplayServerStateEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplayServerStateEventSink_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDisplayServerStateEventSink_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDisplayServerStateEventSink_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDisplayServerStateEventSink_Notify(This,eEvent,dwParam1,dwParam2)	\
    ( (This)->lpVtbl -> Notify(This,eEvent,dwParam1,dwParam2) ) 

#define IDisplayServerStateEventSink_GetStateText(This,pText,uLength)	\
    ( (This)->lpVtbl -> GetStateText(This,pText,uLength) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDisplayServerStateEventSink_INTERFACE_DEFINED__ */


#ifndef __IDispSvrRenderEngineNotify_INTERFACE_DEFINED__
#define __IDispSvrRenderEngineNotify_INTERFACE_DEFINED__

/* interface IDispSvrRenderEngineNotify */
/* [unique][helpstring][uuid][object][local] */ 


enum DispSvrEngineEvent
    {	DISPSVR_RENDER_ENGINE_DEVICE_LOST	= 0,
	DISPSVR_RENDER_ENGINE_HIJACK_DETECTED	= ( DISPSVR_RENDER_ENGINE_DEVICE_LOST + 1 ) ,
	DISPSVR_RENDER_ENGINE_DEVICE_LOST_DETECTED	= ( DISPSVR_RENDER_ENGINE_HIJACK_DETECTED + 1 ) ,
	DISPSVR_RENDER_ENGINE_PROCESS_DEVICE_LOST_REQUIRED	= ( DISPSVR_RENDER_ENGINE_DEVICE_LOST_DETECTED + 1 ) ,
	DISPSVR_RENDER_ENGINE_DEVICE_RECOVERED	= ( DISPSVR_RENDER_ENGINE_PROCESS_DEVICE_LOST_REQUIRED + 1 ) 
    } ;

EXTERN_C const IID IID_IDispSvrRenderEngineNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A07658B5-629B-4402-A9EE-8177C8C28E99")
    IDispSvrRenderEngineNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnNotify( 
            /* [in] */ DWORD dwEvent,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDispSvrRenderEngineNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispSvrRenderEngineNotify * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispSvrRenderEngineNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispSvrRenderEngineNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnNotify )( 
            IDispSvrRenderEngineNotify * This,
            /* [in] */ DWORD dwEvent,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2);
        
        END_INTERFACE
    } IDispSvrRenderEngineNotifyVtbl;

    interface IDispSvrRenderEngineNotify
    {
        CONST_VTBL struct IDispSvrRenderEngineNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispSvrRenderEngineNotify_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDispSvrRenderEngineNotify_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDispSvrRenderEngineNotify_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDispSvrRenderEngineNotify_OnNotify(This,dwEvent,dwParam1,dwParam2)	\
    ( (This)->lpVtbl -> OnNotify(This,dwEvent,dwParam1,dwParam2) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDispSvrRenderEngineNotify_INTERFACE_DEFINED__ */



#ifndef __DispSvrLib_LIBRARY_DEFINED__
#define __DispSvrLib_LIBRARY_DEFINED__

/* library DispSvrLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DispSvrLib;

EXTERN_C const CLSID CLSID_DisplayServer;

#ifdef __cplusplus

class DECLSPEC_UUID("C562E1A9-7DF9-444d-B755-0F502F5351D7")
DisplayServer;
#endif

EXTERN_C const CLSID CLSID_DisplayRenderEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("44E63547-1559-4848-8AA8-E6E14BA8F90B")
DisplayRenderEngine;
#endif

EXTERN_C const CLSID CLSID_CompositeDisplayObject;

#ifdef __cplusplus

class DECLSPEC_UUID("BF7F8E1B-637C-478e-B5DA-492A2D53C698")
CompositeDisplayObject;
#endif

EXTERN_C const CLSID CLSID_VideoRootDisplayObject;

#ifdef __cplusplus

class DECLSPEC_UUID("55F8BDCC-7E1D-4cce-8D74-C9CA69D65ECF")
VideoRootDisplayObject;
#endif

EXTERN_C const CLSID CLSID_ServerStateDisplayObject;

#ifdef __cplusplus

class DECLSPEC_UUID("F8713FA2-F3FB-4219-9BDA-CB46B94C43F9")
ServerStateDisplayObject;
#endif

EXTERN_C const CLSID CLSID_ShineDisplayObject;

#ifdef __cplusplus

class DECLSPEC_UUID("41D51E09-96FE-48a3-8B5B-70751C7F949A")
ShineDisplayObject;
#endif

EXTERN_C const CLSID CLSID_DisplayVideoMixer;

#ifdef __cplusplus

class DECLSPEC_UUID("CD69D3FC-54E9-42fe-BC31-75C151059AB9")
DisplayVideoMixer;
#endif

EXTERN_C const CLSID CLSID_VideoSourceDisplayObject;

#ifdef __cplusplus

class DECLSPEC_UUID("DCB56E62-25A0-49b8-B01B-2CA3B765FA67")
VideoSourceDisplayObject;
#endif
#endif /* __DispSvrLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


