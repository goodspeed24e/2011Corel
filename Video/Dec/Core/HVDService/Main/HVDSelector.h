#ifndef _HVD_SELECTOR_H_
#define _HVD_SELECTOR_H_

#include "Exports\Inc\HVDService.h"

namespace HVDService
{
	class CHVDSelector
	{
	public:
		static HRESULT GetHVDGuids(DWORD dwService, DWORD dwMode, DWORD dwLevel, const GUID** ppGuids, DWORD *pdwCount);
		static HRESULT RecommendSurfaceCount(DWORD dwService, DWORD dwMode, DWORD dwLevel, DWORD *pdwMax, DWORD *pdwMin, DWORD dwWidth, DWORD dwHeight);

	private:
		static const GUID m_sOthers[];
		// DXVA2 MEG2 GUIDs
		static const GUID m_sDxva2MPEG2MC[];
		static const GUID m_sDxva2MPEG2IDCT[];
		static const GUID m_sDxva2MPEG2VLD[];
		// DXVA2 VC1 GUIDs
		static const GUID m_sDxva2VC1MC[];
		static const GUID m_sDxva2VC1IDCT[];
		static const GUID m_sDxva2VC1VLD[];
		// DXVA2 H264 GUIDs
		static const GUID m_sDxva2H264MC[];
		static const GUID m_sDxva2H264IDCT[];
		static const GUID m_sDxva2H264VLD[];
        static const GUID m_sDxva2H264MVCVLD[];

		// DXVA2 MPEG4 GUIDs
		static const GUID m_sDxva2MP4VLD[];
/*
		// DXVA MPEG2 GUIDs
		static const GUID m_sDxvaMPEG2MC[];
		static const GUID m_sDxvaMPEG2IDCT[];
		static const GUID m_sDxvaMPEG2VLD[];
		// DXVA VC1 GUIDs
		static const GUID m_sDxvaVC1MC[];
		static const GUID m_sDxvaVC1IDCT[];
		static const GUID m_sDxvaVC1VLD[];
		// DXVA H264 GUIDs
		static const GUID m_sDxvaH264MC[];
		static const GUID m_sDxvaH264IDCT[];
		static const GUID m_sDxvaH264VLD[];

		static const GUID* m_sDxvaMPEG2[];
		static const GUID* m_sDxvaVC1[];
		static const GUID* m_sDxvaH264[];
		static const GUID** m_sDxva[];
*/
	};
}

#endif
