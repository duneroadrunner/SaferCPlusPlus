
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREGISTERED_H_
#define MSEREGISTERED_H_

//include "mseprimitives.h"
#include "msepointerbasics.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_REGISTEREDPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

namespace mse {

#ifndef MSE_REGISTERED_DEFAULT_CACHE_SIZE
#define MSE_REGISTERED_DEFAULT_CACHE_SIZE 4/* 1 + (the maximum number of pointers expected to target the object at one time) */
#endif // !MSE_REGISTERED_DEFAULT_CACHE_SIZE

	MSE_CONSTEXPR static const int sc_default_cache_size = MSE_REGISTERED_DEFAULT_CACHE_SIZE;

	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredObj;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredPointer;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredConstPointer;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredNotNullPointer;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredNotNullConstPointer;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredFixedPointer;
	template<typename _Ty, int _Tn = sc_default_cache_size> class TWRegisteredFixedConstPointer;

#ifdef MSE_REGISTEREDPOINTER_DISABLED
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredPointer = _Ty*;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredConstPointer = const _Ty*;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredNotNullPointer = _Ty*;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredNotNullConstPointer = const _Ty*;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredFixedPointer = _Ty* /*const*/; /* Can't be const qualified because standard
																											library containers don't support const elements. */
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROy, int _Tn = sc_default_cache_size> using TRegisteredObj = _TROy;
	template <class _TRRWy, int _TRRWn = sc_default_cache_size> using TRegisteredRefWrapper = std::reference_wrapper<_TRRWy>;
	template <class _Ty, int _Tn = sc_default_cache_size, class... Args>
	TRegisteredPointer<_Ty, _Tn> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty, _Tn>(std::forward<Args>(args)...);
	}
	template <class _Ty, int _Tn = sc_default_cache_size>
	void registered_delete(const TRegisteredPointer<_Ty, _Tn>& regPtrRef) {
		auto a = static_cast<TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
		delete a;
	}
	template <class _Ty, int _Tn = sc_default_cache_size>
	void registered_delete(const TRegisteredConstPointer<_Ty, _Tn>& regPtrRef) {
		auto a = static_cast<const TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
		delete a;
	}
	namespace us {
		template <class _Ty, int _Tn = sc_default_cache_size>
		void registered_delete(const TRegisteredPointer<_Ty, _Tn>& regPtrRef) {
			mse::registered_delete(regPtrRef);
		}
		template <class _Ty, int _Tn = sc_default_cache_size>
		void registered_delete(const TRegisteredConstPointer<_Ty, _Tn>& regPtrRef) {
			mse::registered_delete(regPtrRef);
		}
	}

	template<typename _Ty> auto registered_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto registered_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredPointer = TWRegisteredPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredConstPointer = TWRegisteredConstPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredNotNullPointer = TWRegisteredNotNullPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredNotNullConstPointer = TWRegisteredNotNullConstPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredFixedPointer = TWRegisteredFixedPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using TRegisteredFixedConstPointer = TWRegisteredFixedConstPointer<_Ty, _Tn>;
	template<typename _TROy, int _Tn = sc_default_cache_size> using TRegisteredObj = TWRegisteredObj<_TROy, _Tn>;

	template<typename _Ty>
	auto registered_fptr_to(_Ty&& _X) {
		return _X.mse_registered_fptr();
	}
	template<typename _Ty>
	auto registered_fptr_to(const _Ty& _X) {
		return _X.mse_registered_fptr();
	}

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* TRPTracker is intended to keep track of all the pointers pointing to an object. TRPTracker objects are intended to be always
	associated with (infact, a member of) the one object that is the target of the pointers it tracks. Though at the moment, it
	doesn't need to actually know which object it is associated with. */
	template<int _Tn = sc_default_cache_size>
	class TRPTracker {
	public:
		TRPTracker() {}
		TRPTracker(const TRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		TRPTracker(TRPTracker&& src) { /* see above */ }
		~TRPTracker() {
			if (!fast_mode1()) {
				delete m_ptr_to_regptr_set_ptr;
			}
		}
		TRPTracker& operator=(const TRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		TRPTracker& operator=(TRPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const TRPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const TRPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
				(*m_ptr_to_regptr_set_ptr).insert(item);
#ifdef MSE_REGISTERED_INSTRUMENTATION1
				if ((*m_ptr_to_regptr_set_ptr).size() > m_highest_ptr_to_regptr_set_size) {
					m_highest_ptr_to_regptr_set_size = (*m_ptr_to_regptr_set_ptr).size();
				}
#endif // MSE_REGISTERED_INSTRUMENTATION1
			}
			else {
				if (sc_fm1_max_pointers == m_fm1_num_pointers) {
					/* Too many pointers. Initiate and switch to slow mode. */
					/* Initialize slow storage. */
					m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
					/* First copy the pointers from fast storage to slow storage. */
					for (int i = 0; i < sc_fm1_max_pointers; i += 1) {
						std::unordered_set<const CSaferPtrBase*>::value_type item(m_fm1_ptr_to_regptr_array[i]);
						(*m_ptr_to_regptr_set_ptr).insert(item);
					}
					/* Add the new pointer to slow storage. */
					std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
					(*m_ptr_to_regptr_set_ptr).insert(item);
				}
				else {
#ifdef MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
					/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
					if (1 == sc_fm1_max_pointers) {
						m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
						m_fm1_num_pointers = 1;
					}
					else if (2 == sc_fm1_max_pointers) {
						if (1 == m_fm1_num_pointers) {
							m_fm1_ptr_to_regptr_array[1] = (&sp_ref);
							m_fm1_num_pointers = 2;
						}
						else {
							assert(0 == m_fm1_num_pointers);
							m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
							m_fm1_num_pointers = 1;
						}
					}
					else 
#endif // MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
					{
						m_fm1_ptr_to_regptr_array[m_fm1_num_pointers] = (&sp_ref);
						m_fm1_num_pointers += 1;
					}
				}
			}
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
			else {
#ifdef MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
				}
				else if (2 == sc_fm1_max_pointers){
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else if (2 == m_fm1_num_pointers) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[1]) {
							m_fm1_num_pointers = 1;
						}
						else {
							assert((&sp_ref) == m_fm1_ptr_to_regptr_array[0]);
							m_fm1_ptr_to_regptr_array[0] = m_fm1_ptr_to_regptr_array[1];
							m_fm1_num_pointers = 1;
						}
					}
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
				}
				else 
#endif // MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
				{
					int found_index = -1;
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[i]) {
							found_index = i;
							break;
						}
					}
					if (0 <= found_index) {
						m_fm1_num_pointers -= 1;
						assert(0 <= m_fm1_num_pointers);
						for (int j = found_index; j < m_fm1_num_pointers; j += 1) {
							m_fm1_ptr_to_regptr_array[j] = m_fm1_ptr_to_regptr_array[j + 1];
						}
					}
					else { assert(false); }
				}
			}
		}
		void onObjectDestruction() {
			if (!fast_mode1()) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
			else {
#ifdef MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					}
					else {
						assert(1 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else if (2 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					}
					else if (1 == m_fm1_num_pointers) {
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
					else {
						assert(2 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						(*(m_fm1_ptr_to_regptr_array[1])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else 
#endif // MSE_RP_SPECIAL_CASE_OPTIMIZATIONS
				{
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						(*(m_fm1_ptr_to_regptr_array[i])).setToNull();
					}
					m_fm1_num_pointers = 0;
				}
			}
		}
		void reserve_space_for_one_more() {
			/* The purpose of this function is to ensure that the next call to registerPointer() won't
			need to allocate more memory, and thus won't have any chance of throwing an exception due to
			memory allocation failure. */
			if (!fast_mode1()) {
				(*m_ptr_to_regptr_set_ptr).reserve((*m_ptr_to_regptr_set_ptr).size() + 1);
			}
			else if (sc_fm1_max_pointers == m_fm1_num_pointers) {
				/* At this point, a call to registerPointer() would result in a switch out of fast mode
				and the allocation of an std::unordered_set. We'll trigger that event now by adding and
				removing a placeholder pointer. */
				class CPlaceHolderPtr : public CSaferPtrBase {
				public:
					void setToNull() const {}
				};
				CPlaceHolderPtr placeholder_ptr;
				(*this).registerPointer(placeholder_ptr);
				(*this).unregisterPointer(placeholder_ptr);

				assert(m_ptr_to_regptr_set_ptr);
				//(*m_ptr_to_regptr_set_ptr).reserve((*m_ptr_to_regptr_set_ptr).size() + 1);
			}
		}

		bool fast_mode1() const { return (nullptr == m_ptr_to_regptr_set_ptr); }
		int m_fm1_num_pointers = 0;
		MSE_CONSTEXPR static const int sc_fm1_max_pointers = _Tn;
		const CSaferPtrBase* m_fm1_ptr_to_regptr_array[sc_fm1_max_pointers];

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;

#ifdef MSE_REGISTERED_INSTRUMENTATION1
		size_t m_highest_ptr_to_regptr_set_size = 0;
#endif // MSE_REGISTERED_INSTRUMENTATION1
	};

	/* CSORPTracker is a "size optimized" (smaller and slower) version of CSPTracker. Currently not used. */
	class CSORPTracker {
	public:
		CSORPTracker() {}
		CSORPTracker(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		CSORPTracker(CSORPTracker&& src) { /* see above */ }
		~CSORPTracker() {
			delete m_ptr_to_regptr_set_ptr;
		}
		CSORPTracker& operator=(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		CSORPTracker& operator=(CSORPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const CSORPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const CSORPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
			}
			std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
			(*m_ptr_to_regptr_set_ptr).insert(item);
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				assert(false);
			}
			else {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
		}
		void onObjectDestruction() {
			if (m_ptr_to_regptr_set_ptr) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
		}
		void reserve_space_for_one_more() {
			/* The purpose of this function is to ensure that the next call to registerPointer() won't
			need to allocate more memory, and thus won't have any chance of throwing an exception due to
			memory allocation failure. */
			if (!m_ptr_to_regptr_set_ptr) {
				m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
			}
			(*m_ptr_to_regptr_set_ptr).reserve((*m_ptr_to_regptr_set_ptr).size() + 1);
		}

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	/* TWRegisteredPointer behaves similar to (and is largely compatible with) native pointers. It inherits the safety features of
	TSaferPtr (default nullptr initialization and check for null pointer dereference). In addition, when pointed at a
	TWRegisteredObj, it will be checked for attempted access after destruction. It's essentially intended to be a safe pointer like
	std::shared_ptr, but that does not take ownership of the target object (i.e. does not take responsibility for deallocation).
	Because it does not take ownership, unlike std::shared_ptr, TWRegisteredPointer can be used to point to objects on the stack. */
	template<typename _Ty, int _Tn/* = sc_default_cache_size*/>
	class TWRegisteredPointer : public TSaferPtr<TWRegisteredObj<_Ty, _Tn>> {
	public:
		TWRegisteredPointer();
		TWRegisteredPointer(TWRegisteredObj<_Ty, _Tn>* ptr);
		TWRegisteredPointer(const TWRegisteredPointer& src_cref);
		/* The templated copy constructor accepts other TWRegisteredPointer types if type of their target is "convertible"
		to the target type if this TWRegisteredPointer. Additionally, it accepts TWRegisteredPointer types if their target's
		base class is the "non-const" version of this TWRegisteredPointer's target's base class. */
		template<class _Ty2, class = typename std::enable_if<
			std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value
			, void>::type>
		TWRegisteredPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref);
		virtual ~TWRegisteredPointer();
		TWRegisteredPointer<_Ty, _Tn>& operator=(TWRegisteredObj<_Ty, _Tn>* ptr);
		TWRegisteredPointer<_Ty, _Tn>& operator=(const TWRegisteredPointer<_Ty, _Tn>& _Right_cref);
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const;
		explicit operator TWRegisteredObj<_Ty, _Tn>*() const;
		typename std::conditional<std::is_const<_Ty>::value
			, const TWRegisteredObj<_Ty, _Tn>&, TWRegisteredObj<_Ty, _Tn>&>::type operator*() const {
			return TSaferPtr<TWRegisteredObj<_Ty, _Tn>>::operator*();
		}
		typename std::conditional<std::is_const<_Ty>::value
			, const TWRegisteredObj<_Ty, _Tn>*, TWRegisteredObj<_Ty, _Tn>*>::type operator->() const {
			return TSaferPtr<TWRegisteredObj<_Ty, _Tn>>::operator->();
		}

	private:
		/* If you want a pointer to a TWRegisteredPointer<_Ty, _Tn>, declare the TWRegisteredPointer<_Ty, _Tn> as a
		TWRegisteredObj<TWRegisteredPointer<_Ty, _Tn>> instead. So for example:
		auto reg_ptr = TWRegisteredObj<TWRegisteredPointer<_Ty, _Tn>>(mse::registered_new<_Ty, _Tn>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWRegisteredPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredPointer<_Ty, _Tn>* operator&() const { return this; }
	};

	template<typename _Ty, int _Tn/* = sc_default_cache_size*/>
	class TWRegisteredConstPointer : public TSaferPtr<const TWRegisteredObj<_Ty, _Tn>> {
	public:
		TWRegisteredConstPointer();
		TWRegisteredConstPointer(const TWRegisteredObj<_Ty, _Tn>* ptr);
		TWRegisteredConstPointer(const TWRegisteredConstPointer& src_cref);
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredConstPointer(const TWRegisteredConstPointer<_Ty2, _Tn>& src_cref);
		TWRegisteredConstPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref);
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredConstPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref);
		virtual ~TWRegisteredConstPointer();
		TWRegisteredConstPointer<_Ty, _Tn>& operator=(const TWRegisteredObj<_Ty, _Tn>* ptr);
		TWRegisteredConstPointer<_Ty, _Tn>& operator=(const TWRegisteredConstPointer<_Ty, _Tn>& _Right_cref);
		TWRegisteredConstPointer<_Ty, _Tn>& operator=(const TWRegisteredPointer<_Ty, _Tn>& _Right_cref) { return (*this).operator=(TWRegisteredConstPointer(_Right_cref));  }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const;
		explicit operator const TWRegisteredObj<_Ty, _Tn>*() const;

	private:
		/* If you want a pointer to a TWRegisteredConstPointer<_Ty, _Tn>, declare the TWRegisteredConstPointer<_Ty, _Tn> as a
		TWRegisteredObj<TWRegisteredConstPointer<_Ty, _Tn>> instead. So for example:
		auto reg_ptr = TWRegisteredObj<TWRegisteredConstPointer<_Ty, _Tn>>(mse::registered_new<_Ty, _Tn>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TWRegisteredConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TWRegisteredNotNullConstPointer<_Ty, _Tn>;
	};

	template<typename _Ty, int _Tn>
	class TWRegisteredNotNullPointer : public TWRegisteredPointer<_Ty, _Tn> {
	public:
		TWRegisteredNotNullPointer(const TWRegisteredNotNullPointer& src_cref) : TWRegisteredPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TWRegisteredNotNullPointer(const TWRegisteredNotNullPointer<_Ty2, _Tn>& src_cref) : TWRegisteredPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredNotNullPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref) : TWRegisteredPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TWRegisteredNotNullPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref) : TWRegisteredPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		virtual ~TWRegisteredNotNullPointer() {}
		TWRegisteredNotNullPointer<_Ty, _Tn>& operator=(const TWRegisteredNotNullPointer<_Ty, _Tn>& _Right_cref) {
			TWRegisteredPointer<_Ty, _Tn>::operator=(_Right_cref);
			return (*this);
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRegisteredPointer<_Ty, _Tn>::operator _Ty*(); }
		explicit operator TWRegisteredObj<_Ty, _Tn>*() const { return TWRegisteredPointer<_Ty, _Tn>::operator TWRegisteredObj<_Ty, _Tn>*(); }

	private:
		TWRegisteredNotNullPointer(TWRegisteredObj<_Ty, _Tn>* ptr) : TWRegisteredPointer<_Ty, _Tn>(ptr) {}

		TWRegisteredNotNullPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredNotNullPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TWRegisteredFixedPointer<_Ty, _Tn>;
	};

	template<typename _Ty, int _Tn>
	class TWRegisteredNotNullConstPointer : public TWRegisteredConstPointer<_Ty, _Tn> {
	public:
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullConstPointer<_Ty, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullConstPointer<_Ty2, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {}
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullPointer<_Ty, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredNotNullPointer<_Ty2, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredNotNullConstPointer(const TWRegisteredConstPointer<_Ty, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredConstPointer<_Ty2, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWRegisteredNotNullConstPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredNotNullConstPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref) : TWRegisteredConstPointer<_Ty, _Tn>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		virtual ~TWRegisteredNotNullConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRegisteredConstPointer<_Ty, _Tn>::operator const _Ty*(); }
		explicit operator const TWRegisteredObj<_Ty, _Tn>*() const { return TWRegisteredConstPointer<_Ty, _Tn>::operator const TWRegisteredObj<_Ty, _Tn>*(); }

	private:
		TWRegisteredNotNullConstPointer(const TWRegisteredObj<_Ty, _Tn>* ptr) : TWRegisteredConstPointer<_Ty, _Tn>(ptr) {}

		TWRegisteredNotNullConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredNotNullConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TWRegisteredFixedConstPointer<_Ty, _Tn>;
	};

	/* TWRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty, int _Tn>
	class TWRegisteredFixedPointer : public TWRegisteredNotNullPointer<_Ty, _Tn> {
	public:
		TWRegisteredFixedPointer(const TWRegisteredFixedPointer& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TWRegisteredFixedPointer(const TWRegisteredFixedPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredFixedPointer(const TWRegisteredNotNullPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TWRegisteredFixedPointer(const TWRegisteredNotNullPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredFixedPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TWRegisteredFixedPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullPointer<_Ty, _Tn>(src_cref) {}

		virtual ~TWRegisteredFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWRegisteredNotNullPointer<_Ty, _Tn>::operator _Ty*(); }
		explicit operator TWRegisteredObj<_Ty, _Tn>*() const { return TWRegisteredNotNullPointer<_Ty, _Tn>::operator TWRegisteredObj<_Ty, _Tn>*(); }

	private:
		TWRegisteredFixedPointer(TWRegisteredObj<_Ty, _Tn>* ptr) : TWRegisteredNotNullPointer<_Ty, _Tn>(ptr) {}
		TWRegisteredFixedPointer<_Ty, _Tn>& operator=(const TWRegisteredFixedPointer<_Ty, _Tn>& _Right_cref) = delete;

		TWRegisteredFixedPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredFixedPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TWRegisteredObj<_Ty, _Tn>;
	};

	template<typename _Ty, int _Tn>
	class TWRegisteredFixedConstPointer : public TWRegisteredNotNullConstPointer<_Ty, _Tn> {
	public:
		TWRegisteredFixedConstPointer(const TWRegisteredFixedConstPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredFixedConstPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		TWRegisteredFixedConstPointer(const TWRegisteredFixedPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredFixedPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredFixedConstPointer(const TWRegisteredNotNullConstPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullConstPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredNotNullPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}

		TWRegisteredFixedConstPointer(const TWRegisteredConstPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredConstPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		TWRegisteredFixedConstPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<TWRegisteredObj<_Ty2, _Tn> *, TWRegisteredObj<_Ty, _Tn> *>::value, void>::type>
		TWRegisteredFixedConstPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(src_cref) {}

		virtual ~TWRegisteredFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWRegisteredNotNullConstPointer<_Ty, _Tn>::operator const _Ty*(); }
		explicit operator const TWRegisteredObj<_Ty, _Tn>*() const { return TWRegisteredNotNullConstPointer<_Ty, _Tn>::operator const TWRegisteredObj<_Ty, _Tn>*(); }

	private:
		TWRegisteredFixedConstPointer(const TWRegisteredObj<_Ty, _Tn>* ptr) : TWRegisteredNotNullConstPointer<_Ty, _Tn>(ptr) {}
		TWRegisteredFixedConstPointer<_Ty, _Tn>& operator=(const TWRegisteredFixedConstPointer<_Ty, _Tn>& _Right_cref) = delete;

		TWRegisteredFixedConstPointer<_Ty, _Tn>* operator&() { return this; }
		const TWRegisteredFixedConstPointer<_Ty, _Tn>* operator&() const { return this; }

		friend class TWRegisteredObj<_Ty, _Tn>;
	};


	/* TWRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWRegisteredPointers will avoid referencing destroyed objects. Note that TWRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _TROy, int _Tn>
	class TWRegisteredObj : public _TROy , public std::conditional<(!std::is_convertible<_TROy*, NotAsyncShareableTagBase*>::value) && (!std::is_base_of<NotAsyncShareableTagBase, _TROy>::value)
		, NotAsyncShareableTagBase, TPlaceHolder_msepointerbasics<TWRegisteredObj<_TROy, _Tn> > >::type
	{
	public:
		MSE_USING(TWRegisteredObj, _TROy);
		TWRegisteredObj(const TWRegisteredObj& _X) : _TROy(_X) {}
		TWRegisteredObj(TWRegisteredObj&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TWRegisteredObj() {
			mseRPManager().onObjectDestruction();
		}

		TWRegisteredObj& operator=(TWRegisteredObj&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		TWRegisteredObj& operator=(const TWRegisteredObj& _X) { _TROy::operator=(_X); return (*this); }
		template<class _Ty2>
		TWRegisteredObj& operator=(_Ty2&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWRegisteredObj& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

		TWRegisteredFixedPointer<_TROy, _Tn> operator&() {
			return this;
		}
		TWRegisteredFixedConstPointer<_TROy, _Tn> operator&() const {
			return this;
		}
		TWRegisteredFixedPointer<_TROy> mse_registered_fptr() { return this; }
		TWRegisteredFixedConstPointer<_TROy> mse_registered_fptr() const { return this; }

		TRPTracker<_Tn>& mseRPManager() const { return m_mseRPManager; }

	private:
		mutable TRPTracker<_Tn> m_mseRPManager;
	};


	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::TWRegisteredPointer() : TSaferPtr<TWRegisteredObj<_Ty, _Tn>>() {}
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::TWRegisteredPointer(TWRegisteredObj<_Ty, _Tn>* ptr) : TSaferPtr<TWRegisteredObj<_Ty, _Tn>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::TWRegisteredPointer(const TWRegisteredPointer& src_cref) : TSaferPtr<TWRegisteredObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	template<class _Ty2, class>
	TWRegisteredPointer<_Ty, _Tn>::TWRegisteredPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref)
		/* We need to use a reinterpret_cast for the cases when this TWRegisteredPointer's target's base class is
		a "const" type, and src_cref's target's base class is just the "non-const" version. Should be cool to use
		a reinterpret_cast when you're just adding a const qualifier to a member or base class, right? */
		: TSaferPtr<TWRegisteredObj<_Ty, _Tn>>(reinterpret_cast<TWRegisteredObj<_Ty, _Tn> *>(src_cref.m_ptr)) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::~TWRegisteredPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>& TWRegisteredPointer<_Ty, _Tn>::operator=(TWRegisteredObj<_Ty, _Tn>* ptr) {
		if (nullptr != ptr) {
			/* Here we're just ensuring that the registerPointer() call won't throw an exception (due to
			memory allocation failure).*/
			(*ptr).mseRPManager().reserve_space_for_one_more();

			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
			}
			TSaferPtr<TWRegisteredObj<_Ty, _Tn>>::operator=(ptr);
			(*ptr).mseRPManager().registerPointer(*this);
		}
		else {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
			}
			TSaferPtr<TWRegisteredObj<_Ty, _Tn>>::operator=(ptr);
		}
		return (*this);
	}
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>& TWRegisteredPointer<_Ty, _Tn>::operator=(const TWRegisteredPointer<_Ty, _Tn>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::operator _Ty*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
		return (*this).m_ptr;
	}
	/* This cast operator, if possible, should not be used. It is meant to be used exclusively by registered_delete<>(). */
	template<typename _Ty, int _Tn>
	TWRegisteredPointer<_Ty, _Tn>::operator TWRegisteredObj<_Ty, _Tn>*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
		return (*this).m_ptr;
	}


	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer() : TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>() {}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer(const TWRegisteredObj<_Ty, _Tn>* ptr) : TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer(const TWRegisteredConstPointer& src_cref) : TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	template<class _Ty2, class>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer(const TWRegisteredConstPointer<_Ty2, _Tn>& src_cref) : TSaferPtr<TWRegisteredObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer(const TWRegisteredPointer<_Ty, _Tn>& src_cref) : TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	template<class _Ty2, class>
	TWRegisteredConstPointer<_Ty, _Tn>::TWRegisteredConstPointer(const TWRegisteredPointer<_Ty2, _Tn>& src_cref) : TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>(src_cref.m_ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::~TWRegisteredConstPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>& TWRegisteredConstPointer<_Ty, _Tn>::operator=(const TWRegisteredObj<_Ty, _Tn>* ptr) {
		if (nullptr != ptr) {
			/* Here we're just ensuring that the registerPointer() call won't throw an exception (due to
			memory allocation failure).*/
			(*ptr).mseRPManager().reserve_space_for_one_more();

			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
			}
			TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>::operator=(ptr);
			(*ptr).mseRPManager().registerPointer(*this);
		}
		else {
			if (nullptr != (*this).m_ptr) {
				(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
			}
			TSaferPtr<const TWRegisteredObj<_Ty, _Tn>>::operator=(ptr);
		}
		return (*this);
	}
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>& TWRegisteredConstPointer<_Ty, _Tn>::operator=(const TWRegisteredConstPointer<_Ty, _Tn>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::operator const _Ty*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
		return (*this).m_ptr;
	}
	/* This cast operator, if possible, should not be used. It is meant to be used exclusively by registered_delete<>(). */
	template<typename _Ty, int _Tn>
	TWRegisteredConstPointer<_Ty, _Tn>::operator const TWRegisteredObj<_Ty, _Tn>*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
		return (*this).m_ptr;
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TWRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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

	template<typename _Ty, int _Tn>
	class TWRegisteredObj<_Ty*, _Tn> : public TWRegisteredObj<mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredObj<mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredObj<_Ty* const, _Tn> : public TWRegisteredObj<const mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredObj<const mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredObj<const _Ty *, _Tn> : public TWRegisteredObj<mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredObj<mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredObj<const _Ty * const, _Tn> : public TWRegisteredObj<const mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredObj<const mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};

	template<typename _Ty, int _Tn>
	class TWRegisteredPointer<_Ty*, _Tn> : public TWRegisteredPointer<mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredPointer<mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredPointer<_Ty* const, _Tn> : public TWRegisteredPointer<const mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredPointer<const mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredPointer<const _Ty *, _Tn> : public TWRegisteredPointer<mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredPointer<mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredPointer<const _Ty * const, _Tn> : public TWRegisteredPointer<const mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredPointer<const mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};

	template<typename _Ty, int _Tn>
	class TWRegisteredConstPointer<_Ty*, _Tn> : public TWRegisteredConstPointer<mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredConstPointer<_Ty* const, _Tn> : public TWRegisteredConstPointer<const mse::TPointer<_Ty>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<const mse::TPointer<_Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredConstPointer<const _Ty *, _Tn> : public TWRegisteredConstPointer<mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<typename _Ty, int _Tn>
	class TWRegisteredConstPointer<const _Ty * const, _Tn> : public TWRegisteredConstPointer<const mse::TPointer<const _Ty>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<const mse::TPointer<const _Ty>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<int _Tn>
	class TWRegisteredObj<int, _Tn> : public TWRegisteredObj<mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredObj<mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<int _Tn>
	class TWRegisteredObj<const int, _Tn> : public TWRegisteredObj<const mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredObj<const mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<int _Tn>
	class TWRegisteredPointer<int, _Tn> : public TWRegisteredPointer<mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredPointer<mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredPointer<const int, _Tn> : public TWRegisteredPointer<const mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredPointer<const mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredConstPointer<int, _Tn> : public TWRegisteredConstPointer<mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredConstPointer<const int, _Tn> : public TWRegisteredConstPointer<const mse::TInt<int>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<const mse::TInt<int>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};

	template<int _Tn>
	class TWRegisteredObj<size_t, _Tn> : public TWRegisteredObj<mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredObj<mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<int _Tn>
	class TWRegisteredObj<const size_t, _Tn> : public TWRegisteredObj<const mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredObj<const mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredObj, base_class);
	};
	template<int _Tn>
	class TWRegisteredPointer<size_t, _Tn> : public TWRegisteredPointer<mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredPointer<mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredPointer<const size_t, _Tn> : public TWRegisteredPointer<const mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredPointer<const mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredConstPointer<size_t, _Tn> : public TWRegisteredConstPointer<mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
	template<int _Tn>
	class TWRegisteredConstPointer<const size_t, _Tn> : public TWRegisteredConstPointer<const mse::TInt<size_t>, _Tn> {
	public:
		typedef TWRegisteredConstPointer<const mse::TInt<size_t>, _Tn> base_class;
		MSE_USING(TWRegisteredConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4127 )
#endif /*_MSC_VER*/

	/* CSAllocRegistry essentially just maintains a list of all objects allocated by a registered "new" call and not (yet)
	subsequently deallocated with a corresponding registered delete. */
	class CSAllocRegistry {
	public:
		CSAllocRegistry() {}
		~CSAllocRegistry() {}
		bool registerPointer(void *alloc_ptr) {
			if (nullptr == alloc_ptr) { return true; }
			{
				if (1 <= sc_fs1_max_objects) {
					/* We'll add this object to fast storage. */
					if (sc_fs1_max_objects == m_num_fs1_objects) {
						/* Too many objects. We're gonna move the oldest object to slow storage. */
						moveObjectFromFastStorage1ToSlowStorage(0);
					}
					auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
					fs1_object_ref = alloc_ptr;
					m_num_fs1_objects += 1;
					return true;
				}
				else {
					/* Add the mapping to slow storage. */
					std::unordered_set<CFS1Object>::value_type item(alloc_ptr);
					m_pointer_set.insert(item);
				}
			}
			return true;
		}
		bool unregisterPointer(void *alloc_ptr) {
			if (nullptr == alloc_ptr) { return true; }
			bool retval = false;
			{
				/* check if the object is in "fast storage 1" first */
				for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
					if (alloc_ptr == m_fs1_objects[i]) {
						removeObjectFromFastStorage1(i);
						return true;
					}
				}

				/* The object was not in "fast storage 1". It's proably in "slow storage". */
				auto num_erased = m_pointer_set.erase(alloc_ptr);
				if (1 <= num_erased) {
					retval = true;
				}
			}
			return retval;
		}
		bool registerPointer(const void *alloc_ptr) { return (*this).registerPointer(const_cast<void *>(alloc_ptr)); }
		bool unregisterPointer(const void *alloc_ptr) { return (*this).unregisterPointer(const_cast<void *>(alloc_ptr)); }
		void reserve_space_for_one_more() {
			/* The purpose of this function is to ensure that the next call to registerPointer() won't
			need to allocate more memory, and thus won't have any chance of throwing an exception due to
			memory allocation failure. */
			m_pointer_set.reserve(m_pointer_set.size() + 1);
		}

		bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_pointer_set.size())); }

	private:
		/* So this tracker stores the allocation pointers in either "fast storage1" or "slow storage". The code for
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
			std::unordered_set<CFS1Object>::value_type item(fs1_object_ref);
			m_pointer_set.insert(fs1_object_ref);
			/* Then we're gonna remove the object from fast storage */
			removeObjectFromFastStorage1(fs1_obj_index);
		}

		typedef void* CFS1Object;

