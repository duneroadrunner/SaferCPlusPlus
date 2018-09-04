
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* TCRegisteredPointers are basically just like TRegisteredPointers except that unlike TRegisteredPointers, they
are able to reference target object types before those target types are (fully) defined. This is required to support
"mutually referencing" objects or "cyclic references".

The implementation difference is that with TRegisteredPointers, the "pointer tracking registry" is located in the target
object, whereas TCRegisteredPointers use (thread local) global registries that track all the pointers targeting
objects of a given type. */

#pragma once
#ifndef MSECREGISTERED_H_
#define MSECREGISTERED_H_

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

	template<typename _Ty> class TWCRegisteredObj;
	template<typename _Ty> class TWCRegisteredPointer;
	template<typename _Ty> class TWCRegisteredConstPointer;
	template<typename _Ty> class TWCRegisteredNotNullPointer;
	template<typename _Ty> class TWCRegisteredNotNullConstPointer;
	template<typename _Ty> class TWCRegisteredFixedPointer;
	template<typename _Ty> class TWCRegisteredFixedConstPointer;

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty> using TCRegisteredPointer = _Ty*;
	template<typename _Ty> using TCRegisteredConstPointer = const _Ty*;
	template<typename _Ty> using TCRegisteredNotNullPointer = _Ty*;
	template<typename _Ty> using TCRegisteredNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TCRegisteredFixedPointer = _Ty* /*const*/; /* Can't be const qualified because standard
																				  library containers don't support const elements. */
	template<typename _Ty> using TCRegisteredFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROFLy> using TCRegisteredObj = _TROFLy;
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> cregistered_new(Args&&... args) {
		return new TCRegisteredObj<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TCRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TCRegisteredObj<_Ty>*>(regPtrRef);
		delete a;
	}
	namespace us {
		template <class _Ty>
		void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
			mse::cregistered_delete(regPtrRef);
		}
		template <class _Ty>
		void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
			mse::cregistered_delete(regPtrRef);
		}
	}

	template<typename _Ty> auto cregistered_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto cregistered_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	class cregistered_cannot_verify_cast_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> using TCRegisteredPointer = TWCRegisteredPointer<_Ty>;
	template<typename _Ty> using TCRegisteredConstPointer = TWCRegisteredConstPointer<_Ty>;
	template<typename _Ty> using TCRegisteredNotNullPointer = TWCRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using TCRegisteredNotNullConstPointer = TWCRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using TCRegisteredFixedPointer = TWCRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using TCRegisteredFixedConstPointer = TWCRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TCRegisteredObj = TWCRegisteredObj<_TROFLy>;

	template<typename _Ty>
	auto cregistered_fptr_to(_Ty&& _X) {
		return _X.mse_cregistered_fptr();
	}
	template<typename _Ty>
	auto cregistered_fptr_to(const _Ty& _X) {
		return _X.mse_cregistered_fptr();
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

	/* TWCRegisteredPointer is similar to TWRegisteredPointer but uses a different implementation that allows it to be be declared
	before its target type is fully defined. (This is necessary to support mutual and cyclic references.) It's also generally more
	memory efficient. But maybe a bit slower in some cases. */
	template<typename _Ty>
	class TWCRegisteredPointer : public TSaferPtr<TWCRegisteredObj<_Ty>> {
	public:
		TWCRegisteredPointer() : TSaferPtr<TWCRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TWCRegisteredPointer(const TWCRegisteredPointer& src_cref) : TSaferPtr<TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TSaferPtr<TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWCRegisteredPointer(std::nullptr_t) : TSaferPtr<TWCRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TWCRegisteredPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TWCRegisteredPointer<_Ty>& operator=(const TWCRegisteredPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtr<TWCRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredPointer<_Ty>& operator=(const TWCRegisteredPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWCRegisteredPointer(_Right_cref));
		}
		operator bool() const { return (*this).m_ptr; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
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
		void cregistered_delete() const {
			auto a = asANativePointerToTWCRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TWCRegisteredPointer(CSPTracker* sp_tracker_ptr, TWCRegisteredObj<_Ty>* ptr) : TSaferPtr<TWCRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by cregistered_delete<>(). */
		TWCRegisteredObj<_Ty>* asANativePointerToTWCRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TWCRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TWCRegisteredPointer;
		template <class Y> friend class TWCRegisteredConstPointer;
		friend class TWCRegisteredNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredConstPointer : public TSaferPtr<const TWCRegisteredObj<_Ty>> {
	public:
		TWCRegisteredConstPointer() : TSaferPtr<const TWCRegisteredObj<_Ty>>() {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		TWCRegisteredConstPointer(const TWCRegisteredConstPointer& src_cref) : TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer(const TWCRegisteredConstPointer<_Ty2>& src_cref) : TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWCRegisteredConstPointer(const TWCRegisteredPointer<_Ty>& src_cref) : TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TSaferPtr<const TWCRegisteredObj<_Ty>>(src_cref.m_ptr) {
			//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
		}
		TWCRegisteredConstPointer(std::nullptr_t) : TSaferPtr<const TWCRegisteredObj<_Ty>>(nullptr) {
			m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
		}
		virtual ~TWCRegisteredConstPointer() {
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
		}
		TWCRegisteredConstPointer<_Ty>& operator=(const TWCRegisteredConstPointer<_Ty>& _Right_cref) {
			(*m_sp_tracker_ptr).reserve_space_for_one_more();
			(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			TSaferPtr<const TWCRegisteredObj<_Ty>>::operator=(_Right_cref);
			//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredConstPointer<_Ty>& operator=(const TWCRegisteredConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWCRegisteredConstPointer(_Right_cref));
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
		void cregistered_delete() const {
			auto a = asANativePointerToTWCRegisteredObj();
			delete a;
			assert(nullptr == (*this).m_ptr);
		}

	private:
		TWCRegisteredConstPointer(CSPTracker* sp_tracker_ptr, const TWCRegisteredObj<_Ty>* ptr) : TSaferPtr<const TWCRegisteredObj<_Ty>>(ptr) {
			m_sp_tracker_ptr = sp_tracker_ptr;
			(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by cregistered_delete<>(). */
		const TWCRegisteredObj<_Ty>* asANativePointerToTWCRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TWCRegisteredObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CSPTracker* m_sp_tracker_ptr = nullptr;

		template <class Y> friend class TWCRegisteredConstPointer;
		friend class TWCRegisteredNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredNotNullPointer : public TWCRegisteredPointer<_Ty> {
	public:
		TWCRegisteredNotNullPointer(const TWCRegisteredNotNullPointer& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredNotNullPointer() {}
		/*
		TWCRegisteredNotNullPointer<_Ty>& operator=(const TWCRegisteredNotNullPointer<_Ty>& _Right_cref) {
			TWCRegisteredPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		*/
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWCRegisteredPointer<_Ty>::operator _Ty*(); }
		explicit operator TWCRegisteredObj<_Ty>*() const { return TWCRegisteredPointer<_Ty>::operator TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredNotNullPointer(CSPTracker* sp_tracker_ptr, TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredPointer<_Ty>(sp_tracker_ptr, ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWCRegisteredNotNullPointer(const  TWCRegisteredPointer<_Ty>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TWCRegisteredPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		/* If you want a pointer to a TWCRegisteredNotNullPointer<_Ty>, declare the TWCRegisteredNotNullPointer<_Ty> as a
		TWCRegisteredObj<TWCRegisteredNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWCRegisteredObj<TWCRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWCRegisteredNotNullPointer<_Ty>* operator&() {
			return this;
		}
		const TWCRegisteredNotNullPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TWCRegisteredFixedPointer<_Ty>;
		template<typename _Ty2>
		friend TWCRegisteredNotNullPointer<_Ty2> not_null_from_nullable(const TWCRegisteredPointer<_Ty2>& src);
	};

	template<typename _Ty>
	class TWCRegisteredNotNullConstPointer : public TWCRegisteredConstPointer<_Ty> {
	public:
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullConstPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWCRegisteredConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWCRegisteredObj<_Ty>*() const { return TWCRegisteredConstPointer<_Ty>::operator const TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredNotNullConstPointer(CSPTracker* sp_tracker_ptr, const TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredConstPointer<_Ty>(sp_tracker_ptr, ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWCRegisteredNotNullConstPointer(const TWCRegisteredPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWCRegisteredNotNullConstPointer(const TWCRegisteredConstPointer<_Ty>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredNotNullConstPointer(const TWCRegisteredConstPointer<_Ty2>& src_cref) : TWCRegisteredConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		TWCRegisteredNotNullConstPointer<_Ty>* operator&() { return this; }
		const TWCRegisteredNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TWCRegisteredFixedConstPointer<_Ty>;
		template<typename _Ty2>
		friend TWCRegisteredNotNullConstPointer<_Ty2> not_null_from_nullable(const TWCRegisteredConstPointer<_Ty2>& src);
	};

	template<typename _Ty>
	TWCRegisteredNotNullPointer<_Ty> not_null_from_nullable(const TWCRegisteredPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TWCRegisteredNotNullConstPointer<_Ty> not_null_from_nullable(const TWCRegisteredConstPointer<_Ty>& src) {
		return src;
	}

	/* TWCRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TWCRegisteredFixedPointer : public TWCRegisteredNotNullPointer<_Ty> {
	public:
		TWCRegisteredFixedPointer(const TWCRegisteredFixedPointer& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedPointer(const TWCRegisteredFixedPointer<_Ty2>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}

		TWCRegisteredFixedPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredNotNullPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWCRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TWCRegisteredObj<_Ty>*() const { return TWCRegisteredNotNullPointer<_Ty>::operator TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredFixedPointer(CSPTracker* sp_tracker_ptr, TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredNotNullPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TWCRegisteredFixedPointer<_Ty>& operator=(const TWCRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TWCRegisteredFixedPointer<_Ty>, declare the TWCRegisteredFixedPointer<_Ty> as a
		TWCRegisteredObj<TWCRegisteredFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWCRegisteredObj<TWCRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWCRegisteredFixedPointer<_Ty>* operator&() {
			return this;
		}
		const TWCRegisteredFixedPointer<_Ty>* operator&() const {
			return this;
		}

		friend class TWCRegisteredObj<_Ty>;
	};

	template<typename _Ty>
	class TWCRegisteredFixedConstPointer : public TWCRegisteredNotNullConstPointer<_Ty> {
	public:
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedConstPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredFixedConstPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullConstPointer<_Ty>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWCRegisteredFixedConstPointer(const TWCRegisteredNotNullConstPointer<_Ty2>& src_cref) : TWCRegisteredNotNullConstPointer<_Ty>(src_cref) {}

		virtual ~TWCRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWCRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWCRegisteredObj<_Ty>*() const { return TWCRegisteredNotNullConstPointer<_Ty>::operator const TWCRegisteredObj<_Ty>*(); }

	private:
		TWCRegisteredFixedConstPointer(CSPTracker* sp_tracker_ptr, const TWCRegisteredObj<_Ty>* ptr) : TWCRegisteredNotNullConstPointer<_Ty>(sp_tracker_ptr, ptr) {}
		TWCRegisteredFixedConstPointer<_Ty>& operator=(const TWCRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

		TWCRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
		const TWCRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TWCRegisteredObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_CREGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...), m_tracker_notifier(*this) {}

	/* TWCRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWCRegisteredPointers will avoid referencing destroyed objects. Note that TWCRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TWCRegisteredObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, NotAsyncShareableTagBase*>::value) && (!std::is_base_of<NotAsyncShareableTagBase, _TROFLy>::value)
		, NotAsyncShareableTagBase, TPlaceHolder_msepointerbasics<TWCRegisteredObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_CREGISTERED_OBJ_USING(TWCRegisteredObj, _TROFLy);
		TWCRegisteredObj(const TWCRegisteredObj& _X) : _TROFLy(_X), m_tracker_notifier(*this) {}
		TWCRegisteredObj(TWCRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)), m_tracker_notifier(*this) {}
		virtual ~TWCRegisteredObj() {
			(*trackerPtr()).onObjectDestruction(this);
		}

		template<class _Ty2>
		TWCRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWCRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TWCRegisteredFixedPointer<_TROFLy> operator&() {
			return TWCRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
		}
		TWCRegisteredFixedConstPointer<_TROFLy> operator&() const {
			return TWCRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this);
		}
		TWCRegisteredFixedPointer<_TROFLy> mse_cregistered_fptr() { return TWCRegisteredFixedPointer<_TROFLy>(trackerPtr(), this); }
		TWCRegisteredFixedConstPointer<_TROFLy> mse_cregistered_fptr() const { return TWCRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this); }

		CSPTracker* trackerPtr() const { return &(tlSPTracker_ref<_TROFLy>()); }

	private:
		class CTrackerNotifier {
		public:
			template<typename _TWCRegisteredObj>
			CTrackerNotifier(_TWCRegisteredObj& obj_ref) {
				(*(obj_ref.trackerPtr())).onObjectConstruction(std::addressof(obj_ref));
			}
		};
		CTrackerNotifier m_tracker_notifier;
	};


#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> cregistered_new(Args&&... args) {
		auto a = new TCRegisteredObj<_Ty>(std::forward<Args>(args)...);
		tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TCRegisteredObj<_Ty>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::cregistered_delete() \n- tip: If deleting via base class pointer, use mse::us::cregistered_delete() instead. ")); }
		regPtrRef.cregistered_delete();
	}
	template <class _Ty>
	void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TCRegisteredObj<_Ty>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TCRegisteredObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::cregistered_delete() \n- tip: If deleting via base class pointer, use mse::us::cregistered_delete() instead. ")); }
		regPtrRef.cregistered_delete();
	}
	namespace us {
		template <class _Ty>
		void cregistered_delete(const TCRegisteredPointer<_Ty>& regPtrRef) {
			regPtrRef.cregistered_delete();
		}
		template <class _Ty>
		void cregistered_delete(const TCRegisteredConstPointer<_Ty>& regPtrRef) {
			regPtrRef.cregistered_delete();
		}
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

}

namespace std {
	template<class _Ty>
	struct hash<mse::TWCRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWCRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWCRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWCRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWCRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
	class TWCRegisteredObj<_Ty*> : public TWCRegisteredObj<mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredObj<mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<_Ty* const> : public TWCRegisteredObj<const mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredObj<const mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<const _Ty *> : public TWCRegisteredObj<mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredObj<mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredObj<const _Ty * const> : public TWCRegisteredObj<const mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredObj<const mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};

	template<typename _Ty>
	class TWCRegisteredPointer<_Ty*> : public TWCRegisteredPointer<mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredPointer<mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<_Ty* const> : public TWCRegisteredPointer<const mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredPointer<const mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<const _Ty *> : public TWCRegisteredPointer<mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredPointer<mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredPointer<const _Ty * const> : public TWCRegisteredPointer<const mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredPointer<const mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};

	template<typename _Ty>
	class TWCRegisteredConstPointer<_Ty*> : public TWCRegisteredConstPointer<mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<_Ty* const> : public TWCRegisteredConstPointer<const mse::TPointerForLegacy<_Ty>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<const _Ty *> : public TWCRegisteredConstPointer<mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<typename _Ty>
	class TWCRegisteredConstPointer<const _Ty * const> : public TWCRegisteredConstPointer<const mse::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TWCRegisteredObj<int> : public TWCRegisteredObj<mse::TInt<int>> {
	public:
		typedef TWCRegisteredObj<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredObj<const int> : public TWCRegisteredObj<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredObj<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredPointer<int> : public TWCRegisteredPointer<mse::TInt<int>> {
	public:
		typedef TWCRegisteredPointer<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredPointer<const int> : public TWCRegisteredPointer<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<int> : public TWCRegisteredConstPointer<mse::TInt<int>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<const int> : public TWCRegisteredConstPointer<const mse::TInt<int>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};

	template<>
	class TWCRegisteredObj<size_t> : public TWCRegisteredObj<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredObj<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredObj<const size_t> : public TWCRegisteredObj<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredObj, base_class);
	};
	template<>
	class TWCRegisteredPointer<size_t> : public TWCRegisteredPointer<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredPointer<const size_t> : public TWCRegisteredPointer<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<size_t> : public TWCRegisteredConstPointer<mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
	template<>
	class TWCRegisteredConstPointer<const size_t> : public TWCRegisteredConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TWCRegisteredConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWCRegisteredConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#if defined(MSE_REGISTEREDPOINTER_DISABLED)
	/* Omit definition of make_pointer_to_member() as it would clash with the one already defined in mseregistered.h. */
#define MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER
#endif // defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TCRegisteredPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TCRegisteredPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TCRegisteredPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, TCRegisteredConstPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TCRegisteredConstPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, TCRegisteredConstPointer<_Ty>>::make(target, lease_pointer);
	}
#endif // !defined(MSE_CREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)

	/* shorter aliases */
	template<typename _Ty> using rrp = TCRegisteredPointer<_Ty>;
	template<typename _Ty> using rrcp = TCRegisteredConstPointer<_Ty>;
	template<typename _Ty> using rrnnp = TCRegisteredNotNullPointer<_Ty>;
	template<typename _Ty> using rrnncp = TCRegisteredNotNullConstPointer<_Ty>;
	template<typename _Ty> using rrfp = TCRegisteredFixedPointer<_Ty>;
	template<typename _Ty> using rrfcp = TCRegisteredFixedConstPointer<_Ty>;
	template<typename _TROFLy> using rro = TCRegisteredObj<_TROFLy>;
	template <class _Ty, class... Args>
	TCRegisteredPointer<_Ty> rrnew(Args&&... args) { return cregistered_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty>
	void rrdelete(const TCRegisteredPointer<_Ty>& regPtrRef) { cregistered_delete<_Ty>(regPtrRef); }


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

	class CCRegPtrTest1 {
	public:
		static void s_test1() {
#ifdef MSE_SELF_TESTS
			class C;

			class D {
			public:
				virtual ~D() {}
				mse::TCRegisteredPointer<C> m_c_ptr = nullptr;
			};

			class C {
			public:
				C() {}
				mse::TCRegisteredPointer<D> m_d_ptr = nullptr;
			};

			mse::TCRegisteredObj<C> regobjfl_c;
			mse::TCRegisteredPointer<D> d_ptr = mse::cregistered_new<D>();

			regobjfl_c.m_d_ptr = d_ptr;
			d_ptr->m_c_ptr = &regobjfl_c;

			mse::TCRegisteredConstPointer<C> rrcp = d_ptr->m_c_ptr;
			mse::TCRegisteredConstPointer<C> rrcp2 = rrcp;
			const mse::TCRegisteredObj<C> regobjfl_e;
			rrcp = &regobjfl_e;
			mse::TCRegisteredFixedConstPointer<C> rrfcp = &regobjfl_e;
			rrcp = mse::cregistered_new<C>();
			mse::cregistered_delete<C>(rrcp);

			mse::cregistered_delete<D>(d_ptr);

			{
				/* Polymorphic conversions. */
				class FD : public mse::TCRegisteredObj<D> {};
				mse::TCRegisteredObj<FD> cregistered_fd;
				mse::TCRegisteredPointer<FD> FD_cregistered_ptr1 = &cregistered_fd;
				mse::TCRegisteredPointer<D> D_cregistered_ptr4 = FD_cregistered_ptr1;
				D_cregistered_ptr4 = &cregistered_fd;
				mse::TCRegisteredFixedPointer<D> D_cregistered_fptr1 = &cregistered_fd;
				mse::TCRegisteredFixedConstPointer<D> D_cregistered_fcptr1 = &cregistered_fd;
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

#endif // MSECREGISTERED_H_
