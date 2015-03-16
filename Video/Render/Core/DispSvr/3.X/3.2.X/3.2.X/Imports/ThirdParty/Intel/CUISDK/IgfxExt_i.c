

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_ICUIExtClientNotify,0xF4C4B98D,0xF59E,0x4a0c,0xAE,0xE9,0x80,0x1E,0x0C,0xDB,0x67,0x1E);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal2,0x27E7234F,0x429F,0x4787,0xAC,0x8F,0x8A,0xAD,0xDE,0xD0,0x13,0x55);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal3,0x70F8C65F,0x06AA,0x443b,0x9E,0x6B,0x7C,0x73,0x80,0x8F,0x07,0xE5);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal4,0x5DC5B31E,0x0C28,0x4679,0xB8,0xD8,0x32,0xCF,0x2F,0x9B,0xAC,0xED);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal5,0xA05C525D,0xB4CB,0x4108,0xBF,0xF7,0x1A,0xCF,0x1A,0x14,0xF0,0x0A);


MIDL_DEFINE_GUID(IID, IID_ICUIExternalDual,0x3473E05A,0x3317,0x4df5,0x90,0x98,0xE5,0x38,0x7C,0x94,0xD1,0xB0);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal6,0xAFB6489F,0x4515,0x44AA,0x8D,0xF7,0xED,0x28,0xEA,0x46,0x28,0x3C);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal7,0x2CED2F89,0x627B,0x4E5D,0x84,0x0F,0xB1,0x26,0xEE,0x85,0x8C,0xD8);


MIDL_DEFINE_GUID(IID, IID_ICUIDownScale,0x86709F66,0x89C5,0x4b19,0xA8,0x3F,0xE4,0x99,0x5E,0x21,0x59,0x9A);


MIDL_DEFINE_GUID(IID, IID_ICUIExternal8,0xF932C038,0x6484,0x45ca,0x8F,0xA1,0x7C,0x8C,0x27,0x9F,0x7A,0xEE);


MIDL_DEFINE_GUID(IID, LIBID_IGFXEXTLib,0xF282597F,0x2A21,0x490F,0xA4,0xF0,0x14,0x74,0xF3,0xE6,0x6B,0x38);


MIDL_DEFINE_GUID(CLSID, CLSID_CUIExternal,0x7160A13D,0x73DA,0x4CEA,0x95,0xB9,0x37,0x35,0x64,0x78,0x58,0x8A);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



