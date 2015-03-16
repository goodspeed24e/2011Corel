#ifndef _DISPSVR_RENDER_CLOCK_
#define _DISPSVR_RENDER_CLOCK_

#include "Singleton.h"

#ifndef RENDER_CLOCK_EXPIRATION_DURATION
#define RENDER_CLOCK_EXPIRATION_DURATION	150
#endif

namespace DispSvr
{
	class CRenderClock : public Singleton<CRenderClock>
	{
	public:
		CRenderClock()
			: m_dwLastTime(0), m_pOwner(0), ACTIVE_DURATION(RENDER_CLOCK_EXPIRATION_DURATION) {}

		bool Update(const void *pObject)
		{
			CAutoLock lock(&csLock);
			if (m_pOwner == 0 || m_pOwner == pObject)
			{
				m_pOwner = pObject;
				m_dwLastTime = timeGetTime();
				return true;
			}
			return false;
		}

		bool Expired()
		{
			CAutoLock lock(&csLock);
			if (m_dwLastTime > 0 && timeGetTime() - m_dwLastTime < ACTIVE_DURATION)
				return false;

			m_dwLastTime = 0;
			m_pOwner = 0;
			return true;
		}

	protected:
		DWORD m_dwLastTime;
		const void *m_pOwner;
		CCritSec csLock;
		const DWORD ACTIVE_DURATION;
	};
}


#endif // _DISPSVR_RENDER_CLOCK_