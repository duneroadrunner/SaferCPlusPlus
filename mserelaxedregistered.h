
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSERELAXEDREGISTERED_H_
#define MSERELAXEDREGISTERED_H_

#include "mseprimitives.h"
#include <unordered_map>
#include <mutex>

#ifdef _MSC_VER
/* Jan 2016: For some reason std::this_thread::get_id() seems to be really slow in windows. So we're replacing it with the
native GetCurrentThreadId(). */
#define MSE_THREAD_ID_TYPE unsigned long /*DWORD*/
#define MSE_GET_CURRENT_THREAD_ID CSPTrackerMap::mseWindowsGetCurrentThreadId()
#else /*_MSC_VER*/
#include <thread>         // std::thread, MSE_THREAD_ID_TYPE, MSE_GET_CURRENT_THREAD_ID
#define MSE_THREAD_ID_TYPE std::thread::id
#define MSE_GET_CURRENT_THREAD_ID std::this_thread::get_id()
#endif /*_MSC_VER*/

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty> using TRelaxedRegisteredPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredConstPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredNotNullPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredNotNullConstPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredFixedPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredFixedConstPointer = _Ty*;
	template<typename _TROFLy> using TRelaxedRegisteredObj = _TROFLy;
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		return new TRelaxedRegisteredObj<_Ty>(args...);
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		auto a = (TRelaxedRegisteredObj<_Ty>*)regPtrRef;
		delete a;
	}

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* CSPTracker is intended to keep track of all pointers, objects and their lifespans in order to ensure that pointers don't
	end up pointing to deallocated objects. */
	class CSPTracker {
	public:
		CSPTracker() {}
		~CSPTracker() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr);
		void onObjectDestruction(void *obj_ptr);
		void onObjectConstruction(void *obj_ptr);
		bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_obj_pointer_map.size())); }

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
		static const int sc_fs1_max_objects = 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */;
		CFS1Object m_fs1_objects[sc_fs1_max_objects];
		int m_num_fs1_objects = 0;

		/* "slow storage" */
		std::unordered_multimap<void*, const CSaferPtrBase*> m_obj_pointer_map;

		//std::mutex m_mutex;
	};

	class CSPTrackerMap {
	public:
		CSPTrackerMap() {
			m_first_thread_id = MSE_GET_CURRENT_THREAD_ID;
			m_first_sp_tracker_pointer = new CSPTracker();
			//std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*>::value_type item(m_first_thread_id, m_first_sp_tracker_pointer);
			//auto insert_retval = m_tracker_map.insert(item);
		}
		~CSPTrackerMap() {
			delete m_first_sp_tracker_pointer; m_first_sp_tracker_pointer = nullptr;
			for (auto it = m_tracker_map.begin(); m_tracker_map.end() != it; it++) {
				delete (*it).second; (*it).second = nullptr;
			}
		}
		CSPTracker& SPTrackerRef(const MSE_THREAD_ID_TYPE &thread_id_cref) {
			if (thread_id_cref == m_first_thread_id) {
				return (*m_first_sp_tracker_pointer);
			}
			else {
				std::lock_guard<std::mutex> lock(m_mutex);

				auto found_it = m_tracker_map.find(thread_id_cref);
				if (m_tracker_map.end() == found_it) {
					auto new_sp_tracker_ptr = new CSPTracker();
					std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*>::value_type item(thread_id_cref, new_sp_tracker_ptr);
					auto insert_retval = m_tracker_map.insert(item);
					number_of_added_trackers_since_last_pruning += 1;
					if (10000/*arbitrary*/ < number_of_added_trackers_since_last_pruning) {
						remove_empty_trackers();
						number_of_added_trackers_since_last_pruning = 0;
					}

					found_it = insert_retval.first;
				}

				m_first_thread_id = thread_id_cref;
				m_first_sp_tracker_pointer = ((*found_it).second);

				return (*((*found_it).second));
			}
		}
		void remove_empty_trackers() {
			for (auto it = m_tracker_map.begin(); m_tracker_map.end() != it; it++) {
				if ((*it).second->isEmpty()) {
					delete ((*it).second); (*it).second = nullptr;
					m_tracker_map.erase(it);
				}
			}
		}
#ifdef _MSC_VER
		static MSE_THREAD_ID_TYPE mseWindowsGetCurrentThreadId();
#endif /*_MSC_VER*/


		MSE_THREAD_ID_TYPE m_first_thread_id;
		CSPTracker* m_first_sp_tracker_pointer = nullptr;
		std::unordered_map<MSE_THREAD_ID_TYPE, CSPTracker*> m_tracker_map;
		int number_of_added_trackers_since_last_pruning = 0;
		std::mutex m_mutex;
	};

	extern CSPTrackerMap gSPTrackerMap;

	template<typename _Ty> class TRelaxedRegisteredObj;
	template<typename _Ty> class TRelaxedRegisteredNotNullPointer;
	template<typename _Ty> class TRelaxedRegisteredFixedPointer;

	/* TRelaxedRegisteredPointer is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, it may be the case that fewer code changes
	(explicit casts) will be required when using this template. */
	template<typename _Ty>
	class TRelaxedRegisteredPointer : public TSaferPtrForLegacy<_Ty> {
	public:
		TRelaxedRegisteredPointer() : TSaferPtrForLegacy<_Ty>() {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
		}
		TRelaxedRegisteredPointer(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		/* The CSPTracker* parameter is actually kind of redundant. We include it to remove ambiguity in the overloads. */
		TRelaxedRegisteredPointer(CSPTracker* sp_tracker_ptr, TRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		TRelaxedRegisteredPointer(const TRelaxedRegisteredPointer& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		virtual ~TRelaxedRegisteredPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			(*m_sp_tracker_ptr).onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRelaxedRegisteredPointer<_Ty>& operator=(_Ty* ptr) {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			return (*this);
		}
		TRelaxedRegisteredPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			if (m_sp_tracker_ptr != _Right_cref.m_sp_tracker_ptr) {
				/* This indicates that the target object may have been created in a different thread than this pointer. If these
				threads are asynchronous this can be unsafe. We'll allow it here because in many of these cases the threads are
				not asynchoronous. Usually because (at least) one of the original threads is deceased. */
				int q = 7;
			}
			(*m_sp_tracker_ptr).registerPointer((*this), _Right_cref);
			m_might_not_point_to_a_TRelaxedRegisteredObj = _Right_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		TRelaxedRegisteredObj<_Ty>* asANativePointerToTRelaxedRegisteredObj() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			/* It should be ok to hard cast to (TRelaxedRegisteredObj<_Ty>*) even if (*this).m_ptr points to a _Ty and not a
			TRelaxedRegisteredObj<_Ty>, because TRelaxedRegisteredObj<_Ty> doesn't have any extra data members that _Ty
			doesn't. */
			return (TRelaxedRegisteredObj<_Ty>*)((*this).m_ptr);
		}
		/*Ideally these "address of" operators shouldn't be used. If you want a pointer to a TRelaxedRegisteredPointer<_Ty>,
		declare the TRelaxedRegisteredPointer<_Ty> as a TRelaxedRegisteredObj<TRelaxedRegisteredPointer<_Ty>> instead. So
		for example:
		auto reg_ptr = TRelaxedRegisteredObj<TRelaxedRegisteredPointer<_Ty>>(mse::relaxed_registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRelaxedRegisteredPointer<TRelaxedRegisteredPointer<_Ty>> operator&() {
			return this;
		}
		TRelaxedRegisteredPointer<const TRelaxedRegisteredPointer<_Ty>> operator&() const {
			return this;
		}
		TRelaxedRegisteredPointer<_Ty>* real_address() {
			return this;
		}
		const TRelaxedRegisteredPointer<_Ty>* real_address() const {
			return this;
		}

		CSPTracker* m_sp_tracker_ptr = nullptr;
		bool m_might_not_point_to_a_TRelaxedRegisteredObj = false;
	};

	template<typename _Ty>
	class TRelaxedRegisteredConstPointer : public TRelaxedRegisteredPointer<_Ty> {
	public:
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

		const _Ty& operator*() const {
			return TRelaxedRegisteredPointer<_Ty>::operator*();
		}
		const _Ty* operator->() const {
			return TRelaxedRegisteredPointer<_Ty>::operator->();
		}

	private:
		TRelaxedRegisteredConstPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredPointer<_Ty>(ptr) {}

		TRelaxedRegisteredConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredConstPointer<_Ty>* operator&() const { return this; }
	};

	template<typename _Ty>
	class TRelaxedRegisteredNotNullPointer : public TRelaxedRegisteredPointer<_Ty> {
	public:
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredNotNullPointer& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredNotNullPointer() {}
		TRelaxedRegisteredNotNullPointer<_Ty>& operator=(const TRelaxedRegisteredNotNullPointer<_Ty>& _Right_cref) {
			TRelaxedRegisteredPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRelaxedRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredNotNullPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredPointer<_Ty>(ptr) {}
		TRelaxedRegisteredNotNullPointer(CSPTracker* sp_tracker_ptr, TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredPointer<_Ty>(sp_tracker_ptr, ptr) {}

		/* If you want a pointer to a TRelaxedRegisteredNotNullPointer<_Ty>, declare the TRelaxedRegisteredNotNullPointer<_Ty> as a
		TRelaxedRegisteredObj<TRelaxedRegisteredNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TRelaxedRegisteredObj<TRelaxedRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRelaxedRegisteredNotNullPointer<_Ty>* operator&() {
			return this;
		}
		const TRelaxedRegisteredNotNullPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TRelaxedRegisteredFixedPointer<_Ty>;
	};

	template<typename _Ty>
	class TRelaxedRegisteredNotNullConstPointer : public TRelaxedRegisteredNotNullPointer<_Ty> {
	public:
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

		const _Ty& operator*() const {
			return TRelaxedRegisteredNotNullPointer<_Ty>::operator*();
		}
		const _Ty* operator->() const {
			return TRelaxedRegisteredNotNullPointer<_Ty>::operator->();
		}

	private:
		TRelaxedRegisteredNotNullConstPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullPointer<_Ty>(ptr) {}

		TRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() const { return this; }
	};

	/* TRelaxedRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRelaxedRegisteredFixedPointer : public TRelaxedRegisteredNotNullPointer<_Ty> {
	public:
		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredFixedPointer& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredFixedPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullPointer<_Ty>(ptr) {}
		TRelaxedRegisteredFixedPointer(CSPTracker* sp_tracker_ptr, TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullPointer<_Ty>(sp_tracker_ptr, ptr) {}

		/* If you want a pointer to a TRelaxedRegisteredFixedPointer<_Ty>, declare the TRelaxedRegisteredFixedPointer<_Ty> as a
		TRelaxedRegisteredObj<TRelaxedRegisteredFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TRelaxedRegisteredObj<TRelaxedRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRelaxedRegisteredFixedPointer<_Ty>* operator&() {
			return this;
		}
		const TRelaxedRegisteredFixedPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TRelaxedRegisteredObj<_Ty>;
	};

	template<typename _Ty>
	class TRelaxedRegisteredFixedConstPointer : public TRelaxedRegisteredFixedPointer<_Ty> {
	public:
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredFixedPointer<_Ty>& src_cref) : TRelaxedRegisteredFixedPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredFixedPointer<_Ty>::operator _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredFixedPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

		const _Ty& operator*() const {
			return TRelaxedRegisteredFixedPointer<_Ty>::operator*();
		}
		const _Ty* operator->() const {
			return TRelaxedRegisteredFixedPointer<_Ty>::operator->();
		}

	private:
		TRelaxedRegisteredFixedConstPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredFixedPointer<_Ty>(ptr) {}

		TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }
	};

	class CTrackerNotifier {
	public:
		CTrackerNotifier() {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			(*m_sp_tracker_ptr).onObjectConstruction(this);
		}
		~CTrackerNotifier() {
			(*m_sp_tracker_ptr).onObjectDestruction(this);
		}
		CSPTracker* trackerPtr() const { return m_sp_tracker_ptr; }

		CSPTracker* m_sp_tracker_ptr = nullptr;
	};

	/* TRelaxedRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TRelaxedRegisteredPointers will avoid referencing destroyed objects. Note that TRelaxedRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TRelaxedRegisteredObj : public _TROFLy {
	public:
		//using _TROFLy::_TROFLy;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRelaxedRegisteredObj, _TROFLy);
		virtual ~TRelaxedRegisteredObj() {
			//gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID).onObjectDestruction(this);
		}
		TRelaxedRegisteredObj& operator=(TRelaxedRegisteredObj&& _X) { _TROFLy::operator=(std::move(_X)); return (*this); }
		TRelaxedRegisteredObj& operator=(const TRelaxedRegisteredObj& _X) { _TROFLy::operator=(_X); return (*this); }
		TRelaxedRegisteredFixedPointer<_TROFLy> operator&() {
			return TRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
			//return this;
		}
		TRelaxedRegisteredFixedPointer<const _TROFLy> operator&() const {
			return TRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
			//return this;
		}
		CSPTracker* trackerPtr() const { return m_tracker_notifier.trackerPtr(); }

		CTrackerNotifier m_tracker_notifier;
	};

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		auto a = new TRelaxedRegisteredObj<_Ty>(args...);
		return TRelaxedRegisteredPointer<_Ty>((*a).trackerPtr(), a);
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		//auto a = dynamic_cast<TRelaxedRegisteredObj<_Ty> *>((_Ty*)regPtrRef);
		//auto a = (TRelaxedRegisteredObj<_Ty>*)regPtrRef;
		if (regPtrRef.m_might_not_point_to_a_TRelaxedRegisteredObj) {
			/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
			TRelaxedRegisteredPointer<_Ty> to point to a _Ty instead of a TRelaxedRegisteredObj<_Ty>. But in those
			situations it doesn't make sense that someone would be calling this delete function. */
			//_Ty* a = &regPtrRef;
			_Ty* a = regPtrRef.m_ptr;
			gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID).onObjectDestruction(a);
			delete a;
		}
		else {
			auto a = regPtrRef.asANativePointerToTRelaxedRegisteredObj();
			delete a;
		}
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/
}


#endif // MSERELAXEDREGISTERED_H_
