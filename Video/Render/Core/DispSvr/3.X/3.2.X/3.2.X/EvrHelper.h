#pragma once

#include <evr9.h>
#include <mferror.h>
#include <mfapi.h>

template <class T>
struct NoOp
{
	void operator()(T& t)
	{
	}
};

template <class T>
class List
{
protected:

	// Nodes in the linked list
	struct Node
	{
		Node *prev;
		Node *next;
		T    item;

		Node() : prev(NULL), next(NULL)
		{
		}

		Node(T item) : prev(NULL), next(NULL)
		{
			this->item = item;
		}

		T Item() const { return item; }
	};

public:

	// Object for enumerating the list.
	class POSITION
	{
		friend class List<T>;

	public:
		POSITION() : pNode(NULL)
		{
		}

		bool operator==(const POSITION &p) const
		{
			return pNode == p.pNode;
		}

		bool operator!=(const POSITION &p) const
		{
			return pNode != p.pNode;
		}

	private:
		const Node *pNode;

		POSITION(Node *p) : pNode(p) 
		{
		}
	};

protected:
	Node    m_anchor;  // Anchor node for the linked list.
	DWORD   m_count;   // Number of items in the list.

	Node* Front() const
	{
		return m_anchor.next;
	}

	Node* Back() const
	{
		return m_anchor.prev;
	}

	virtual HRESULT InsertAfter(T item, Node *pBefore)
	{
		if (pBefore == NULL)
		{
			return E_POINTER;
		}

		Node *pNode = new Node(item);
		if (pNode == NULL)
		{
			return E_OUTOFMEMORY;
		}

		Node *pAfter = pBefore->next;

		pBefore->next = pNode;
		pAfter->prev = pNode;

		pNode->prev = pBefore;
		pNode->next = pAfter;

		m_count++;

		return S_OK;
	}

	virtual HRESULT GetItem(const Node *pNode, T* ppItem)
	{
		if (pNode == NULL || ppItem == NULL)
		{
			return E_POINTER;
		}

		*ppItem = pNode->item;
		return S_OK;
	}

	// RemoveItem:
	// Removes a node and optionally returns the item.
	// ppItem can be NULL.
	virtual HRESULT RemoveItem(Node *pNode, T *ppItem)
	{
		if (pNode == NULL)
		{
			return E_POINTER;
		}

		ASSERT(pNode != &m_anchor); // We should never try to remove the anchor node.
		if (pNode == &m_anchor)
		{
			return E_INVALIDARG;
		}


		T item;

		// The next node's previous is this node's previous.
		pNode->next->prev = pNode->prev;

		// The previous node's next is this node's next.
		pNode->prev->next = pNode->next;

		item = pNode->item;
		delete pNode;

		m_count--;

		if (ppItem)
		{
			*ppItem = item;
		}

		return S_OK;
	}

public:

	List()
	{
		m_anchor.next = &m_anchor;
		m_anchor.prev = &m_anchor;

		m_count = 0;
	}

	virtual ~List()
	{
		Clear();
	}

	// Insertion functions
	HRESULT InsertBack(T item)
	{
		return InsertAfter(item, m_anchor.prev);
	}


	HRESULT InsertFront(T item)
	{
		return InsertAfter(item, &m_anchor);
	}

	// RemoveBack: Removes the tail of the list and returns the value.
	// ppItem can be NULL if you don't want the item back. (But the method does not release the item.)
	HRESULT RemoveBack(T *ppItem)
	{
		if (IsEmpty())
		{
			return E_FAIL;
		}
		else
		{
			return RemoveItem(Back(), ppItem);
		}
	}

	// RemoveFront: Removes the head of the list and returns the value.
	// ppItem can be NULL if you don't want the item back. (But the method does not release the item.)
	HRESULT RemoveFront(T *ppItem)
	{
		if (IsEmpty())
		{
			return E_FAIL;
		}
		else
		{
			return RemoveItem(Front(), ppItem);
		}
	}

	// GetBack: Gets the tail item.
	HRESULT GetBack(T *ppItem)
	{
		if (IsEmpty())
		{
			return E_FAIL;
		}
		else
		{
			return GetItem(Back(), ppItem);
		}
	}

	// GetFront: Gets the front item.
	HRESULT GetFront(T *ppItem)
	{
		if (IsEmpty())
		{
			return E_FAIL;
		}
		else
		{
			return GetItem(Front(), ppItem);
		}
	}


	// GetCount: Returns the number of items in the list.
	DWORD GetCount() const { return m_count; }

