

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue Apr 27 14:56:51 2010
 */
/* Compiler settings for .\IgfxExt.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
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

#ifndef __IgfxExt_h__
#define __IgfxExt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICUIExtClientNotify_FWD_DEFINED__
#define __ICUIExtClientNotify_FWD_DEFINED__
typedef interface ICUIExtClientNotify ICUIExtClientNotify;
#endif 	/* __ICUIExtClientNotify_FWD_DEFINED__ */


#ifndef __ICUIExternal2_FWD_DEFINED__
#define __ICUIExternal2_FWD_DEFINED__
typedef interface ICUIExternal2 ICUIExternal2;
#endif 	/* __ICUIExternal2_FWD_DEFINED__ */


#ifndef __ICUIExternal3_FWD_DEFINED__
#define __ICUIExternal3_FWD_DEFINED__
typedef interface ICUIExternal3 ICUIExternal3;
#endif 	/* __ICUIExternal3_FWD_DEFINED__ */


#ifndef __ICUIExternal4_FWD_DEFINED__
#define __ICUIExternal4_FWD_DEFINED__
typedef interface ICUIExternal4 ICUIExternal4;
#endif 	/* __ICUIExternal4_FWD_DEFINED__ */


#ifndef __ICUIExternal5_FWD_DEFINED__
#define __ICUIExternal5_FWD_DEFINED__
typedef interface ICUIExternal5 ICUIExternal5;
#endif 	/* __ICUIExternal5_FWD_DEFINED__ */


#ifndef __ICUIExternalDual_FWD_DEFINED__
#define __ICUIExternalDual_FWD_DEFINED__
typedef interface ICUIExternalDual ICUIExternalDual;
#endif 	/* __ICUIExternalDual_FWD_DEFINED__ */


#ifndef __ICUIExternal6_FWD_DEFINED__
#define __ICUIExternal6_FWD_DEFINED__
typedef interface ICUIExternal6 ICUIExternal6;
#endif 	/* __ICUIExternal6_FWD_DEFINED__ */


#ifndef __ICUIExternal7_FWD_DEFINED__
#define __ICUIExternal7_FWD_DEFINED__
typedef interface ICUIExternal7 ICUIExternal7;
#endif 	/* __ICUIExternal7_FWD_DEFINED__ */


#ifndef __ICUIDownScale_FWD_DEFINED__
#define __ICUIDownScale_FWD_DEFINED__
typedef interface ICUIDownScale ICUIDownScale;
#endif 	/* __ICUIDownScale_FWD_DEFINED__ */


#ifndef __ICUIExternal8_FWD_DEFINED__
#define __ICUIExternal8_FWD_DEFINED__
typedef interface ICUIExternal8 ICUIExternal8;
#endif 	/* __ICUIExternal8_FWD_DEFINED__ */


#ifndef __CUIExternal_FWD_DEFINED__
#define __CUIExternal_FWD_DEFINED__

#ifdef __cplusplus
typedef class CUIExternal CUIExternal;
#else
typedef struct CUIExternal CUIExternal;
#endif /* __cplusplus */

#endif 	/* __CUIExternal_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_IgfxExt_0000_0000 */
/* [local] */ 

//Copyright?2003, Intel Corporation




extern RPC_IF_HANDLE __MIDL_itf_IgfxExt_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_IgfxExt_0000_0000_v0_0_s_ifspec;

#ifndef __ICUIExtClientNotify_INTERFACE_DEFINED__
#define __ICUIExtClientNotify_INTERFACE_DEFINED__

