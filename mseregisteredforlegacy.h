#ifndef MSEREGISTEREDFORLEGACY_H_
#define MSEREGISTEREDFORLEGACY_H_

#include "mseprimitives.h"
#include <unordered_map>
#include <mutex>

namespace mse {

	/* CSPTracker is intended to keep track of all pointers, objects and their lifespans in order to ensure that pointers don't
	end up pointing to deallocated objects. */
	class CSPTracker {
	public:
		CSPTracker() {}
		~CSPTracker() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		void onObjectDestruction(void *obj_ptr);

		/* So this tracker stores the object-pointer mappings in either "fast storage1" or "slow storage". The code for
		"fast storage1" is ugly. The code for "slow storage" is more readable. */
		void removeObjectFromFastStorage1(int fs1_obj_index);
		void moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index);
		static const int sc_fs1_max_pointers = 3/* must be at least 1 */;
		class CFS1Object {
		public:
			void* m_object_ptr;
			const CSaferPtrBase* m_pointer_ptrs[sc_fs1_max_pointers];
			int m_num_pointers = 0;
		};
		static const int sc_fs1_max_objects = 3/*aritrary*/;
		CFS1Object m_fs1_objects[sc_fs1_max_objects];
		int m_num_fs1_objects = 0;

		/* "slow storage" */
		std::unordered_multimap<void*, const CSaferPtrBase*> m_obj_pointer_map;

		std::mutex m_mutex;
	};

	extern CSPTracker gSPTracker;

	template<typename _Ty>
	class TRegisteredObjForLegacy;

	/* TRegisteredPointerForLegacy is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, it may be the case that fewer code changes
	(explicit casts) will be required when using this template. */
	template<typename _Ty>
	class TRegisteredPointerForLegacy : public TSaferPtrForLegacy<_Ty> {
	public:
		TRegisteredPointerForLegacy() : TSaferPtrForLegacy<_Ty>() { /*gSPTracker.registerPointer((*this), null_ptr);*/ }
		TRegisteredPointerForLegacy(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) { gSPTracker.registerPointer((*this), ptr); }
		TRegisteredPointerForLegacy(const TRegisteredPointerForLegacy& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) { gSPTracker.registerPointer((*this), src_cref.m_ptr); }
		virtual ~TRegisteredPointerForLegacy() {
			gSPTracker.unregisterPointer((*this), (*this).m_ptr);
			gSPTracker.onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(_Ty* ptr) {
			gSPTracker.unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			gSPTracker.registerPointer((*this), ptr);
			return (*this);
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(const TRegisteredPointerForLegacy<_Ty>& _Right_cref) {
			return operator=(_Right_cref.m_ptr);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
		/* This cast operator, if possible, should not be used. It is meant to be used exclusively by registered_delete_for_legacy<>(). */
		explicit operator TRegisteredObjForLegacy<_Ty>*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			/* It should be ok to hard cast to (TRegisteredObjForLegacy<_Ty>*) even if (*this).m_ptr points to a _Ty and not a
			TRegisteredObjForLegacy<_Ty>, because TRegisteredObjForLegacy<_Ty> doesn't have any extra data members that _Ty
			doesn't. */
			return (TRegisteredObjForLegacy<_Ty>*)((*this).m_ptr);
		}
		/*Ideally these "address of" operators shouldn't be used. If you want a pointer to a TRegisteredPointerForLegacy<_Ty>,
		declare the TRegisteredPointerForLegacy<_Ty> as a TRegisteredObjForLegacy<TRegisteredPointerForLegacy<_Ty>> instead. So
		for example:
		auto reg_ptr = TRegisteredObjForLegacy<TRegisteredPointerForLegacy<_Ty>>(mse::registered_new_for_legacy<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRegisteredPointerForLegacy<TRegisteredPointerForLegacy<_Ty>> operator&() {
			return this;
		}
		TRegisteredPointerForLegacy<const TRegisteredPointerForLegacy<_Ty>> operator&() const {
			return this;
		}
	};

	/* See TRegisteredObj. */
	template<typename _Ty>
	class TRegisteredObjForLegacy : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObjForLegacy, _Ty);
		virtual ~TRegisteredObjForLegacy() {
			gSPTracker.onObjectDestruction(this);
		}
		TRegisteredObjForLegacy& operator=(TRegisteredObjForLegacy&& _X) { _Ty::operator=(std::move(_X)); return (*this); }
		TRegisteredObjForLegacy& operator=(const TRegisteredObjForLegacy& _X) { _Ty::operator=(_X); return (*this); }
		TRegisteredPointerForLegacy<_Ty> operator&() {
			return this;
		}
		TRegisteredPointerForLegacy<const _Ty> operator&() const {
			return this;
		}
	};

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRegisteredPointerForLegacy<_Ty> registered_new_for_legacy(Args&&... args) {
		return new TRegisteredObjForLegacy<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete_for_legacy(const TRegisteredPointerForLegacy<_Ty>& regPtrRef) {
		//auto a = dynamic_cast<TRegisteredObjForLegacy<_Ty> *>((_Ty*)regPtrRef);
		auto a = (TRegisteredObjForLegacy<_Ty>*)regPtrRef;
		delete a;
	}
}


#endif // MSEREGISTEREDFORLEGACY_H_
