
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* Relaxed registered pointers and objects is a set of data types serving dual purposes and should probably, at some point,
be split into to separate sets of data types. One purpose is to simply act as a set of registered pointers that can target
classes before they are fully defined (possibly at the expense of some speed and/or safety). Regular registered pointers
don't support this.
The other (and the original) purpose is to be very compatible with legacy code and interfaces that use native pointers. To
that end relaxed registered pointers support construction/assignment from native pointers which may, or may not point to a
relaxed registered object. In order to determine whether or not the native pointer does indeed point to a relaxed
registered object, we need the assistance of a global registry.
This use of this global registry enhances the fulfillment of the latter purpose and is not necessary for the former. The
former purpose could be satisfied with a faster, safer, "header file only" set of data types. 
*/

#pragma once
#ifndef MSERELAXEDREGISTERED_H_
#define MSERELAXEDREGISTERED_H_

//include "mseprimitives.h"
#include "msepointerbasics.h"
#include <utility>
#include <unordered_map>
#include <mutex>
#include <cassert>
//include <typeinfo>      // std::bad_cast
#include <stdexcept>


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

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty> using TRelaxedRegisteredPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredConstPointer = const _Ty*;
	template<typename _Ty> using TRelaxedRegisteredNotNullPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TRelaxedRegisteredFixedPointer = _Ty*;
	template<typename _Ty> using TRelaxedRegisteredFixedConstPointer = const _Ty*;
	template<typename _TROFLy> using TRelaxedRegisteredObj = _TROFLy;
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		return new TRelaxedRegisteredObj<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		auto a = (TRelaxedRegisteredObj<_Ty>*)regPtrRef;
		delete a;
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = (const TRelaxedRegisteredObj<_Ty>*)regPtrRef;
		delete a;
	}

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	class relaxedregistered_cannot_verify_cast_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

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
		bool registerPointer(const CSaferPtrBase& sp_ref, const void *obj_ptr) { return (*this).registerPointer(sp_ref, (void *)obj_ptr); }
		bool unregisterPointer(const CSaferPtrBase& sp_ref, const void *obj_ptr) { return (*this).unregisterPointer(sp_ref, (void *)obj_ptr); }
		void onObjectDestruction(const void *obj_ptr) { (*this).onObjectDestruction((void *)obj_ptr); }
		void onObjectConstruction(const void *obj_ptr) { (*this).onObjectConstruction((void *)obj_ptr); }
		void reserve_space_for_one_more() {
			/* The purpose of this function is to ensure that the next call to registerPointer() won't
			need to allocate more memory, and thus won't have any chance of throwing an exception due to
			memory allocation failure. */
			m_obj_pointer_map.reserve(m_obj_pointer_map.size() + 1);
		}

		bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_obj_pointer_map.size())); }

		/* So this tracker stores the object-pointer mappings in either "fast storage1" or "slow storage". The code for
		"fast storage1" is ugly. The code for "slow storage" is more readable. */
		void removeObjectFromFastStorage1(int fs1_obj_index);
		void moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index);
		MSE_CONSTEXPR static const int sc_fs1_max_pointers = 3/* must be at least 1 */;
		class CFS1Object {
		public:
			void* m_object_ptr;
			const CSaferPtrBase* m_pointer_ptrs[sc_fs1_max_pointers];
			int m_num_pointers = 0;
		};
		MSE_CONSTEXPR static const int sc_fs1_max_objects = 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */;
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
	template<typename _Ty> class TRelaxedRegisteredConstPointer;
	template<typename _Ty> class TRelaxedRegisteredNotNullPointer;
	template<typename _Ty> class TRelaxedRegisteredNotNullConstPointer;
	template<typename _Ty> class TRelaxedRegisteredFixedPointer;
	template<typename _Ty> class TRelaxedRegisteredFixedConstPointer;

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
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			//m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		virtual ~TRelaxedRegisteredPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			(*m_sp_tracker_ptr).onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRelaxedRegisteredPointer<_Ty>& operator=(_Ty* ptr) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<_Ty>::operator=(ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			return (*this);
		}
		TRelaxedRegisteredPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
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
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TRelaxedRegisteredPointer(_Right_cref));
		}
		operator bool() const { return (*this).m_ptr; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
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

		void relaxed_registered_delete() const {
			if (m_might_not_point_to_a_TRelaxedRegisteredObj) {
				/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
				TRelaxedRegisteredPointer<_Ty> to point to a _Ty instead of a TRelaxedRegisteredObj<_Ty>. But in those
				situations it doesn't make sense that someone would be calling this delete function. */
				//_Ty* a = this;
				_Ty* a = (*this).m_ptr;
				(*m_sp_tracker_ptr).onObjectDestruction(a);
				delete a;
			}
			else {
				auto a = asANativePointerToTRelaxedRegisteredObj();
				delete a;
			}
		}

	private:
		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		TRelaxedRegisteredObj<_Ty>* asANativePointerToTRelaxedRegisteredObj() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			if (m_might_not_point_to_a_TRelaxedRegisteredObj) { MSE_THROW(relaxedregistered_cannot_verify_cast_error("cannot verify cast validity - mse::TRelaxedRegisteredPointer")); }
			return (TRelaxedRegisteredObj<_Ty>*)((*this).m_ptr);
		}

		CSPTracker* m_sp_tracker_ptr = nullptr;
		bool m_might_not_point_to_a_TRelaxedRegisteredObj = false;

		template <class Y> friend class TRelaxedRegisteredPointer;
		template <class Y> friend class TRelaxedRegisteredConstPointer;
	};

	template<typename _Ty>
	class TRelaxedRegisteredConstPointer : public TSaferPtrForLegacy<const _Ty> {
	public:
		TRelaxedRegisteredConstPointer() : TSaferPtrForLegacy<const _Ty>() {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
		}
		TRelaxedRegisteredConstPointer(const _Ty* ptr) : TSaferPtrForLegacy<const _Ty>(ptr) {
			m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		/* The CSPTracker* parameter is actually kind of redundant. We include it to remove ambiguity in the overloads. */
		TRelaxedRegisteredConstPointer(CSPTracker* sp_tracker_ptr, const TRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<const _Ty>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
		}
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredConstPointer& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			//m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID));
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			m_might_not_point_to_a_TRelaxedRegisteredObj = src_cref.m_might_not_point_to_a_TRelaxedRegisteredObj;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		virtual ~TRelaxedRegisteredConstPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			(*m_sp_tracker_ptr).onObjectDestruction(this); /* Just in case there are pointers to this pointer out there. */
		}
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const _Ty* ptr) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<const _Ty>::operator=(ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), ptr);
			m_might_not_point_to_a_TRelaxedRegisteredObj = true;
			return (*this);
		}
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const TRelaxedRegisteredConstPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<const _Ty>::operator=(_Right_cref);
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
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const TRelaxedRegisteredConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TRelaxedRegisteredConstPointer(_Right_cref));
		}
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			return (*this).operator=(TRelaxedRegisteredConstPointer<_Ty>(_Right_cref));
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TRelaxedRegisteredPointer<_Ty>(_Right_cref));
		}
		operator bool() const { return (*this).m_ptr; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator const _Ty*() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			return (*this).m_ptr;
		}
		/*Ideally these "address of" operators shouldn't be used. If you want a pointer to a TRelaxedRegisteredConstPointer<_Ty>,
		declare the TRelaxedRegisteredConstPointer<_Ty> as a TRelaxedRegisteredObj<TRelaxedRegisteredConstPointer<_Ty>> instead. So
		for example:
		auto reg_ptr = TRelaxedRegisteredObj<TRelaxedRegisteredConstPointer<_Ty>>(mse::relaxed_registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRelaxedRegisteredPointer<TRelaxedRegisteredConstPointer<_Ty>> operator&() {
			return this;
		}
		TRelaxedRegisteredPointer<const TRelaxedRegisteredConstPointer<_Ty>> operator&() const {
			return this;
		}
		TRelaxedRegisteredConstPointer<_Ty>* real_address() {
			return this;
		}
		const TRelaxedRegisteredConstPointer<_Ty>* real_address() const {
			return this;
		}

		void relaxed_registered_delete() const {
			if (m_might_not_point_to_a_TRelaxedRegisteredObj) {
				/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
				TRelaxedRegisteredPointer<_Ty> to point to a _Ty instead of a TRelaxedRegisteredObj<_Ty>. But in those
				situations it doesn't make sense that someone would be calling this delete function. */
				//const _Ty* a = this;
				const _Ty* a = (*this).m_ptr;
				(*m_sp_tracker_ptr).onObjectDestruction(a);
				delete a;
			}
			else {
				auto a = asANativePointerToTRelaxedRegisteredObj();
				delete a;
			}
		}

	private:
		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		const TRelaxedRegisteredObj<_Ty>* asANativePointerToTRelaxedRegisteredObj() const {
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
			if (m_might_not_point_to_a_TRelaxedRegisteredObj) { MSE_THROW(relaxedregistered_cannot_verify_cast_error("cannot verify cast validity - mse::TRelaxedRegisteredConstPointer")); }
			return (const TRelaxedRegisteredObj<_Ty>*)((*this).m_ptr);
		}

		CSPTracker* m_sp_tracker_ptr = nullptr;
		bool m_might_not_point_to_a_TRelaxedRegisteredObj = false;

		template <class Y> friend class TRelaxedRegisteredConstPointer;
		friend class TRelaxedRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TRelaxedRegisteredNotNullPointer : public TRelaxedRegisteredPointer<_Ty> {
	public:
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredNotNullPointer& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredNotNullPointer() {}
		/*
		TRelaxedRegisteredNotNullPointer<_Ty>& operator=(const TRelaxedRegisteredNotNullPointer<_Ty>& _Right_cref) {
			TRelaxedRegisteredPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		*/
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
	class TRelaxedRegisteredNotNullConstPointer : public TRelaxedRegisteredConstPointer<_Ty> {
	public:
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredNotNullConstPointer<_Ty>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredNotNullConstPointer<_Ty2>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredConstPointer<_Ty>::operator const TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredConstPointer<_Ty>(ptr) {}

		TRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TRelaxedRegisteredFixedConstPointer<_Ty>;
	};

	/* TRelaxedRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRelaxedRegisteredFixedPointer : public TRelaxedRegisteredNotNullPointer<_Ty> {
	public:
		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredFixedPointer& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredFixedPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredFixedPointer(TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullPointer<_Ty>(ptr) {}
		TRelaxedRegisteredFixedPointer(CSPTracker* sp_tracker_ptr, TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TRelaxedRegisteredFixedPointer<_Ty>& operator=(const TRelaxedRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

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
	class TRelaxedRegisteredFixedConstPointer : public TRelaxedRegisteredNotNullConstPointer<_Ty> {
	public:
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredFixedPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredFixedPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredFixedConstPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredFixedConstPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TRelaxedRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullConstPointer<_Ty>::operator const TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullConstPointer<_Ty>(ptr) {}
		TRelaxedRegisteredFixedConstPointer<_Ty>& operator=(const TRelaxedRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

		TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TRelaxedRegisteredObj<_Ty>;
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
		MSE_USING(TRelaxedRegisteredObj, _TROFLy);
		TRelaxedRegisteredObj(const TRelaxedRegisteredObj& _X) : _TROFLy(_X) {}
		TRelaxedRegisteredObj(TRelaxedRegisteredObj&& _X) : _TROFLy(std::move(_X)) {}
		virtual ~TRelaxedRegisteredObj() {
			//gSPTrackerMap.SPTrackerRef(MSE_GET_CURRENT_THREAD_ID).onObjectDestruction(this);
		}
		using _TROFLy::operator=;
		//TRelaxedRegisteredObj& operator=(TRelaxedRegisteredObj&& _X) { _TROFLy::operator=(std::move(_X)); return (*this); }
		TRelaxedRegisteredObj& operator=(typename std::conditional<std::is_const<_TROFLy>::value
			, std::nullptr_t, TRelaxedRegisteredObj>::type&& _X) { _TROFLy::operator=(std::move(_X)); return (*this); }
		//TRelaxedRegisteredObj& operator=(const TRelaxedRegisteredObj& _X) { _TROFLy::operator=(_X); return (*this); }
		TRelaxedRegisteredObj& operator=(const typename std::conditional<std::is_const<_TROFLy>::value
			, std::nullptr_t, TRelaxedRegisteredObj>::type& _X) { _TROFLy::operator=(_X); return (*this); }
		TRelaxedRegisteredFixedPointer<_TROFLy> operator&() {
			return TRelaxedRegisteredFixedPointer<_TROFLy>(this);
			//return this;
		}
		TRelaxedRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TRelaxedRegisteredFixedConstPointer<_TROFLy>(this);
			//return this;
		}
		CSPTracker* trackerPtr() const { return m_tracker_notifier.trackerPtr(); }

		CTrackerNotifier m_tracker_notifier;
	};

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		auto a = new TRelaxedRegisteredObj<_Ty>(std::forward<Args>(args)...);
		return TRelaxedRegisteredPointer<_Ty>((*a).trackerPtr(), a);
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		regPtrRef.relaxed_registered_delete();
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
		regPtrRef.relaxed_registered_delete();
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

#if defined(MSE_REGISTEREDPOINTER_DISABLED)
	/* Omit definition of make_pointer_to_member() as it would clash with the one already defined in mseregistered.h. */
#define MSE_RELAXEDREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER
#endif // defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_RELAXEDREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TRelaxedRegisteredPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TRelaxedRegisteredPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TRelaxedRegisteredPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TRelaxedRegisteredConstPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TRelaxedRegisteredConstPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TRelaxedRegisteredConstPointer<_Ty>>::make(target, lease_pointer);
	}