/* interface ICUIExtClientNotify */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExtClientNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4C4B98D-F59E-4a0c-AEE9-801E0CDB671E")
    ICUIExtClientNotify : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Notify( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExtClientNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExtClientNotify * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExtClientNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExtClientNotify * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Notify )( 
            ICUIExtClientNotify * This);
        
        END_INTERFACE
    } ICUIExtClientNotifyVtbl;

    interface ICUIExtClientNotify
    {
        CONST_VTBL struct ICUIExtClientNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExtClientNotify_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExtClientNotify_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExtClientNotify_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExtClientNotify_Notify(This)	\
    ( (This)->lpVtbl -> Notify(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExtClientNotify_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal2_INTERFACE_DEFINED__
#define __ICUIExternal2_INTERFACE_DEFINED__

/* interface ICUIExternal2 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("27E7234F-429F-4787-AC8F-8AADDED01355")
    ICUIExternal2 : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnumActiveDevices( 
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConfiguration( 
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedEvents( 
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RegisterEvent( 
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RegisterNotify( 
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DeRegisterEvent( 
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DeRegisterNotify( 
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetConfiguration( 
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal2 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal2 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal2 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal2 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal2 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal2 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal2 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal2 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal2 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal2Vtbl;

    interface ICUIExternal2
    {
        CONST_VTBL struct ICUIExternal2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal2_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal2_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal2_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal2_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal2_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal2_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal2_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal2_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal2_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal3_INTERFACE_DEFINED__
#define __ICUIExternal3_INTERFACE_DEFINED__

/* interface ICUIExternal3 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70F8C65F-06AA-443b-9E6B-7C73808F07E5")
    ICUIExternal3 : public ICUIExternal2
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ChangeActiveDevices( 
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnumAttachableDevices( 
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal3 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal3 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal3 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal3 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal3 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal3 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal3 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal3 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal3 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal3 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal3 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal3 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        END_INTERFACE
    } ICUIExternal3Vtbl;

    interface ICUIExternal3
    {
        CONST_VTBL struct ICUIExternal3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal3_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal3_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal3_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal3_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal3_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal3_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal3_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal3_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal3_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal3_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal3_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal3_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal3_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal3_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal4_INTERFACE_DEFINED__
#define __ICUIExternal4_INTERFACE_DEFINED__

/* interface ICUIExternal4 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal4;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5DC5B31E-0C28-4679-B8D8-32CF2F9BACED")
    ICUIExternal4 : public ICUIExternal3
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsOpen( 
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsClose( 
            /* [in] */ UINT Handle) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsGetMax( 
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsGetMin( 
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsGetCurrent( 
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsSetCurrent( 
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MccsResetControl( 
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal4Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal4 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal4 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal4 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal4 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal4 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal4 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal4 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal4 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal4 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal4 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal4 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal4 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal4 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIExternal4 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIExternal4 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal4Vtbl;

    interface ICUIExternal4
    {
        CONST_VTBL struct ICUIExternal4Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal4_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal4_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal4_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal4_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal4_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal4_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal4_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal4_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal4_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal4_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal4_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal4_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal4_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIExternal4_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIExternal4_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIExternal4_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal4_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal4_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal4_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIExternal4_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal4_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal5_INTERFACE_DEFINED__
#define __ICUIExternal5_INTERFACE_DEFINED__

/* interface ICUIExternal5 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal5;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A05C525D-B4CB-4108-BFF7-1ACF1A14F00A")
    ICUIExternal5 : public ICUIExternal4
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PowerApiOpen( 
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PowerApiClose( 
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPowerConsCaps( 
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPowerPolicy( 
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetPowerPolicy( 
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal5Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal5 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal5 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal5 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal5 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal5 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal5 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal5 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal5 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal5 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal5 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal5 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal5 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal5 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIExternal5 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiOpen )( 
            ICUIExternal5 * This,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiClose )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerConsCaps )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerPolicy )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPowerPolicy )( 
            ICUIExternal5 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal5Vtbl;

    interface ICUIExternal5
    {
        CONST_VTBL struct ICUIExternal5Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal5_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal5_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal5_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal5_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal5_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal5_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal5_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal5_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal5_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal5_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal5_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal5_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal5_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIExternal5_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIExternal5_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIExternal5_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal5_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal5_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal5_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIExternal5_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 


#define ICUIExternal5_PowerApiOpen(This,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiOpen(This,pHandle,pExtraErrorCode) ) 

#define ICUIExternal5_PowerApiClose(This,Handle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiClose(This,Handle,pExtraErrorCode) ) 

#define ICUIExternal5_GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode) ) 

#define ICUIExternal5_GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#define ICUIExternal5_SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal5_INTERFACE_DEFINED__ */


#ifndef __ICUIExternalDual_INTERFACE_DEFINED__
#define __ICUIExternalDual_INTERFACE_DEFINED__

/* interface ICUIExternalDual */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternalDual;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3473E05A-3317-4df5-9098-E5387C94D1B0")
    ICUIExternalDual : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternalDualVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternalDual * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternalDual * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternalDual * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICUIExternalDual * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICUIExternalDual * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICUIExternalDual * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICUIExternalDual * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } ICUIExternalDualVtbl;

    interface ICUIExternalDual
    {
        CONST_VTBL struct ICUIExternalDualVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternalDual_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternalDual_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternalDual_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternalDual_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICUIExternalDual_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICUIExternalDual_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICUIExternalDual_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternalDual_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal6_INTERFACE_DEFINED__
#define __ICUIExternal6_INTERFACE_DEFINED__

/* interface ICUIExternal6 */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal6;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AFB6489F-4515-44AA-8DF7-ED28EA46283C")
    ICUIExternal6 : public ICUIExternal5
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGammaRamp( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGammaRamp( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAspectScalingCapabilities( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwOperatingMode,
            /* [out] */ DWORD *pdwAspectScalingCaps,
            /* [out] */ DWORD *pdwCurrentAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetAspectScalingPreference( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAvailableConnectors( 
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *dwAvailableConnectors,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetConnectorSelection( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwConnector,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConnectorAttachedStatus( 
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnectors,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConnectorSelection( 
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnector,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCloneRefreshRates( 
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRArray,
            /* [size_is][out] */ BYTE *pPrimaryRR,
            /* [size_is][out] */ BYTE *pSecondaryRR,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCloneView( 
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRStruct,
            /* [size_is][in] */ BYTE *pPrimRRToSet,
            /* [size_is][in] */ BYTE *pSecndRRToSet,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTvParameters( 
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTvParameters( 
            /* [in] */ ULONG uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal6Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal6 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal6 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal6 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal6 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal6 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal6 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal6 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal6 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal6 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal6 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal6 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal6 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiOpen )( 
            ICUIExternal6 * This,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiClose )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerConsCaps )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerPolicy )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPowerPolicy )( 
            ICUIExternal6 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGammaRamp )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetGammaRamp )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAspectScalingCapabilities )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwOperatingMode,
            /* [out] */ DWORD *pdwAspectScalingCaps,
            /* [out] */ DWORD *pdwCurrentAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetAspectScalingPreference )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAvailableConnectors )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *dwAvailableConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConnectorSelection )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorAttachedStatus )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorSelection )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCloneRefreshRates )( 
            ICUIExternal6 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRArray,
            /* [size_is][out] */ BYTE *pPrimaryRR,
            /* [size_is][out] */ BYTE *pSecondaryRR,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCloneView )( 
            ICUIExternal6 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRStruct,
            /* [size_is][in] */ BYTE *pPrimRRToSet,
            /* [size_is][in] */ BYTE *pSecndRRToSet,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTvParameters )( 
            ICUIExternal6 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTvParameters )( 
            ICUIExternal6 * This,
            /* [in] */ ULONG uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal6Vtbl;

    interface ICUIExternal6
    {
        CONST_VTBL struct ICUIExternal6Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal6_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal6_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal6_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal6_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal6_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal6_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal6_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal6_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal6_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal6_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal6_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal6_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal6_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIExternal6_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIExternal6_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIExternal6_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal6_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal6_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal6_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIExternal6_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 


#define ICUIExternal6_PowerApiOpen(This,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiOpen(This,pHandle,pExtraErrorCode) ) 

#define ICUIExternal6_PowerApiClose(This,Handle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiClose(This,Handle,pExtraErrorCode) ) 

#define ICUIExternal6_GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode) ) 

#define ICUIExternal6_GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#define ICUIExternal6_SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 


#define ICUIExternal6_GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal6_SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal6_GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal6_SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal6_GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode) ) 

#define ICUIExternal6_SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode) ) 

#define ICUIExternal6_GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode) ) 

#define ICUIExternal6_GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode) ) 

#define ICUIExternal6_GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode) ) 

#define ICUIExternal6_SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode) ) 

#define ICUIExternal6_GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 

#define ICUIExternal6_SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal6_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal7_INTERFACE_DEFINED__
#define __ICUIExternal7_INTERFACE_DEFINED__

/* interface ICUIExternal7 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal7;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2CED2F89-627B-4E5D-840F-B126EE858CD8")
    ICUIExternal7 : public ICUIExternal6
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInverterParams( 
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetInverterParams( 
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal7Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal7 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal7 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal7 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal7 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal7 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal7 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal7 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal7 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal7 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal7 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal7 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal7 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiOpen )( 
            ICUIExternal7 * This,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiClose )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerConsCaps )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerPolicy )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPowerPolicy )( 
            ICUIExternal7 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGammaRamp )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetGammaRamp )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAspectScalingCapabilities )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwOperatingMode,
            /* [out] */ DWORD *pdwAspectScalingCaps,
            /* [out] */ DWORD *pdwCurrentAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetAspectScalingPreference )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAvailableConnectors )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *dwAvailableConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConnectorSelection )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorAttachedStatus )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorSelection )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCloneRefreshRates )( 
            ICUIExternal7 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRArray,
            /* [size_is][out] */ BYTE *pPrimaryRR,
            /* [size_is][out] */ BYTE *pSecondaryRR,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCloneView )( 
            ICUIExternal7 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRStruct,
            /* [size_is][in] */ BYTE *pPrimRRToSet,
            /* [size_is][in] */ BYTE *pSecndRRToSet,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTvParameters )( 
            ICUIExternal7 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTvParameters )( 
            ICUIExternal7 * This,
            /* [in] */ ULONG uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetInverterParams )( 
            ICUIExternal7 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetInverterParams )( 
            ICUIExternal7 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal7Vtbl;

    interface ICUIExternal7
    {
        CONST_VTBL struct ICUIExternal7Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal7_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal7_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal7_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal7_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal7_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal7_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal7_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal7_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal7_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal7_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal7_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal7_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal7_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIExternal7_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIExternal7_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIExternal7_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal7_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal7_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal7_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIExternal7_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 


#define ICUIExternal7_PowerApiOpen(This,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiOpen(This,pHandle,pExtraErrorCode) ) 

#define ICUIExternal7_PowerApiClose(This,Handle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiClose(This,Handle,pExtraErrorCode) ) 

#define ICUIExternal7_GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode) ) 

#define ICUIExternal7_GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#define ICUIExternal7_SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 


#define ICUIExternal7_GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal7_SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal7_GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal7_SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal7_GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode) ) 

#define ICUIExternal7_SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode) ) 

#define ICUIExternal7_GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode) ) 

#define ICUIExternal7_GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode) ) 

#define ICUIExternal7_GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode) ) 

#define ICUIExternal7_SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode) ) 

#define ICUIExternal7_GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 

#define ICUIExternal7_SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 


#define ICUIExternal7_GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 

#define ICUIExternal7_SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal7_INTERFACE_DEFINED__ */


#ifndef __ICUIDownScale_INTERFACE_DEFINED__
#define __ICUIDownScale_INTERFACE_DEFINED__

/* interface ICUIDownScale */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIDownScale;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("86709F66-89C5-4b19-A83F-E4995E21599A")
    ICUIDownScale : public ICUIExternal7
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsDownScalingSupported( 
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsDownScalingEnabled( 
            /* [out] */ BOOL *pbIsEnabled,
            /* [out] */ DWORD *pdwError) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnableDownScaling( 
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DisableDownScaling( 
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIDownScaleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIDownScale * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIDownScale * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIDownScale * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIDownScale * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIDownScale * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIDownScale * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIDownScale * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIDownScale * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIDownScale * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIDownScale * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIDownScale * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIDownScale * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiOpen )( 
            ICUIDownScale * This,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiClose )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerConsCaps )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerPolicy )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPowerPolicy )( 
            ICUIDownScale * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGammaRamp )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetGammaRamp )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAspectScalingCapabilities )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwOperatingMode,
            /* [out] */ DWORD *pdwAspectScalingCaps,
            /* [out] */ DWORD *pdwCurrentAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetAspectScalingPreference )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAvailableConnectors )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *dwAvailableConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConnectorSelection )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorAttachedStatus )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorSelection )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCloneRefreshRates )( 
            ICUIDownScale * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRArray,
            /* [size_is][out] */ BYTE *pPrimaryRR,
            /* [size_is][out] */ BYTE *pSecondaryRR,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCloneView )( 
            ICUIDownScale * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRStruct,
            /* [size_is][in] */ BYTE *pPrimRRToSet,
            /* [size_is][in] */ BYTE *pSecndRRToSet,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTvParameters )( 
            ICUIDownScale * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTvParameters )( 
            ICUIDownScale * This,
            /* [in] */ ULONG uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetInverterParams )( 
            ICUIDownScale * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetInverterParams )( 
            ICUIDownScale * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsDownScalingSupported )( 
            ICUIDownScale * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsDownScalingEnabled )( 
            ICUIDownScale * This,
            /* [out] */ BOOL *pbIsEnabled,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnableDownScaling )( 
            ICUIDownScale * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DisableDownScaling )( 
            ICUIDownScale * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        END_INTERFACE
    } ICUIDownScaleVtbl;

    interface ICUIDownScale
    {
        CONST_VTBL struct ICUIDownScaleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIDownScale_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIDownScale_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIDownScale_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIDownScale_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIDownScale_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIDownScale_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIDownScale_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIDownScale_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIDownScale_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIDownScale_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIDownScale_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIDownScale_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIDownScale_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIDownScale_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIDownScale_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIDownScale_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIDownScale_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIDownScale_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIDownScale_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIDownScale_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 


#define ICUIDownScale_PowerApiOpen(This,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiOpen(This,pHandle,pExtraErrorCode) ) 

#define ICUIDownScale_PowerApiClose(This,Handle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiClose(This,Handle,pExtraErrorCode) ) 

#define ICUIDownScale_GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode) ) 

#define ICUIDownScale_GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#define ICUIDownScale_SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 


#define ICUIDownScale_GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIDownScale_SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIDownScale_GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIDownScale_SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIDownScale_GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode) ) 

#define ICUIDownScale_SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode) ) 

#define ICUIDownScale_GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode) ) 

#define ICUIDownScale_GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode) ) 

#define ICUIDownScale_GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode) ) 

#define ICUIDownScale_SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode) ) 

#define ICUIDownScale_GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 

#define ICUIDownScale_SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 


#define ICUIDownScale_GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 

#define ICUIDownScale_SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 


#define ICUIDownScale_IsDownScalingSupported(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> IsDownScalingSupported(This,nSize,pDownScaleParams,pdwError) ) 

#define ICUIDownScale_IsDownScalingEnabled(This,pbIsEnabled,pdwError)	\
    ( (This)->lpVtbl -> IsDownScalingEnabled(This,pbIsEnabled,pdwError) ) 

#define ICUIDownScale_EnableDownScaling(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> EnableDownScaling(This,nSize,pDownScaleParams,pdwError) ) 

#define ICUIDownScale_DisableDownScaling(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> DisableDownScaling(This,nSize,pDownScaleParams,pdwError) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIDownScale_INTERFACE_DEFINED__ */


#ifndef __ICUIExternal8_INTERFACE_DEFINED__
#define __ICUIExternal8_INTERFACE_DEFINED__

/* interface ICUIExternal8 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ICUIExternal8;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F932C038-6484-45ca-8FA1-7C8C279F7AEE")
    ICUIExternal8 : public ICUIDownScale
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDeviceData( 
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ BYTE *pData,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDeviceData( 
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ BYTE *pData,
            /* [out] */ DWORD *pExtraErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICUIExternal8Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICUIExternal8 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICUIExternal8 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICUIExternal8 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumActiveDevices )( 
            ICUIExternal8 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConfiguration )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedEvents )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [retval][out] */ DWORD *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterEvent )( 
            ICUIExternal8 * This,
            /* [in] */ BSTR strEventName,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotify )( 
            ICUIExternal8 * This,
            /* [in] */ ICUIExtClientNotify *pExtClientNotify,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwEventMask,
            /* [out] */ UINT *pRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterEvent )( 
            ICUIExternal8 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DeRegisterNotify )( 
            ICUIExternal8 * This,
            /* [in] */ UINT nRegistrationID,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConfiguration )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDisplayConfig,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *ChangeActiveDevices )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pDeviceDisplays,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnumAttachableDevices )( 
            ICUIExternal8 * This,
            /* [in] */ BSTR strDeviceName,
            /* [in] */ UINT nIndex,
            /* [out] */ DWORD *puidMonitor,
            /* [out][in] */ DWORD *pdwDeviceType,
            /* [out] */ DWORD *pdwStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsOpen )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsClose )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMax )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetMin )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsGetCurrent )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pVal,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsSetCurrent )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [in] */ DWORD Val,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *MccsResetControl )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ UINT ControlCode,
            /* [in] */ UINT Size,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiOpen )( 
            ICUIExternal8 * This,
            /* [out] */ UINT *pHandle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PowerApiClose )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerConsCaps )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [out] */ DWORD *pCaps,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPowerPolicy )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetPowerPolicy )( 
            ICUIExternal8 * This,
            /* [in] */ UINT Handle,
            /* [in] */ DWORD PolicyID,
            /* [in] */ REFGUID guid,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pPolicy,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetGammaRamp )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetGammaRamp )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pGammaRamp,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAspectScalingCapabilities )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwOperatingMode,
            /* [out] */ DWORD *pdwAspectScalingCaps,
            /* [out] */ DWORD *pdwCurrentAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetAspectScalingPreference )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwAspectScalingPreference,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAvailableConnectors )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *dwAvailableConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetConnectorSelection )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ DWORD dwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorAttachedStatus )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnectors,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetConnectorSelection )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [out] */ DWORD *pdwConnector,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetCloneRefreshRates )( 
            ICUIExternal8 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRArray,
            /* [size_is][out] */ BYTE *pPrimaryRR,
            /* [size_is][out] */ BYTE *pSecondaryRR,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetCloneView )( 
            ICUIExternal8 * This,
            /* [in] */ UINT nSizeCfg,
            /* [size_is][in] */ BYTE *pDispCfg,
            /* [in] */ UINT nSizeRRStruct,
            /* [size_is][in] */ BYTE *pPrimRRToSet,
            /* [size_is][in] */ BYTE *pSecndRRToSet,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetTvParameters )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][out] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetTvParameters )( 
            ICUIExternal8 * This,
            /* [in] */ ULONG uidMonitor,
            /* [in] */ UINT nSize,
            /* [size_is][in] */ BYTE *pTvParameter,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetInverterParams )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetInverterParams )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD PolicyID,
            /* [in] */ DWORD cb,
            /* [size_is][out][in] */ BYTE *pPwrParams,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsDownScalingSupported )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsDownScalingEnabled )( 
            ICUIExternal8 * This,
            /* [out] */ BOOL *pbIsEnabled,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *EnableDownScaling )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *DisableDownScaling )( 
            ICUIExternal8 * This,
            /* [in] */ DWORD nSize,
            /* [size_is][out][in] */ BYTE *pDownScaleParams,
            /* [out] */ DWORD *pdwError);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceData )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ BYTE *pData,
            /* [out] */ DWORD *pExtraErrorCode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetDeviceData )( 
            ICUIExternal8 * This,
            /* [in] */ REFGUID guid,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ BYTE *pData,
            /* [out] */ DWORD *pExtraErrorCode);
        
        END_INTERFACE
    } ICUIExternal8Vtbl;

    interface ICUIExternal8
    {
        CONST_VTBL struct ICUIExternal8Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICUIExternal8_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICUIExternal8_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICUIExternal8_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICUIExternal8_EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType)	\
    ( (This)->lpVtbl -> EnumActiveDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType) ) 

