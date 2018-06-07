
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* Relaxed registered pointers are basically just like registered pointers except that unlike registered pointers, they
are able to reference target object types before those target types are (fully) defined. This is required to support
"mutually referencing" objects or "cyclic references".

The implementation difference is that with registered pointers, the "pointer tracking registry" is located in the target
object, whereas relaxed registered pointers use (thread local) global registries that track all the pointers targeting
objects of a given type. */

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

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

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

	template<typename _Ty> auto relaxed_registered_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto relaxed_registered_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	class relaxed_registered_cannot_verify_cast_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};


#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4127 )
#endif /*_MSC_VER*/

	/* CSPTracker is intended to keep track of all pointers, objects and their lifespans in order to ensure that pointers don't
	end up pointing to deallocated objects. */
	class CSPTracker {
	public:
		CSPTracker() {}
		~CSPTracker() {}
		bool registerPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
			if (nullptr == obj_ptr) { return true; }
			{
				//std::lock_guard<std::mutex> lock(m_mutex);

				/* check if the object is in "fast storage 1" first */
				for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
					if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
						auto& fs1_object_ref = m_fs1_objects[i];
						if (sc_fs1_max_pointers == fs1_object_ref.m_num_pointers) {
							/* Too many pointers. We're gonna move this object to slow storage. */
							moveObjectFromFastStorage1ToSlowStorage(i);
							/* Then add the new object-pointer mapping to slow storage. */
							std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
							m_obj_pointer_map.insert(item);
							return true;
						}
						else {
							/* register the object-pointer mapping in "fast storage 1" */
							fs1_object_ref.m_pointer_ptrs[fs1_object_ref.m_num_pointers] = (&sp_ref);
							fs1_object_ref.m_num_pointers += 1;
							return true;
						}
					}
				}

				/* The object was not in "fast storage 1". Check if it's in "slow storage". */
				bool object_is_in_slow_storage = false;
				if (1 <= m_obj_pointer_map.size()) {
					auto found_it = m_obj_pointer_map.find(obj_ptr);
					if (m_obj_pointer_map.end() != found_it) {
						object_is_in_slow_storage = true;
					}
				}

				if ((!object_is_in_slow_storage) && (1 <= sc_fs1_max_objects) && (1 <= sc_fs1_max_pointers)) {
					/* We'll add this object to fast storage. */
					if (sc_fs1_max_objects == m_num_fs1_objects) {
						/* Too many objects. We're gonna move the oldest object to slow storage. */
						moveObjectFromFastStorage1ToSlowStorage(0);
					}
					auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
					fs1_object_ref.m_object_ptr = obj_ptr;
					fs1_object_ref.m_pointer_ptrs[0] = &sp_ref;
					fs1_object_ref.m_num_pointers = 1;
					m_num_fs1_objects += 1;
					return true;
				}
				else {
					/* Add the mapping to slow storage. */
					std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(obj_ptr, &sp_ref);
					m_obj_pointer_map.insert(item);
				}
			}
			return true;
		}
		bool unregisterPointer(const CSaferPtrBase& sp_ref, void *obj_ptr) {
			if (nullptr == obj_ptr) { return true; }
			bool retval = false;
			{
				//std::lock_guard<std::mutex> lock(m_mutex);

				/* check if the object is in "fast storage 1" first */
				for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
					if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
						auto& fs1_object_ref = m_fs1_objects[i];
						if (1 == fs1_object_ref.m_num_pointers) {
							/* Special case code just for speed. */
							if ((&sp_ref) == fs1_object_ref.m_pointer_ptrs[0]) {
								fs1_object_ref.m_num_pointers = 0;
								//removeObjectFromFastStorage1(i);
							}
							else {
								/* We should really never get here. It seems someone's trying to unregister a pointer that does not
								seem to be registered. */
								return false;
							}
							return true;
						}
						else {
							for (int j = (fs1_object_ref.m_num_pointers - 1); j >= 0; j -= 1) {
								if ((&sp_ref) == fs1_object_ref.m_pointer_ptrs[j]) {
									/* Found the mapping for the pointer. We'll now remove it. */
									for (int k = j; k < (fs1_object_ref.m_num_pointers - 1); k += 1) {
										fs1_object_ref.m_pointer_ptrs[k] = fs1_object_ref.m_pointer_ptrs[k + 1];
									}
									fs1_object_ref.m_num_pointers -= 1;

									if (0 == fs1_object_ref.m_num_pointers) {
										//removeObjectFromFastStorage1(i);
									}

									return true;
								}
							}
						}
						/* We should really never get here. It seems someone's trying to unregister a pointer that does not
						seem to be registered. */
						return false;
					}
				}

				/* The object was not in "fast storage 1". It's proably in "slow storage". */
				auto range = m_obj_pointer_map.equal_range(obj_ptr);
				if (true) {
					for (auto& it = range.first; range.second != it; it++) {
						if (((*it).second) == &sp_ref)/*we're comparing "native pointers pointing to smart pointers" here*/ {
							m_obj_pointer_map.erase(it);
							retval = true;
							break;
						}
					}
				}
			}
			return retval;
		}
		void onObjectDestruction(void *obj_ptr) {
			if (nullptr == obj_ptr) { assert(false); return; }
			{
				//std::lock_guard<std::mutex> lock(m_mutex);

				/* check if the object is in "fast storage 1" first */
				for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
					if (obj_ptr == m_fs1_objects[i].m_object_ptr) {
						auto& fs1_object_ref = m_fs1_objects[i];
						for (int j = 0; j < fs1_object_ref.m_num_pointers; j += 1) {
							(*(fs1_object_ref.m_pointer_ptrs[j])).setToNull();
						}
						removeObjectFromFastStorage1(i);
						return;
					}
				}

				/* The object was not in "fast storage 1". It's proably in "slow storage". */
				auto range = m_obj_pointer_map.equal_range(obj_ptr);
				for (auto it = range.first; range.second != it; it++) {
					(*((*it).second)).setToNull();
				}
				m_obj_pointer_map.erase(obj_ptr);
			}
		}
		void onObjectConstruction(void *obj_ptr) {
			if (nullptr == obj_ptr) { assert(false); return; }
			if ((1 <= sc_fs1_max_objects) && (1 <= sc_fs1_max_pointers)) {
				/* We'll add this object to fast storage. */
				if (sc_fs1_max_objects == m_num_fs1_objects) {
					/* Too many objects. We're gonna move the oldest object to slow storage. */
					moveObjectFromFastStorage1ToSlowStorage(0);
				}
				auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
				fs1_object_ref.m_object_ptr = obj_ptr;
				fs1_object_ref.m_num_pointers = 0;
				m_num_fs1_objects += 1;
				return;
			}
		}
		bool registerPointer(const CSaferPtrBase& sp_ref, const void *obj_ptr) { return (*this).registerPointer(sp_ref, const_cast<void *>(obj_ptr)); }
		bool unregisterPointer(const CSaferPtrBase& sp_ref, const void *obj_ptr) { return (*this).unregisterPointer(sp_ref, const_cast<void *>(obj_ptr)); }
		void onObjectDestruction(const void *obj_ptr) { (*this).onObjectDestruction(const_cast<void *>(obj_ptr)); }
		void onObjectConstruction(const void *obj_ptr) { (*this).onObjectConstruction(const_cast<void *>(obj_ptr)); }
		void reserve_space_for_one_more() {
			/* The purpose of this function is to ensure that the next call to registerPointer() won't
			need to allocate more memory, and thus won't have any chance of throwing an exception due to
			memory allocation failure. */
			m_obj_pointer_map.reserve(m_obj_pointer_map.size() + 1);
		}

		bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_obj_pointer_map.size())); }

	private:
		/* So this tracker stores the object-pointer mappings in either "fast storage1" or "slow storage". The code for
		"fast storage1" is ugly. The code for "slow storage" is more readable. */
		void removeObjectFromFastStorage1(int fs1_obj_index) {
			for (int j = fs1_obj_index; j < (m_num_fs1_objects - 1); j += 1) {
				m_fs1_objects[j] = m_fs1_objects[j + 1];
			}
			m_num_fs1_objects -= 1;
		}
		void moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index) {
			auto& fs1_object_ref = m_fs1_objects[fs1_obj_index];
			/* First we're gonna copy this object to slow storage. */
			for (int j = 0; j < fs1_object_ref.m_num_pointers; j += 1) {
				std::unordered_multimap<void*, const CSaferPtrBase*>::value_type item(fs1_object_ref.m_object_ptr, fs1_object_ref.m_pointer_ptrs[j]);
				m_obj_pointer_map.insert(item);
			}
			/* Then we're gonna remove the object from fast storage */
			removeObjectFromFastStorage1(fs1_obj_index);
		}

