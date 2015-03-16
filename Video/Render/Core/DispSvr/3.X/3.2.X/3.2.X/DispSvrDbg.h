

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Wed Sep 22 00:46:38 2010
 */
/* Compiler settings for .\DispSvrDbg.idl:
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


#ifndef __DispSvrDbg_h__
#define __DispSvrDbg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDispSvrDbg_FWD_DEFINED__
#define __IDispSvrDbg_FWD_DEFINED__
typedef interface IDispSvrDbg IDispSvrDbg;
#endif 	/* __IDispSvrDbg_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __DispSvrDbgLib_LIBRARY_DEFINED__
#define __DispSvrDbgLib_LIBRARY_DEFINED__

/* library DispSvrDbgLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DispSvrDbgLib;

#ifndef __IDispSvrDbg_INTERFACE_DEFINED__
#define __IDispSvrDbg_INTERFACE_DEFINED__

/* interface IDispSvrDbg */
/* [dual][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IDispSvrDbg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10ECCEAC-EE07-48bb-B2BA-0658351A8355")
    IDispSvrDbg : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDispSvrVersion( 
            /* [out] */ BSTR *bstrVer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInitFlags( 
            /* [in] */ DWORD dwData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInitFlags( 
            /* [out] */ DWORD *pdwData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDispSvrDbgVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispSvrDbg * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispSvrDbg * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispSvrDbg * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDispSvrVersion )( 
            IDispSvrDbg * This,
            /* [out] */ BSTR *bstrVer);
        
        HRESULT ( STDMETHODCALLTYPE *SetInitFlags )( 
            IDispSvrDbg * This,
            /* [in] */ DWORD dwData);
        
        HRESULT ( STDMETHODCALLTYPE *GetInitFlags )( 
            IDispSvrDbg * This,
            /* [out] */ DWORD *pdwData);
        
        END_INTERFACE
    } IDispSvrDbgVtbl;

    interface IDispSvrDbg
    {
        CONST_VTBL struct IDispSvrDbgVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispSvrDbg_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDispSvrDbg_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDispSvrDbg_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDispSvrDbg_GetDispSvrVersion(This,bstrVer)	\
    ( (This)->lpVtbl -> GetDispSvrVersion(This,bstrVer) ) 

#define IDispSvrDbg_SetInitFlags(This,dwData)	\
    ( (This)->lpVtbl -> SetInitFlags(This,dwData) ) 

#define IDispSvrDbg_GetInitFlags(This,pdwData)	\
    ( (This)->lpVtbl -> GetInitFlags(This,pdwData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDispSvrDbg_INTERFACE_DEFINED__ */

#endif /* __DispSvrDbgLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