#define ICUIExternal8_GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 

#define ICUIExternal8_get_SupportedEvents(This,uidMonitor,pVal)	\
    ( (This)->lpVtbl -> get_SupportedEvents(This,uidMonitor,pVal) ) 

#define ICUIExternal8_RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterEvent(This,strEventName,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal8_RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> RegisterNotify(This,pExtClientNotify,uidMonitor,dwEventMask,pRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal8_DeRegisterEvent(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterEvent(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal8_DeRegisterNotify(This,nRegistrationID,pExtraErrorCode)	\
    ( (This)->lpVtbl -> DeRegisterNotify(This,nRegistrationID,pExtraErrorCode) ) 

#define ICUIExternal8_SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConfiguration(This,guid,nSize,pDisplayConfig,pExtraErrorCode) ) 


#define ICUIExternal8_ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode)	\
    ( (This)->lpVtbl -> ChangeActiveDevices(This,guid,nSize,pDeviceDisplays,pExtraErrorCode) ) 

#define ICUIExternal8_EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus)	\
    ( (This)->lpVtbl -> EnumAttachableDevices(This,strDeviceName,nIndex,puidMonitor,pdwDeviceType,pdwStatus) ) 


#define ICUIExternal8_MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsOpen(This,uidMonitor,pHandle,pExtraErrorCode) ) 

