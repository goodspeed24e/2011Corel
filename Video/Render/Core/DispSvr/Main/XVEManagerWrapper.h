#ifndef _DISPSVR_XVEMANAGER_WRAPPER_H_
#define _DISPSVR_XVEMANAGER_WRAPPER_H_

//#include "Imports/xve/Inc/XVE.h"
interface IXVideoEffectManager;
MIDL_INTERFACE("CEFCD8F9-EF5D-4e88-B325-71F4B21E87D3") IDispSvrXVEManagerWrapper : public IUnknown
{
    STDMETHOD(SetXVEManager)(IXVideoEffectManager *pManager) = 0;
    STDMETHOD(GetXVEManager)(IXVideoEffectManager **ppManager) = 0;
};

namespace DispSvr
{
    /// A basic implementation for D3D9 plug-in.
    class CDispSvrXVEManagerWrapper : 
        public CD3D9PluginBase,
        public IDispSvrXVEManagerWrapper
    {
    public:
        CDispSvrXVEManagerWrapper();
        ~CDispSvrXVEManagerWrapper();

        // IUnkonwn
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)() { return CD3D9PluginBase::AddRef(); }
        STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

        // IDispSvrPlugin
        STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

        //IDispSvrXVEManagerWrapper
        STDMETHOD(SetXVEManager)(IXVideoEffectManager *pManager);
        STDMETHOD(GetXVEManager)(IXVideoEffectManager **ppManager);

    protected:
        HRESULT _InitManager();
        HRESULT _ReleaseManager();
        HRESULT _SetDevice(LPVOID ulParam);
        HRESULT _ReleaseDevice(LPVOID ulParam);
        HRESULT _Terminate(LPVOID ulParam);

    protected:
        BOOL m_bInitialized;
        IXVideoEffectManager *m_pXVEManager;
    };
}
#endif