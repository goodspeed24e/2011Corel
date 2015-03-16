#ifndef IVI_SmartCoTaskPtr_H
#define IVI_SmartCoTaskPtr_H

#include	<ObjBase.h>
#include	"../../../ThirdParty/loki/include/loki/SmartPtr.h"

namespace IVI
{
	template <class T>
	struct CoTaskMemStorage
	{
	public:
		typedef T* StoredType;		// the type of the pointee_ object
		typedef T* InitPointerType;	// type used to declare OwnershipPolicy type.
		typedef T* PointerType;		// type returned by operator->
		typedef T& ReferenceType;	// type returned by operator*
		typedef T** PointerToPointerType;

		CoTaskMemStorage() : pointee_(Default()) 
		{}

		// The storage policy doesn't initialize the stored pointer 
		//     which will be initialized by the OwnershipPolicy's Clone fn
		CoTaskMemStorage(const CoTaskMemStorage&) : pointee_(0)
		{}

		template <class U>
		CoTaskMemStorage(const CoTaskMemStorage<U>&) : pointee_(0)
		{}

		CoTaskMemStorage(const StoredType& p) : pointee_(p) {}

		PointerType operator->() const { return pointee_; }

		ReferenceType operator*() const { return *pointee_; }

		PointerToPointerType operator&() {
			assert(!pointee_ || "This pointer should be NULL! There might be a bug here!");
			return &pointee_;
		}

		void Swap(CoTaskMemStorage& rhs)
		{ std::swap(pointee_, rhs.pointee_); }

		// Accessors
		friend inline PointerType GetImpl(const CoTaskMemStorage& sp)
		{ return sp.pointee_; }

		friend inline const StoredType& GetImplRef(const CoTaskMemStorage& sp)
		{ return sp.pointee_; }

		friend inline StoredType& GetImplRef(CoTaskMemStorage& sp)
		{ return sp.pointee_; }

	protected:
		// Destroys the data stored
		// (Destruction might be taken over by the OwnershipPolicy)
		void Destroy()
		{
			::CoTaskMemFree(pointee_);
		}

		// Default value to initialize the pointer
		static StoredType Default()
		{ return 0; }

	private:
		// Data
		StoredType pointee_;
	};

	typedef Loki::SmartPtr< WCHAR, Loki::RefCounted, Loki::AllowConversion,	Loki::AssertCheck, CoTaskMemStorage, Loki::DontPropagateConst >
		SmartCoTaskPtr;

}
#endif // IVI_SmartCoTaskPtr_H
