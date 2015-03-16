#ifndef IVI_SmartModule_H
#define IVI_SmartModule_H

#include	<windows.h>
#include	"loki/SmartPtr.h"

namespace IVI
{
	template <class T>
	struct HModuleStorage
	{
	public:
		typedef T StoredType;		// the type of the pointee_ object
		typedef T InitPointerType;	// type used to declare OwnershipPolicy type.
		typedef T PointerType;		// type returned by operator->
		typedef T ReferenceType;	// type returned by operator*

		HModuleStorage() : pointee_(Default()) 
		{}

		// The storage policy doesn't initialize the stored pointer 
		// which will be initialized by the OwnershipPolicy's Clone fn
		HModuleStorage(const HModuleStorage&) : pointee_(0)
		{}

		HModuleStorage(const StoredType& p) : pointee_(p) {}

		void Swap(HModuleStorage& rhs)
		{ std::swap(pointee_, rhs.pointee_); }

		// Accessors
		friend inline PointerType GetImpl(const HModuleStorage& sp)
		{ return sp.pointee_; }

		friend inline const StoredType& GetImplRef(const HModuleStorage& sp)
		{ return sp.pointee_; }

		friend inline StoredType& GetImplRef(HModuleStorage& sp)
		{ return sp.pointee_; }

	protected:
		// Destroys the data stored
		// (Destruction might be taken over by the OwnershipPolicy)
		void Destroy()
		{
			if ( pointee_) 
				::FreeLibrary(pointee_);
		}

		// Default value to initialize the pointer
		static StoredType Default()
		{ return 0; }

	private:
		// Data
		StoredType pointee_;
	};

	typedef Loki::SmartPtr< HMODULE, Loki::RefCounted, Loki::AllowConversion, Loki::AssertCheck, HModuleStorage, Loki::DontPropagateConst >
		SmartModule;

}
#endif // IVI_SmartModule_H
