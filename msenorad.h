
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

	template<typename _Ty> class TWNoradObj;
	template<typename _Ty> class TWNoradPointer;
	template<typename _Ty> class TWNoradConstPointer;
	template<typename _Ty> class TWNoradNotNullPointer;
	template<typename _Ty> class TWNoradNotNullConstPointer;
	template<typename _Ty> class TWNoradFixedPointer;
	template<typename _Ty> class TWNoradFixedConstPointer;

#ifdef MSE_NORADPOINTER_DISABLED
	template<typename _Ty> using TNoradPointer = _Ty * ;
	template<typename _Ty> using TNoradConstPointer = const _Ty*;
	template<typename _Ty> using TNoradNotNullPointer = _Ty * ;
	template<typename _Ty> using TNoradNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TNoradFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																			 library containers don't support const elements. */
	template<typename _Ty> using TNoradFixedConstPointer = const _Ty* /*const*/;
	template<typename _TROFLy> using TNoradObj = _TROFLy;
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

	template<typename _Ty> auto norad_fptr_to(_Ty&& _X) { return &_X; }
	template<typename _Ty> auto norad_fptr_to(const _Ty& _X) { return &_X; }

#else /*MSE_NORADPOINTER_DISABLED*/

	class norad_cannot_verify_cast_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> using TNoradPointer = TWNoradPointer<_Ty>;
	template<typename _Ty> using TNoradConstPointer = TWNoradConstPointer<_Ty>;
	template<typename _Ty> using TNoradNotNullPointer = TWNoradNotNullPointer<_Ty>;
	template<typename _Ty> using TNoradNotNullConstPointer = TWNoradNotNullConstPointer<_Ty>;
	template<typename _Ty> using TNoradFixedPointer = TWNoradFixedPointer<_Ty>;
	template<typename _Ty> using TNoradFixedConstPointer = TWNoradFixedConstPointer<_Ty>;
	template<typename _TROFLy> using TNoradObj = TWNoradObj<_TROFLy>;

	template<typename _Ty>
	auto norad_fptr_to(_Ty&& _X) {
		return _X.mse_norad_fptr();
	}
	template<typename _Ty>
	auto norad_fptr_to(const _Ty& _X) {
		return _X.mse_norad_fptr();
	}