#ifndef MSE_SPTRACKER_FS1_MAX_POINTERS
#define MSE_SPTRACKER_FS1_MAX_POINTERS 3/* must be at least 1 */
#endif // !MSE_SPTRACKER_FS1_MAX_POINTERS
		MSE_CONSTEXPR static const int sc_fs1_max_pointers = MSE_SPTRACKER_FS1_MAX_POINTERS;
		class CFS1Object {
		public:
			void* m_object_ptr;
			const CSaferPtrBase* m_pointer_ptrs[sc_fs1_max_pointers];
			int m_num_pointers = 0;
		};

#ifndef MSE_SPTRACKER_FS1_MAX_OBJECTS
#define MSE_SPTRACKER_FS1_MAX_OBJECTS 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */
#endif // !MSE_SPTRACKER_FS1_MAX_OBJECTS
		MSE_CONSTEXPR static const int sc_fs1_max_objects = MSE_SPTRACKER_FS1_MAX_OBJECTS;
		CFS1Object m_fs1_objects[sc_fs1_max_objects];
		int m_num_fs1_objects = 0;

		/* "slow storage" */
		std::unordered_multimap<void*, const CSaferPtrBase*> m_obj_pointer_map;

		//std::mutex m_mutex;
	};

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

	template<typename _Ty>
	inline CSPTracker& tlSPTracker_ref() {
		thread_local static CSPTracker tlSPTracker;
		return tlSPTracker;
	}

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
	class TRelaxedRegisteredPointer : public TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>> {
	public:
		TRelaxedRegisteredPointer() : TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TRelaxedRegisteredPointer(const TRelaxedRegisteredPointer& src_cref) : TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TRelaxedRegisteredPointer(std::nullptr_t) : TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TRelaxedRegisteredPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TRelaxedRegisteredPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredPointer<_Ty>& operator=(const TRelaxedRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TRelaxedRegisteredPointer(_Right_cref));
		}
		operator bool() const { return (*this).m_ptr; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		operator _Ty*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return (*this).m_ptr;
		}

		/* In C++, if an object is deleted via a pointer to its base class and the base class' destructor is not virtual,
		then the (derived) object's destructor won't be called possibly resulting in resource leaks. With registered
		objects, the destructor not being called also circumvents their memory safety mechanism. */
		void relaxed_registered_delete() const {
			auto a = asANativePointerToTRelaxedRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TRelaxedRegisteredPointer(CSPTracker* sp_tracker_ptr, TRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<TRelaxedRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		TRelaxedRegisteredObj<_Ty>* asANativePointerToTRelaxedRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TRelaxedRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TRelaxedRegisteredPointer;
		template <class Y> friend class TRelaxedRegisteredConstPointer;
		friend class TRelaxedRegisteredNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TRelaxedRegisteredConstPointer : public TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>> {
	public:
		TRelaxedRegisteredConstPointer() : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredConstPointer& src_cref) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredConstPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TRelaxedRegisteredConstPointer(std::nullptr_t) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TRelaxedRegisteredConstPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TRelaxedRegisteredConstPointer<_Ty>& operator=(const TRelaxedRegisteredConstPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
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
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return (*this).m_ptr;
		}

		/* In C++, if an object is deleted via a pointer to its base class and the base class' destructor is not virtual,
		then the (derived) object's destructor won't be called possibly resulting in resource leaks. With registered
		objects, the destructor not being called also circumvents their memory safety mechanism. */
		void relaxed_registered_delete() const {
			auto a = asANativePointerToTRelaxedRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TRelaxedRegisteredConstPointer(CSPTracker* sp_tracker_ptr, const TRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<const TRelaxedRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		const TRelaxedRegisteredObj<_Ty>* asANativePointerToTRelaxedRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TRelaxedRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TRelaxedRegisteredConstPointer;
		friend class TRelaxedRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TRelaxedRegisteredNotNullPointer : public TRelaxedRegisteredPointer<_Ty> {
	public:
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredNotNullPointer& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {}

		TRelaxedRegisteredNotNullPointer(const  TRelaxedRegisteredPointer<_Ty>& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TRelaxedRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

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

		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredConstPointer<_Ty>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredNotNullConstPointer(const TRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		virtual ~TRelaxedRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredConstPointer<_Ty>::operator const TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredNotNullConstPointer(CSPTracker* sp_tracker_ptr, const TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredConstPointer<_Ty>(sp_tracker_ptr, ptr) {}

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

		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}

		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}

		virtual ~TRelaxedRegisteredFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullPointer<_Ty>::operator TRelaxedRegisteredObj<_Ty>*(); }

	private:
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

		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredNotNullConstPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredNotNullConstPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredConstPointer<_Ty>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRelaxedRegisteredFixedConstPointer(const TRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		virtual ~TRelaxedRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRelaxedRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TRelaxedRegisteredObj<_Ty>*() const { return TRelaxedRegisteredNotNullConstPointer<_Ty>::operator const TRelaxedRegisteredObj<_Ty>*(); }

	private:
		TRelaxedRegisteredFixedConstPointer(CSPTracker* sp_tracker_ptr, const TRelaxedRegisteredObj<_Ty>* ptr) : TRelaxedRegisteredNotNullConstPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TRelaxedRegisteredFixedConstPointer<_Ty>& operator=(const TRelaxedRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

		TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
		const TRelaxedRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TRelaxedRegisteredObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_RELAXED_REGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...), m_tracker_notifier(*this) {}

	/* TRelaxedRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TRelaxedRegisteredPointers will avoid referencing destroyed objects. Note that TRelaxedRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TRelaxedRegisteredObj : public _TROFLy
		, public std::conditional<!std::is_convertible<_TROFLy*, NotAsyncShareableTagBase*>::value, NotAsyncShareableTagBase, TPlaceHolder_msepointerbasics<TRelaxedRegisteredObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_RELAXED_REGISTERED_OBJ_USING(TRelaxedRegisteredObj, _TROFLy);
		TRelaxedRegisteredObj(const TRelaxedRegisteredObj& _X) : _TROFLy(_X), m_tracker_notifier(*this) {}
		TRelaxedRegisteredObj(TRelaxedRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)), m_tracker_notifier(*this) {}
		virtual ~TRelaxedRegisteredObj() {
			(*trackerPtr()).onObjectDestruction(this);
		}

		template<class _Ty2>
		TRelaxedRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TRelaxedRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TRelaxedRegisteredFixedPointer<_TROFLy> operator&() {
			return TRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
		}
		TRelaxedRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TRelaxedRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this);
		}
		TRelaxedRegisteredFixedPointer<_TROFLy> mse_relaxed_registered_fptr() { return TRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this); }
		TRelaxedRegisteredFixedConstPointer<_TROFLy> mse_relaxed_registered_fptr() const { return TRelaxedRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this); }

		CSPTracker* trackerPtr() const { return &(tlSPTracker_ref<_TROFLy>()); }

	private:
		class CTrackerNotifier {
		public:
			template<typename _TRelaxedRegisteredObj>
			CTrackerNotifier(_TRelaxedRegisteredObj& obj_ref) {
				(*(obj_ref.trackerPtr())).onObjectConstruction(std::addressof(obj_ref));
			}
		};
		CTrackerNotifier m_tracker_notifier;
	};

	template<typename _Ty>
	auto relaxed_registered_fptr_to(_Ty&& _X) {
		return _X.mse_relaxed_registered_fptr();
	}
	template<typename _Ty>
	auto relaxed_registered_fptr_to(const _Ty& _X) {
		return _X.mse_relaxed_registered_fptr();
	}

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TRelaxedRegisteredPointer<_Ty> relaxed_registered_new(Args&&... args) {
		auto a = new TRelaxedRegisteredObj<_Ty>(std::forward<Args>(args)...);
		tlSAllocRegistry_ref<TRelaxedRegisteredObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TRelaxedRegisteredObj<_Ty>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TRelaxedRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::relaxed_registered_delete() \n- tip: If deleting via base class pointer, use mse::us::relaxed_registered_delete() instead. ")); }
		regPtrRef.relaxed_registered_delete();
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TRelaxedRegisteredObj<_Ty>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TRelaxedRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::relaxed_registered_delete() \n- tip: If deleting via base class pointer, use mse::us::relaxed_registered_delete() instead. ")); }
		regPtrRef.relaxed_registered_delete();
	}
	namespace us {
		template <class _Ty>
		void relaxed_registered_delete(const TRelaxedRegisteredPointer<_Ty>& regPtrRef) {
			regPtrRef.relaxed_registered_delete();
		}
		template <class _Ty>
		void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
			regPtrRef.relaxed_registered_delete();
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRelaxedRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TRelaxedRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRelaxedRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

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


#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class CRelaxedRegPtrTest1 {
	public:
		static void s_test1() {
#ifdef MSE_SELF_TESTS
			class C;

			class D {
			public:
				virtual ~D() {}
				mse::TRelaxedRegisteredPointer<C> m_c_ptr = nullptr;
			};

			class C {
			public:
				C() {}
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
				mse::TRelaxedRegisteredObj<FD> relaxed_registered_fd;
				mse::TRelaxedRegisteredPointer<FD> FD_relaxed_registered_ptr1 = &relaxed_registered_fd;
				mse::TRelaxedRegisteredPointer<D> D_relaxed_registered_ptr4 = FD_relaxed_registered_ptr1;
				D_relaxed_registered_ptr4 = &relaxed_registered_fd;
				mse::TRelaxedRegisteredFixedPointer<D> D_relaxed_registered_fptr1 = &relaxed_registered_fd;
				mse::TRelaxedRegisteredFixedConstPointer<D> D_relaxed_registered_fcptr1 = &relaxed_registered_fd;
			}

#endif // MSE_SELF_TESTS
		}
	};

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

}

#undef MSE_THROW

#endif // MSERELAXEDREGISTERED_H_
