
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

	template<typename _Ty> class TWRelaxedRegisteredObj;
	template<typename _Ty> class TWRelaxedRegisteredPointer;
	template<typename _Ty> class TWRelaxedRegisteredConstPointer;
	template<typename _Ty> class TWRelaxedRegisteredNotNullPointer;
	template<typename _Ty> class TWRelaxedRegisteredNotNullConstPointer;
	template<typename _Ty> class TWRelaxedRegisteredFixedPointer;
	template<typename _Ty> class TWRelaxedRegisteredFixedConstPointer;

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
		auto a = static_cast<TRelaxedRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	template <class _Ty>
	void relaxed_registered_delete(const TRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TRelaxedRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	namespace us {
		template <class _Ty>
		void relaxed_registered_delete(const TWRelaxedRegisteredPointer<_Ty>& regPtrRef) {
			mse::relaxed_registered_delete(regPtrRef);
		}
		template <class _Ty>
		void relaxed_registered_delete(const TWRelaxedRegisteredConstPointer<_Ty>& regPtrRef) {
			mse::relaxed_registered_delete(regPtrRef);
		}
	}

	template<typename _Ty> auto relaxed_registered_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto relaxed_registered_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	class relaxed_registered_cannot_verify_cast_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> using TRelaxedRegisteredPointer = TWRelaxedRegisteredPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredConstPointer = TWRelaxedRegisteredConstPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredNotNullPointer = TWRelaxedRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredNotNullConstPointer = TWRelaxedRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredFixedPointer = TWRelaxedRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using TRelaxedRegisteredFixedConstPointer = TWRelaxedRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TRelaxedRegisteredObj = TWRelaxedRegisteredObj<_TROFLy>;

	template<typename _Ty>
	auto relaxed_registered_fptr_to(_Ty&& _X) {
		return _X.mse_relaxed_registered_fptr();
	}
	template<typename _Ty>
	auto relaxed_registered_fptr_to(const _Ty& _X) {
		return _X.mse_relaxed_registered_fptr();
	}

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

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

	/* TWRelaxedRegisteredPointer is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
	when replacing native pointers with "registered" pointers in legacy code, it may be the case that fewer code changes
	(explicit casts) will be required when using this template. */
	template<typename _Ty>
	class TWRelaxedRegisteredPointer : public TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>> {
	public:
		TWRelaxedRegisteredPointer() : TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TWRelaxedRegisteredPointer(const TWRelaxedRegisteredPointer& src_cref) : TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWRelaxedRegisteredPointer(std::nullptr_t) : TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TWRelaxedRegisteredPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TWRelaxedRegisteredPointer<_Ty>& operator=(const TWRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredPointer<_Ty>& operator=(const TWRelaxedRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWRelaxedRegisteredPointer(_Right_cref));
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
			auto a = asANativePointerToTWRelaxedRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TWRelaxedRegisteredPointer(CSPTracker* sp_tracker_ptr, TWRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<TWRelaxedRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		TWRelaxedRegisteredObj<_Ty>* asANativePointerToTWRelaxedRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TWRelaxedRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TWRelaxedRegisteredPointer;
		template <class Y> friend class TWRelaxedRegisteredConstPointer;
		friend class TWRelaxedRegisteredNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TWRelaxedRegisteredConstPointer : public TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>> {
	public:
		TWRelaxedRegisteredConstPointer() : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TWRelaxedRegisteredConstPointer(const TWRelaxedRegisteredConstPointer& src_cref) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredConstPointer(const TWRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWRelaxedRegisteredConstPointer(const TWRelaxedRegisteredPointer<_Ty>& src_cref) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredConstPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWRelaxedRegisteredConstPointer(std::nullptr_t) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TWRelaxedRegisteredConstPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TWRelaxedRegisteredConstPointer<_Ty>& operator=(const TWRelaxedRegisteredConstPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredConstPointer<_Ty>& operator=(const TWRelaxedRegisteredConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWRelaxedRegisteredConstPointer(_Right_cref));
		}
		TWRelaxedRegisteredConstPointer<_Ty>& operator=(const TWRelaxedRegisteredPointer<_Ty>& _Right_cref) {
			return (*this).operator=(TWRelaxedRegisteredConstPointer<_Ty>(_Right_cref));
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredConstPointer<_Ty>& operator=(const TWRelaxedRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWRelaxedRegisteredPointer<_Ty>(_Right_cref));
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
			auto a = asANativePointerToTWRelaxedRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TWRelaxedRegisteredConstPointer(CSPTracker* sp_tracker_ptr, const TWRelaxedRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<const TWRelaxedRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by relaxed_registered_delete<>(). */
		const TWRelaxedRegisteredObj<_Ty>* asANativePointerToTWRelaxedRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TWRelaxedRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TWRelaxedRegisteredConstPointer;
		friend class TWRelaxedRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TWRelaxedRegisteredNotNullPointer : public TWRelaxedRegisteredPointer<_Ty> {
	public:
		TWRelaxedRegisteredNotNullPointer(const TWRelaxedRegisteredNotNullPointer& src_cref) : TWRelaxedRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullPointer(const TWRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TWRelaxedRegisteredPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredNotNullPointer(const  TWRelaxedRegisteredPointer<_Ty>& src_cref) : TWRelaxedRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TWRelaxedRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		virtual ~TWRelaxedRegisteredNotNullPointer() {}
		/*
		TWRelaxedRegisteredNotNullPointer<_Ty>& operator=(const TWRelaxedRegisteredNotNullPointer<_Ty>& _Right_cref) {
			TWRelaxedRegisteredPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		*/
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRelaxedRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator TWRelaxedRegisteredObj<_Ty>*() const { return TWRelaxedRegisteredPointer<_Ty>::operator TWRelaxedRegisteredObj<_Ty>*(); }

	private:
		TWRelaxedRegisteredNotNullPointer(CSPTracker* sp_tracker_ptr, TWRelaxedRegisteredObj<_Ty>* ptr) : TWRelaxedRegisteredPointer<_Ty>(sp_tracker_ptr, ptr) {}

		/* If you want a pointer to a TWRelaxedRegisteredNotNullPointer<_Ty>, declare the TWRelaxedRegisteredNotNullPointer<_Ty> as a
		TWRelaxedRegisteredObj<TWRelaxedRegisteredNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWRelaxedRegisteredObj<TWRelaxedRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWRelaxedRegisteredNotNullPointer<_Ty>* operator&() {
			return this;
		}
		const TWRelaxedRegisteredNotNullPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TWRelaxedRegisteredFixedPointer<_Ty>;
	};

	template<typename _Ty>
	class TWRelaxedRegisteredNotNullConstPointer : public TWRelaxedRegisteredConstPointer<_Ty> {
	public:
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredNotNullConstPointer<_Ty>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredPointer<_Ty>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredConstPointer<_Ty>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredNotNullConstPointer(const TWRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TWRelaxedRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		virtual ~TWRelaxedRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRelaxedRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWRelaxedRegisteredObj<_Ty>*() const { return TWRelaxedRegisteredConstPointer<_Ty>::operator const TWRelaxedRegisteredObj<_Ty>*(); }

	private:
		TWRelaxedRegisteredNotNullConstPointer(CSPTracker* sp_tracker_ptr, const TWRelaxedRegisteredObj<_Ty>* ptr) : TWRelaxedRegisteredConstPointer<_Ty>(sp_tracker_ptr, ptr) {}

		TWRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() { return this; }
		const TWRelaxedRegisteredNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TWRelaxedRegisteredFixedConstPointer<_Ty>;
	};

	/* TWRelaxedRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TWRelaxedRegisteredFixedPointer : public TWRelaxedRegisteredNotNullPointer<_Ty> {
	public:
		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredFixedPointer& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredFixedPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullPointer<_Ty>(src_cref) {}

		virtual ~TWRelaxedRegisteredFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRelaxedRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TWRelaxedRegisteredObj<_Ty>*() const { return TWRelaxedRegisteredNotNullPointer<_Ty>::operator TWRelaxedRegisteredObj<_Ty>*(); }

	private:
		TWRelaxedRegisteredFixedPointer(CSPTracker* sp_tracker_ptr, TWRelaxedRegisteredObj<_Ty>* ptr) : TWRelaxedRegisteredNotNullPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TWRelaxedRegisteredFixedPointer<_Ty>& operator=(const TWRelaxedRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TWRelaxedRegisteredFixedPointer<_Ty>, declare the TWRelaxedRegisteredFixedPointer<_Ty> as a
		TWRelaxedRegisteredObj<TWRelaxedRegisteredFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWRelaxedRegisteredObj<TWRelaxedRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWRelaxedRegisteredFixedPointer<_Ty>* operator&() {
			return this;
		}
		const TWRelaxedRegisteredFixedPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TWRelaxedRegisteredObj<_Ty>;
	};

	template<typename _Ty>
	class TWRelaxedRegisteredFixedConstPointer : public TWRelaxedRegisteredNotNullConstPointer<_Ty> {
	public:
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredFixedPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredFixedPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredFixedConstPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredFixedConstPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredNotNullPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredNotNullPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredNotNullConstPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredConstPointer<_Ty>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWRelaxedRegisteredFixedConstPointer(const TWRelaxedRegisteredConstPointer<_Ty2>& src_cref) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		virtual ~TWRelaxedRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRelaxedRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWRelaxedRegisteredObj<_Ty>*() const { return TWRelaxedRegisteredNotNullConstPointer<_Ty>::operator const TWRelaxedRegisteredObj<_Ty>*(); }

	private:
		TWRelaxedRegisteredFixedConstPointer(CSPTracker* sp_tracker_ptr, const TWRelaxedRegisteredObj<_Ty>* ptr) : TWRelaxedRegisteredNotNullConstPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TWRelaxedRegisteredFixedConstPointer<_Ty>& operator=(const TWRelaxedRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

		TWRelaxedRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
		const TWRelaxedRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TWRelaxedRegisteredObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_RELAXED_REGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...), m_tracker_notifier(*this) {}

	/* TWRelaxedRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWRelaxedRegisteredPointers will avoid referencing destroyed objects. Note that TWRelaxedRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TWRelaxedRegisteredObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, NotAsyncShareableTagBase*>::value) && (!std::is_base_of<NotAsyncShareableTagBase, _TROFLy>::value)
		, NotAsyncShareableTagBase, TPlaceHolder_msepointerbasics<TWRelaxedRegisteredObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_RELAXED_REGISTERED_OBJ_USING(TWRelaxedRegisteredObj, _TROFLy);
		TWRelaxedRegisteredObj(const TWRelaxedRegisteredObj& _X) : _TROFLy(_X), m_tracker_notifier(*this) {}
		TWRelaxedRegisteredObj(TWRelaxedRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)), m_tracker_notifier(*this) {}
		virtual ~TWRelaxedRegisteredObj() {
			(*trackerPtr()).onObjectDestruction(this);
		}

		template<class _Ty2>
		TWRelaxedRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWRelaxedRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TWRelaxedRegisteredFixedPointer<_TROFLy> operator&() {
			return TWRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
		}
		TWRelaxedRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TWRelaxedRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this);
		}
		TWRelaxedRegisteredFixedPointer<_TROFLy> mse_relaxed_registered_fptr() { return TWRelaxedRegisteredFixedPointer<_TROFLy>(trackerPtr(), this); }
		TWRelaxedRegisteredFixedConstPointer<_TROFLy> mse_relaxed_registered_fptr() const { return TWRelaxedRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this); }

		CSPTracker* trackerPtr() const { return &(tlSPTracker_ref<_TROFLy>()); }

	private:
		class CTrackerNotifier {
		public:
			template<typename _TWRelaxedRegisteredObj>
			CTrackerNotifier(_TWRelaxedRegisteredObj& obj_ref) {
				(*(obj_ref.trackerPtr())).onObjectConstruction(std::addressof(obj_ref));
			}
		};
		CTrackerNotifier m_tracker_notifier;
	};


#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

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
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

}

namespace std {
	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRelaxedRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRelaxedRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRelaxedRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	/* template specializations */

	template<typename _Ty>
	class TWRelaxedRegisteredObj<_Ty*> : public TWRelaxedRegisteredObj<mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredObj<mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredObj<_Ty* const> : public TWRelaxedRegisteredObj<const mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredObj<const mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredObj<const _Ty *> : public TWRelaxedRegisteredObj<mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredObj<mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredObj<const _Ty * const> : public TWRelaxedRegisteredObj<const mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredObj<const mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};

	template<typename _Ty>
	class TWRelaxedRegisteredPointer<_Ty*> : public TWRelaxedRegisteredPointer<mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredPointer<mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredPointer<_Ty* const> : public TWRelaxedRegisteredPointer<const mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredPointer<const mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredPointer<const _Ty *> : public TWRelaxedRegisteredPointer<mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredPointer<mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredPointer<const _Ty * const> : public TWRelaxedRegisteredPointer<const mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredPointer<const mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};

	template<typename _Ty>
	class TWRelaxedRegisteredConstPointer<_Ty*> : public TWRelaxedRegisteredConstPointer<mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredConstPointer<_Ty* const> : public TWRelaxedRegisteredConstPointer<const mse::TPointer<_Ty>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<const mse::TPointer<_Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredConstPointer<const _Ty *> : public TWRelaxedRegisteredConstPointer<mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWRelaxedRegisteredConstPointer<const _Ty * const> : public TWRelaxedRegisteredConstPointer<const mse::TPointer<const _Ty>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<const mse::TPointer<const _Ty>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TWRelaxedRegisteredObj<int> : public TWRelaxedRegisteredObj<mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredObj<mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<>
	class TWRelaxedRegisteredObj<const int> : public TWRelaxedRegisteredObj<const mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredObj<const mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<>
	class TWRelaxedRegisteredPointer<int> : public TWRelaxedRegisteredPointer<mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredPointer<mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredPointer<const int> : public TWRelaxedRegisteredPointer<const mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredConstPointer<int> : public TWRelaxedRegisteredConstPointer<mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredConstPointer<const int> : public TWRelaxedRegisteredConstPointer<const mse::TInt<int>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};

	template<>
	class TWRelaxedRegisteredObj<size_t> : public TWRelaxedRegisteredObj<mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredObj<mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<>
	class TWRelaxedRegisteredObj<const size_t> : public TWRelaxedRegisteredObj<const mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredObj, base_class);
	};
	template<>
	class TWRelaxedRegisteredPointer<size_t> : public TWRelaxedRegisteredPointer<mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredPointer<const size_t> : public TWRelaxedRegisteredPointer<const mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredConstPointer<size_t> : public TWRelaxedRegisteredConstPointer<mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
	template<>
	class TWRelaxedRegisteredConstPointer<const size_t> : public TWRelaxedRegisteredConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TWRelaxedRegisteredConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWRelaxedRegisteredConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

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