#define ICUIExternal8_MccsClose(This,Handle)	\
    ( (This)->lpVtbl -> MccsClose(This,Handle) ) 

#define ICUIExternal8_MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMax(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal8_MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetMin(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal8_MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsGetCurrent(This,Handle,ControlCode,Size,pVal,pExtraErrorCode) ) 

#define ICUIExternal8_MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsSetCurrent(This,Handle,ControlCode,Size,Val,pExtraErrorCode) ) 

#define ICUIExternal8_MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode)	\
    ( (This)->lpVtbl -> MccsResetControl(This,Handle,ControlCode,Size,pExtraErrorCode) ) 


#define ICUIExternal8_PowerApiOpen(This,pHandle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiOpen(This,pHandle,pExtraErrorCode) ) 

#define ICUIExternal8_PowerApiClose(This,Handle,pExtraErrorCode)	\
    ( (This)->lpVtbl -> PowerApiClose(This,Handle,pExtraErrorCode) ) 

#define ICUIExternal8_GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerConsCaps(This,Handle,pCaps,pExtraErrorCode) ) 

#define ICUIExternal8_GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 

#define ICUIExternal8_SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetPowerPolicy(This,Handle,PolicyID,guid,nSize,pPolicy,pExtraErrorCode) ) 


#define ICUIExternal8_GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal8_SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetGammaRamp(This,uidMonitor,nSize,pGammaRamp,pExtraErrorCode) ) 