#endif // !defined(MSE_RELAXEDREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)

	/* shorter aliases */
	template<typename _Ty> using rrp = TRelaxedRegisteredPointer<_Ty>;
	template<typename _Ty> using rrcp = TRelaxedRegisteredConstPointer<_Ty>;
	template<typename _Ty> using rrnnp = TRelaxedRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using rrnncp = TRelaxedRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using rrfp = TRelaxedRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using rrfcp = TRelaxedRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using rro = TRelaxedRegisteredObj<_TROFLy>;
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> rrnew(Args&&... args) { return relaxed_registered_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty>
	void rrdelete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) { relaxed_registered_delete<_Ty>(regPtrRef); }


	static void s_relaxedregptr_test1() {
#ifdef MSE_SELF_TESTS
		class C;

		class D {
		public:
			virtual ~D() {}
			mse::TRelaxedRegisteredPointer<C> m_c_ptr = nullptr;
		};

		class C {
		public:
			mse::TRelaxedRegisteredPointer<D> m_d_ptr = nullptr;
		};

		mse::TRelaxedRegisteredObj<C> regobjfl_c;
		mse::TRelaxedRegisteredPointer<D> d_ptr = mse::relaxed_registered_new<D>();

		regobjfl_c.m_d_ptr = d_ptr;
		d_ptr->m_c_ptr = &regobjfl_c;

		mse::TRelaxedRegisteredConstPointer<C> rrcp = d_ptr->m_c_ptr;
		mse::TRelaxedRegisteredConstPointer<C> rrcp2 = rrcp;
		const mse::TRelaxedRegisteredObj<C> regobjfl_e;
		rrcp = &regobjfl_e;
		mse::TRelaxedRegisteredFixedConstPointer<C> rrfcp = &regobjfl_e;
		rrcp = mse::relaxed_registered_new<C>();
		mse::relaxed_registered_delete<C>(rrcp);

		mse::relaxed_registered_delete<D>(d_ptr);

		{
			/* Polymorphic conversions. */
			class FD : public mse::TRelaxedRegisteredObj<D> {};
			mse::TRelaxedRegisteredObj<FD> relaxedregistered_fd;
			mse::TRelaxedRegisteredPointer<FD> FD_relaxedregistered_ptr1 = &relaxedregistered_fd;
			mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr4 = FD_relaxedregistered_ptr1;
			D_relaxedregistered_ptr4 = &relaxedregistered_fd;
			mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr1 = &relaxedregistered_fd;
			mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr1 = &relaxedregistered_fd;

			/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
			class GD : public D {};
			mse::TRelaxedRegisteredObj<GD> relaxedregistered_gd;
			mse::TRelaxedRegisteredPointer<GD> GD_relaxedregistered_ptr1 = &relaxedregistered_gd;
			mse::TRelaxedRegisteredPointer<D> D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
			D_relaxedregistered_ptr5 = GD_relaxedregistered_ptr1;
			mse::TRelaxedRegisteredFixedPointer<GD> GD_relaxedregistered_fptr1 = &relaxedregistered_gd;
			D_relaxedregistered_ptr5 = &relaxedregistered_gd;
			mse::TRelaxedRegisteredFixedPointer<D> D_relaxedregistered_fptr2 = &relaxedregistered_gd;
			mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxedregistered_fcptr2 = &relaxedregistered_gd;
		}
#endif // MSE_SELF_TESTS
	}
}

#undef MSE_THROW

#endif // MSERELAXEDREGISTERED_H_