#ifndef MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
#define MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */
#endif // !MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
		MSE_CONSTEXPR static const int sc_fs1_max_objects = MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS;
		CFS1Object m_fs1_objects[sc_fs1_max_objects];
		int m_num_fs1_objects = 0;

		/* "slow storage" */
		std::unordered_set<CFS1Object> m_pointer_set;
	};

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

	template<typename _Ty>
	inline CSAllocRegistry& tlSAllocRegistry_ref() {
		thread_local static CSAllocRegistry tlSAllocRegistry;
		return tlSAllocRegistry;
	}

#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* registered_new is intended to be analogous to std::make_shared */
	template <class _Ty, int _Tn = sc_default_cache_size, class... Args>
	TRegisteredPointer<_Ty, _Tn> registered_new(Args&&... args) {
		auto retval = new TRegisteredObj<_Ty, _Tn>(std::forward<Args>(args)...);
		tlSAllocRegistry_ref<TRegisteredObj<_Ty, _Tn> >().registerPointer(retval);
		return retval;
	}
	template <class _Ty, int _Tn = sc_default_cache_size>
	void registered_delete(const TRegisteredPointer<_Ty, _Tn>& regPtrRef) {
		auto a = static_cast<TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TRegisteredObj<_Ty, _Tn> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::registered_delete() \n- tip: If deleting via base class pointer, use mse::us::registered_delete() instead. ")); }
		delete a;
	}
	template <class _Ty, int _Tn = sc_default_cache_size>
	void registered_delete(const TRegisteredConstPointer<_Ty, _Tn>& regPtrRef) {
		auto a = static_cast<const TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
		auto res = tlSAllocRegistry_ref<TRegisteredObj<_Ty, _Tn> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::registered_delete() \n- tip: If deleting via base class pointer, use mse::us::registered_delete() instead. ")); }
		delete a;
	}
	namespace us {
		template <class _Ty, int _Tn = sc_default_cache_size>
		void registered_delete(const TRegisteredPointer<_Ty, _Tn>& regPtrRef) {
			auto a = static_cast<TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
			tlSAllocRegistry_ref<TRegisteredObj<_Ty, _Tn> >().unregisterPointer(a);
			delete a;
		}
		template <class _Ty, int _Tn = sc_default_cache_size>
		void registered_delete(const TRegisteredConstPointer<_Ty, _Tn>& regPtrRef) {
			auto a = static_cast<const TRegisteredObj<_Ty, _Tn>*>(regPtrRef);
			tlSAllocRegistry_ref<TRegisteredObj<_Ty, _Tn> >().unregisterPointer(a);
			delete a;
		}
	}
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