	bool IsEmpty() const
	{
		return (GetCount() == 0);
	}

	// Clear: Takes a functor object whose operator()
	// frees the object on the list.
	template <class FN>
	void Clear(FN& clear_fn)
	{
		Node *n = m_anchor.next;

		// Delete the nodes
		while (n != &m_anchor)
		{
			clear_fn(n->item);

			Node *tmp = n->next;
			delete n;
			n = tmp;
		}

		// Reset the anchor to point at itself
		m_anchor.next = &m_anchor;
		m_anchor.prev = &m_anchor;

		m_count = 0;
	}

	// Clear: Clears the list. (Does not delete or release the list items.)
	virtual void Clear()
	{
		Clear<NoOp<T>>(NoOp<T>());
	}


	// Enumerator functions

	POSITION FrontPosition()
	{
		if (IsEmpty())
		{
			return POSITION(NULL);
		}
		else
		{
			return POSITION(Front());
		}
	}

	POSITION EndPosition() const
	{
		return POSITION();
	}

	HRESULT GetItemPos(POSITION pos, T *ppItem)
	{   
		if (pos.pNode)
		{
			return GetItem(pos.pNode, ppItem);
		}
		else 
		{
			return E_FAIL;
		}
	}

	POSITION Next(const POSITION pos)
	{
		if (pos.pNode && (pos.pNode->next != &m_anchor))
		{
			return POSITION(pos.pNode->next);
		}
		else
		{
			return POSITION(NULL);
		}
	}

	// Remove an item at a position. 
	// The item is returns in ppItem, unless ppItem is NULL.
	// NOTE: This method invalidates the POSITION object.
	HRESULT Remove(POSITION& pos, T *ppItem)
	{
		if (pos.pNode)
		{
			// Remove const-ness temporarily...
			Node *pNode = const_cast<Node*>(pos.pNode);

			pos = POSITION();

			return RemoveItem(pNode, ppItem);
		}
		else
		{
			return E_INVALIDARG;
		}
	}

};



// Typical functors for Clear method.

// ComAutoRelease: Releases COM pointers.
// MemDelete: Deletes pointers to new'd memory.

class ComAutoRelease
{
public: 
	void operator()(IUnknown *p)
	{
		if (p)
		{
			int ref = p->Release();
			ASSERT(ref <= 0);
		}
	}
};

class MemDelete
{
public: 
	void operator()(void *p)
	{
		if (p)
		{
			delete p;
		}
	}
};


// ComPtrList class
// Derived class that makes it safer to store COM pointers in the List<> class.
// It automatically AddRef's the pointers that are inserted onto the list
// (unless the insertion method fails). 
//
// T must be a COM interface type. 
// example: ComPtrList<IUnknown>
//
// NULLABLE: If true, client can insert NULL pointers. This means GetItem can
// succeed but return a NULL pointer. By default, the list does not allow NULL
// pointers.

template <class T, bool NULLABLE = FALSE>
class ComPtrList : public List<T*>
{
public:

	typedef T* Ptr;

	void Clear()
	{
		List<Ptr>::Clear(ComAutoRelease());
	}

	~ComPtrList()
	{
		Clear();
	}

protected:
	HRESULT InsertAfter(Ptr item, Node *pBefore)
	{
		// Do not allow NULL item pointers unless NULLABLE is true.
		if (!item && !NULLABLE)
		{
			return E_POINTER;
		}

		if (item)
		{
			item->AddRef();
		}

		HRESULT hr = List<Ptr>::InsertAfter(item, pBefore);
		if (FAILED(hr))
		{
			SAFE_RELEASE(item);
		}
		return hr;
	}

	HRESULT GetItem(const Node *pNode, Ptr* ppItem)
	{
		Ptr pItem = NULL;

		// The base class gives us the pointer without AddRef'ing it.
		// If we return the pointer to the caller, we must AddRef().
		HRESULT hr = List<Ptr>::GetItem(pNode, &pItem);
		if (SUCCEEDED(hr))
		{
			ASSERT(pItem || NULLABLE);
			if (pItem)
			{
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}
		}
		return hr;
	}

	HRESULT RemoveItem(Node *pNode, Ptr *ppItem)
	{
		// ppItem can be NULL, but we need to get the
		// item so that we can release it. 

		// If ppItem is not NULL, we will AddRef it on the way out.

		Ptr pItem = NULL;

		HRESULT hr = List<Ptr>::RemoveItem(pNode, &pItem);

		if (SUCCEEDED(hr))
		{
			ASSERT(pItem || NULLABLE);
			if (ppItem && pItem)
			{
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}

			SAFE_RELEASE(pItem);
		}

		return hr;
	}
};

