#ifndef __XVE_H__
#define __XVE_H__

MIDL_INTERFACE("31F83F41-4734-4ce3-A7FB-1F15D12AB4F8") IXVEAttributes;
MIDL_INTERFACE("323A860A-A516-43e3-B89F-155F915FACD7") IXVESample;
MIDL_INTERFACE("34A8605C-122F-4fb4-8AA4-691CC882C4FD") IXVEType;
MIDL_INTERFACE("3667EEC8-5C62-4a25-9F4D-8D2B27EE8697") IXVideoEffect;
MIDL_INTERFACE("36694AB9-F360-4092-8BDD-831840488C71") IXVideoEffectManager;
MIDL_INTERFACE("37063C85-2F0D-4c3e-812C-E2DAB17CA517") IXVEManagerNotify;
MIDL_INTERFACE("3710DC71-DEFD-427e-A01C-77B31305DD2A") IXVEFrameResourcePool;

namespace XVE
{
    enum XVE_MESSAGE_TYPE
    {
        XVE_MESSAGE_INITIALIZE                    = 0,
        XVE_MESSAGE_UNINITIALIZE,
        XVE_MESSAGE_SETDEVICE,
        XVE_MESSAGE_RELEASEDEVICE,
        XVE_MESSAGE_SETMANAGER,
        XVE_MESSAGE_RELEASEMANAGER,
    };

    enum XVE_EVENT_TYPE
    {
        XVE_EVENT_EFFECTENABLE                  = 0,
    };

    enum XVE_FORMAT_TYPE
    {
        XVE_FORMAT_UNKNOWN                     = 0,
        XVE_FORMAT_TEXTURE                        = 1,
        XVE_FORMAT_SURFACE,
    };

    enum XVE_DEVICE_TYPE
    {
        XVE_DEVICE_UNKNOWN                       = 0x0,
        XVE_DEVICE_D3D9                               = 0x1,
        XVE_DEVICE_D3D10                             = 0x2,
        XVE_DEVICE_D3D11                             = 0x4,
    };

    enum XVE_INTERLACED_MODE
    {
        XVE_INTERLACEDMODE_UNKNOWN                                  = 0,
        XVE_INTERLACEDMODE_PROGRESSIVE                             = 1,
        XVE_INTERLACEDMODE_INTERLEAVED_UPPER_FIRST,
        XVE_INTERLACEDMODE_INTERLEAVED_LOWER_FIRST,
    };

    enum XVE_FIELD_SELECT_MODE
    {
        XVE_FIELDSELECT_UNKNOWN                  = 0,
        XVE_FIELDSELECT_FIRST                          = 1,
        XVE_FIELDSELECT_SECOND,
        XVE_FIELDSELECT_REPEAT_FIRST,
    };
}

interface IXVEAttributes : public IUnknown
{
    STDMETHOD(SetItem)(REFGUID rguidKey, REFPROPVARIANT  varValue) = 0;
    STDMETHOD(GetItem)(REFGUID rguidKey, PROPVARIANT  *pvarValue) = 0;
    STDMETHOD(CopyAllItems)(IXVEAttributes *pDestAttributes) = 0;
    STDMETHOD(DeleteItem)(REFGUID rguidKey) = 0;
    STDMETHOD(DeleteAllItems)() = 0;
    STDMETHOD(GetItemCount)(UINT32*  puItemCount) = 0;
    STDMETHOD(GetItemByIndex)(UINT32 uItemIndex, GUID *pGUID, PROPVARIANT  *pvarValue) = 0;
    STDMETHOD(LockStore)() = 0;
    STDMETHOD(UnLockStore)() = 0;
};

interface IXVESample : public IXVEAttributes
{
    STDMETHOD(GetFrame)(IUnknown **ppUnk) = 0;
    STDMETHOD(GetFrameSize)(UINT *puWidth, UINT *puHeight) = 0;
    STDMETHOD(GetAspectRatio)(UINT *puNumerator, UINT *puDenominator) = 0;
    STDMETHOD(GetFrameFormat)(DWORD *pdwFormat) = 0;
    STDMETHOD(GetFrameDuration)(LONGLONG *pllDuration) = 0;
    STDMETHOD(GetFrameTimeStamp)(LONGLONG *pllTimeStamp) = 0;
    STDMETHOD(GetInterlacedMode)(XVE::XVE_INTERLACED_MODE *pdwMode) = 0;
    STDMETHOD(GetFieldSelectMode)(XVE::XVE_FIELD_SELECT_MODE *pdwMode) = 0;
    STDMETHOD(GetFrameType)(XVE::XVE_FORMAT_TYPE *pdwType) = 0;
    STDMETHOD(GetPastFrame)(IUnknown ***pppUnk) = 0;
    STDMETHOD(GetPastFrameCount)(UINT *puCount) = 0;
    STDMETHOD(GetFutureFrame)(IUnknown ***pppUnk) = 0;
    STDMETHOD(GetFutureFrameCount)(UINT *puCount) = 0;
    STDMETHOD(GetDisplaySize)(UINT *puWidth, UINT *puHeight) = 0;