#ifdef MSE_REGISTEREDPOINTER_DISABLED
#else /*MSE_REGISTEREDPOINTER_DISABLED*/

#ifdef _MSC_VER
#if (1900 <= _MSC_VER)
#define MSEREGISTEREDREFWRAPPER 1
#endif // (1900 <= _MSC_VER)
#else /*_MSC_VER*/
#define MSEREGISTEREDREFWRAPPER 1
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBLE 1
#else /*(defined(__GNUC__) || defined(__GNUG__))*/
#ifdef __clang__
#define CLANG_COMPATIBLE 1
#endif // __clang__
#endif /*(defined(__GNUC__) || defined(__GNUG__))*/
#endif /*_MSC_VER*/

#ifdef MSEREGISTEREDREFWRAPPER
	template <class _TRRWy, int _TRRWn = sc_default_cache_size>
	class TRegisteredRefWrapper : public NotAsyncShareableTagBase {
	public:
		// types
		typedef TRegisteredObj<_TRRWy, _TRRWn> type;

		// construct/copy/destroy
		TRegisteredRefWrapper(TRegisteredObj<_TRRWy, _TRRWn>& ref) : _ptr(&ref) {}
		TRegisteredRefWrapper(TRegisteredObj<_TRRWy, _TRRWn>&&) = delete;
		TRegisteredRefWrapper(const TRegisteredRefWrapper&) = default;

		// assignment
		TRegisteredRefWrapper& operator=(const TRegisteredRefWrapper& x) = default;

		// access
		operator TRegisteredObj<_TRRWy, _TRRWn>& () const { return *_ptr; }
		TRegisteredObj<_TRRWy, _TRRWn>& get() const { return *_ptr; }

		template< class... ArgTypes >
		typename std::result_of<TRegisteredObj<_TRRWy, _TRRWn>&(ArgTypes&&...)>::type
			operator() (ArgTypes&&... args) const {
#if defined(GPP_COMPATIBLE) || defined(CLANG_COMPATIBLE)
			return __invoke(get(), std::forward<ArgTypes>(args)...);
#else // defined(GPP_COMPATIBLE) || definded(CLANG_COMPATIBLE)
			return std::invoke(get(), std::forward<ArgTypes>(args)...);
#endif // defined(GPP_COMPATIBLE) || definded(CLANG_COMPATIBLE)
		}

	private:
		TRegisteredPointer<_TRRWy, _TRRWn> _ptr;
	};