template<class T>
class CAsyncCallback : public IMFAsyncCallback
{
public: 
	typedef HRESULT (T::*InvokeFn)(IMFAsyncResult *pAsyncResult);

	CAsyncCallback(T *pParent, InvokeFn fn) : m_pParent(pParent), m_pInvokeFn(fn)
	{
	}

	// IUnknown
	STDMETHODIMP_(ULONG) AddRef() { 
		// Delegate to parent class.
		return m_pParent->AddRef(); 
	}
	STDMETHODIMP_(ULONG) Release() { 
		// Delegate to parent class.
		return m_pParent->Release(); 
	}
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		if (!ppv)
		{
			return E_POINTER;
		}
		if (iid == __uuidof(IUnknown))
		{
			*ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
		}
		else if (iid == __uuidof(IMFAsyncCallback))
		{
			*ppv = static_cast<IMFAsyncCallback*>(this);
		}
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}


	// IMFAsyncCallback methods
	STDMETHODIMP GetParameters(DWORD*, DWORD*)
	{
		// Implementation of this method is optional.
		return E_NOTIMPL;
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult)
	{
		return (m_pParent->*m_pInvokeFn)(pAsyncResult);
	}

	T *m_pParent;
	InvokeFn m_pInvokeFn;
};

//-----------------------------------------------------------------------------
// ThreadSafeQueue template
// Thread-safe queue of COM interface pointers.
//
// T: COM interface type.
//
// This class is used by the scheduler. 
//
// Note: This class uses a critical section to protect the state of the queue.
// With a little work, the scheduler could probably use a lock-free queue.
//-----------------------------------------------------------------------------

template <class T>
class ThreadSafeQueue
{
public:
	HRESULT Queue(T *p)
	{
		CAutoLock lock(&m_lock);
		return m_list.InsertBack(p);
	}

	HRESULT Dequeue(T **pp)
	{
		CAutoLock lock(&m_lock);

		if (m_list.IsEmpty())
		{
			*pp = NULL;
			return S_FALSE;
		}

		return m_list.RemoveFront(pp);
	}

	HRESULT PutBack(T *p)
	{
		CAutoLock lock(&m_lock);
		return m_list.InsertFront(p);
	}

	void Clear() 
	{
		CAutoLock lock(&m_lock);
		m_list.Clear();
	}


private:
	CCritSec			m_lock;	
	ComPtrList<T>	m_list;
};

typedef ComPtrList<IMFSample>           CEvrVideoSampleList;

MIDL_INTERFACE("168717B3-297E-427b-8C6B-2A5513AFC6EC") IEvrSchedulerCallback;

interface IEvrSchedulerCallback : IUnknown
{
	STDMETHOD(OnSampleFree)(IMFSample *pSample) = 0;
	STDMETHOD(OnUpdateLastSample)(IMFSample *pSample, LONGLONG llDelta) = 0;
};
// Custom Attributes

// MFSamplePresenter_SampleCounter
// Data type: UINT32
//
// Version number for the video samples. When the presenter increments the version
// number, all samples with the previous version number are stale and should be
// discarded.
// {B91BF44F-F2ED-4265-92D9-D86BABFEF1A4}
static const GUID MFSamplePresenter_SampleCounter = 
{ 0xb91bf44f, 0xf2ed, 0x4265, { 0x92, 0xd9, 0xd8, 0x6b, 0xab, 0xfe, 0xf1, 0xa4 } };


// MFSamplePresenter_DisplayTarget
// Data type: UINT64
//
// Desired target time for frame rate conversion.
// {A49696DB-68B1-4d48-97C6-5880F0B9B75A}
static const GUID MFSamplePresenter_DisplayTargetTime = 
{ 0xa49696db, 0x68b1, 0x4d48, { 0x97, 0xc6, 0x58, 0x80, 0xf0, 0xb9, 0xb7, 0x5a } };

// {5816A9DB-24C6-4501-AC40-FBDEA32EA684}
// Data type: UINT32
//
// Whether the frame is intentionally repeated.
static const GUID MFSamplePresenter_RepeatFrame = 
{ 0x5816a9db, 0x24c6, 0x4501, { 0xac, 0x40, 0xfb, 0xde, 0xa3, 0x2e, 0xa6, 0x84 } };
