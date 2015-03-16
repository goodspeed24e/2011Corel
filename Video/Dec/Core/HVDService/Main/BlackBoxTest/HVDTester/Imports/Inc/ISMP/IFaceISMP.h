/*
 File   : IFaceISMP.h
 Autour : Chih-Yen Lin (fantasialin@ulead.com.tw)

 Copyright (c) 2006 Ulead Systems, Inc. All rights reserved.

 Purpose: 	Interface for Sink filter (Input pin)
 
 History:
 		2006-08-10 initial
*/

#ifndef SINK_MODULE_IFACE_ISMP_H
#define SINK_MODULE_IFACE_ISMP_H 1

#include <stdio.h>

#define SPCONNECTION_MAGIC_CODE 0xfefeaabb

#define JOEDEBUG_ENABLE 1
#define JOEDEBUG_ALLOW_FAILURE_CONNECTION 1

// {F037776B-9B5A-4590-91AF-D566C0989C34}
EXTERN_GUID(IID_IfaceISMP, 
0xf037776b, 0x9b5a, 0x4590, 0x91, 0xaf, 0xd5, 0x66, 0xc0, 0x98, 0x9c, 0x34);

typedef int (*pfnSPConnectReq)(const BYTE *requestdata, DWORD datastreamtype, BYTE *responsedata, int iMagicCode, void *pData);

EXTERN_C const IID IID_IfaceISMP;

#if defined(__cplusplus) && !defined(CINTERFACE)
    MIDL_INTERFACE("F037776B-9B5A-4590-91AF-D566C0989C34")
	IfaceISMP : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE SPConnect( 
			/* [in] */ pfnSPConnectReq pfnSPConnectReqInst, /* [in] */ void *pData) = 0;        
	};
	
//#else 	/* C style interface */
//
//    typedef struct IfaceISMPVtbl
//    {
//        BEGIN_INTERFACE
//        
//        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
//            IMemInputPin * This,
//            /* [in] */ REFIID riid,
//            /* [iid_is][out] */ void **ppvObject);
//        
//        ULONG ( STDMETHODCALLTYPE *AddRef )( 
//            IMemInputPin * This);
//        
//        ULONG ( STDMETHODCALLTYPE *Release )( 
//            IMemInputPin * This);
//
//		virtual HRESULT STDMETHODCALLTYPE SPConnect( 
//			/* [in] */ pfnSPConnectReq pfnSPConnectReqInst, /* [in] */ void *pData) = 0;      
//
//
//		END_INTERFACE
//    } IfaceISMPVtbl;
//
//    interface IfaceISMP
//    {
//        CONST_VTBL struct IfaceISMPVtbl *lpVtbl;
//    };
#endif

#endif /* SINK_MODULE_IFACE_ISMP_H */