    STDMETHOD(SetFrame)(IUnknown *pUnk) = 0;
    STDMETHOD(SetFrameSize)(UINT uWidth, UINT uHeight) = 0;
    STDMETHOD(SetAspectRatio)(UINT uNumerator, UINT uDenominator) = 0;
    STDMETHOD(SetFrameFormat)(DWORD dwFormat) = 0;
    STDMETHOD(SetFrameDuration)(LONGLONG llDuration) = 0;
    STDMETHOD(SetFrameTimeStamp)(LONGLONG llTimeStamp) = 0;
    STDMETHOD(SetInterlacedMode)(XVE::XVE_INTERLACED_MODE dwMode) = 0;
    STDMETHOD(SetFieldSelectMode)(XVE::XVE_FIELD_SELECT_MODE dwMode) = 0;
    STDMETHOD(SetFrameType)(XVE::XVE_FORMAT_TYPE dwType) = 0;
    STDMETHOD(SetPastFrame)(IUnknown **ppUnk) = 0;
    STDMETHOD(SetPastFrameCount)(UINT uCount) = 0;
    STDMETHOD(SetFutureFrame)(IUnknown **ppUnk) = 0;
    STDMETHOD(SetFutureFrameCount)(UINT uCount) = 0;
    STDMETHOD(SetDisplaySize)(UINT uWidth, UINT uHeight) = 0;
};

interface IXVEType : public IXVEAttributes
{
    STDMETHOD(GetFrameSize)(UINT *puWidth, UINT *puHeight) = 0;
    STDMETHOD(GetAspectRatio)(UINT *puNumerator, UINT *puDenominator) = 0;
    STDMETHOD(GetFrameFormat)(DWORD *pdwFormat) = 0;
    STDMETHOD(GetFrameType)(XVE::XVE_FORMAT_TYPE *pdwType) = 0;
    	
    STDMETHOD(SetFrameSize)(UINT uWidth, UINT uHeight) = 0;
    STDMETHOD(SetAspectRatio)(UINT uNumerator, UINT uDenominator) = 0;
    STDMETHOD(SetFrameFormat)(DWORD dwFormat) = 0;
    STDMETHOD(SetFrameType)(XVE::XVE_FORMAT_TYPE dwType) = 0;
};

interface IXVideoEffect : public IXVEAttributes
{
    STDMETHOD(GetResourceID)(GUID *pGUID) = 0;
    STDMETHOD(GetSupportedDevice)(DWORD *dwSupportedDevice) = 0;
    STDMETHOD(Enable)(BOOL bEnable) = 0;
    STDMETHOD(IsEnabled)(BOOL *pbEnabled) = 0;

    STDMETHOD(GetInputAvailableType)(DWORD dwTypeIndex, IXVEType **ppType) = 0;
    STDMETHOD(GetInputCurrentType)(IXVEType **ppType) = 0;
    STDMETHOD(GetOutputAvailableType)(DWORD dwTypeIndex, IXVEType **ppType) = 0;
    STDMETHOD(GetOutputCurrentType)(IXVEType **ppType) = 0;

    STDMETHOD(SetInputType)(IXVEType *pType, DWORD dwFlags) = 0;
    STDMETHOD(SetOutputType)(IXVEType *pType, DWORD dwFlags) = 0;

    STDMETHOD(ProcessEffect)(IXVESample *pInputSample, IXVESample *pOutputSample) = 0;
    STDMETHOD(ProcessMessage)(XVE::XVE_MESSAGE_TYPE dwMessage, ULONGLONG ullParams, ULONGLONG ullParamsEx) = 0;

    STDMETHOD(CreateEmptyContainer)(REFIID riid, VOID **ppNewContainer) = 0;
};

interface IXVideoEffectManager : public IXVEAttributes
{
    STDMETHOD(GetResourceID)(GUID *pGUID) = 0;
    STDMETHOD(GetSupportedDevice)(XVE::XVE_DEVICE_TYPE *dwSupportedDevice) = 0;

    STDMETHOD(IsEnabled)(DWORD dwStreamID, BOOL *pbEnabled) = 0;

    STDMETHOD(GetInputAvailableType)(DWORD dwStreamID, DWORD dwTypeIndex, IXVEType **ppType) = 0;
    STDMETHOD(GetInputCurrentType)(DWORD dwStreamID, IXVEType **ppType) = 0;
    STDMETHOD(GetOutputAvailableType)(DWORD dwStreamID, DWORD dwTypeIndex, IXVEType **ppType) = 0;
    STDMETHOD(GetOutputCurrentType)(DWORD dwStreamID, IXVEType **ppType) = 0;

