
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/* TNoradPointers are basically just like TRegisteredPointers except that unlike TRegisteredPointers, they
are able to reference target object types before those target types are (fully) defined. This is required to support
"mutually referencing" objects or "cyclic references".

The implementation difference is that with TRegisteredPointers, the "pointer tracking registry" is located in the target
object, whereas TNoradPointers use (thread local) global registries that track all the pointers targeting
objects of a given type. */

#pragma once
#ifndef MSENORAD_H_
#define MSENORAD_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_map>
#include <mutex>
#include <cassert>
//include <typeinfo>      // std::bad_cast
#include <stdexcept>
#include <iostream>

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_NORADPOINTER_DISABLED
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

	template<typename _Ty> class TNDNoradObj;
	template<typename _Ty> class TNDNoradPointer;
	template<typename _Ty> class TNDNoradConstPointer;
	template<typename _Ty> class TNDNoradNotNullPointer;
	template<typename _Ty> class TNDNoradNotNullConstPointer;
	template<typename _Ty> class TNDNoradFixedPointer;
	template<typename _Ty> class TNDNoradFixedConstPointer;
	
	template<typename _Ty>
	auto ndnorad_fptr_to(_Ty&& _X) {
		return _X.mse_norad_fptr();
	}
	template<typename _Ty>
	auto ndnorad_fptr_to(const _Ty& _X) {
		return _X.mse_norad_fptr();
	}

	template <class _Ty, class... Args> TNDNoradPointer<_Ty> ndnorad_new(Args&&... args);
	template <class _Ty> void ndnorad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef);
	template <class _Ty> void ndnorad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef);
	namespace us {
		template <class _Ty> void ndnorad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef);
		template <class _Ty> void ndnorad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef);
	}

	namespace impl {
		template<typename _Ty, class... Args>
		auto make_ndnorad_helper(std::true_type, Args&&... args) {
			return _Ty(std::forward<Args>(args)...);
		}
		template<typename _Ty, class... Args>
		auto make_ndnorad_helper(std::false_type, Args&&... args) {
			return TNDNoradObj<_Ty>(std::forward<Args>(args)...);
		}
	}
	template <class X, class... Args>
	auto make_ndnorad(Args&&... args) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_ndnorad_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TNDNoradObj>::type(), std::forward<Args>(args)...);
	}
	template <class X>
	auto make_ndnorad(const X& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_ndnorad_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TNDNoradObj>::type(), arg);
	}
	template <class X>
	auto make_ndnorad(X&& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_ndnorad_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TNDNoradObj>::type(), std::forward<decltype(arg)>(arg));
	}

#ifdef MSE_HAS_CXX17
	/* deduction guide */
	template<class _TROy> TNDNoradObj(_TROy)->TNDNoradObj<_TROy>;
#endif /* MSE_HAS_CXX17 */

#ifdef MSE_NORADPOINTER_DISABLED
	template<typename _Ty> using TNoradPointer = _Ty * ;
	template<typename _Ty> using TNoradConstPointer = const _Ty*;
	template<typename _Ty> using TNoradNotNullPointer = _Ty * ;
	template<typename _Ty> using TNoradNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TNoradFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																			 library containers don't support const elements. */
	template<typename _Ty> using TNoradFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROFLy> using TNoradObj = _TROFLy;

	template<typename _Ty> auto norad_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto norad_fptr_to(const _Ty& _X) { return &_X; }

	template <class _Ty, class... Args>
	TNoradPointer<_Ty> norad_new(Args&&... args) {
		return new TNoradObj<_Ty>(std::forward<Args>(args)...);
	}
	template <class _Ty>
	void norad_delete(TNoradPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TNoradObj<_Ty>*>(regPtrRef);
		regPtrRef = nullptr;
		delete a;
	}
	template <class _Ty>
	void norad_delete(TNoradConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TNoradObj<_Ty>*>(regPtrRef);
		regPtrRef = nullptr;
		delete a;
	}
	namespace us {
		template <class _Ty>
		void norad_delete(TNoradPointer<_Ty>& regPtrRef) {
			mse::norad_delete(regPtrRef);
		}
		template <class _Ty>
		void norad_delete(TNoradConstPointer<_Ty>& regPtrRef) {
			mse::norad_delete(regPtrRef);
		}
	}

	template <class X, class... Args>
	auto make_norad(Args&&... args) {
		typedef typename std::remove_reference<X>::type nrX;
		return nrX(std::forward<Args>(args)...);
	}
	template <class X>
	auto make_norad(const X& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return nrX(arg);
	}
	template <class X>
	auto make_norad(X&& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return nrX(std::forward<decltype(arg)>(arg));
	}

