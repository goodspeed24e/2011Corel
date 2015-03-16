#ifndef _DISPSVR_SINGLETON_H_
#define _DISPSVR_SINGLETON_H_

namespace DispSvr
{
	template<typename T>
	class Singleton
	{
	public:
		Singleton()
		{
			ASSERT(!ms_pInstance);
			ms_pInstance = static_cast<T *> (this);
		}

		~Singleton()
		{
			ASSERT(ms_pInstance);
			ms_pInstance = 0;
		}

		static T *GetInstance()
		{
			ASSERT(ms_pInstance);
			return ms_pInstance;
		}
	protected:
		static T *ms_pInstance;
	};

	template<typename T> T *Singleton<T>::ms_pInstance = 0;
}

#endif	// _DISPSVR_SINGLETON_H_