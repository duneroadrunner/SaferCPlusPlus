#ifndef MSEREGISTEREDFORLEGACY_H_
#define MSEREGISTEREDFORLEGACY_H_

#include "mseprimitives.h"
#include <unordered_map>

namespace mse {

	class CSPManager {
	public:
		CSPManager() {}
		~CSPManager() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		void onObjectDestruction(void *obj_ptr);
		std::unordered_multimap<void*, const CSaferPtrBase&> m_obj_pointer_map;
	};

	extern CSPManager gSPManager;

	/* TRegisteredPointerForLegacy is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, fewer code changes (explicit casts) may be required
	when using this template. */
	template<typename _Ty>
	class TRegisteredPointerForLegacy : public TSaferPtrForLegacy<_Ty> {
	public:
		TRegisteredPointerForLegacy() : TSaferPtrForLegacy<_Ty>() { /*gSPManager.registerPointer((*this), null_ptr);*/ }
		TRegisteredPointerForLegacy(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) { gSPManager.registerPointer((*this), ptr); }
		TRegisteredPointerForLegacy(const TRegisteredPointerForLegacy& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) { gSPManager.registerPointer((*this), src_cref.m_ptr); }
		virtual ~TRegisteredPointerForLegacy() {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			gSPManager.onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRegisteredPointerForLegacy<_Ty>& operator=(_Ty* ptr) {
			gSPManager.unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			gSPManager.registerPointer((*this), ptr);
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
	};

	template<typename _Ty>
	class TRegisteredObjForLegacy : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObjForLegacy, _Ty);
		virtual ~TRegisteredObjForLegacy() {
			gSPManager.onObjectDestruction(this);
		}
		TRegisteredPointerForLegacy<_Ty> operator&() {
			return TRegisteredPointerForLegacy<_Ty>(this);
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return TRegisteredPointer<const _Ty>(this);
		}
	};

	template <class _Ty, class... Args>
	TRegisteredPointerForLegacy<_Ty> registered_new_for_legacy(Args&&... args) {
		return new TRegisteredObjForLegacy<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete_for_legacy(const TRegisteredPointerForLegacy<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObjForLegacy<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}
}


#endif // MSEREGISTEREDFORLEGACY_H_