#else /*MSE_NORADPOINTER_DISABLED*/

	class norad_cannot_verify_cast_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> using TNoradPointer = TNDNoradPointer<_Ty>;
	template<typename _Ty> using TNoradConstPointer = TNDNoradConstPointer<_Ty>;
	template<typename _Ty> using TNoradNotNullPointer = TNDNoradNotNullPointer<_Ty>;
	template<typename _Ty> using TNoradNotNullConstPointer = TNDNoradNotNullConstPointer<_Ty>;
	template<typename _Ty> using TNoradFixedPointer = TNDNoradFixedPointer<_Ty>;
	template<typename _Ty> using TNoradFixedConstPointer = TNDNoradFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TNoradObj = TNDNoradObj<_TROFLy>;

	template<typename _Ty> auto norad_fptr_to(_Ty&& _X) { return ndnorad_fptr_to(std::forward<decltype(_X )>(_X)); }
	template<typename _Ty> auto norad_fptr_to(const _Ty& _X) { return ndnorad_fptr_to(_X); }

	template <class _Ty, class... Args> TNDNoradPointer<_Ty> norad_new(Args&&... args) { return ndnorad_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty> void norad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef) { return mse::ndnorad_delete<_Ty>(ndnoradPtrRef); }
	template <class _Ty> void norad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef) { return mse::ndnorad_delete<_Ty>(ndnoradPtrRef); }
	namespace us {
		template <class _Ty> void norad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef) { return mse::us::ndnorad_delete<_Ty>(ndnoradPtrRef); }
		template <class _Ty> void norad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef) { return mse::us::ndnorad_delete<_Ty>(ndnoradPtrRef); }
	}

	template <class X, class... Args>
	auto make_norad(Args&&... args) {
		return make_ndnorad<X>(std::forward<Args>(args)...);
	}
	template <class X>
	auto make_norad(const X& arg) {
		return make_ndnorad(arg);
	}
	template <class X>
	auto make_norad(X&& arg) {
		return make_ndnorad(std::forward<decltype(arg)>(arg));
	}