#endif // MSEREGISTEREDREFWRAPPER

#endif /*MSE_REGISTEREDPOINTER_DISABLED*/

	/* shorter aliases */
	template<typename _Ty, int _Tn = sc_default_cache_size> using rp = TRegisteredPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using rcp = TRegisteredConstPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using rnnp = TRegisteredNotNullPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using rnncp = TRegisteredNotNullConstPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using rfp = TRegisteredFixedPointer<_Ty, _Tn>;
	template<typename _Ty, int _Tn = sc_default_cache_size> using rfcp = TRegisteredFixedConstPointer<_Ty, _Tn>;
	template<typename _TROy, int _Tn = sc_default_cache_size> using ro = TRegisteredObj<_TROy, _Tn>;
	template <class _Ty, int _Tn = sc_default_cache_size, class... Args>
	TRegisteredPointer<_Ty, _Tn> rnew(Args&&... args) { return registered_new<_Ty, _Tn>(std::forward<Args>(args)...); }
	template <class _Ty, int _Tn = sc_default_cache_size>
	void rdelete(const TRegisteredPointer<_Ty, _Tn>& regPtrRef) { registered_delete<_Ty, _Tn>(regPtrRef); }

	/* These functions create a registered object assuming the type is the same as the lone parameter, so you don't need
	to explicitly specify it. */
	template <typename _TLoneParam>
	auto mkrolp(const _TLoneParam& lone_param) {
		return TRegisteredObj<_TLoneParam>(lone_param);
	}
	template <typename _TLoneParam>
	auto mkrolp(_TLoneParam&& lone_param) {
		return TRegisteredObj<_TLoneParam>(std::forward<decltype(lone_param)>(lone_param));
	}

	/* deprecated aliases */
	template<class _TTargetType, class _TLeasePointerType> using swkfp = TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>;
	template<class _TTargetType, class _TLeasePointerType> using swkfcp = TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType>;


