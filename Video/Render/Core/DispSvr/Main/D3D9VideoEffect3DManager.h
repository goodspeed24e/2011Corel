#ifndef _DISPSVR_D3D9_VIDEO_EFFECT3D_MANAGER_H_
#define _DISPSVR_D3D9_VIDEO_EFFECT3D_MANAGER_H_

#include <vector>

#include "Exports/Inc/VideoEffect3D.h"
#include "D3D9VideoMixerBase.h"

struct ProcessEffectRequest
{
	IUnknown *pInput;
	RECT *lpSourceRect;
	const RECT *lpTargetRect;
	const DispSvr::VideoProperty *pVideoProperty;
	IUnknown **ppOutput;
};

MIDL_INTERFACE("85D701C4-0866-4d9a-A67E-AD986B55B3B9") ID3D9VideoEffect3DProcessor : IUnknown
{
	/// Get enabled effect count. ProcessEffect should be called when it returns non-zero.
	STDMETHOD_(BOOL, IsEffectEnabled)() = 0;

	/// ProcessEffect sets valid surface to output buffer when succeeded.
	/// Any plugin failure will break the processing pipeline and return the failure code.
	STDMETHOD(ProcessEffect)(ProcessEffectRequest *pRequest) = 0;
};

namespace DispSvr
{

	class CD3D9VideoEffect3DManager :
		public virtual CD3D9PluginBase,
		public virtual IDispSvrVideoEffect3DManager,
		public virtual ID3D9VideoEffect3DProcessor
	{
	public:
		CD3D9VideoEffect3DManager();
		virtual ~CD3D9VideoEffect3DManager();

		// IUnknown
		STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
		STDMETHOD_(ULONG, AddRef)()	{ return CD3D9PluginBase::AddRef();	}
		STDMETHOD_(ULONG, Release)() { return CD3D9PluginBase::Release(); }

		// IDispSvrPlugin
		STDMETHOD(ProcessMessage)(RESOURCE_MESSAGE_TYPE msg, LPVOID ulParam);

		// IDispSvrVideoEffect3DManager
		STDMETHOD(Install)(IDispSvrVideoEffect3DPlugin *pPlugin);
		STDMETHOD(Uninstall)(IDispSvrVideoEffect3DPlugin *pPlugin);
		STDMETHOD(Enable)(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL bEnable);
		STDMETHOD(GetEffectCount)(UINT *puCount);
		STDMETHOD(IsEnabled)(const IDispSvrVideoEffect3DPlugin *pPlugin, BOOL *pbEnable);
		STDMETHOD(GetEffectAt)(UINT uIndex, IDispSvrVideoEffect3DPlugin **ppPlugin);
		STDMETHOD(GetCaps)(VE3DManagerCaps *pCaps);
		STDMETHOD(SetProperty)(const VE3DManagerProperty *pProperty);
		STDMETHOD(GetProperty)(VE3DManagerProperty *pProperty);
        STDMETHOD(GetAvailableEffectGUID)(UINT *puCount, GUID **ppAvailabeEffectGUID);
        STDMETHOD(GetEffectByGUID)(const GUID *pGUID, IDispSvrVideoEffect3DPlugin **ppPlugin);

		// IDispSvrVideoEffect3DBlt
		STDMETHOD_(BOOL, IsEffectEnabled)();
		STDMETHOD(ProcessEffect)(ProcessEffectRequest *pRequest);

	protected:
		struct Effect
		{
			BOOL bEnable;
			IDispSvrVideoEffect3DPlugin *pInstance;

			bool operator== (const Effect &r) const { return r.pInstance == pInstance; }
			bool operator== (const IDispSvrVideoEffect3DPlugin *p) const { return p == pInstance; }
		};
		typedef std::vector<Effect> EffectList;

	protected:
		CCritSec m_csObj;
		EffectList m_listEffect;
		UINT m_uEnabledCount;
		VE3DManagerProperty m_Property;
	};

}	// namespace DispSvr

#endif	// _DISPSVR_D3D9_VIDEO_EFFECT3D_MANAGER_H_