#define ICUIExternal8_GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAspectScalingCapabilities(This,uidMonitor,dwOperatingMode,pdwAspectScalingCaps,pdwCurrentAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal8_SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetAspectScalingPreference(This,uidMonitor,dwAspectScalingPreference,pExtraErrorCode) ) 

#define ICUIExternal8_GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetAvailableConnectors(This,uidMonitor,dwAvailableConnectors,pExtraErrorCode) ) 

#define ICUIExternal8_SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetConnectorSelection(This,uidMonitor,dwConnector,pExtraErrorCode) ) 

#define ICUIExternal8_GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorAttachedStatus(This,uidMonitor,pdwConnectors,pExtraErrorCode) ) 

#define ICUIExternal8_GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetConnectorSelection(This,uidMonitor,pdwConnector,pExtraErrorCode) ) 

#define ICUIExternal8_GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetCloneRefreshRates(This,nSizeCfg,pDispCfg,nSizeRRArray,pPrimaryRR,pSecondaryRR,pExtraErrorCode) ) 

#define ICUIExternal8_SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetCloneView(This,nSizeCfg,pDispCfg,nSizeRRStruct,pPrimRRToSet,pSecndRRToSet,pExtraErrorCode) ) 

#define ICUIExternal8_GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 

#define ICUIExternal8_SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetTvParameters(This,uidMonitor,nSize,pTvParameter,pExtraErrorCode) ) 


