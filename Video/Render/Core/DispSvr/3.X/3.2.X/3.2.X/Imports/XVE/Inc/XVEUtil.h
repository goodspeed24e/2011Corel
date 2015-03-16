#ifndef __XVE_UTIL_H__
#define __XVE_UTIL_H__

struct CPROPVARIANT : public PROPVARIANT
{
    CPROPVARIANT(VARENUM VT)
    {
        PropVariantInit(this);
        vt = VT;
    }
};

inline HRESULT XVEGetAttributeUINT32(IXVEAttributes *pType, const GUID &guid, UINT32 &uGet)
{
    CPROPVARIANT var(VT_UI4);
    HRESULT hr = pType->GetItem(guid, &var);
    if (FAILED(hr))
        return hr;
    uGet = var.uintVal;
    return hr;
}

inline HRESULT XVESetAttributeUINT32(IXVEAttributes *pType, const GUID &guid, UINT32 uSet)
{
    CPROPVARIANT var(VT_UI4);
    var.uintVal = uSet;
    HRESULT hr = pType->SetItem(guid, var);
    return hr;
}

inline HRESULT XVEGetAttributeUINT64(IXVEAttributes *pType, const GUID &guid, UINT64 &uGet)
{
    CPROPVARIANT var(VT_UI8);
    HRESULT hr = pType->GetItem(guid, &var);
    if (FAILED(hr))
        return hr;
    uGet = var.uhVal.QuadPart;
    return hr;
}

inline HRESULT XVESetAttributeUINT64(IXVEAttributes *pType, const GUID &guid, UINT64 uSet)
{
    CPROPVARIANT var(VT_UI8);
    var.uhVal.QuadPart = uSet;
    HRESULT hr = pType->SetItem(guid, var);
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Help function for IXVEAttributes
///////////////////////////////////////////////////////////////////////////////
inline HRESULT XVEGetAttributeVertexShaderVersion(IXVEAttributes *pType, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pType, XVE_ATTR_VERTEX_SHADER_VERSION, uGet);
}

inline HRESULT XVESetAttributeVertexShaderVersion(IXVEAttributes *pType, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pType, XVE_ATTR_VERTEX_SHADER_VERSION, uSet);
}

inline HRESULT XVEGetAttributePixelShaderVersion(IXVEAttributes *pType, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pType, XVE_ATTR_PIXEL_SHADER_VERSION, uGet);
}

inline HRESULT XVESetAttributePixelShaderVersion(IXVEAttributes *pType, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pType, XVE_ATTR_PIXEL_SHADER_VERSION, uSet);
}

///////////////////////////////////////////////////////////////////////////////
// Help function for IXVEType
///////////////////////////////////////////////////////////////////////////////
inline HRESULT XVEGetTypeMultipleOutput(IXVEType *pType, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pType, XVE_ATTR_MULTIPLE_OUTPUT, uGet);
}

inline HRESULT XVESetTypeMultipleOutput(IXVEType *pType, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pType, XVE_ATTR_MULTIPLE_OUTPUT, uSet);
}

// The RestrictedContent type is for IXVEFrameResourcePool::RequestResource
inline HRESULT XVEGetTypeRestrictedContent(IXVEType *pType, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pType, XVE_ATTR_EXTENSION_RESTRICTED_CONTENT, uGet);
}

inline HRESULT XVESetTypeRestrictedContent(IXVEType *pType, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pType, XVE_ATTR_EXTENSION_RESTRICTED_CONTENT, uSet);
}

///////////////////////////////////////////////////////////////////////////////
// Help function for IXVESample
///////////////////////////////////////////////////////////////////////////////
inline HRESULT XVEGetSampleViewID(IXVESample *pSample, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_VIEW_ID, uGet);
}

inline HRESULT XVESetSampleViewID(IXVESample *pSample, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_VIEW_ID, uSet);
}

inline HRESULT XVEGetSampleDisplayTimeStamp(IXVESample *pSample, UINT64 &uGet)
{
    return XVEGetAttributeUINT64(pSample, XVE_ATTR_EXTENSION_DISPLAY_TIMESTAMP, uGet);
}

inline HRESULT XVESetSampleDisplayTimeStamp(IXVESample *pSample, const UINT64 &uSet)
{
    return XVESetAttributeUINT64(pSample, XVE_ATTR_EXTENSION_DISPLAY_TIMESTAMP, uSet);
}

inline HRESULT XVEGetSampleRepeatFrame(IXVESample *pSample, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_REPEAT_FRAME, uGet);
}

inline HRESULT XVESetSampleRepeatFrame(IXVESample *pSample, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_REPEAT_FRAME, uSet);
}

inline HRESULT XVEGetSampleRestrictedContent(IXVESample *pSample, UINT32 &uGet)
{
    return XVEGetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_RESTRICTED_CONTENT, uGet);
}

inline HRESULT XVESetSampleRestrictedContent(IXVESample *pSample, const UINT32 &uSet)
{
    return XVESetAttributeUINT32(pSample, XVE_ATTR_EXTENSION_RESTRICTED_CONTENT, uSet);
}

#endif  // __XVE_UTIL_H__