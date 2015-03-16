#ifndef __GMOBASE_H
#define __GMOBASE_H

#include <windows.h>
#include <crtdbg.h>

#ifdef CheckPointer
#undef CheckPointer
#endif
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

class IGMOUnknown
{
public:
	virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **)= 0;
	virtual STDMETHODIMP_(ULONG) NonDelegatingAddRef()= 0;
	virtual STDMETHODIMP_(ULONG)NonDelegatingRelease()= 0;
};

#ifdef _WIN32
#pragma warning( disable : 4355)
#endif

class CGMOUnknown: public IGMOUnknown{
protected:
	volatile LONG m_cRef;
private:
	const LPUNKNOWN m_pUnknown; /* Owner of this object */
public:
	CGMOUnknown(IUnknown* pUnk):m_cRef(0), m_pUnknown( pUnk != 0 ? pUnk : reinterpret_cast<IUnknown*>( static_cast<IGMOUnknown*>(this) ) ){}
	virtual ~CGMOUnknown(){}
	
	virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		CheckPointer(ppv, E_POINTER);
		
		if (riid == IID_IUnknown) {
			return GetInterface((IUnknown*) (IGMOUnknown*) this, ppv);
		} 
		else 
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
	}
	virtual STDMETHODIMP_(ULONG) NonDelegatingAddRef()
	{
		LONG lRef = 0;
		lRef = InterlockedIncrement( const_cast<LONG*>(&m_cRef) );
		_ASSERT(lRef > 0);
		return m_cRef;
	}
	virtual STDMETHODIMP_(ULONG) NonDelegatingRelease()
	{
		LONG lRef = InterlockedDecrement( const_cast<LONG*>(&m_cRef) );
		_ASSERT(lRef >= 0);
		
		if (lRef == 0) {
			delete this;
			return ULONG(0);
		} else {
			return m_cRef;
		}				
	}
	LPUNKNOWN GetOwner() const {
		return m_pUnknown;
	};
	HRESULT STDMETHODCALLTYPE GetInterface(LPUNKNOWN pUnk, void **ppv)
	{
		CheckPointer(ppv, E_POINTER);
		*ppv = pUnk;
		pUnk->AddRef();
		return NOERROR;
	}
};

#ifdef _WIN32
#pragma warning( default : 4355)
#endif

#ifndef DECLARE_IUNKNOWN
#define DECLARE_IUNKNOWN									\
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)	\
{return GetOwner()->QueryInterface(riid, ppv);}		\
	STDMETHODIMP_(ULONG) AddRef()							\
{return GetOwner()->AddRef();}							\
	STDMETHODIMP_(ULONG) Release()							\
{return GetOwner()->Release();}						
#endif //DECLARE_IUNKNOWN

#ifndef IMPLEMENT_MAKE
#define IMPLEMENT_MAKE(class_name)			\
	static HRESULT STDMETHODCALLTYPE Make(class_name** pp)		\
{										\
	IUnknown* pTemp= reinterpret_cast<IUnknown*>(*pp);	\
	if(pTemp)							\
{									\
	*pp= 0;							\
	pTemp->Release();				\
}									\
	*pp= new class_name;					\
	if(!(*pp))							\
	return E_FAIL;					\
	(*pp)->AddRef();						\
	return S_OK;						\
}
#endif

#define IMPLEMENT_CREATEINSTANCE(class_name, OuterUnknown)			\
	static HRESULT STDMETHODCALLTYPE CreateInstance(class_name** pp, IUnknown* pUnkOuter= OuterUnknown)		\
{										\
	IUnknown* pTemp= reinterpret_cast<IUnknown*>(*pp);	\
	if(pTemp)							\
{									\
	*pp= 0;							\
	pTemp->Release();				\
}									\
	*pp= new class_name(pUnkOuter);					\
	if(!(*pp))							\
	return E_FAIL;					\
	(*pp)->AddRef();						\
	return S_OK;						\
}
template <class T>
class _NoAddRefReleaseOnCGMOPtr : public T
{
	private:
		STDMETHOD_(ULONG, AddRef)()=0;
		STDMETHOD_(ULONG, Release)()=0;
};


template<class T>
class CGMOPtr
{
public:
	typedef T _PtrClass;
	CGMOPtr()
	{
		p=NULL;
	}
	CGMOPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CGMOPtr(const CGMOPtr<T>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	~CGMOPtr()
	{
		if (p)
			p->Release();
	}
	void Release()
	{
		IUnknown* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	operator T*() const
	{
		return (T*)p;
	}
	T& operator*() const
	{
		_ASSERT(p!=NULL);
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&()
	{
		_ASSERT(p==NULL);
		return &p;
	}
	_NoAddRefReleaseOnCGMOPtr<T>* operator->() const
	{
		_ASSERT(p!=NULL);
		return (_NoAddRefReleaseOnCGMOPtr<T>*)p;
	}
	T* operator=(T* lp)
	{
		if (lp != NULL)
			lp->AddRef();

		if (p)
			p->Release();

		p = lp;
		return (T*)lp;
	}
	T* operator=(const CGMOPtr<T>& lp)
	{
		if (lp.p != NULL)
			(lp.p)->AddRef();

		if (p)
			p->Release();

		p = lp.p;
		return (T*)lp;
	}
	bool operator!() const
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const
	{
		return p < pT;
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}
	void Attach(T* p2)
	{
		if (p)
			p->Release();
		p = p2;
	}
	T* Detach()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	HRESULT CopyTo(T** ppT)
	{
		_ASSERT(ppT != NULL);
		if (ppT == NULL)
			return E_POINTER;
		*ppT = p;
		if (p)
			p->AddRef();
		return S_OK;
	}
	T* p;
};
#endif