#ifdef MSEREGISTEREDREFWRAPPER
	template <class _TRRWy, int _TRRWn = sc_default_cache_size> using rrw = TRegisteredRefWrapper<_TRRWy, _TRRWn>;

	// TEMPLATE FUNCTIONS ref AND cref
	template<class _TRRy, int _TRRn = sc_default_cache_size> inline
		TRegisteredRefWrapper<_TRRy, _TRRn>
		registered_ref(TRegisteredObj<_TRRy, _TRRn>& _Val)
	{	// create TRegisteredRefWrapper<_TRRy, _TRRn> object
		return (TRegisteredRefWrapper<_TRRy, _TRRn>(_Val));
	}

	template<class _TRRy, int _TRRn = sc_default_cache_size>
	void registered_ref(const TRegisteredObj<_TRRy, _TRRn>&&) = delete;

	template<class _TRRy, int _TRRn = sc_default_cache_size> inline
		TRegisteredRefWrapper<_TRRy, _TRRn>
		registered_ref(TRegisteredRefWrapper<_TRRy, _TRRn> _Val)
	{	// create TRegisteredRefWrapper<_TRRy, _TRRn> object
		return (registered_ref(_Val.get()));
	}

	template<class _TRCRy, int _TRCRn = sc_default_cache_size> inline
		TRegisteredRefWrapper<const _TRCRy, _TRCRn>
		registered_cref(const TRegisteredObj<_TRCRy, _TRCRn>& _Val)
	{	// create TRegisteredRefWrapper<const _TRCRy, _TRCRn> object
		return (TRegisteredRefWrapper<const _TRCRy, _TRCRn>(_Val));
	}

	template<class _TRCRy, int _TRCRn = sc_default_cache_size>
	void registered_cref(const TRegisteredObj<_TRCRy, _TRCRn>&&) = delete;

	template<class _TRCRy, int _TRCRn = sc_default_cache_size> inline
		TRegisteredRefWrapper<const _TRCRy, _TRCRn>
		registered_cref(TRegisteredRefWrapper<_TRCRy, _TRCRn> _Val)
	{	// create TRegisteredRefWrapper<const _TRCRy, _TRCRn> object
		return (registered_cref(_Val.get()));
	}
