#ifndef IVI_SmartGMediatype_H
#define IVI_SmartGMediatype_H

#include	"loki/SmartPtr.h"
#include	"GMO/GMOHelper.h"

namespace IVI
{
	template <class T>
	struct GMediatypeStorage
	{
	public:
		typedef T* StoredType;		// the type of the pointee_ object
		typedef T* InitPointerType;	// type used to declare OwnershipPolicy type.
		typedef T* PointerType;		// type returned by operator->
		typedef T& ReferenceType;	// type returned by operator*
		typedef T** PointerToPointerType;

		GMediatypeStorage() : pointee_(Default()) 
		{}

		// The storage policy doesn't initialize the stored pointer 
		//     which will be initialized by the OwnershipPolicy's Clone fn
		GMediatypeStorage(const GMediatypeStorage&) : pointee_(0)
		{}

		template <class U>
		GMediatypeStorage(const GMediatypeStorage<U>&) : pointee_(0)
		{}

		GMediatypeStorage(const StoredType& p) : pointee_(p) {}

		PointerType operator->() const { return pointee_; }

		ReferenceType operator*() const { return *pointee_; }

		PointerToPointerType operator&() {
			assert(!pointee_ || "This pointer should be NULL! There might be a bug here!");
			return &pointee_;
		}

		void Swap(GMediatypeStorage& rhs)
		{ std::swap(pointee_, rhs.pointee_); }

		// Accessors
		friend inline PointerType GetImpl(const GMediatypeStorage& sp)
		{ return sp.pointee_; }

		friend inline const StoredType& GetImplRef(const GMediatypeStorage& sp)
		{ return sp.pointee_; }

		friend inline StoredType& GetImplRef(GMediatypeStorage& sp)
		{ return sp.pointee_; }

	protected:
		// Destroys the data stored
		// (Destruction might be taken over by the OwnershipPolicy)
		void Destroy()
		{
			GMediaObjectHelper::MoFreeMediaType(pointee_);
			delete pointee_;
		}

		// Default value to initialize the pointer
		static StoredType Default()
		{ return 0; }

	private:
		// Data
		StoredType pointee_;
	};

	typedef Loki::SmartPtr< GMO_MEDIA_TYPE, Loki::RefCounted, Loki::AllowConversion, Loki::AssertCheck, GMediatypeStorage, Loki::DontPropagateConst >
		SmartGMediatype;
	// Use SmartGMediatype as a GMO_Media_Type* and you don't need to worry about the resource management ( it's a reference counted smart pointer )

}
#endif // IVI_SmartGMediatype_H