    STDMETHOD(SetInputType)(DWORD dwStreamID, IXVEType *pType, DWORD dwFlags) = 0;
    STDMETHOD(SetOutputType)(DWORD dwStreamID, IXVEType *pType, DWORD dwFlags) = 0;
	
    STDMETHOD(InstallEffect)(DWORD dwStreamID, IXVideoEffect *pEffect) = 0;
    STDMETHOD(UninstallEffect)(DWORD dwStreamID, IXVideoEffect *pEffect) = 0;

    STDMETHOD(GetEffectCount)(DWORD dwStreamID, UINT *puCount) = 0;
    STDMETHOD(GetAvailableEffectResourceID)(UINT *puCount, GUID **ppAvailableEffectID) = 0;
    STDMETHOD(GetEffectByResourceID)(DWORD dwStreamID, REFGUID rguidEffectID, IXVideoEffect **ppEffect) = 0;

    STDMETHOD(AddEffectToStream)(DWORD dwStreamID, REFGUID rguidEffectID, DWORD dwFlags,  IXVideoEffect **ppEffect) = 0;
    STDMETHOD(RemoveEffectFromStream)(DWORD dwStreamID, REFGUID rguidEffectID) = 0;

    STDMETHOD(GetEffectAt)(DWORD dwStreamID, UINT uIndex, IXVideoEffect **ppEffect) = 0;
    STDMETHOD(GetEffectIndex)(DWORD dwStreamID, REFGUID rguidEffectID, UINT *puIndex) = 0;

    STDMETHOD(MoveEffectTo)(DWORD dwStreamID, UINT uIndex, REFGUID rguidEffectID) = 0;
    STDMETHOD(SwapEffectLocation)(DWORD dwStreamID, REFGUID rguidFirstEffectID, REFGUID rguidSecondEffectID) = 0;

    STDMETHOD(ProcessEvent)(REFGUID rguidEffectID, IXVideoEffect *pInstance, XVE::XVE_EVENT_TYPE dwEvent, DWORD dwParams) = 0;
    STDMETHOD(ProcessEffect)(DWORD dwStreamID, IXVESample *pInputSample, IXVESample *pOutputSample) = 0;
    STDMETHOD(ProcessMessage)(XVE::XVE_MESSAGE_TYPE dwMessage, ULONGLONG ullParams, ULONGLONG ullParamsEx) = 0;
    	    	
    STDMETHOD(CreateEmptyContainer)(REFIID riid, VOID **ppNewContainer) = 0;

    STDMETHOD(AdviseEventNotify)(IXVEManagerNotify *pEventNotify) = 0;
    STDMETHOD(UnAdviseEventNotify)(IXVEManagerNotify *pEventNotify) = 0;

    STDMETHOD(GetService)(REFIID riid, VOID **ppService) = 0;

    STDMETHOD(EnableEffect)(DWORD dwStreamID, REFGUID rguidEffectID, BOOL bEnable) = 0;
    STDMETHOD(IsEffectEnabled)(DWORD dwStreamID, REFGUID rguidEffectID, BOOL *pbEnabled) = 0;
    STDMETHOD(SetEffectItem)(DWORD dwStreamID, REFGUID rguidEffectID, REFGUID rguidKey, REFPROPVARIANT  varValue) = 0;
    STDMETHOD(GetEffectItem)(DWORD dwStreamID, REFGUID rguidEffectID, REFGUID rguidKey, PROPVARIANT  *pvarValue) = 0;
};

interface IXVEManagerNotify : IUnknown
{
    STDMETHOD(OnNotify)(DWORD dwStreamID, REFGUID rguidEffectID, XVE::XVE_EVENT_TYPE dwEvent, DWORD dwParams) = 0;
};

interface IXVEFrameResourcePool : IUnknown
{
    STDMETHOD(RequestResource)(IXVEType *pRequestType, IUnknown **ppResource, IXVideoEffect *pRequester) = 0;
    STDMETHOD(ReturnResource)(IUnknown *pResource, DWORD dwFlags, IXVideoEffect *pRequester) = 0;
};

typedef HRESULT (__cdecl *TpfnCreateXVideoEffect)(IXVideoEffect **ppXVideoEffect);
typedef HRESULT (__cdecl *TpfnCreateXVideoEffectManager)(DWORD dwFlags, IXVideoEffectManager** ppXVideoEffectManager);

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) HRESULT __cdecl CreateXVideoEffect(IXVideoEffect **ppXVideoEffect);
    __declspec(dllexport) HRESULT __cdecl CreateXVideoEffectManager(DWORD dwFlags, IXVideoEffectManager** ppXVideoEffectManager);
#ifdef __cplusplus
}	// extern "C"
#endif

#endif