#endif // MSEREGISTEREDREFWRAPPER


#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

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

	class CRegPtrTest1 {
	public:
		static void s_test1() {
#ifdef MSE_SELF_TESTS
			class A {
			public:
				A() {}
				A(const A& _X) : b(_X.b) {}
				A(A&& _X) : b(std::forward<decltype(_X.b)>(_X.b)) {}
				virtual ~A() {}
				A& operator=(A&& _X) { b = std::forward<decltype(_X.b)>(_X.b); return (*this); }
				A& operator=(const A& _X) { b = _X.b; return (*this); }

				int b = 3;
			};
			class B {
			public:
				static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
				static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
			protected:
				~B() {}
			};

			A* A_native_ptr = nullptr;
			/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
			mse::TRegisteredPointer<A> A_registered_ptr1;

			{
				A a;
				mse::TRegisteredObj<A> registered_a;
				/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

				assert(a.b == registered_a.b);
				A_native_ptr = &a;
				A_registered_ptr1 = &registered_a;
				assert(A_native_ptr->b == A_registered_ptr1->b);

				mse::TRegisteredPointer<A> A_registered_ptr2 = &registered_a;
				A_registered_ptr2 = nullptr;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
				bool expected_exception = false;
				try {
					int i = A_registered_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "registered pointer". */
				}
				assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED

				/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
				B::foo1(static_cast<A*>(A_registered_ptr1));

				if (A_registered_ptr2) {
					assert(false);
				}
				else if (A_registered_ptr2 != A_registered_ptr1) {
					A_registered_ptr2 = A_registered_ptr1;
					assert(A_registered_ptr2 == A_registered_ptr1);
				}
				else {
					assert(false);
				}

				A a2 = a;
				mse::TRegisteredObj<A> registered_a2 = registered_a;

				a2 = A();
				registered_a2 = mse::TRegisteredObj<A>();

				A a3((A()));
				mse::TRegisteredObj<A> registered_a3((A()));
				{
					mse::TRegisteredObj<A> registered_a4((mse::TRegisteredObj<A>()));
				}

				mse::TRegisteredConstPointer<A> rcp = A_registered_ptr1;
				mse::TRegisteredConstPointer<A> rcp2 = rcp;
				const mse::TRegisteredObj<A> cregistered_a;
				rcp = &cregistered_a;
				mse::TRegisteredFixedConstPointer<A> rfcp = &cregistered_a;
				rcp = mse::registered_new<A>();
				mse::registered_delete<A>(rcp);
			}

			bool expected_exception = false;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
			try {
				/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
				int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
			}
			catch (...) {
				//std::cerr << "expected exception" << std::endl;
				expected_exception = true;
			}
			assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED

			{
				/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
			mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
				auto A_registered_ptr3 = mse::registered_new<A>();
				assert(3 == A_registered_ptr3->b);
				mse::registered_delete<A>(A_registered_ptr3);
				bool expected_exception = false;
#ifndef MSE_REGISTEREDPOINTER_DISABLED
				try {
					/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
					int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
				}
				catch (...) {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
				}
				assert(expected_exception);
#endif // !MSE_REGISTEREDPOINTER_DISABLED
			}

			{
				/* Remember that registered pointers can only point to registered objects. So, for example, if you want
			a registered pointer to an object's base class object, that base class object has to be a registered
			object. */
				class DA : public mse::TRegisteredObj<A> {};
				mse::TRegisteredObj<DA> registered_da;
				mse::TRegisteredPointer<DA> DA_registered_ptr1 = &registered_da;
				mse::TRegisteredPointer<A> A_registered_ptr4 = DA_registered_ptr1;
				A_registered_ptr4 = &registered_da;
				mse::TRegisteredFixedPointer<A> A_registered_fptr1 = &registered_da;
				mse::TRegisteredFixedConstPointer<A> A_registered_fcptr1 = &registered_da;
			}

			{
				/* Obtaining safe pointers to members of registered objects: */
				class E {
				public:
					virtual ~E() {}
					mse::TRegisteredObj<std::string> reg_s = "some text ";
					std::string s2 = "some other text ";
				};

				mse::TRegisteredObj<E> registered_e;
				mse::TRegisteredPointer<E> E_registered_ptr1 = &registered_e;

				/* To obtain a safe pointer to a member of a registered object you could just make the
			member itself a registered object. */
				mse::TRegisteredPointer<std::string> reg_s_registered_ptr1 = &(E_registered_ptr1->reg_s);

				/* Or you can use the "mse::make_pointer_to_member()" function. */
				auto s2_safe_ptr1 = mse::make_pointer_to_member(E_registered_ptr1->s2, E_registered_ptr1);
				(*s2_safe_ptr1) = "some new text";
				auto s2_safe_const_ptr1 = mse::make_const_pointer_to_member(E_registered_ptr1->s2, E_registered_ptr1);

				/* Just testing the convertibility of mse::TSyncWeakFixedPointers. */
				auto E_registered_fixed_ptr1 = &registered_e;
				auto swfptr1 = mse::make_syncweak<std::string>(E_registered_fixed_ptr1->s2, E_registered_fixed_ptr1);
				mse::TSyncWeakFixedPointer<std::string, mse::TRegisteredPointer<E>> swfptr2 = swfptr1;
				mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredFixedPointer<E>> swfcptr1 = swfptr1;
				mse::TSyncWeakFixedConstPointer<std::string, mse::TRegisteredPointer<E>> swfcptr2 = swfcptr1;
				if (swfcptr1 == swfptr1) {
					int q = 7;
				}
				if (swfptr1 == swfcptr1) {
					int q = 7;
				}
				if (swfptr1) {
					int q = 7;
				}
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

}

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#undef MSE_THROW

#endif // MSEREGISTERED_H_
