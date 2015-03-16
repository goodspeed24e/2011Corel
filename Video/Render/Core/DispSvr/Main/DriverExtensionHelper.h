#ifndef __DISPSVR_DRIVER_EXTENSION_HELPER_H__
#define __DISPSVR_DRIVER_EXTENSION_HELPER_H__

#include "Exports/Inc/VideoPresenter.h"

namespace DispSvr
{
	/// Adapter interface to convert given device to extension query interface.
	interface IDriverExtensionAdapter
	{
		virtual ~IDriverExtensionAdapter() { }
		virtual HRESULT QueryPresenterCaps(DWORD VideoDecodeCaps, PresenterCaps* pCaps) = 0;
	};

	/// Helper class for driver extensions.
	class CDriverExtensionHelper
	{
	public:
		static HRESULT GetAdapter(IDirect3DDevice9 *pDevice, IDriverExtensionAdapter **ppAdapter);
	};
}

#endif	// __DISPSVR_DRIVER_EXTENSION_HELPER_H__