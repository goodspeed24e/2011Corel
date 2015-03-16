

/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Wed Sep 22 00:46:37 2010
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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "DispSvr.h"

#define TYPE_FORMAT_STRING_SIZE   3                                 
#define PROC_FORMAT_STRING_SIZE   1                                 
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _DispSvr_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } DispSvr_MIDL_TYPE_FORMAT_STRING;

typedef struct _DispSvr_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } DispSvr_MIDL_PROC_FORMAT_STRING;

typedef struct _DispSvr_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } DispSvr_MIDL_EXPR_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const DispSvr_MIDL_TYPE_FORMAT_STRING DispSvr__MIDL_TypeFormatString;
extern const DispSvr_MIDL_PROC_FORMAT_STRING DispSvr__MIDL_ProcFormatString;
extern const DispSvr_MIDL_EXPR_FORMAT_STRING DispSvr__MIDL_ExprFormatString;



#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const DispSvr_MIDL_PROC_FORMAT_STRING DispSvr__MIDL_ProcFormatString =
    {
        0,
        {

			0x0
        }
    };

static const DispSvr_MIDL_TYPE_FORMAT_STRING DispSvr__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */

			0x0
        }
    };


/* Standard interface: __MIDL_itf_DispSvr_0000_0000, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDisplayVideoSourceManager, ver. 0.0,
   GUID={0x56A32D0E,0x5649,0x4b5a,{0xA7,0x46,0xD9,0x4E,0x56,0xE9,0xCA,0xF8}} */


/* Object interface: IDisplayServer, ver. 0.0,
   GUID={0x32A4CD3D,0x47C3,0x476b,{0xB2,0xFF,0x0A,0x02,0x00,0x08,0x0C,0xA9}} */


/* Object interface: IDisplayVideoSource, ver. 0.0,
   GUID={0x3604A4CC,0x2610,0x4ef2,{0x97,0xDB,0x9F,0xBE,0x7D,0x63,0x8E,0x55}} */


/* Object interface: IDisplayLock, ver. 0.0,
   GUID={0x87A59958,0x7DF9,0x4e67,{0x98,0x60,0x8C,0xFD,0x50,0x54,0xCF,0x1E}} */


/* Standard interface: __MIDL_itf_DispSvr_0000_0004, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IDisplayRenderEngine, ver. 0.0,
   GUID={0xD89A29FF,0xE2C3,0x4e61,{0xA1,0xD1,0xC5,0x1D,0x17,0x7D,0x12,0x00}} */


/* Object interface: IDisplayEventHandler, ver. 0.0,
   GUID={0x6CA13811,0xC666,0x4ba2,{0xB9,0xBE,0x01,0x09,0xB4,0xC1,0x21,0xAF}} */


/* Object interface: IDisplayEventHost, ver. 0.0,
   GUID={0xECEBF734,0xBDF3,0x487d,{0xB8,0x4B,0xDB,0x9E,0xC4,0xE1,0xD5,0x14}} */


/* Object interface: IDisplayObject, ver. 0.0,
   GUID={0xF4B500E2,0xBD7F,0x4e2f,{0x86,0x3A,0x12,0x94,0x63,0x9D,0x90,0xCE}} */


/* Object interface: IParentDisplayObject, ver. 0.0,
   GUID={0x5B26B321,0x8B17,0x435c,{0x83,0x90,0xA3,0xF8,0x37,0x84,0x72,0xC1}} */


/* Object interface: IDisplayVideoSink, ver. 0.0,
   GUID={0x1EE35D5C,0x4DAB,0x4356,{0x80,0x8C,0xEC,0xC5,0xBA,0x2A,0x19,0xA5}} */


/* Object interface: IDisplayVideoMixer, ver. 0.0,
   GUID={0x8ADD1492,0xCA9D,0x42c5,{0x9F,0x0E,0x4A,0x5D,0x6D,0x9D,0xEF,0xF7}} */


/* Object interface: IDisplayProperties, ver. 0.0,
   GUID={0x403A5DE1,0xA8DA,0x4b8a,{0x84,0xF9,0x38,0xA2,0x69,0x3E,0x19,0x1F}} */


/* Object interface: IDisplayOptimizedRender, ver. 0.0,
   GUID={0xCEF8B340,0x07B7,0x4568,{0xA2,0x70,0x4D,0xB5,0xE6,0xF9,0x70,0x99}} */


/* Standard interface: __MIDL_itf_DispSvr_0000_0013, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IDisplayServerStateEventSink, ver. 0.0,
   GUID={0x3311AD7C,0x6816,0x4e83,{0x9D,0xFE,0x01,0x46,0x93,0xC2,0x3E,0xED}} */


/* Object interface: IDispSvrRenderEngineNotify, ver. 0.0,
   GUID={0xA07658B5,0x629B,0x4402,{0xA9,0xEE,0x81,0x77,0xC8,0xC2,0x8E,0x99}} */


/* Object interface: IDisplayStereoControl, ver. 0.0,
   GUID={0x4E1D464A,0xAB73,0x4782,{0x94,0xEF,0x57,0xAB,0xFF,0xBA,0x42,0x37}} */

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    DispSvr__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x70001f3, /* MIDL Version 7.0.499 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * _DispSvr_ProxyVtblList[] = 
{
    0
};

const CInterfaceStubVtbl * _DispSvr_StubVtblList[] = 
{
    0
};

PCInterfaceName const _DispSvr_InterfaceNamesList[] = 
{
    0
};


#define _DispSvr_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _DispSvr, pIID, n)

int __stdcall _DispSvr_IID_Lookup( const IID * pIID, int * pIndex )
{
    return 0;
}

const ExtendedProxyFileInfo DispSvr_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _DispSvr_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _DispSvr_StubVtblList,
    (const PCInterfaceName * ) & _DispSvr_InterfaceNamesList,
    0, // no delegation
    & _DispSvr_IID_Lookup, 
    0,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