#define ICUIExternal8_GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 

#define ICUIExternal8_SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetInverterParams(This,guid,PolicyID,cb,pPwrParams,pExtraErrorCode) ) 


#define ICUIExternal8_IsDownScalingSupported(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> IsDownScalingSupported(This,nSize,pDownScaleParams,pdwError) ) 

#define ICUIExternal8_IsDownScalingEnabled(This,pbIsEnabled,pdwError)	\
    ( (This)->lpVtbl -> IsDownScalingEnabled(This,pbIsEnabled,pdwError) ) 

#define ICUIExternal8_EnableDownScaling(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> EnableDownScaling(This,nSize,pDownScaleParams,pdwError) ) 

#define ICUIExternal8_DisableDownScaling(This,nSize,pDownScaleParams,pdwError)	\
    ( (This)->lpVtbl -> DisableDownScaling(This,nSize,pDownScaleParams,pdwError) ) 


#define ICUIExternal8_GetDeviceData(This,guid,dwSize,pData,pExtraErrorCode)	\
    ( (This)->lpVtbl -> GetDeviceData(This,guid,dwSize,pData,pExtraErrorCode) ) 

#define ICUIExternal8_SetDeviceData(This,guid,dwSize,pData,pExtraErrorCode)	\
    ( (This)->lpVtbl -> SetDeviceData(This,guid,dwSize,pData,pExtraErrorCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICUIExternal8_INTERFACE_DEFINED__ */



#ifndef __IGFXEXTLib_LIBRARY_DEFINED__
#define __IGFXEXTLib_LIBRARY_DEFINED__

/* library IGFXEXTLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_IGFXEXTLib;

EXTERN_C const CLSID CLSID_CUIExternal;

#ifdef __cplusplus

class DECLSPEC_UUID("7160A13D-73DA-4CEA-95B9-37356478588A")
CUIExternal;
#endif
#endif /* __IGFXEXTLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