#endif /*MSE_NORADPOINTER_DISABLED*/

	/* TWNoradPointer<>, like TWCRegisteredPointer<>, behaves similar to native pointers. But where registered pointers are
	automatically set to nullptr when their target is destroyed, the destruction of an object while a "norad" pointer is targeting
	it results in program termination. This drastic consequence allows norad pointers' run-time safety mechanism to be very
	lightweight (compared to that of registered pointers). */
	template<typename _Ty>
	class TWNoradPointer : public mse::us::impl::TPointer<TWNoradObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
	public:
		TWNoradPointer() : mse::us::impl::TPointer<TWNoradObj<_Ty>>() {}
		TWNoradPointer(const TWNoradPointer& src_cref) : mse::us::impl::TPointer<TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradPointer(const TWNoradPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		TWNoradPointer(TWNoradPointer&& src_ref) : mse::us::impl::TPointer<TWNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}
		TWNoradPointer(std::nullptr_t) : mse::us::impl::TPointer<TWNoradObj<_Ty>>(nullptr) {}
		virtual ~TWNoradPointer() {
			if (*this) { (*(*this)).decrement_refcount(); }
		}
		TWNoradPointer<_Ty>& operator=(const TWNoradPointer<_Ty>& _Right_cref) {
			if (*this) { (*(*this)).decrement_refcount(); }
			mse::us::impl::TPointer<TWNoradObj<_Ty>>::operator=(_Right_cref);
			if (*this) { (*(*this)).increment_refcount(); }
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradPointer<_Ty>& operator=(const TWNoradPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWNoradPointer(_Right_cref));
		}

		TWNoradObj<_Ty>& operator*() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		TWNoradObj<_Ty>* operator->() const {
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
			auto a = asANativePointerToTWNoradObj();
			(*this) = nullptr;
			delete a;
		}

	private:
		TWNoradPointer(TWNoradObj<_Ty>* ptr) : mse::us::impl::TPointer<TWNoradObj<_Ty>>(ptr) {
			assert(*this);
			(*(*this)).increment_refcount();
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by norad_delete<>(). */
		TWNoradObj<_Ty>* asANativePointerToTWNoradObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<TWNoradObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWNoradPointer;
		template <class Y> friend class TWNoradConstPointer;
		friend class TWNoradNotNullPointer<_Ty>;
	};

	template<typename _Ty>
	class TWNoradConstPointer : public mse::us::impl::TPointer<const TWNoradObj<_Ty> >, public mse::us::impl::StrongPointerTagBase {
	public:
		TWNoradConstPointer() : mse::us::impl::TPointer<const TWNoradObj<_Ty>>() {}
		TWNoradConstPointer(const TWNoradConstPointer& src_cref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradConstPointer(const TWNoradConstPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		TWNoradConstPointer(const TWNoradPointer<_Ty>& src_cref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradConstPointer(const TWNoradPointer<_Ty2>& src_cref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(src_cref.m_ptr) {
			if (*this) { (*(*this)).increment_refcount(); }
		}

		TWNoradConstPointer(TWNoradConstPointer&& src_ref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}
		TWNoradConstPointer(TWNoradPointer<_Ty>&& src_ref) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(std::forward<decltype(src_ref.m_ptr)>(src_ref.m_ptr)) {
			src_ref.m_ptr = nullptr;
		}

		TWNoradConstPointer(std::nullptr_t) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(nullptr) {}
		virtual ~TWNoradConstPointer() {
			if (*this) { (*(*this)).decrement_refcount(); }
		}
		TWNoradConstPointer<_Ty>& operator=(const TWNoradConstPointer<_Ty>& _Right_cref) {
			if (*this) { (*(*this)).decrement_refcount(); }
			mse::us::impl::TPointer<const TWNoradObj<_Ty>>::operator=(_Right_cref);
			if (*this) { (*(*this)).increment_refcount(); }
			return (*this);
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradConstPointer<_Ty>& operator=(const TWNoradConstPointer<_Ty2>& _Right_cref) {
			return (*this).operator=(TWNoradConstPointer(_Right_cref));
		}

		const TWNoradObj<_Ty>& operator*() const {
			if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		const TWNoradObj<_Ty>* operator->() const {
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
			auto a = asANativePointerToTWNoradObj();
			(*this) = nullptr;
			delete a;
		}

	private:
		TWNoradConstPointer(const TWNoradObj<_Ty>* ptr) : mse::us::impl::TPointer<const TWNoradObj<_Ty>>(ptr) {
			assert(*this);
			(*(*this)).increment_refcount();
		}

		/* This function, if possible, should not be used. It is meant to be used exclusively by norad_delete<>(). */
		const TWNoradObj<_Ty>* asANativePointerToTWNoradObj() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == (*this).m_ptr) {
				int q = 5; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return static_cast<const TWNoradObj<_Ty>*>((*this).m_ptr);
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template <class Y> friend class TWNoradConstPointer;
		friend class TWNoradNotNullConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TWNoradNotNullPointer : public TWNoradPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		TWNoradNotNullPointer(const TWNoradNotNullPointer& src_cref) : TWNoradPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullPointer(const TWNoradNotNullPointer<_Ty2>& src_cref) : TWNoradPointer<_Ty>(src_cref) {}
		TWNoradNotNullPointer(TWNoradNotNullPointer&& src_ref) : TWNoradPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TWNoradNotNullPointer() {}
		/*
		TWNoradNotNullPointer<_Ty>& operator=(const TWNoradNotNullPointer<_Ty>& _Right_cref) {
		TWNoradPointer<_Ty>::operator=(_Right_cref);
		return (*this);
		}
		*/

		TWNoradObj<_Ty>& operator*() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		TWNoradObj<_Ty>* operator->() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWNoradPointer<_Ty>::operator _Ty*(); }
		explicit operator TWNoradObj<_Ty>*() const { return TWNoradPointer<_Ty>::operator TWNoradObj<_Ty>*(); }

	private:
		TWNoradNotNullPointer(TWNoradObj<_Ty>* ptr) : TWNoradPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWNoradNotNullPointer(const  TWNoradPointer<_Ty>& src_cref) : TWNoradPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullPointer(const TWNoradPointer<_Ty2>& src_cref) : TWNoradPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		/* If you want a pointer to a TWNoradNotNullPointer<_Ty>, declare the TWNoradNotNullPointer<_Ty> as a
		TWNoradObj<TWNoradNotNullPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWNoradObj<TWNoradNotNullPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWNoradFixedPointer<_Ty>;
		template<typename _Ty2>
		friend TWNoradNotNullPointer<_Ty2> not_null_from_nullable(const TWNoradPointer<_Ty2>& src);
	};

	template<typename _Ty>
	class TWNoradNotNullConstPointer : public TWNoradConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		TWNoradNotNullConstPointer(const TWNoradNotNullPointer<_Ty>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullConstPointer(const TWNoradNotNullPointer<_Ty2>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {}
		TWNoradNotNullConstPointer(const TWNoradNotNullConstPointer<_Ty>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullConstPointer(const TWNoradNotNullConstPointer<_Ty2>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {}

		TWNoradNotNullConstPointer(TWNoradNotNullPointer<_Ty>&& src_ref) : TWNoradConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TWNoradNotNullConstPointer(TWNoradNotNullConstPointer<_Ty>&& src_ref) : TWNoradConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TWNoradNotNullConstPointer() {}

		const TWNoradObj<_Ty>& operator*() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return *((*this).m_ptr);
		}
		const TWNoradObj<_Ty>* operator->() const {
			//if (!((*this).m_ptr)) { MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TNoradPointer")); }
			return (*this).m_ptr;
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWNoradConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWNoradObj<_Ty>*() const { return TWNoradConstPointer<_Ty>::operator const TWNoradObj<_Ty>*(); }

	private:
		TWNoradNotNullConstPointer(const TWNoradObj<_Ty>* ptr) : TWNoradConstPointer<_Ty>(ptr) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TWNoradNotNullConstPointer(const TWNoradPointer<_Ty>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullConstPointer(const TWNoradPointer<_Ty2>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TWNoradNotNullConstPointer(const TWNoradConstPointer<_Ty>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradNotNullConstPointer(const TWNoradConstPointer<_Ty2>& src_cref) : TWNoradConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWNoradFixedConstPointer<_Ty>;
		template<typename _Ty2>
		friend TWNoradNotNullConstPointer<_Ty2> not_null_from_nullable(const TWNoradConstPointer<_Ty2>& src);
	};

	template<typename _Ty>
	TWNoradNotNullPointer<_Ty> not_null_from_nullable(const TWNoradPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TWNoradNotNullConstPointer<_Ty> not_null_from_nullable(const TWNoradConstPointer<_Ty>& src) {
		return src;
	}

	/* TWNoradFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TWNoradFixedPointer : public TWNoradNotNullPointer<_Ty> {
	public:
		TWNoradFixedPointer(const TWNoradFixedPointer& src_cref) : TWNoradNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedPointer(const TWNoradFixedPointer<_Ty2>& src_cref) : TWNoradNotNullPointer<_Ty>(src_cref) {}

		TWNoradFixedPointer(const TWNoradNotNullPointer<_Ty>& src_cref) : TWNoradNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedPointer(const TWNoradNotNullPointer<_Ty2>& src_cref) : TWNoradNotNullPointer<_Ty>(src_cref) {}

		TWNoradFixedPointer(TWNoradFixedPointer&& src_ref) : TWNoradNotNullPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TWNoradFixedPointer(TWNoradNotNullPointer<_Ty>&& src_ref) : TWNoradNotNullPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TWNoradFixedPointer() {}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TWNoradNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TWNoradObj<_Ty>*() const { return TWNoradNotNullPointer<_Ty>::operator TWNoradObj<_Ty>*(); }

	private:
		TWNoradFixedPointer(TWNoradObj<_Ty>* ptr) : TWNoradNotNullPointer<_Ty>(ptr) {}
		TWNoradFixedPointer<_Ty>& operator=(const TWNoradFixedPointer<_Ty>& _Right_cref) = delete;

		/* If you want a pointer to a TWNoradFixedPointer<_Ty>, declare the TWNoradFixedPointer<_Ty> as a
		TWNoradObj<TWNoradFixedPointer<_Ty>> instead. So for example:
		auto reg_ptr = TWNoradObj<TWNoradFixedPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWNoradObj<_Ty>;
	};

	template<typename _Ty>
	class TWNoradFixedConstPointer : public TWNoradNotNullConstPointer<_Ty> {
	public:
		TWNoradFixedConstPointer(const TWNoradFixedPointer<_Ty>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedConstPointer(const TWNoradFixedPointer<_Ty2>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		TWNoradFixedConstPointer(const TWNoradFixedConstPointer<_Ty>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedConstPointer(const TWNoradFixedConstPointer<_Ty2>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}

		TWNoradFixedConstPointer(const TWNoradNotNullPointer<_Ty>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedConstPointer(const TWNoradNotNullPointer<_Ty2>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		TWNoradFixedConstPointer(const TWNoradNotNullConstPointer<_Ty>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TWNoradFixedConstPointer(const TWNoradNotNullConstPointer<_Ty2>& src_cref) : TWNoradNotNullConstPointer<_Ty>(src_cref) {}

		TWNoradFixedConstPointer(TWNoradFixedPointer<_Ty>&& src_ref) : TWNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TWNoradFixedConstPointer(TWNoradFixedConstPointer<_Ty>&& src_ref) : TWNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		TWNoradFixedConstPointer(TWNoradNotNullPointer<_Ty>&& src_ref) : TWNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}
		TWNoradFixedConstPointer(TWNoradNotNullConstPointer<_Ty>&& src_ref) : TWNoradNotNullConstPointer<_Ty>(std::forward<decltype(src_ref)>(src_ref)) {}

		virtual ~TWNoradFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TWNoradNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TWNoradObj<_Ty>*() const { return TWNoradNotNullConstPointer<_Ty>::operator const TWNoradObj<_Ty>*(); }

	private:
		TWNoradFixedConstPointer(const TWNoradObj<_Ty>* ptr) : TWNoradNotNullConstPointer<_Ty>(ptr) {}
		TWNoradFixedConstPointer<_Ty>& operator=(const TWNoradFixedConstPointer<_Ty>& _Right_cref) = delete;

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TWNoradObj<_Ty>;
	};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_NORAD_OBJ_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	/* TWNoradObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TWNoradPointers will avoid referencing destroyed objects. Note that TWNoradObj can be used with
	objects allocated on the stack. */
	template<typename _TROFLy>
	class TWNoradObj : public _TROFLy, public std::conditional<(!std::is_convertible<_TROFLy*, mse::us::impl::NotAsyncShareableTagBase*>::value) && (!std::is_base_of<mse::us::impl::NotAsyncShareableTagBase, _TROFLy>::value)
		, mse::us::impl::NotAsyncShareableTagBase, impl::TPlaceHolder_msepointerbasics<TWNoradObj<_TROFLy> > >::type
	{
	public:
		typedef _TROFLy base_class;

		MSE_NORAD_OBJ_USING(TWNoradObj, _TROFLy);
		TWNoradObj(const TWNoradObj& _X) : _TROFLy(_X) {}
		TWNoradObj(TWNoradObj&& _X) : _TROFLy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TWNoradObj() {
			if (0 != m_counter) {
				/* It would be unsafe to allow this object to be destroyed as there are outstanding references to this object. */
				std::cerr << "\n\nFatal Error: mse::TWNoradObj<> destructed with outstanding references \n\n";
				std::terminate();
			}
		}

		template<class _Ty2>
		TWNoradObj& operator=(_Ty2&& _X) { _TROFLy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TWNoradObj& operator=(const _Ty2& _X) { _TROFLy::operator=(_X); return (*this); }

		TWNoradFixedPointer<_TROFLy> operator&() {
			return TWNoradFixedPointer<_TROFLy>(this);
		}
		TWNoradFixedConstPointer<_TROFLy> operator&() const {
			return TWNoradFixedConstPointer<_TROFLy>(this);
		}
		TWNoradFixedPointer<_TROFLy> mse_norad_fptr() { return TWNoradFixedPointer<_TROFLy>(this); }
		TWNoradFixedConstPointer<_TROFLy> mse_norad_fptr() const { return TWNoradFixedConstPointer<_TROFLy>(this); }

		/* todo: make these private */
		void increment_refcount() const { m_counter += 1; }
		void decrement_refcount() const { m_counter -= 1; }

	private:
		mutable int m_counter = 0;
	};


#ifdef MSE_NORADPOINTER_DISABLED
#else /*MSE_NORADPOINTER_DISABLED*/

	/* See registered_new(). */
	template <class _Ty, class... Args>
	TNoradPointer<_Ty> norad_new(Args&&... args) {
		auto a = new TNoradObj<_Ty>(std::forward<Args>(args)...);
		mse::us::impl::tlSAllocRegistry_ref<TNoradObj<_Ty> >().registerPointer(a);
		return &(*a);
	}
	template <class _Ty>
	void norad_delete(TNoradPointer<_Ty>& regPtrRef) {
		auto a = static_cast<TNoradObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TNoradObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::norad_delete() \n- tip: If deleting via base class pointer, use mse::us::norad_delete() instead. ")); }
		regPtrRef.norad_delete();
	}
	template <class _Ty>
	void norad_delete(TNoradConstPointer<_Ty>& regPtrRef) {
		auto a = static_cast<const TNoradObj<_Ty>*>(regPtrRef);
		auto res = mse::us::impl::tlSAllocRegistry_ref<TNoradObj<_Ty> >().unregisterPointer(a);
		if (!res) { assert(false); MSE_THROW(std::invalid_argument("invalid argument, no corresponding allocation found - mse::norad_delete() \n- tip: If deleting via base class pointer, use mse::us::norad_delete() instead. ")); }
		regPtrRef.norad_delete();
	}
	namespace us {
		template <class _Ty>
		void norad_delete(TNoradPointer<_Ty>& regPtrRef) {
			regPtrRef.norad_delete();
		}
		template <class _Ty>
		void norad_delete(TNoradConstPointer<_Ty>& regPtrRef) {
			regPtrRef.norad_delete();
		}
	}
#endif /*MSE_NORADPOINTER_DISABLED*/

}

namespace std {
	template<class _Ty>
	struct hash<mse::TWNoradPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWNoradNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWNoradFixedPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TWNoradConstPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWNoradNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TWNoradFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TWNoradFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TWNoradFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
	class TWNoradObj<_Ty*> : public TWNoradObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<typename _Ty>
	class TWNoradObj<_Ty* const> : public TWNoradObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<typename _Ty>
	class TWNoradObj<const _Ty *> : public TWNoradObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<typename _Ty>
	class TWNoradObj<const _Ty * const> : public TWNoradObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};

	template<typename _Ty>
	class TWNoradPointer<_Ty*> : public TWNoradPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradPointer<_Ty* const> : public TWNoradPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradPointer<const _Ty *> : public TWNoradPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradPointer<const _Ty * const> : public TWNoradPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};

	template<typename _Ty>
	class TWNoradConstPointer<_Ty*> : public TWNoradConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradConstPointer<_Ty* const> : public TWNoradConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TWNoradConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradConstPointer<const _Ty *> : public TWNoradConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};
	template<typename _Ty>
	class TWNoradConstPointer<const _Ty * const> : public TWNoradConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TWNoradConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TWNoradObj<int> : public TWNoradObj<mse::TInt<int>> {
	public:
		typedef TWNoradObj<mse::TInt<int>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<>
	class TWNoradObj<const int> : public TWNoradObj<const mse::TInt<int>> {
	public:
		typedef TWNoradObj<const mse::TInt<int>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<>
	class TWNoradPointer<int> : public TWNoradPointer<mse::TInt<int>> {
	public:
		typedef TWNoradPointer<mse::TInt<int>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<>
	class TWNoradPointer<const int> : public TWNoradPointer<const mse::TInt<int>> {
	public:
		typedef TWNoradPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<>
	class TWNoradConstPointer<int> : public TWNoradConstPointer<mse::TInt<int>> {
	public:
		typedef TWNoradConstPointer<mse::TInt<int>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};
	template<>
	class TWNoradConstPointer<const int> : public TWNoradConstPointer<const mse::TInt<int>> {
	public:
		typedef TWNoradConstPointer<const mse::TInt<int>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};

	template<>
	class TWNoradObj<size_t> : public TWNoradObj<mse::TInt<size_t>> {
	public:
		typedef TWNoradObj<mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<>
	class TWNoradObj<const size_t> : public TWNoradObj<const mse::TInt<size_t>> {
	public:
		typedef TWNoradObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradObj, base_class);
	};
	template<>
	class TWNoradPointer<size_t> : public TWNoradPointer<mse::TInt<size_t>> {
	public:
		typedef TWNoradPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<>
	class TWNoradPointer<const size_t> : public TWNoradPointer<const mse::TInt<size_t>> {
	public:
		typedef TWNoradPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradPointer, base_class);
	};
	template<>
	class TWNoradConstPointer<size_t> : public TWNoradConstPointer<mse::TInt<size_t>> {
	public:
		typedef TWNoradConstPointer<mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
	};
	template<>
	class TWNoradConstPointer<const size_t> : public TWNoradConstPointer<const mse::TInt<size_t>> {
	public:
		typedef TWNoradConstPointer<const mse::TInt<size_t>> base_class;
		MSE_USING(TWNoradConstPointer, base_class);
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