#endif /*MSE_NORADPOINTER_DISABLED*/

	/* TNDNoradPointer<>, like TNDCRegisteredPointer<>, behaves similar to native pointers. But where registered pointers are
	automatically set to nullptr when their target is destroyed, the destruction of an object while a "norad" pointer is targeting
	it results in program termination. This drastic consequence allows norad pointers' run-time safety mechanism to be very
	lightweight (compared to that of registered pointers). */
	template<typename _Ty>
	class TNDNoradPointer : public mse::us::impl::TPointer<TNDNoradObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
	public:
		TNDNoradPointer() : mse::us::impl::TPointer<TNDNoradObj<_Ty>>() {}
		TNDNoradPointer(const TNDNoradPointer& src_cref) : mse::us::impl::TPointer<TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradPointer(const TNDNoradPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		TNDNoradPointer(TNDNoradPointer&& src_ref) : mse::us::impl::TPointer<TNDNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}
		TNDNoradPointer(std::nullptr_t) : mse::us::impl::TPointer<TNDNoradObj<_Ty>>(nullptr) {}
		virtual ~TNDNoradPointer() {
			if (*this) { (*(*this)).decrement_refcount(); }
		}
		TNDNoradPointer<_Ty>& operator=(const TNDNoradPointer<_Ty>& _Right_cref) {
			if (*this) { (*(*this)).decrement_refcount(); }
			mse::us::impl::TPointer<TNDNoradObj<_Ty>>::operator=(_Right_cref);
			if (*this) { (*(*this)).increment_refcount(); }
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradPointer<_Ty>& operator=(const TNDNoradPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TNDNoradPointer(_Right_cref));
		}

		TNDNoradObj<_Ty>& operator*() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		TNDNoradObj<_Ty>* operator->() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		operator bool() const { return !(!((*this).m_ptr)); }
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
		void norad_delete() {
			auto a = asANativePointerToTNDNoradObj();
			(*this) = nullptr;
			delete a;
		}

	private:
		TNDNoradPointer(TNDNoradObj<_Ty>* ptr) : mse::us::impl::TPointer<TNDNoradObj<_Ty>>(ptr) {
			assert(*this);
			(*(*this)).increment_refcount();
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by norad_delete<>(). */
		TNDNoradObj<_Ty>* asANativePointerToTNDNoradObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TNDNoradObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TNDNoradPointer;
		template <class Y> friend class TNDNoradConstPointer;
		friend class TNDNoradNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TNDNoradConstPointer : public mse::us::impl::TPointer<const TNDNoradObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
	public:
		TNDNoradConstPointer() : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>() {}
		TNDNoradConstPointer(const TNDNoradConstPointer& src_cref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradConstPointer(const TNDNoradConstPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		TNDNoradConstPointer(const TNDNoradPointer<_Ty>& src_cref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradConstPointer(const TNDNoradPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}

		TNDNoradConstPointer(TNDNoradConstPointer&& src_ref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}
		TNDNoradConstPointer(TNDNoradPointer<_Ty>&& src_ref) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}

		TNDNoradConstPointer(std::nullptr_t) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(nullptr) {}
		virtual ~TNDNoradConstPointer() {
			if (*this) { (*(*this)).decrement_refcount(); }
		}
		TNDNoradConstPointer<_Ty>& operator=(const TNDNoradConstPointer<_Ty>& _Right_cref) {
			if (*this) { (*(*this)).decrement_refcount(); }
			mse::us::impl::TPointer<const TNDNoradObj<_Ty>>::operator=(_Right_cref);
			if (*this) { (*(*this)).increment_refcount(); }
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradConstPointer<_Ty>& operator=(const TNDNoradConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TNDNoradConstPointer(_Right_cref));
		}

		const TNDNoradObj<_Ty>& operator*() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		const TNDNoradObj<_Ty>* operator->() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		operator bool() const { return !(!((*this).m_ptr)); }
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
		void norad_delete() {
			auto a = asANativePointerToTNDNoradObj();
			(*this) = nullptr;
			delete a;
		}

	private:
		TNDNoradConstPointer(const TNDNoradObj<_Ty>* ptr) : mse::us::impl::TPointer<const TNDNoradObj<_Ty>>(ptr) {
			assert(*this);
			(*(*this)).increment_refcount();
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by norad_delete<>(). */
		const TNDNoradObj<_Ty>* asANativePointerToTNDNoradObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TNDNoradObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TNDNoradConstPointer;
		friend class TNDNoradNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TNDNoradNotNullPointer : public TNDNoradPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		TNDNoradNotNullPointer(const TNDNoradNotNullPointer& src_cref) : TNDNoradPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullPointer(const TNDNoradNotNullPointer<_Ty2>& src_cref) : TNDNoradPointer<_Ty>(src_cref) {}
		TNDNoradNotNullPointer(TNDNoradNotNullPointer&& src_ref) : TNDNoradPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TNDNoradNotNullPointer() {}
		/*
		TNDNoradNotNullPointer<_Ty>& operator=(const TNDNoradNotNullPointer<_Ty>& _Right_cref) {
		TNDNoradPointer<_Ty>::operator=(_Right_cref);
		return (*this);
		}
		*/

		TNDNoradObj<_Ty>& operator*() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		TNDNoradObj<_Ty>* operator->() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TNDNoradPointer<_Ty>::operator _Ty*(); }
		explicit operator TNDNoradObj<_Ty>*() const { return TNDNoradPointer<_Ty>::operator TNDNoradObj<_Ty>*(); }

	private:
		TNDNoradNotNullPointer(TNDNoradObj<_Ty>* ptr) : TNDNoradPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TNDNoradNotNullPointer(const  TNDNoradPointer<_Ty>& src_cref) : TNDNoradPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullPointer(const TNDNoradPointer<_Ty2>& src_cref) : TNDNoradPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		/* If you want a pointer to a TNDNoradNotNullPointer<_Ty>, declare the TNDNoradNotNullPointer<_Ty> as a
		TNDNoradObj<TNDNoradNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TNDNoradObj<TNDNoradNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TNDNoradFixedPointer<_Ty>;
		template<typename _Ty2>
		friend TNDNoradNotNullPointer<_Ty2> not_null_from_nullable(const TNDNoradPointer<_Ty2>& src);
	};

	template<typename _Ty>
	class TNDNoradNotNullConstPointer : public TNDNoradConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		TNDNoradNotNullConstPointer(const TNDNoradNotNullPointer<_Ty>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullConstPointer(const TNDNoradNotNullPointer<_Ty2>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {}
		TNDNoradNotNullConstPointer(const TNDNoradNotNullConstPointer<_Ty>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullConstPointer(const TNDNoradNotNullConstPointer<_Ty2>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {}

		TNDNoradNotNullConstPointer(TNDNoradNotNullPointer<_Ty>&& src_ref) : TNDNoradConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TNDNoradNotNullConstPointer(TNDNoradNotNullConstPointer<_Ty>&& src_ref) : TNDNoradConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TNDNoradNotNullConstPointer() {}

		const TNDNoradObj<_Ty>& operator*() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		const TNDNoradObj<_Ty>* operator->() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TNDNoradConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TNDNoradObj<_Ty>*() const { return TNDNoradConstPointer<_Ty>::operator const TNDNoradObj<_Ty>*(); }

	private:
		TNDNoradNotNullConstPointer(const TNDNoradObj<_Ty>* ptr) : TNDNoradConstPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TNDNoradNotNullConstPointer(const TNDNoradPointer<_Ty>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullConstPointer(const TNDNoradPointer<_Ty2>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TNDNoradNotNullConstPointer(const TNDNoradConstPointer<_Ty>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradNotNullConstPointer(const TNDNoradConstPointer<_Ty2>& src_cref) : TNDNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TNDNoradFixedConstPointer<_Ty>;
		template<typename _Ty2>
		friend TNDNoradNotNullConstPointer<_Ty2> not_null_from_nullable(const TNDNoradConstPointer<_Ty2>& src);
	};

	template<typename _Ty>
	TNDNoradNotNullPointer<_Ty> not_null_from_nullable(const TNDNoradPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TNDNoradNotNullConstPointer<_Ty> not_null_from_nullable(const TNDNoradConstPointer<_Ty>& src) {
		return src;
	}

	/* TNDNoradFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TNDNoradFixedPointer : public TNDNoradNotNullPointer<_Ty> {
	public:
		TNDNoradFixedPointer(const TNDNoradFixedPointer& src_cref) : TNDNoradNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedPointer(const TNDNoradFixedPointer<_Ty2>& src_cref) : TNDNoradNotNullPointer<_Ty>(src_cref) {}

		TNDNoradFixedPointer(const TNDNoradNotNullPointer<_Ty>& src_cref) : TNDNoradNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedPointer(const TNDNoradNotNullPointer<_Ty2>& src_cref) : TNDNoradNotNullPointer<_Ty>(src_cref) {}

		TNDNoradFixedPointer(TNDNoradFixedPointer&& src_ref) : TNDNoradNotNullPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TNDNoradFixedPointer(TNDNoradNotNullPointer<_Ty>&& src_ref) : TNDNoradNotNullPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TNDNoradFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TNDNoradNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TNDNoradObj<_Ty>*() const { return TNDNoradNotNullPointer<_Ty>::operator TNDNoradObj<_Ty>*(); }

	private:
		TNDNoradFixedPointer(TNDNoradObj<_Ty>* ptr) : TNDNoradNotNullPointer<_Ty>(ptr) {}
		TNDNoradFixedPointer<_Ty>& operator=(const TNDNoradFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TNDNoradFixedPointer<_Ty>, declare the TNDNoradFixedPointer<_Ty> as a
		TNDNoradObj<TNDNoradFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TNDNoradObj<TNDNoradFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TNDNoradObj<_Ty>;
	};

	template<typename _Ty>
	class TNDNoradFixedConstPointer : public TNDNoradNotNullConstPointer<_Ty> {
	public:
		TNDNoradFixedConstPointer(const TNDNoradFixedPointer<_Ty>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedConstPointer(const TNDNoradFixedPointer<_Ty2>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		TNDNoradFixedConstPointer(const TNDNoradFixedConstPointer<_Ty>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedConstPointer(const TNDNoradFixedConstPointer<_Ty2>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}

		TNDNoradFixedConstPointer(const TNDNoradNotNullPointer<_Ty>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedConstPointer(const TNDNoradNotNullPointer<_Ty2>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		TNDNoradFixedConstPointer(const TNDNoradNotNullConstPointer<_Ty>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TNDNoradFixedConstPointer(const TNDNoradNotNullConstPointer<_Ty2>& src_cref) : TNDNoradNotNullConstPointer<_Ty>(src_cref) {}

		TNDNoradFixedConstPointer(TNDNoradFixedPointer<_Ty>&& src_ref) : TNDNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TNDNoradFixedConstPointer(TNDNoradFixedConstPointer<_Ty>&& src_ref) : TNDNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		TNDNoradFixedConstPointer(TNDNoradNotNullPointer<_Ty>&& src_ref) : TNDNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TNDNoradFixedConstPointer(TNDNoradNotNullConstPointer<_Ty>&& src_ref) : TNDNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TNDNoradFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TNDNoradNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TNDNoradObj<_Ty>*() const { return TNDNoradNotNullConstPointer<_Ty>::operator const TNDNoradObj<_Ty>*(); }

	private:
		TNDNoradFixedConstPointer(const TNDNoradObj<_Ty>* ptr) : TNDNoradNotNullConstPointer<_Ty>(ptr) {}
		TNDNoradFixedConstPointer<_Ty>& operator=(const TNDNoradFixedConstPointer<_Ty>& _Right_cref) = delete;

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TNDNoradObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_NORAD_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	/* TNDNoradObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TNDNoradPointers will avoid referencing destroyed objects. Note that TNDNoradObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TNDNoradObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, mse::us::impl::NotAsyncShareableTagBase*>::value) && (!std::is_base_of<mse::us::impl::NotAsyncShareableTagBase, _TROFLy>::value)
		, mse::us::impl::NotAsyncShareableTagBase, impl::TPlaceHolder_msepointerbasics<TNDNoradObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_NORAD_OBJ_USING(TNDNoradObj, _TROFLy);
		TNDNoradObj(const TNDNoradObj& _X) : _TROFLy(_X) {}
		TNDNoradObj(TNDNoradObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TNDNoradObj() {
			if (0 != m_counter) {
				/* It would be unsafe to allow this object to be destroyed as there are outstanding references to this object. */
				std::cerr << "\n\nFatal Error: mse::TNDNoradObj<> destructed with outstanding references \n\n";
				std::terminate();
			}
		}

		template<class _Ty2>
		TNDNoradObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TNDNoradObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TNDNoradFixedPointer<_TROFLy> operator&() {
			return TNDNoradFixedPointer<_TROFLy>(this);
		}
		TNDNoradFixedConstPointer<_TROFLy> operator&() const {
			return TNDNoradFixedConstPointer<_TROFLy>(this);
		}
		TNDNoradFixedPointer<_TROFLy> mse_norad_fptr() { return TNDNoradFixedPointer<_TROFLy>(this); }
		TNDNoradFixedConstPointer<_TROFLy> mse_norad_fptr() const { return TNDNoradFixedConstPointer<_TROFLy>(this); }

		/* todo: make these private */
		void increment_refcount() const { m_counter += 1; }
		void decrement_refcount() const { m_counter -= 1; }

	private:
		mutable int m_counter = 0;
	};

	/* See ndregistered_new(). */
	template <class _Ty, class... Args>
	TNDNoradPointer<_Ty> ndnorad_new(Args&&... args) {
		auto a = new TNDNoradObj<_Ty>(std::forward<Args>(args)...);
		mse::us::impl::tlSAllocRegistry_ref<TNDNoradObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void ndnorad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef) {
		auto a = static_cast<TNDNoradObj<_Ty>*>(ndnoradPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TNDNoradObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::ndnorad_delete() \n- tip: If deleting via base class pointer, use mse::us::ndnorad_delete() instead. ")); }
		ndnoradPtrRef.norad_delete();
	}
	template <class _Ty>
	void ndnorad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef) {
		auto a = static_cast<const TNDNoradObj<_Ty>*>(ndnoradPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TNDNoradObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::ndnorad_delete() \n- tip: If deleting via base class pointer, use mse::us::ndnorad_delete() instead. ")); }
		ndnoradPtrRef.norad_delete();
	}
	namespace us {
		template <class _Ty>
		void ndnorad_delete(TNDNoradPointer<_Ty>& ndnoradPtrRef) {
			ndnoradPtrRef.norad_delete();
		}
		template <class _Ty>
		void ndnorad_delete(TNDNoradConstPointer<_Ty>& ndnoradPtrRef) {
			ndnoradPtrRef.norad_delete();
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TNDNoradPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TNDNoradNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TNDNoradFixedPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TNDNoradConstPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TNDNoradNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TNDNoradFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TNDNoradFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TNDNoradFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
	class TNDNoradObj<_Ty*> : public TNDNoradObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<typename _Ty>
	class TNDNoradObj<_Ty* const> : public TNDNoradObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<typename _Ty>
	class TNDNoradObj<const _Ty *> : public TNDNoradObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<typename _Ty>
	class TNDNoradObj<const _Ty * const> : public TNDNoradObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};

	template<typename _Ty>
	class TNDNoradPointer<_Ty*> : public TNDNoradPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradPointer<_Ty* const> : public TNDNoradPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradPointer<const _Ty *> : public TNDNoradPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradPointer<const _Ty * const> : public TNDNoradPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};

	template<typename _Ty>
	class TNDNoradConstPointer<_Ty*> : public TNDNoradConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradConstPointer<_Ty* const> : public TNDNoradConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TNDNoradConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradConstPointer<const _Ty *> : public TNDNoradConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TNDNoradConstPointer<const _Ty * const> : public TNDNoradConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TNDNoradConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TNDNoradObj<int> : public TNDNoradObj<mse::TInt<int>> {
	public:
		typedef TNDNoradObj<mse::TInt<int>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<>
	class TNDNoradObj<const int> : public TNDNoradObj<const mse::TInt<int>> {
	public:
		typedef TNDNoradObj<const mse::TInt<int>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<>
	class TNDNoradPointer<int> : public TNDNoradPointer<mse::TInt<int>> {
	public:
		typedef TNDNoradPointer<mse::TInt<int>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<>
	class TNDNoradPointer<const int> : public TNDNoradPointer<const mse::TInt<int>> {
	public:
		typedef TNDNoradPointer<const mse::TInt<int>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<>
	class TNDNoradConstPointer<int> : public TNDNoradConstPointer<mse::TInt<int>> {
	public:
		typedef TNDNoradConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
	template<>
	class TNDNoradConstPointer<const int> : public TNDNoradConstPointer<const mse::TInt<int>> {
	public:
		typedef TNDNoradConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};

	template<>
	class TNDNoradObj<size_t> : public TNDNoradObj<mse::TInt<size_t>> {
	public:
		typedef TNDNoradObj<mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<>
	class TNDNoradObj<const size_t> : public TNDNoradObj<const mse::TInt<size_t>> {
	public:
		typedef TNDNoradObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradObj, base_class);
	};
	template<>
	class TNDNoradPointer<size_t> : public TNDNoradPointer<mse::TInt<size_t>> {
	public:
		typedef TNDNoradPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<>
	class TNDNoradPointer<const size_t> : public TNDNoradPointer<const mse::TInt<size_t>> {
	public:
		typedef TNDNoradPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradPointer, base_class);
	};
	template<>
	class TNDNoradConstPointer<size_t> : public TNDNoradConstPointer<mse::TInt<size_t>> {
	public:
		typedef TNDNoradConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
	template<>
	class TNDNoradConstPointer<const size_t> : public TNDNoradConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TNDNoradConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TNDNoradConstPointer, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

	/* shorter aliases */
	template<typename _Ty> using np = TNoradPointer<_Ty>;
	template<typename _Ty> using ncp = TNoradConstPointer<_Ty>;
	template<typename _Ty> using nnnp = TNoradNotNullPointer<_Ty>;
	template<typename _Ty> using nnncp = TNoradNotNullConstPointer<_Ty>;
	template<typename _Ty> using nfp = TNoradFixedPointer<_Ty>;
	template<typename _Ty> using nfcp = TNoradFixedConstPointer<_Ty>;
	template<typename _TROFLy> using no = TNoradObj<_TROFLy>;
	template <class _Ty, class... Args>
	TNoradPointer<_Ty> nnew(Args&&... args) { return norad_new<_Ty>(std::forward<Args>(args)...); }
	template <class _Ty>
	void ndelete(const TNoradPointer<_Ty>& regPtrRef) { norad_delete<_Ty>(regPtrRef); }

	/* deprecated aliases */
	template<typename _Ty> using TWNoradPointer = TNDNoradPointer<_Ty>;
	template<typename _Ty> using TWNoradConstPointer = TNDNoradConstPointer<_Ty>;
	template<typename _Ty> using TWNoradNotNullPointer = TNDNoradNotNullPointer<_Ty>;
	template<typename _Ty> using TWNoradNotNullConstPointer = TNDNoradNotNullConstPointer<_Ty>;
	template<typename _Ty> using TWNoradFixedPointer = TNDNoradFixedPointer<_Ty>;
	template<typename _Ty> using TWNoradFixedConstPointer = TNDNoradFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TWNoradObj = TNDNoradObj<_TROFLy>;

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

	namespace self_test {
		class CNoradPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class C;

				class D {
				public:
					virtual ~D() {}
					mse::TNoradPointer<C> m_c_ptr = nullptr;
				};

				class C {
				public:
					C() {}
					mse::TNoradPointer<D> m_d_ptr = nullptr;
				};

				mse::TNoradObj<C> noradobj_c;
				mse::TNoradPointer<D> d_ptr = mse::norad_new<D>();

				noradobj_c.m_d_ptr = d_ptr;
				d_ptr->m_c_ptr = &noradobj_c;

				mse::TNoradConstPointer<C> rrcp = d_ptr->m_c_ptr;
				mse::TNoradConstPointer<C> rrcp2 = rrcp;
				const mse::TNoradObj<C> noradobj_e;
				rrcp = &noradobj_e;
				mse::TNoradFixedConstPointer<C> rrfcp = &noradobj_e;
				rrcp = mse::norad_new<C>();
				mse::norad_delete<C>(rrcp);

				/* We must make sure that there are no other references to the target of d_ptr before deleting it. Registered pointers don't
				have the same requirement. */
				noradobj_c.m_d_ptr = nullptr;

				mse::norad_delete<D>(d_ptr);

				{
					/* Polymorphic conversions. */
					class FD : public mse::TNoradObj<D> {};
					mse::TNoradObj<FD> norad_fd;
					mse::TNoradPointer<FD> FD_norad_ptr1 = &norad_fd;
					mse::TNoradPointer<D> D_norad_ptr4 = FD_norad_ptr1;
					D_norad_ptr4 = &norad_fd;
					mse::TNoradFixedPointer<D> D_norad_fptr1 = &norad_fd;
					mse::TNoradFixedConstPointer<D> D_norad_fcptr1 = &norad_fd;
				}

#endif // MSE_SELF_TESTS
			}
		};
	}

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

#endif // MSENORAD_H_
