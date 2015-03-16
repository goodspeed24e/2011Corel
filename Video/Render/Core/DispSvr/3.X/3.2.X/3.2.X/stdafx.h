#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "SDKConfigDef.h"

#ifdef _DEBUG_D3D
#define D3D_DEBUG_INFO
#endif

#if _MSC_VER >= 1400
#include <windows.h>
#include <tchar.h>
#endif //_MSC_VER
#include <streams.h>
#if _MSC_VER >= 1400
#undef lstrlenW
#endif //_MSC_VER

#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9.h"
#include "Imports/ThirdParty/Microsoft/DXVAHD/d3d9types.h"
#include "Imports/ThirdParty/Microsoft/DXVAHD/dxvahd.h"

#ifdef _NO_USE_D3DXDLL
#include "./Imports/Inc/_D3DX9Math.h"
//#include <d3dx9.h>
#else

#include <d3dx9.h>
#ifndef _DEBUG_D3D
#pragma comment(lib, "d3dx9.lib")
#endif

#include <vmr9.h>
#endif
#include <dshow.h>
#include <atlbase.h>
#include "./Imports/Diagnosis/GPS/Inc/DogCategory.h"
#include "./Imports/Diagnosis/GPS/Inc/DogProfile.h"
#include "./Imports/Diagnosis/GPS/Inc/SubCategory.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if(p) { (p)->Release(); p = NULL; }
#endif  // SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	if(p) { delete p; p = NULL; }
#endif // SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	if(p) { delete [] p; p = NULL; }
#endif  // SAFE_DELETE_ARRAY

#ifndef CHECK_HR
#define CHECK_HR(hr, f) \
    if( FAILED(hr) ){ f; throw hr; }
#endif

#ifndef CHECK_POINTER
#define CHECK_POINTER(p)	\
	if (p == NULL)	{ return E_POINTER; }
#endif

#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif


namespace DispSvr
{
	int GetRegInt(const TCHAR *pKey, int iDefault);
}

// Global prototypes
#ifndef __QUOTE
#define __QUOTE__(x) #x
#define __QUOTE(x) __QUOTE__(x)
#endif // _QUOTE

#if defined(TR_ENABLE_NEWMACROS) || defined(DISABLE_DEBUG_MESSAGES)
#define DbgMsg(...) __noop
#else
#define DbgMsg(fmt, ...) DbgMsgInfo(fmt, __VA_ARGS__);
#endif
void DbgMsgInfo( char* szMessage, ... );

#ifndef DXToFile
#if defined (_NO_USE_D3DXDLL)
#define DXToFile(pSurface) __noop
#else
static inline HRESULT __DXResourceToFile(LPCTSTR fn, IDirect3DSurface9 *pS) { return D3DXSaveSurfaceToFile(fn, D3DXIFF_PNG, pS, NULL, NULL); }
static inline HRESULT __DXResourceToFile(LPCTSTR fn, IDirect3DBaseTexture9 *pTx) { return D3DXSaveTextureToFile(fn, D3DXIFF_PNG, pTx, NULL); }
#define DXToFile(pResource) __DXResourceToFile("C:\\\\" __FILE__ "-" __QUOTE(__LINE__) ".png", pResource)
#endif
#endif	// #ifndef DXToFile

#pragma warning(disable:4786)
#pragma warning(disable:4245)

#include "DispSvr.h"
#include "ObjectCounter.h"
#include <initguid.h>
#include <dxva2api.h>
#include <vector>
#include "Exports/Inc/VideoMixer.h"
#include "Exports/Inc/VideoPresenter.h"
#include "ResourceManager.h"
#include "RegistryService.h"
#include "DynLibManager.h"
#include "MathVideoMixing.h"
#include "ColorSpaceVideoMixing.h"
#include "Imports/LibGPU/GPUID.h"
