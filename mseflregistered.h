
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* "fl" registered pointers are similar to relaxed registered pointers but additionally support construction 
and assignment from raw pointers. In cases where the raw pointer does not point to an "fl" registered object, 
the memory safety mechanism will not be able to ensure memory safety. "fl" registered pointers might be useful
during a transition of legacy code to a safer implementation, but they generally would not be used in new code. */

#pragma once
#ifndef MSEFLREGISTERED_H_
#define MSEFLREGISTERED_H_

//include "mseprimitives.h"
#include "msepointerbasics.h"
#include "mserelaxedregistered.h"
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
	namespace us {

#ifdef MSE_REGISTEREDPOINTER_DISABLED
		template<typename _Ty> using TFLRegisteredPointer = _Ty * ;
		template<typename _Ty> using TFLRegisteredConstPointer = const _Ty*;
		template<typename _Ty> using TFLRegisteredNotNullPointer = _Ty * ;
		template<typename _Ty> using TFLRegisteredNotNullConstPointer = const _Ty*;
		template<typename _Ty> using TFLRegisteredFixedPointer = _Ty * ;
		template<typename _Ty> using TFLRegisteredFixedConstPointer = const _Ty*;
		template<typename _TROFLy> using TFLRegisteredObj = _TROFLy;
		template <class _Ty, class... Args>
		TFLRegisteredPointer<_Ty> fl_registered_new(Args&&... args) {
			return new TFLRegisteredObj<_Ty>(std::forward<Args>(args)...);
		}
		template <class _Ty>
		void fl_registered_delete(const TFLRegisteredPointer<_Ty>& regPtrRef) {
			auto a = (TFLRegisteredObj<_Ty>*)regPtrRef;
			delete a;
		}
		template <class _Ty>
		void fl_registered_delete(const TFLRegisteredConstPointer<_Ty>& regPtrRef) {
			auto a = (const TFLRegisteredObj<_Ty>*)regPtrRef;
			delete a;
		}

		template<typename _Ty> auto fl_registered_fptr_to(_Ty&& _X) { return &_X; }
		template<typename _Ty> auto fl_registered_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_REGISTEREDPOINTER_DISABLED*/

		class fl_registered_cannot_verify_cast_error : public std::logic_error {
		public:
			using std::logic_error::logic_error;
		};


		template<typename _Ty> class TFLRegisteredObj;
		template<typename _Ty> class TFLRegisteredConstPointer;
		template<typename _Ty> class TFLRegisteredNotNullPointer;
		template<typename _Ty> class TFLRegisteredNotNullConstPointer;
		template<typename _Ty> class TFLRegisteredFixedPointer;
		template<typename _Ty> class TFLRegisteredFixedConstPointer;

		/* TFLRegisteredPointer is similar to TRegisteredPointer, but more readily converts to a native pointer implicitly. So
		when replacing native pointers with "registered" pointers in legacy code, it may be the case that fewer code changes
		(explicit casts) will be required when using this template. */
		template<typename _Ty>
		class TFLRegisteredPointer : public TSaferPtrForLegacy<_Ty> {
		public:
			TFLRegisteredPointer() : TSaferPtrForLegacy<_Ty>() {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			}
			TFLRegisteredPointer(const TFLRegisteredPointer& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<_Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				//m_might_not_point_to_a_TFLRegisteredObj = true;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			TFLRegisteredPointer(std::nullptr_t) : TSaferPtrForLegacy<_Ty>(nullptr) {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			}
			TFLRegisteredPointer(_Ty* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_might_not_point_to_a_TFLRegisteredObj = true;
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			}
			TFLRegisteredPointer(CSPTracker* sp_tracker_ptr, TFLRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<_Ty>(ptr) {
				m_sp_tracker_ptr = sp_tracker_ptr;
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			}
			virtual ~TFLRegisteredPointer() {
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			}
			TFLRegisteredPointer<_Ty>& operator=(const TFLRegisteredPointer<_Ty>& _Right_cref) {
				(*m_sp_tracker_ptr).reserve_space_for_one_more();
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
				TSaferPtrForLegacy<_Ty>::operator=(_Right_cref);
				//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
				m_might_not_point_to_a_TFLRegisteredObj = _Right_cref.m_might_not_point_to_a_TFLRegisteredObj;
				return (*this);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredPointer<_Ty>& operator=(const TFLRegisteredPointer<_Ty2>& _Right_cref) {
				return (*this).operator=(TFLRegisteredPointer(_Right_cref));
			}
			TFLRegisteredPointer<_Ty>& operator=(_Ty* ptr) {
				(*m_sp_tracker_ptr).reserve_space_for_one_more();
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
				TSaferPtrForLegacy<_Ty>::operator=(ptr);
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
				m_might_not_point_to_a_TFLRegisteredObj = true;
				return (*this);
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
			void fl_registered_delete() const {
				if (m_might_not_point_to_a_TFLRegisteredObj) {
					/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
					TFLRegisteredPointer<_Ty> to point to a _Ty instead of a TFLRegisteredObj<_Ty>. But in those
					situations it doesn't make sense that someone would be calling this delete function. */
					//_Ty* a = this;
					_Ty* a = (*this).m_ptr;
					(*m_sp_tracker_ptr).onObjectDestruction(a);
					delete a;
					(*this).setToNull();
				}
				else {
					auto a = asANativePointerToTFLRegisteredObj();
					delete a;
					assert(nullptr == (*this).m_ptr);
				}
			}

		private:
			/* This function, if possible, should not be used. It is meant to be used exclusively by fl_registered_delete<>(). */
			TFLRegisteredObj<_Ty>* asANativePointerToTFLRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
				if (nullptr == (*this).m_ptr) {
					int q = 5; /* just a line of code for putting a debugger break point */
				}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
				if (m_might_not_point_to_a_TFLRegisteredObj) { MSE_THROW(fl_registered_cannot_verify_cast_error("cannot verify cast validity - mse::us::TFLRegisteredPointer")); }
				return static_cast<TFLRegisteredObj<_Ty>*>((*this).m_ptr);
			}

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			CSPTracker* m_sp_tracker_ptr = nullptr;
			bool m_might_not_point_to_a_TFLRegisteredObj = false;

			template <class Y> friend class TFLRegisteredPointer;
			template <class Y> friend class TFLRegisteredConstPointer;
			friend class TFLRegisteredNotNullPointer<_Ty>;
		};

		template<typename _Ty>
		class TFLRegisteredConstPointer : public TSaferPtrForLegacy<const _Ty> {
		public:
			TFLRegisteredConstPointer() : TSaferPtrForLegacy<const _Ty>() {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			}
			TFLRegisteredConstPointer(const TFLRegisteredConstPointer& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredConstPointer(const TFLRegisteredConstPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				//m_might_not_point_to_a_TFLRegisteredObj = true;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			TFLRegisteredConstPointer(const TFLRegisteredPointer<_Ty>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredConstPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TSaferPtrForLegacy<const _Ty>(src_cref.m_ptr) {
				//m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_sp_tracker_ptr = src_cref.m_sp_tracker_ptr;
				m_might_not_point_to_a_TFLRegisteredObj = src_cref.m_might_not_point_to_a_TFLRegisteredObj;
				(*m_sp_tracker_ptr).registerPointer((*this), src_cref.m_ptr);
			}
			TFLRegisteredConstPointer(std::nullptr_t) : TSaferPtrForLegacy<const _Ty>(nullptr) {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
			}
			TFLRegisteredConstPointer(const _Ty* ptr) : TSaferPtrForLegacy<const _Ty>(ptr) {
				m_sp_tracker_ptr = &(tlSPTracker_ref<_Ty>());
				m_might_not_point_to_a_TFLRegisteredObj = true;
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			}
			TFLRegisteredConstPointer(CSPTracker* sp_tracker_ptr, const TFLRegisteredObj<_Ty>* ptr) : TSaferPtrForLegacy<const _Ty>(ptr) {
				m_sp_tracker_ptr = sp_tracker_ptr;
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
			}
			virtual ~TFLRegisteredConstPointer() {
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
			}
			TFLRegisteredConstPointer<_Ty>& operator=(const TFLRegisteredConstPointer<_Ty>& _Right_cref) {
				(*m_sp_tracker_ptr).reserve_space_for_one_more();
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
				TSaferPtrForLegacy<const _Ty>::operator=(_Right_cref);
				//assert(m_sp_tracker_ptr == _Right_cref.m_sp_tracker_ptr);
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
				m_might_not_point_to_a_TFLRegisteredObj = _Right_cref.m_might_not_point_to_a_TFLRegisteredObj;
				return (*this);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredConstPointer<_Ty>& operator=(const TFLRegisteredConstPointer<_Ty2>& _Right_cref) {
				return (*this).operator=(TFLRegisteredConstPointer(_Right_cref));
			}
			TFLRegisteredConstPointer<_Ty>& operator=(const TFLRegisteredPointer<_Ty>& _Right_cref) {
				return (*this).operator=(TFLRegisteredConstPointer<_Ty>(_Right_cref));
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredConstPointer<_Ty>& operator=(const TFLRegisteredPointer<_Ty2>& _Right_cref) {
				return (*this).operator=(TFLRegisteredPointer<_Ty>(_Right_cref));
			}
			TFLRegisteredConstPointer<_Ty>& operator=(const _Ty* ptr) {
				(*m_sp_tracker_ptr).reserve_space_for_one_more();
				(*m_sp_tracker_ptr).unregisterPointer((*this), (*this).m_ptr);
				TSaferPtrForLegacy<const _Ty>::operator=(ptr);
				(*m_sp_tracker_ptr).registerPointer((*this), (*this).m_ptr);
				m_might_not_point_to_a_TFLRegisteredObj = true;
				return (*this);
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
			void fl_registered_delete() const {
				if (m_might_not_point_to_a_TFLRegisteredObj) {
					/* It would be a very strange case to arrive here. For (aggressive) compatibility reasons we allow
					TFLRegisteredPointer<_Ty> to point to a _Ty instead of a TFLRegisteredObj<_Ty>. But in those
					situations it doesn't make sense that someone would be calling this delete function. */
					//const _Ty* a = this;
					const _Ty* a = (*this).m_ptr;
					(*m_sp_tracker_ptr).onObjectDestruction(a);
					delete a;
					(*this).setToNull();
				}
				else {
					auto a = asANativePointerToTFLRegisteredObj();
					delete a;
					assert(nullptr == (*this).m_ptr);
				}
			}

		private:
			/* This function, if possible, should not be used. It is meant to be used exclusively by fl_registered_delete<>(). */
			const TFLRegisteredObj<_Ty>* asANativePointerToTFLRegisteredObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
				if (nullptr == (*this).m_ptr) {
					int q = 5; /* just a line of code for putting a debugger break point */
				}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
				if (m_might_not_point_to_a_TFLRegisteredObj) { MSE_THROW(fl_registered_cannot_verify_cast_error("cannot verify cast validity - mse::us::TFLRegisteredConstPointer")); }
				return static_cast<const TFLRegisteredObj<_Ty>*>((*this).m_ptr);
			}

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			CSPTracker* m_sp_tracker_ptr = nullptr;
			bool m_might_not_point_to_a_TFLRegisteredObj = false;

			template <class Y> friend class TFLRegisteredConstPointer;
			friend class TFLRegisteredNotNullConstPointer<_Ty>;
		};

		template<typename _Ty>
		class TFLRegisteredNotNullPointer : public TFLRegisteredPointer<_Ty> {
		public:
			TFLRegisteredNotNullPointer(const TFLRegisteredNotNullPointer& src_cref) : TFLRegisteredPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullPointer(const TFLRegisteredNotNullPointer<_Ty2>& src_cref) : TFLRegisteredPointer<_Ty>(src_cref) {}

			TFLRegisteredNotNullPointer(const  TFLRegisteredPointer<_Ty>& src_cref) : TFLRegisteredPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TFLRegisteredPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}

			virtual ~TFLRegisteredNotNullPointer() {}
			/*
			TFLRegisteredNotNullPointer<_Ty>& operator=(const TFLRegisteredNotNullPointer<_Ty>& _Right_cref) {
				TFLRegisteredPointer<_Ty>::operator=(_Right_cref);
				return (*this);
			}
			*/
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator _Ty*() const { return TFLRegisteredPointer<_Ty>::operator _Ty*(); }
			explicit operator TFLRegisteredObj<_Ty>*() const { return TFLRegisteredPointer<_Ty>::operator TFLRegisteredObj<_Ty>*(); }

		private:
			TFLRegisteredNotNullPointer(TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredPointer<_Ty>(ptr) {}
			TFLRegisteredNotNullPointer(CSPTracker* sp_tracker_ptr, TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredPointer<_Ty>(sp_tracker_ptr, ptr) {}

			/* If you want a pointer to a TFLRegisteredNotNullPointer<_Ty>, declare the TFLRegisteredNotNullPointer<_Ty> as a
			TFLRegisteredObj<TFLRegisteredNotNullPointer<_Ty>> instead. So for example:
			auto reg_ptr = TFLRegisteredObj<TFLRegisteredNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
			auto reg_ptr_to_reg_ptr = &reg_ptr;
			*/
			TFLRegisteredNotNullPointer<_Ty>* operator&() {
				return this;
			}
			const TFLRegisteredNotNullPointer<_Ty>* operator&() const {
				return this;
			}

			friend class TFLRegisteredFixedPointer<_Ty>;
		};

		template<typename _Ty>
		class TFLRegisteredNotNullConstPointer : public TFLRegisteredConstPointer<_Ty> {
		public:
			TFLRegisteredNotNullConstPointer(const TFLRegisteredNotNullPointer<_Ty>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullConstPointer(const TFLRegisteredNotNullPointer<_Ty2>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {}
			TFLRegisteredNotNullConstPointer(const TFLRegisteredNotNullConstPointer<_Ty>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullConstPointer(const TFLRegisteredNotNullConstPointer<_Ty2>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {}

			TFLRegisteredNotNullConstPointer(const TFLRegisteredPointer<_Ty>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullConstPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}
			TFLRegisteredNotNullConstPointer(const TFLRegisteredConstPointer<_Ty>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredNotNullConstPointer(const TFLRegisteredConstPointer<_Ty2>& src_cref) : TFLRegisteredConstPointer<_Ty>(src_cref) {
				*src_cref; // to ensure that src_cref points to a valid target
			}

			virtual ~TFLRegisteredNotNullConstPointer() {}
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TFLRegisteredConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TFLRegisteredObj<_Ty>*() const { return TFLRegisteredConstPointer<_Ty>::operator const TFLRegisteredObj<_Ty>*(); }

		private:
			TFLRegisteredNotNullConstPointer(const TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredConstPointer<_Ty>(ptr) {}
			TFLRegisteredNotNullConstPointer(CSPTracker* sp_tracker_ptr, const TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredConstPointer<_Ty>(sp_tracker_ptr, ptr) {}

			TFLRegisteredNotNullConstPointer<_Ty>* operator&() { return this; }
			const TFLRegisteredNotNullConstPointer<_Ty>* operator&() const { return this; }

			friend class TFLRegisteredFixedConstPointer<_Ty>;
		};

		/* TFLRegisteredFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
		parameters by reference. */
		template<typename _Ty>
		class TFLRegisteredFixedPointer : public TFLRegisteredNotNullPointer<_Ty> {
		public:
			TFLRegisteredFixedPointer(const TFLRegisteredFixedPointer& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedPointer(const TFLRegisteredFixedPointer<_Ty2>& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}

			TFLRegisteredFixedPointer(const TFLRegisteredNotNullPointer<_Ty>& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedPointer(const TFLRegisteredNotNullPointer<_Ty2>& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}

			TFLRegisteredFixedPointer(const TFLRegisteredPointer<_Ty>& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TFLRegisteredNotNullPointer<_Ty>(src_cref) {}

			virtual ~TFLRegisteredFixedPointer() {}

			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator _Ty*() const { return TFLRegisteredNotNullPointer<_Ty>::operator _Ty*(); }
			explicit operator TFLRegisteredObj<_Ty>*() const { return TFLRegisteredNotNullPointer<_Ty>::operator TFLRegisteredObj<_Ty>*(); }

		private:
			TFLRegisteredFixedPointer(TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredNotNullPointer<_Ty>(ptr) {}
			TFLRegisteredFixedPointer(CSPTracker* sp_tracker_ptr, TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredNotNullPointer<_Ty>(sp_tracker_ptr, ptr) {}
			TFLRegisteredFixedPointer<_Ty>& operator=(const TFLRegisteredFixedPointer<_Ty>& _Right_cref) = delete;

			/* If you want a pointer to a TFLRegisteredFixedPointer<_Ty>, declare the TFLRegisteredFixedPointer<_Ty> as a
			TFLRegisteredObj<TFLRegisteredFixedPointer<_Ty>> instead. So for example:
			auto reg_ptr = TFLRegisteredObj<TFLRegisteredFixedPointer<_Ty>>(mse::registered_new<_Ty>());
			auto reg_ptr_to_reg_ptr = &reg_ptr;
			*/
			TFLRegisteredFixedPointer<_Ty>* operator&() {
				return this;
			}
			const TFLRegisteredFixedPointer<_Ty>* operator&() const {
				return this;
			}

			friend class TFLRegisteredObj<_Ty>;
		};

		template<typename _Ty>
		class TFLRegisteredFixedConstPointer : public TFLRegisteredNotNullConstPointer<_Ty> {
		public:
			TFLRegisteredFixedConstPointer(const TFLRegisteredFixedPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredFixedPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			TFLRegisteredFixedConstPointer(const TFLRegisteredFixedConstPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredFixedConstPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}

			TFLRegisteredFixedConstPointer(const TFLRegisteredNotNullPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredNotNullPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			TFLRegisteredFixedConstPointer(const TFLRegisteredNotNullConstPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredNotNullConstPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}

			TFLRegisteredFixedConstPointer(const TFLRegisteredPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			TFLRegisteredFixedConstPointer(const TFLRegisteredConstPointer<_Ty>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TFLRegisteredFixedConstPointer(const TFLRegisteredConstPointer<_Ty2>& src_cref) : TFLRegisteredNotNullConstPointer<_Ty>(src_cref) {}

			virtual ~TFLRegisteredFixedConstPointer() {}
			/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
			explicit operator const _Ty*() const { return TFLRegisteredNotNullConstPointer<_Ty>::operator const _Ty*(); }
			explicit operator const TFLRegisteredObj<_Ty>*() const { return TFLRegisteredNotNullConstPointer<_Ty>::operator const TFLRegisteredObj<_Ty>*(); }

		private:
			TFLRegisteredFixedConstPointer(const TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredNotNullConstPointer<_Ty>(ptr) {}
			TFLRegisteredFixedConstPointer(CSPTracker* sp_tracker_ptr, const TFLRegisteredObj<_Ty>* ptr) : TFLRegisteredNotNullConstPointer<_Ty>(sp_tracker_ptr, ptr) {}
			TFLRegisteredFixedConstPointer<_Ty>& operator=(const TFLRegisteredFixedConstPointer<_Ty>& _Right_cref) = delete;

			TFLRegisteredFixedConstPointer<_Ty>* operator&() { return this; }
			const TFLRegisteredFixedConstPointer<_Ty>* operator&() const { return this; }

			friend class TFLRegisteredObj<_Ty>;
		};

		/* This macro roughly simulates constructor inheritance. */
#define MSE_RELAXED_REGISTERED_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...), m_tracker_notifier(*this) {}

	/* TFLRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TFLRegisteredPointers will avoid referencing destroyed objects. Note that TFLRegisteredObj can be used with
	objects allocated on the stack. */
		template<typename _TROFLy>
		class TFLRegisteredObj : public _TROFLy
			, public std::conditional<!std::is_convertible<_TROFLy*, NotAsyncShareableTagBase*>::value, NotAsyncShareableTagBase, TPlaceHolder_msepointerbasics<TFLRegisteredObj<_TROFLy> > >::type
		{
		public:
			typedef _TROFLy base_class;

			MSE_RELAXED_REGISTERED_OBJ_USING(TFLRegisteredObj, _TROFLy);
			TFLRegisteredObj(const TFLRegisteredObj& _X) : _TROFLy(_X), m_tracker_notifier(*this) {}
			TFLRegisteredObj(TFLRegisteredObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)), m_tracker_notifier(*this) {}
			virtual ~TFLRegisteredObj() {
				(*trackerPtr()).onObjectDestruction(static_cast<_TROFLy*>(this));
			}

			template<class _Ty2>
			TFLRegisteredObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			template<class _Ty2>
			TFLRegisteredObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

			TFLRegisteredFixedPointer<_TROFLy> operator&() {
				return TFLRegisteredFixedPointer<_TROFLy>(trackerPtr(), this);
			}
			TFLRegisteredFixedConstPointer<_TROFLy> operator&() const {
				return TFLRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this);
			}
			TFLRegisteredFixedPointer<_TROFLy> mse_fl_registered_fptr() { return TFLRegisteredFixedPointer<_TROFLy>(trackerPtr(), this); }
			TFLRegisteredFixedConstPointer<_TROFLy> mse_fl_registered_fptr() const { return TFLRegisteredFixedConstPointer<_TROFLy>(trackerPtr(), this); }

			CSPTracker* trackerPtr() const { return &(tlSPTracker_ref<_TROFLy>()); }

		private:
			class CTrackerNotifier {
			public:
				template<typename _TFLRegisteredObj>
				CTrackerNotifier(_TFLRegisteredObj& obj_ref) {
					(*(obj_ref.trackerPtr())).onObjectConstruction(static_cast<typename _TFLRegisteredObj::base_class*>(std::addressof(obj_ref)));
				}
			};
			CTrackerNotifier m_tracker_notifier;
		};

		template<typename _Ty>
		auto fl_registered_fptr_to(_Ty&& _X) {
			return _X.mse_fl_registered_fptr();
		}
		template<typename _Ty>
		auto fl_registered_fptr_to(const _Ty& _X) {
			return _X.mse_fl_registered_fptr();
		}

		/* See registered_new(). */
		template <class _Ty, class... Args>
		TFLRegisteredPointer<_Ty> fl_registered_new(Args&&... args) {
			auto a = new TFLRegisteredObj<_Ty>(std::forward<Args>(args)...);
			return &(*a);
		}
		template <class _Ty>
		void fl_registered_delete(const TFLRegisteredPointer<_Ty>& regPtrRef) {
			regPtrRef.fl_registered_delete();
		}
		template <class _Ty>
		void fl_registered_delete(const TFLRegisteredConstPointer<_Ty>& regPtrRef) {
			regPtrRef.fl_registered_delete();
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::us::TFLRegisteredPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::us::TFLRegisteredNotNullPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::us::TFLRegisteredFixedPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::us::TFLRegisteredConstPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::us::TFLRegisteredNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::us::TFLRegisteredFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::us::TFLRegisteredFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::us::TFLRegisteredFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {
	namespace us {
#endif /*MSE_REGISTEREDPOINTER_DISABLED*/
	}

#if defined(MSE_REGISTEREDPOINTER_DISABLED)
	/* Omit definition of make_pointer_to_member() as it would clash with the one already defined in mseregistered.h. */
#define MSE_FLREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER
#endif // defined(MSE_REGISTEREDPOINTER_DISABLED)
#if !defined(MSE_FLREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, us::TFLRegisteredPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const us::TFLRegisteredPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, us::TFLRegisteredPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, us::TFLRegisteredConstPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const us::TFLRegisteredConstPointer<_Ty> &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, us::TFLRegisteredConstPointer<_Ty>>::make(target, lease_pointer);
	}
#endif // !defined(MSE_FLREGISTERED_OMIT_MAKE_POINTER_TO_MEMBER) && defined(MSEREGISTERED_H_)

	namespace us {

		/* shorter aliases */
		template<typename _Ty> using flrp = TFLRegisteredPointer<_Ty>;
		template<typename _Ty> using flrcp = TFLRegisteredConstPointer<_Ty>;
		template<typename _Ty> using flrnnp = TFLRegisteredNotNullPointer<_Ty>;
		template<typename _Ty> using flrnncp = TFLRegisteredNotNullConstPointer<_Ty>;
		template<typename _Ty> using flrfp = TFLRegisteredFixedPointer<_Ty>;
		template<typename _Ty> using flrfcp = TFLRegisteredFixedConstPointer<_Ty>;
		template<typename _TROFLy> using flro = TFLRegisteredObj<_TROFLy>;
		template <class _Ty, class... Args>
		TFLRegisteredPointer<_Ty> flrnew(Args&&... args) { return fl_registered_new<_Ty>(std::forward<Args>(args)...); }
		template <class _Ty>
		void flrdelete(const TFLRegisteredPointer<_Ty>& regPtrRef) { fl_registered_delete<_Ty>(regPtrRef); }
	}


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

	class CForLegRegPtrTest1 {
	public:
		static void s_test1() {
#ifdef MSE_SELF_TESTS
			class C;

			class D {
			public:
				virtual ~D() {}
				mse::us::TFLRegisteredPointer<C> m_c_ptr = nullptr;
			};

			class C {
			public:
				C() {}
				mse::us::TFLRegisteredPointer<D> m_d_ptr = nullptr;
			};

			mse::us::TFLRegisteredObj<C> flregobj_c;
			mse::us::TFLRegisteredPointer<D> d_ptr = mse::us::fl_registered_new<D>();

			flregobj_c.m_d_ptr = d_ptr;
			d_ptr->m_c_ptr = &flregobj_c;

			mse::us::TFLRegisteredConstPointer<C> flrcp = d_ptr->m_c_ptr;
			mse::us::TFLRegisteredConstPointer<C> flrcp2 = flrcp;
			const mse::us::TFLRegisteredObj<C> flregobj_e;
			flrcp = &flregobj_e;
			mse::us::TFLRegisteredFixedConstPointer<C> flrfcp = &flregobj_e;
			flrcp = mse::us::fl_registered_new<C>();
			mse::us::fl_registered_delete<C>(flrcp);

			mse::us::fl_registered_delete<D>(d_ptr);

			{
				/* Polymorphic conversions. */
				class FD : public mse::us::TFLRegisteredObj<D> {};
				mse::us::TFLRegisteredObj<FD> fl_registered_fd;
				mse::us::TFLRegisteredPointer<FD> FD_fl_registered_ptr1 = &fl_registered_fd;
				mse::us::TFLRegisteredPointer<D> D_fl_registered_ptr4 = FD_fl_registered_ptr1;
				D_fl_registered_ptr4 = &fl_registered_fd;
				mse::us::TFLRegisteredFixedPointer<D> D_fl_registered_fptr1 = &fl_registered_fd;
				mse::us::TFLRegisteredFixedConstPointer<D> D_fl_registered_fcptr1 = &fl_registered_fd;

				/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
				class GD : public D {};
				mse::us::TFLRegisteredObj<GD> fl_registered_gd;
				mse::us::TFLRegisteredPointer<GD> GD_fl_registered_ptr1 = &fl_registered_gd;
				mse::us::TFLRegisteredPointer<D> D_fl_registered_ptr5 = GD_fl_registered_ptr1;
				D_fl_registered_ptr5 = GD_fl_registered_ptr1;
				mse::us::TFLRegisteredFixedPointer<GD> GD_fl_registered_fptr1 = &fl_registered_gd;
				D_fl_registered_ptr5 = &fl_registered_gd;
				mse::us::TFLRegisteredFixedPointer<D> D_fl_registered_fptr2 = &fl_registered_gd;
				mse::us::TFLRegisteredFixedConstPointer<D> D_fl_registered_fcptr2 = &fl_registered_gd;
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

#endif // MSEFLREGISTERED_H_
