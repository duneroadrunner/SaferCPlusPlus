
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTING_H_
#define MSEREFCOUNTING_H_

#include "msepointerbasics.h"
#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
#include "msescope.h"
#include "mseslta.h"
#include "mseoptional.h"
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
#include <memory>
#include <utility>
#include <cassert>
#include <stdexcept>

#ifdef MSE_SELF_TESTS
#include <map>
#include <string>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <locale>
#endif // MSE_SELF_TESTS

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/


#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTINGPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NOEXCEPT")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifndef MSE_CONSTEXPR23
	#ifdef MSE_HAS_CXX23
		#define MSE_CONSTEXPR23 constexpr
	#else // MSE_HAS_CXX23
		#define MSE_CONSTEXPR23
	#endif // MSE_HAS_CXX23
#endif // !MSE_CONSTEXPR23



namespace mse {

	namespace us {
		namespace impl {
#ifdef MSEPOINTERBASICS_H
			typedef mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase RefCStrongPointerTagBase;
#else // MSEPOINTERBASICS_H
			class mse::us::impl::RefCStrongPointerTagBase {};
#endif // MSEPOINTERBASICS_H
		}
	}

	class refcounting_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	class CRefCounter {
	private:
		int m_counter;

	public:
		CRefCounter() : m_counter(1) {}
		virtual ~CRefCounter() {}
		void increment() { m_counter++; }
		void decrement() { assert(0 <= m_counter); m_counter--; }
		int use_count() const { return m_counter; }
		virtual void* target_obj_address() const = 0;
	};

	template<class Y>
	class TRefWithTargetObj : public CRefCounter {
	public:
		Y m_object;

		template<class ... Args>
		TRefWithTargetObj(Args && ...args) : m_object(std::forward<Args>(args)...) {}

		void* target_obj_address() const {
			return const_cast<void*>(static_cast<const void*>(std::addressof(m_object)));
		}
	};

#ifdef MSE_REFCOUNTINGPOINTER_DISABLED
	template <class X> using TRefCountingPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingNotNullPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingFixedPointer = /*const*/ std::shared_ptr<X>; /* Can't be const qualified because standard
																					  library containers don't support const elements. */
	template <class X> using TRefCountingConstPointer = std::shared_ptr<const X>;
	template <class X> using TRefCountingNotNullConstPointer = std::shared_ptr<const X>;
	template <class X> using TRefCountingFixedConstPointer = /*const*/ std::shared_ptr<const X>;

	template<typename _Ty> TRefCountingNotNullPointer<_Ty> not_null_from_nullable(const TRefCountingPointer<_Ty>& src);
	template<typename _Ty> TRefCountingNotNullConstPointer<_Ty> not_null_from_nullable(const TRefCountingConstPointer<_Ty>& src);

	template <class X, class... Args>
	TRefCountingNotNullPointer<X> make_refcounting(Args&&... args) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)") {
		return std::make_shared<X>(std::forward<Args>(args)...);
	}

	template <class X, class... Args>
	TRefCountingPointer<X> make_nullable_refcounting(Args&&... args) {
		return std::make_shared<X>(std::forward<Args>(args)...);
	}

#else /*MSE_REFCOUNTINGPOINTER_DISABLED*/

	template<typename _Ty> class TRefCountingPointer;
	template<typename _Ty> class TRefCountingNotNullPointer;
	template<typename _Ty> class TRefCountingFixedPointer;
	template<typename _Ty> class TRefCountingConstPointer;
	template<typename _Ty> class TRefCountingNotNullConstPointer;
	template<typename _Ty> class TRefCountingFixedConstPointer;

	template<typename _Ty> TRefCountingNotNullPointer<_Ty> not_null_from_nullable(const TRefCountingPointer<_Ty>& src);
	template<typename _Ty> TRefCountingNotNullConstPointer<_Ty> not_null_from_nullable(const TRefCountingConstPointer<_Ty>& src);

	/* Some code originally came from this stackoverflow post:
	http://stackoverflow.com/questions/6593770/creating-a-non-thread-safe-shared-ptr */

	template <class X> class TRefCountingConstPointer;

	/* TRefCountingPointer behaves similar to an std::shared_ptr. Some differences being that it foregoes any thread safety
	mechanisms, it does not accept raw pointer assignment or construction (use make_refcounting<>() instead), and it will throw
	an exception on attempted nullptr dereference. And it's faster. */
	template <class X>
	class TRefCountingPointer : public mse::us::impl::RefCStrongPointerTagBase {
	public:
		using element_type = X;
		TRefCountingPointer() : m_ref_with_target_obj_ptr(nullptr) {}
		TRefCountingPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
		~TRefCountingPointer() {
			//release();
			/* Doing it this way instead of just calling release() protects against potential reentrant destructor
			calls caused by a misbehaving (user-defined) destructor of the target object. */
			auto_release keep(m_ref_with_target_obj_ptr);
			m_ref_with_target_obj_ptr = nullptr;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = nullptr;)

			/* This is just a no-op function that will cause a compile error when X is not an eligible type. */
			valid_if_X_is_not_an_xscope_type();
		}
		TRefCountingPointer(const TRefCountingPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}
		TRefCountingPointer(TRefCountingPointer&& r) {
			m_ref_with_target_obj_ptr = r.m_ref_with_target_obj_ptr;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			r.m_ref_with_target_obj_ptr = nullptr;
			MSE_IF_DEBUG(r.m_debug_target_obj_cptr = nullptr;)
		}
		TRefCountingPointer(const TRefCountingNotNullPointer<X>& r);
		TRefCountingPointer(TRefCountingNotNullPointer<X>&& r);
		explicit operator bool() const { return nullptr != get(); }
		void reset() { (*this) = TRefCountingPointer<X>(nullptr); }
		MSE_DEPRECATED void clear() { (*this) = TRefCountingPointer<X>(nullptr); }
		TRefCountingPointer& operator=(const TRefCountingPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}
			return *this;
		}
		MSE_DEPRECATED bool operator<(const TRefCountingPointer& r) const {
			return get() < r.get();
		}
		bool operator==(const TRefCountingPointer& r) const {
			return get() == r.get();
		}
		bool operator!=(const TRefCountingPointer& r) const {
			return get() != r.get();
		}

#ifndef MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES
		/* Apparently msvc2015 requires that templated member functions come before regular ones.
		From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
		"If, in the same class, you have a regular member function and a templated member function with the same
		signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
		*/
		template <class Y> friend class TRefCountingPointer;
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingPointer(const TRefCountingPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingPointer& operator=(const TRefCountingPointer<Y>& r) {
			if (((void const*)this) != ((void const*)std::addressof(r))) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}
			return *this;
		}
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullConstPointer<Y>& r) const { return !((*this) == r); }
#endif // !MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES

		X& operator*() const {
			if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return (*x_ptr);
		}
		X* operator->() const {
			if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return x_ptr;
		}
		bool unique() const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

		template <class... Args>
		static TRefCountingPointer make(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<X>(std::forward<Args>(args)...);
			TRefCountingPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingPointer(TRefWithTargetObj<X>* p/* = nullptr*/) {
			m_ref_with_target_obj_ptr = p;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}

		struct auto_release {
			auto_release(CRefCounter* c) : m_ref_with_target_obj_ptr(c) {}
			~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
			CRefCounter* m_ref_with_target_obj_ptr;
		};

		void static dorelease(CRefCounter* ref_with_target_obj_ptr) {
			// decrement the count, delete if it is nullptr
			if (ref_with_target_obj_ptr) {
				if (1 == ref_with_target_obj_ptr->use_count()) {
					delete ref_with_target_obj_ptr;
				}
				else {
					ref_with_target_obj_ptr->decrement();
				}
			}
		}

		X* get() const {
			if (!m_ref_with_target_obj_ptr) {
				return nullptr;
			}
			else {
				X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				return x_ptr;
			}
		}
		X* unchecked_get() const {
			X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return x_ptr;
		}

#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class X2 = X, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<X2, X>::value) && (mse::impl::is_potentially_not_xscope<X2>::value)> MSE_IMPL_EIS >
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		void valid_if_X_is_not_an_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CRefCounter* m_ref_with_target_obj_ptr;

		MSE_IF_DEBUG(X const* m_debug_target_obj_cptr = nullptr;)

		template <class Y> friend class TRefCountingPointer;
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y>  friend class TRefCountingNotNullPointer;
		template <class Y> friend class TRefCountingNotNullConstPointer;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)");

	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TRefCountingPointer);
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(std::shared_ptr);

	template<typename _Ty>
	class TRefCountingNotNullPointer : public mse::us::impl::RefCStrongPointerTagBase, public mse::us::impl::NeverNullTagBase {
	public:
		using element_type = _Ty;
		TRefCountingNotNullPointer(const TRefCountingNotNullPointer& src_cref) : m_rcptr(src_cref.m_rcptr) {
			if (!(src_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullPointer")); }
		}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<_Ty, Y>::value> MSE_IMPL_EIS >
		TRefCountingNotNullPointer(const TRefCountingNotNullPointer<Y>& r) : m_rcptr(r.m_rcptr) {
			if (!(r.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullPointer")); }
		}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TRefCountingNotNullPointer() {}
		explicit operator bool() const { return true; }
		TRefCountingNotNullPointer<_Ty>& operator=(const TRefCountingNotNullPointer<_Ty>& _Right_cref) {
			if (!(_Right_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullPointer")); }
			m_rcptr  = (_Right_cref.m_rcptr);
			return (*this);
		}
		bool operator==(const TRefCountingNotNullPointer& r) const { return m_rcptr == r.m_rcptr; }
		bool operator!=(const TRefCountingNotNullPointer& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullConstPointer<Y>& r) const { return !((*this) == r); }

		_Ty& operator*() const {
			//if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			_Ty* x_ptr = m_rcptr.unchecked_get();
			return *x_ptr;
		}
		_Ty* operator->() const {
			//if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingPointer")); }
			_Ty* x_ptr = m_rcptr.unchecked_get();
			return x_ptr;
		}
		bool unique() const {
			return m_rcptr.unique();
		}

		template <class... Args>
		static TRefCountingNotNullPointer make(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(std::forward<Args>(args)...);
			TRefCountingNotNullPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingNotNullPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : m_rcptr(p) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TRefCountingNotNullPointer(const TRefCountingPointer<_Ty>& src_cref) : m_rcptr(src_cref) {
			if (!m_rcptr) { MSE_THROW(refcounting_null_dereference_error("attempt to construct a 'not null' pointer from a null pointer value - mse::TRefCountingNotNullPointer")); }
		}
		_Ty* unchecked_get() const {
			return m_rcptr.unchecked_get();
		}
		_Ty* get() const {
			return unchecked_get();
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		TRefCountingPointer<_Ty> m_rcptr;

		template <class Y>  friend class TRefCountingPointer;
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y> friend class TRefCountingNotNullPointer;
		template <class Y> friend class TRefCountingNotNullConstPointer;
		template <class Y> friend class TRefCountingFixedPointer;
		template <class Y> friend class TRefCountingFixedConstPointer;
		template<typename _Ty2>
		friend TRefCountingNotNullPointer<_Ty2> not_null_from_nullable(const TRefCountingPointer<_Ty2>& src);
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TRefCountingNotNullPointer);

	template <class X> TRefCountingPointer<X>::TRefCountingPointer(const TRefCountingNotNullPointer<X>& r) : TRefCountingPointer(r.m_rcptr){}
	template <class X> TRefCountingPointer<X>::TRefCountingPointer(TRefCountingNotNullPointer<X>&& r) : TRefCountingPointer(MSE_FWD(r.m_rcptr)) {}

	/* TRefCountingFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. (This implementation that publicly inherits from TRefCountingNotNullPointer<> is just a 
	placeholder implementation that doesn't fully prevent its claimed invariant from being violated.) */
	template<typename _Ty>
	class TRefCountingFixedPointer : public TRefCountingNotNullPointer<_Ty> {
	public:
		TRefCountingFixedPointer(const TRefCountingFixedPointer& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		TRefCountingFixedPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TRefCountingFixedPointer() {}

		template <class... Args>
		static TRefCountingFixedPointer make(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(std::forward<Args>(args)...);
			TRefCountingFixedPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingFixedPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountingNotNullPointer<_Ty>(p) {}

		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TRefCountingFixedPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}

		TRefCountingFixedPointer<_Ty>& operator=(const TRefCountingFixedPointer<_Ty>& _Right_cref) = delete;

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		friend class TRefCountingConstPointer<_Ty>;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TRefCountingFixedPointer);

	template <class X, class... Args>
	TRefCountingNotNullPointer<X> make_refcounting(Args&&... args) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)") {
		return TRefCountingNotNullPointer<X>::make(std::forward<Args>(args)...);
	}

	template <class X, class... Args>
	TRefCountingPointer<X> make_nullable_refcounting(Args&&... args) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)") {
		return TRefCountingPointer<X>::make(std::forward<Args>(args)...);
	}


	template <class X>
	class TRefCountingConstPointer : public mse::us::impl::RefCStrongPointerTagBase {
	public:
		using element_type = X;
		TRefCountingConstPointer() : m_ref_with_target_obj_ptr(nullptr) {}
		TRefCountingConstPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
		~TRefCountingConstPointer() {
			//release();
			/* Doing it this way instead of just calling release() protects against potential reentrant destructor
			calls caused by a misbehaving (user-defined) destructor of the target object. */
			auto_release keep(m_ref_with_target_obj_ptr);
			m_ref_with_target_obj_ptr = nullptr;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = nullptr;)

			/* This is just a no-op function that will cause a compile error when X is not an eligible type. */
			valid_if_X_is_not_an_xscope_type();
		}
		TRefCountingConstPointer(const TRefCountingConstPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}
		TRefCountingConstPointer(const TRefCountingPointer<X>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}
		TRefCountingConstPointer(TRefCountingConstPointer&& r) {
			m_ref_with_target_obj_ptr = r.m_ref_with_target_obj_ptr;
			r.m_ref_with_target_obj_ptr = nullptr;
		}
		TRefCountingConstPointer(TRefCountingPointer<X>&& r) {
			m_ref_with_target_obj_ptr = r.m_ref_with_target_obj_ptr;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			r.m_ref_with_target_obj_ptr = nullptr;
			MSE_IF_DEBUG(r.m_debug_target_obj_cptr = nullptr;)
		}
		TRefCountingConstPointer(const TRefCountingNotNullConstPointer<X>& r);
		TRefCountingConstPointer(TRefCountingNotNullConstPointer<X>&& r);
		explicit operator bool() const { return nullptr != get(); }
		void reset() { (*this) = TRefCountingConstPointer<X>(nullptr); }
		MSE_DEPRECATED void clear() { (*this) = TRefCountingConstPointer<X>(nullptr); }
		TRefCountingConstPointer& operator=(const TRefCountingConstPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}
			return *this;
		}
		MSE_DEPRECATED bool operator<(const TRefCountingConstPointer& r) const {
			return get() < r.get();
		}
		bool operator==(const TRefCountingConstPointer& r) const {
			return get() == r.get();
		}
		bool operator!=(const TRefCountingConstPointer& r) const {
			return get() != r.get();
		}

#ifndef MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES
		/* Apparently msvc2015 requires that templated member functions come before regular ones.
		From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
		"If, in the same class, you have a regular member function and a templated member function with the same
		signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
		*/
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingConstPointer(const TRefCountingConstPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingConstPointer& operator=(const TRefCountingConstPointer<Y>& r) {
			if (((void const*)this) != ((void const*)std::addressof(r))) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}
			return *this;
		}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingConstPointer(const TRefCountingNotNullPointer<Y>& r) : TRefCountingConstPointer(r.m_rcptr) {}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
		TRefCountingConstPointer(TRefCountingNotNullPointer<Y>&& r) : TRefCountingConstPointer(MSE_FWD(r.m_rcptr)) {}
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullConstPointer<Y>& r) const { return !((*this) == r); }
#endif // !MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES

		const X& operator*() const {
			if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return (*x_ptr);
		}
		const X* operator->() const {
			if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return x_ptr;
		}
		bool unique() const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

	private:
		explicit TRefCountingConstPointer(TRefWithTargetObj<X>* p/* = nullptr*/) {
			m_ref_with_target_obj_ptr = p;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
			MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
		}

		struct auto_release {
			auto_release(CRefCounter* c) : m_ref_with_target_obj_ptr(c) {}
			~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
			CRefCounter* m_ref_with_target_obj_ptr;
		};

		void static dorelease(CRefCounter* ref_with_target_obj_ptr) {
			// decrement the count, delete if it is nullptr
			if (ref_with_target_obj_ptr) {
				if (1 == ref_with_target_obj_ptr->use_count()) {
					delete ref_with_target_obj_ptr;
				}
				else {
					ref_with_target_obj_ptr->decrement();
				}
				ref_with_target_obj_ptr = nullptr;
			}
		}

		const X* get() const {
			if (!m_ref_with_target_obj_ptr) {
				return nullptr;
			}
			else {
				X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				return x_ptr;
			}
		}
		const X* unchecked_get() const {
			const X* x_ptr = static_cast<const X*>(m_ref_with_target_obj_ptr->target_obj_address());
			return x_ptr;
		}

#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class X2 = X, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<X2, X>::value) && (mse::impl::is_potentially_not_xscope<X2>::value)> MSE_IMPL_EIS >
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		void valid_if_X_is_not_an_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		CRefCounter* m_ref_with_target_obj_ptr;

		MSE_IF_DEBUG(X const* m_debug_target_obj_cptr = nullptr;)

		template <class Y> friend class TRefCountingPointer;
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y>  friend class TRefCountingNotNullPointer;
		template <class Y> friend class TRefCountingNotNullConstPointer;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)");

	template<typename _Ty>
	class TRefCountingNotNullConstPointer : public mse::us::impl::RefCStrongPointerTagBase, public mse::us::impl::NeverNullTagBase {
	public:
		using element_type = _Ty;
		TRefCountingNotNullConstPointer(const TRefCountingNotNullConstPointer& src_cref) : m_rcptr(src_cref.m_rcptr) {
			if (!(src_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullConstPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullConstPointer")); }
		}
		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<_Ty, Y>::value> MSE_IMPL_EIS >
		TRefCountingNotNullConstPointer(const TRefCountingNotNullConstPointer<Y>& r) : m_rcptr(r.m_rcptr) {
			if (!(r.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullConstPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullConstPointer")); }
		}

		template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<_Ty, Y>::value> MSE_IMPL_EIS >
		TRefCountingNotNullConstPointer(const TRefCountingNotNullPointer<Y>& src_cref) : m_rcptr(src_cref.m_rcptr) {
			if (!(src_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullConstPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullConstPointer")); }
		}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TRefCountingNotNullConstPointer() {}
		explicit operator bool() const { return true; }
		TRefCountingNotNullConstPointer<_Ty>& operator=(const TRefCountingNotNullConstPointer<_Ty>& _Right_cref) {
			if (!(_Right_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TRefCountingNotNullConstPointer<> that's in a partially destructed (or constructed?) state - mse::TRefCountingNotNullConstPointer")); }
			m_rcptr = (_Right_cref.m_rcptr);
			return (*this);
		}
		bool operator==(const TRefCountingNotNullConstPointer& r) const { return m_rcptr == r.m_rcptr; }
		bool operator!=(const TRefCountingNotNullConstPointer& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullPointer<Y>& r) const { return !((*this) == r); }
		template <class Y> bool operator==(const TRefCountingNotNullConstPointer<Y>& r) const { return get() == r.get(); }
		template <class Y> bool operator!=(const TRefCountingNotNullConstPointer<Y>& r) const { return !((*this) == r); }

		const _Ty& operator*() const {
			//if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			const _Ty* x_ptr = (*this).unchecked_get();
			return *x_ptr;
		}
		const _Ty* operator->() const {
			//if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::TRefCountingConstPointer")); }
			const _Ty* x_ptr = (*this).unchecked_get();
			return x_ptr;
		}
		bool unique() const {
			return m_rcptr.unique();
		}

		template <class... Args>
		static TRefCountingNotNullConstPointer make(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(std::forward<Args>(args)...);
			TRefCountingNotNullConstPointer retval(new_ptr);
			return retval;
		}

	private:
		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TRefCountingNotNullConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : m_rcptr(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TRefCountingNotNullConstPointer(const TRefCountingPointer<_Ty>& src_cref) : m_rcptr(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		const _Ty* unchecked_get() const {
			return m_rcptr.unchecked_get();
		}
		const _Ty* get() const {
			return unchecked_get();
		}

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		TRefCountingConstPointer<_Ty> m_rcptr;

		template <class Y>  friend class TRefCountingPointer;
		template <class Y> friend class TRefCountingConstPointer;
		template <class Y> friend class TRefCountingNotNullPointer;
		template <class Y> friend class TRefCountingNotNullConstPointer;
		template <class Y> friend class TRefCountingFixedPointer;
		template <class Y> friend class TRefCountingFixedConstPointer;
		template<typename _Ty2>
		friend TRefCountingNotNullConstPointer<_Ty2> not_null_from_nullable(const TRefCountingConstPointer<_Ty2>& src);
	};

	template <class X> TRefCountingConstPointer<X>::TRefCountingConstPointer(const TRefCountingNotNullConstPointer<X>& r) : TRefCountingConstPointer(r.m_rcptr) {}
	template <class X> TRefCountingConstPointer<X>::TRefCountingConstPointer(TRefCountingNotNullConstPointer<X>&& r) : TRefCountingConstPointer(MSE_FWD(r.m_rcptr)) {}

	/* TRefCountingFixedConstPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. (This implementation that publicly inherits from TRefCountingNotNullConstPointer<> is just a 
	placeholder implementation that doesn't fully prevent its claimed invariant from being violated.) */
	template<typename _Ty>
	class TRefCountingFixedConstPointer : public TRefCountingNotNullConstPointer<_Ty> {
	public:
		TRefCountingFixedConstPointer(const TRefCountingFixedConstPointer& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingFixedPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingNotNullConstPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}

		TRefCountingFixedConstPointer(TRefCountingFixedConstPointer&& src_ref) : TRefCountingNotNullConstPointer<_Ty>(MSE_FWD(src_ref)) {}
		TRefCountingFixedConstPointer(TRefCountingFixedPointer<_Ty>&& src_ref) : TRefCountingNotNullConstPointer<_Ty>(MSE_FWD(src_ref)) {}
		TRefCountingFixedConstPointer(TRefCountingNotNullConstPointer<_Ty>&& src_ref) : TRefCountingNotNullConstPointer<_Ty>(MSE_FWD(src_ref)) {}
		TRefCountingFixedConstPointer(TRefCountingNotNullPointer<_Ty>&& src_ref) : TRefCountingNotNullConstPointer<_Ty>(MSE_FWD(src_ref)) {}

		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TRefCountingFixedConstPointer() {}

	private:
		/* If you want to use this constructor, use not_null_from_nullable() instead. */
		TRefCountingFixedConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}

		TRefCountingFixedConstPointer<_Ty>& operator=(const TRefCountingFixedConstPointer<_Ty>& _Right_cref) = delete;

		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
	};
}

namespace std {
	template<class _Ty>
	struct hash<mse::TRefCountingPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRefCountingNotNullPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRefCountingFixedPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TRefCountingConstPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRefCountingNotNullConstPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingNotNullConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingNotNullConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TRefCountingFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TRefCountingFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TRefCountingFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

#endif /*MSE_REFCOUNTINGPOINTER_DISABLED*/

	template<typename _Ty>
	TRefCountingNotNullPointer<_Ty> not_null_from_nullable(const TRefCountingPointer<_Ty>& src) {
		return src;
	}
	template<typename _Ty>
	TRefCountingNotNullConstPointer<_Ty> not_null_from_nullable(const TRefCountingConstPointer<_Ty>& src) {
		return src;
	}

#ifdef MSEPOINTERBASICS_H
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedPointer<_TTarget, TRefCountingPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingNotNullPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedPointer<_TTarget, TRefCountingNotNullPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingNotNullConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingNotNullConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingNotNullPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingNotNullPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingNotNullConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingNotNullConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingFixedPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedPointer<_TTarget, TRefCountingFixedPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingFixedConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingFixedConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingFixedPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingFixedPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingFixedConstPointer<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, TRefCountingFixedConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)

	template<class _TTargetType, class _Ty>
	us::TStrongFixedPointer<_TTargetType, std::shared_ptr<_Ty>> make_pointer_to_member(_TTargetType& target, const std::shared_ptr<_Ty>& lease_pointer) {
		return us::TStrongFixedPointer<_TTargetType, std::shared_ptr<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	us::TStrongFixedConstPointer<_TTargetType, std::shared_ptr<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const std::shared_ptr<_Ty>& lease_pointer) {
		return us::TStrongFixedConstPointer<_TTargetType, std::shared_ptr<_Ty>>::make(target, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const std::shared_ptr<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedPointer<_TTarget, std::shared_ptr<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const std::shared_ptr<_Ty>& lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef mse::impl::remove_reference_t<decltype((*lease_pointer).*member_object_ptr)> _TTarget;
		mse::impl::make_pointer_to_member_v2_checks_pb(lease_pointer, member_object_ptr);
		return us::TStrongFixedConstPointer<_TTarget, std::shared_ptr<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
#endif // MSEPOINTERBASICS_H


#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeRefCountingStore = TXScopeStrongPointerStore<TRefCountingPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingConstStore = TXScopeStrongConstPointerStore<TRefCountingConstPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingNotNullStore = TXScopeStrongNotNullPointerStore<TRefCountingNotNullPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingNotNullConstStore = TXScopeStrongNotNullConstPointerStore<TRefCountingNotNullConstPointer<_Ty> >;
#endif // MSESCOPE_H_


	/* shorter aliases */
	template<typename _Ty> using refcp = TRefCountingPointer<_Ty>;
	template<typename _Ty> using refccp = TRefCountingConstPointer<_Ty>;
	template<typename _Ty> using refcnnp = TRefCountingNotNullPointer<_Ty>;
	template<typename _Ty> using refcnncp = TRefCountingNotNullConstPointer<_Ty>;
	template<typename _Ty> using refcfp = TRefCountingFixedPointer<_Ty>;
	template<typename _Ty> using refcfcp = TRefCountingFixedConstPointer<_Ty>;

	template<typename _Ty> using refc_ptr = TRefCountingPointer<_Ty>;
	template<typename _Ty> using refc_cptr = TRefCountingConstPointer<_Ty>;
	template<typename _Ty> using refc_nnptr = TRefCountingNotNullPointer<_Ty>;
	template<typename _Ty> using refc_nncptr = TRefCountingNotNullConstPointer<_Ty>;
	template<typename _Ty> using refc_fptr = TRefCountingFixedPointer<_Ty>;
	template<typename _Ty> using refc_fcptr = TRefCountingFixedConstPointer<_Ty>;

	template <class X, class... Args>
	auto mkrc(Args&&... args) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(X)") {
		return make_refcounting<X>(std::forward<Args>(args)...);
	}

	/* These functions assume the type is the same as the lone parameter, so you don't need to explicitly specify it. */
	template <typename _TLoneParam>
	auto mkrclp(const _TLoneParam& lone_param) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TLoneParam)") {
		return make_refcounting<_TLoneParam>(lone_param);
	}
	template <typename _TLoneParam>
	auto mkrclp(_TLoneParam&& lone_param) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TLoneParam)") {
		return make_refcounting<mse::impl::remove_reference_t<_TLoneParam> >(MSE_FWD(lone_param));
	}


	namespace impl {
		template <typename T, typename = void>
		struct HasOrInheritsGetMethod_mserefcounting : std::false_type {};
		template <typename T>
		struct HasOrInheritsGetMethod_mserefcounting<T, mse::impl::void_t<decltype((mse::impl::decl_lval<T>()).get())> > : std::true_type {};
	}

#ifdef MSESLTA_H_
	namespace rsv {

		template<class TAccessingFixed, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::target_type<_TLender> >
		class TXSLTADynamicStrongPointerElementProxyRef : public mse::us::impl::XSLTATagBase {
		public:
			typedef TAccessingFixed accessing_fixed_type;
			typedef _TPointerToLender pointer_to_lender_type;
			typedef _TLender lender_type;
			typedef _Ty element_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyRef(TXSLTADynamicStrongPointerElementProxyRef&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyRef(TXSLTADynamicStrongPointerElementProxyRef&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyRef(const TXSLTADynamicStrongPointerElementProxyRef&) = delete;

#if 0
			TXSLTADynamicStrongPointerElementProxyRef(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_accessing_fixed(src_xs_ptr) {}
			TXSLTADynamicStrongPointerElementProxyRef(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_accessing_fixed(src_xs_ptr) {}
#endif // 0
			TXSLTADynamicStrongPointerElementProxyRef(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : m_accessing_fixed(src_xs_ptr) {}

			operator _Ty() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return m_accessing_fixed.operator *();
			}
			operator _Ty() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") = delete;
			void operator=(const _Ty& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))&& {
				m_accessing_fixed.operator *() = src_ref;
			}
			void operator=(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))&& {
				m_accessing_fixed.operator *() = MSE_FWD(src_ref);
			}
			void operator=(const _Ty& src_ref) & = delete;
			void operator=(_Ty&& src_ref) & = delete;

			void operator=(TXSLTADynamicStrongPointerElementProxyRef&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))&& {
				m_accessing_fixed.operator *() = _Ty(MSE_FWD(src_ref));
			}
			void operator=(TXSLTADynamicStrongPointerElementProxyRef&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) & = delete;
			void operator=(const TXSLTADynamicStrongPointerElementProxyRef& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) = delete;

		private:
			TAccessingFixed m_accessing_fixed MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TAccessingFixed, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::target_type<_TLender> >
		class TXSLTADynamicStrongPointerElementProxyConstRef : public mse::us::impl::XSLTATagBase {
		public:
			typedef TAccessingFixed accessing_fixed_type;
			typedef _TPointerToLender pointer_to_lender_type;
			typedef _TLender lender_type;
			typedef _Ty element_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstRef(TXSLTADynamicStrongPointerElementProxyConstRef&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstRef(TXSLTADynamicStrongPointerElementProxyConstRef&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstRef(const TXSLTADynamicStrongPointerElementProxyConstRef&) = delete;

#if 0
			TXSLTADynamicStrongPointerElementProxyConstRef(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_accessing_fixed(src_xs_ptr) {}
			TXSLTADynamicStrongPointerElementProxyConstRef(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_accessing_fixed(src_xs_ptr) {}
#endif // 0
			TXSLTADynamicStrongPointerElementProxyConstRef(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : m_accessing_fixed(src_xs_ptr) {}

			operator _Ty() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return m_accessing_fixed.operator *();
			}
			operator _Ty() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") = delete;
			template<typename _Ty2>
			void operator=(const _Ty2& src_ref) & = delete;
		private:
			TAccessingFixed m_accessing_fixed MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TElementProxyRef, class _TLender, class _Ty = typename _TLender::element_type>
		class TXSLTADynamicStrongPointerElementProxyPtr : public mse::us::impl::XSLTATagBase {
		public:
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyPtr(TXSLTADynamicStrongPointerElementProxyPtr&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyPtr(TXSLTADynamicStrongPointerElementProxyPtr&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyPtr(const TXSLTADynamicStrongPointerElementProxyPtr&) = delete;

			TXSLTADynamicStrongPointerElementProxyPtr(const mse::rsv::TXSLTAConstPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_proxy_ref(src_xs_ptr) {}
#if !defined(MSE_SLTAPOINTER_DISABLED)
			TXSLTADynamicStrongPointerElementProxyPtr(_TLender const* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_proxy_ref(src_xs_ptr) {}
#endif // !defined(MSE_SLTAPOINTER_DISABLED)
			auto operator->() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}
			auto operator->() && MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}

		private:
			TElementProxyRef m_proxy_ref MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TElementProxyConstRef, class _TLender, class _Ty = typename _TLender::element_type>
		class TXSLTADynamicStrongPointerElementProxyConstPtr : public mse::us::impl::XSLTATagBase {
		public:
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstPtr(TXSLTADynamicStrongPointerElementProxyConstPtr&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstPtr(TXSLTADynamicStrongPointerElementProxyConstPtr&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicStrongPointerElementProxyConstPtr(const TXSLTADynamicStrongPointerElementProxyConstPtr&) = delete;

			TXSLTADynamicStrongPointerElementProxyConstPtr(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_proxy_ref(src_xs_ptr) {}
#if !defined(MSE_SLTAPOINTER_DISABLED)
			TXSLTADynamicStrongPointerElementProxyConstPtr(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_proxy_ref(src_xs_ptr) {}
#endif // !defined(MSE_SLTAPOINTER_DISABLED)
			auto operator->() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}

		private:
			TElementProxyConstRef m_proxy_ref MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}

	namespace rsv {

		template<typename _Ty> class TXSLTARefCountingPointer;
		template<typename _Ty> class TXSLTARefCountingNotNullPointer;
		template<typename _Ty> class TXSLTARefCountingFixedPointer;

		template<typename _Ty> TXSLTARefCountingNotNullPointer<_Ty> not_null_from_nullable(const TXSLTARefCountingPointer<_Ty>& src);

		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = typename _TLender::element_type, bool _ExclusiveAccess = false
			, MSE_IMPL_EIP mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)> MSE_IMPL_EIS >
		class xslta_accessing_fixed_owning_pointer {
		public:
			typedef xslta_accessing_fixed_owning_pointer _Myt;
			typedef typename _TLender::element_type element_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_accessing_fixed_owning_pointer(xslta_accessing_fixed_owning_pointer&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_accessing_fixed_owning_pointer(xslta_accessing_fixed_owning_pointer&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

			xslta_accessing_fixed_owning_pointer(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : m_rfc_ptr(*src_xs_ptr)
				MSE_IF_DEBUG(, m_xs_structure_lock_guard(sl_make_xscope_shared_structure_lock_guard(*src_xs_ptr)) ) {}

			explicit operator bool() const { return bool(m_rfc_ptr); }

			auto& operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				auto ptr = m_rfc_ptr.get();
				if (!ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return *ptr;
			}
			auto operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				auto ptr = m_rfc_ptr.get();
				if (!ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return ptr;
			}

		private:
			xslta_accessing_fixed_owning_pointer(const xslta_accessing_fixed_owning_pointer&) = delete;
			_TLender m_rfc_ptr;

#if !defined(NDEBUG)
			/* In debug mode we will grab a shared lock on the lending pointer if the pointer provides such a lock. */
			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_shared_structure_lock_guard_helper1(std::true_type, _TLender2& src) { return _TLender2::s_make_xscope_shared_structure_lock_guard(src); }

			/* If the lending pointer doesn't support being locked then we'll just use a dummy lock guard. */
			struct dummy_xscope_shared_structure_lock_guard_t {};
			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_shared_structure_lock_guard_helper1(std::false_type, _TLender2& src) { return dummy_xscope_shared_structure_lock_guard_t{}; }

			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_shared_structure_lock_guard(_TLender2& src) {
				return sl_make_xscope_shared_structure_lock_guard_helper1(typename mse::impl::Has_s_make_xscope_shared_structure_lock_guard_MemberFunction<_TLender2>::type{}, src);
			}

			typedef decltype(sl_make_xscope_shared_structure_lock_guard(mse::impl::decl_lval<_TLender&>())) xscope_shared_structure_lock_guard_t;
			xscope_shared_structure_lock_guard_t m_xs_structure_lock_guard;

#endif // !defined(NDEBUG)
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
			MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _TPointerToLender>
		xslta_accessing_fixed_owning_pointer(_TPointerToLender) -> xslta_accessing_fixed_owning_pointer<_TPointerToLender>;
#endif /* MSE_HAS_CXX17 */

		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = typename _TLender::element_type
			, MSE_IMPL_EIP mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)> MSE_IMPL_EIS >
		auto make_xslta_accessing_fixed_owning_pointer(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_accessing_fixed_owning_pointer<_TPointerToLender, _TLender, _Ty>(src_xs_ptr);
		}

		template <class _TLender, class T = typename mse::impl::remove_reference_t<_TLender>::element_type>
		class xslta_borrowing_fixed_owning_pointer : public mse::us::impl::ContainsNonOwningScopeReferenceTagBase {
		public:
			typedef xslta_borrowing_fixed_owning_pointer _Myt;
			typedef typename _TLender::element_type element_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_borrowing_fixed_owning_pointer(xslta_borrowing_fixed_owning_pointer&&) = delete;
#define MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF m_src_ref(*src_xs_ptr)
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_borrowing_fixed_owning_pointer(xslta_borrowing_fixed_owning_pointer&& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) = default;
#define MSE_IMPL_BORROWING_FIXED_OPTIONAL_CONSTRUCT_SRC_REF m_src_ptr(std::addressof(*src_xs_ptr))
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_borrowing_fixed_owning_pointer(mse::rsv::TXSLTAPointer<_TLender> const src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) : m_obj1(*src_xs_ptr)
				MSE_IF_DEBUG(, m_xs_structure_lock_guard(sl_make_xscope_exclusive_structure_lock_guard(*src_xs_ptr))) {}
#if !defined(MSE_SLTAPOINTER_DISABLED)
			xslta_borrowing_fixed_owning_pointer(_TLender* const src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) : m_obj1(*src_xs_ptr)
				MSE_IF_DEBUG(, m_xs_structure_lock_guard(sl_make_xscope_exclusive_structure_lock_guard(*src_xs_ptr))) {}
#endif // !defined(MSE_SLTAPOINTER_DISABLED)

			explicit operator bool() const { return bool(m_obj1.m_borrowed); }

			auto& operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				auto ptr = m_obj1.m_borrowed.get();
				if (!ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return *ptr;
			}
			auto operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				auto ptr = m_obj1.m_borrowed.get();
				if (!ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return ptr;
			}

			MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(T);

		private:
			xslta_borrowing_fixed_owning_pointer(const xslta_borrowing_fixed_owning_pointer&) = delete;

			/* We put everything into a "subobject" member because we need the destructor to execute after the (debug) 
			lock object member has been destroyed already. */
			struct CObj1 {
				CObj1(_TLender& src_ref) : m_src_ref(src_ref), m_borrowed(std::move(src_ref)) {}
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				CObj1(CObj1&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				CObj1(CObj1&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				~CObj1() {
					m_src_ref = std::move(m_borrowed);
				}
				_TLender& m_src_ref;
				_TLender m_borrowed;
			};
			CObj1 m_obj1;

#if !defined(NDEBUG)
			/* In debug mode we will grab an exclusive lock on the lending pointer if the pointer provides such a lock. */
			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_exclusive_structure_lock_guard_helper1(std::true_type, _TLender2& src) { return _TLender2::s_make_xscope_exclusive_structure_lock_guard(src); }

			/* If the lending pointer doesn't support being locked then we'll just use a dummy lock guard. */
			struct dummy_xscope_exclusive_structure_lock_guard_t {};
			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_exclusive_structure_lock_guard_helper1(std::false_type, _TLender2& src) { return dummy_xscope_exclusive_structure_lock_guard_t{}; }

			template<class _TLender2 = _TLender>
			static auto sl_make_xscope_exclusive_structure_lock_guard(_TLender2& src) {
				return sl_make_xscope_exclusive_structure_lock_guard_helper1(typename mse::impl::Has_s_make_xscope_exclusive_structure_lock_guard_MemberFunction<_TLender2>::type{}, src);
			}

			typedef decltype(sl_make_xscope_exclusive_structure_lock_guard(mse::impl::decl_lval<_TLender&>())) xscope_exclusive_structure_lock_guard_t;
			xscope_exclusive_structure_lock_guard_t m_xs_structure_lock_guard;

#endif // !defined(NDEBUG)
		} MSE_ATTR_STR("mse::lifetime_labels(99)") MSE_ATTR_STR("mse::lifetime_label_for_base_class(99)");

		template <class _TLender, class T = typename mse::impl::remove_reference_t<_TLender>::element_type>
		using xl_bf_owning_pointer = xslta_borrowing_fixed_owning_pointer<_TLender, T>; /* provisional shorter alias */

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _TLender>
		xslta_borrowing_fixed_owning_pointer(mse::rsv::TXSLTAPointer<_TLender>) -> xslta_borrowing_fixed_owning_pointer<_TLender>;
#endif /* MSE_HAS_CXX17 */

		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xslta_borrowing_fixed_owning_pointer(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>(src_xs_ptr);
		}
		/* provisional shorter alias */
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xl_bf_owning_pointer(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return make_xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>(src_xs_ptr);
		}
#if !defined(MSE_SLTAPOINTER_DISABLED)
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xslta_borrowing_fixed_owning_pointer(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>(src_xs_ptr);
		}
		/* provisional shorter alias */
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xl_bf_owning_pointer(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return make_xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>(src_xs_ptr);
		}
#endif // !defined(MSE_SLTAPOINTER_DISABLED)

		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTARefCountingPointerElementProxyRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTARefCountingPointerElementProxyConstRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyConstRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;

		template<class TElementProxyRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTARefCountingPointerElementProxyPtr = TXSLTADynamicStrongPointerElementProxyPtr<TElementProxyRef, _TLender, _Ty>;
		template<class TElementProxyConstRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTARefCountingPointerElementProxyConstPtr = TXSLTADynamicStrongPointerElementProxyConstPtr<TElementProxyConstRef, _TLender, _Ty>;

		/* TXSLTARefCountingPointer behaves somewhat like an std::shared_ptr. */
		template <class X>
		class TXSLTARefCountingPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::RefCStrongPointerTagBase
			, MSE_INHERIT_XSLTA_TAG_BASE_SET_FROM(X, TXSLTARefCountingPointer<X>)
		{
		public:
			typedef X element_type;
			typedef TXSLTARefCountingPointer _Myt;
			/* If an initialization value is not given, any lifetimes will be "deduced" to be a (minimum) default value. */
			TXSLTARefCountingPointer() : m_ref_with_target_obj_ptr(nullptr) {}
			TXSLTARefCountingPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
			/* Constructs a null refcounting pointer, uses the second argument only to deduce lifetime. */
			TXSLTARefCountingPointer(std::nullptr_t, const X& MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_ref_with_target_obj_ptr(nullptr) {}
			~TXSLTARefCountingPointer() {
				//release();
				/* Doing it this way instead of just calling release() protects against potential reentrant destructor
				calls caused by a misbehaving (user-defined) destructor of the target object. */
				auto_release keep(m_ref_with_target_obj_ptr);
				m_ref_with_target_obj_ptr = nullptr;
				MSE_IF_DEBUG(m_debug_target_obj_cptr = nullptr;)
			}
			TXSLTARefCountingPointer(const TXSLTARefCountingPointer& r) {
				MSE_IF_DEBUG(r.assert_access_is_unlocked();)
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}
			TXSLTARefCountingPointer(TXSLTARefCountingPointer&& r) {
				MSE_IF_DEBUG(r.assert_access_is_unlocked();)
				m_ref_with_target_obj_ptr = r.m_ref_with_target_obj_ptr;
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
				r.m_ref_with_target_obj_ptr = nullptr;
				MSE_IF_DEBUG(r.m_debug_target_obj_cptr = nullptr;)
			}
			TXSLTARefCountingPointer(const TXSLTARefCountingNotNullPointer<X>& r);
			TXSLTARefCountingPointer(TXSLTARefCountingNotNullPointer<X>&& r);
			explicit operator bool() const { MSE_IF_DEBUG(assert_access_is_unlocked();) return nullptr != get(); }
			void reset() { MSE_IF_DEBUG(assert_access_is_unlocked();) (*this) = TXSLTARefCountingPointer<X>(nullptr); }
			MSE_DEPRECATED void clear() { MSE_IF_DEBUG(assert_access_is_unlocked();) (*this) = TXSLTARefCountingPointer<X>(nullptr); }
			TXSLTARefCountingPointer& operator=(const TXSLTARefCountingPointer& r MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (this != &r) {
					auto_release keep(m_ref_with_target_obj_ptr);
					acquire(r.m_ref_with_target_obj_ptr);
					MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
				}
				return *this;
			}
			MSE_DEPRECATED bool operator<(const TXSLTARefCountingPointer& r) const {
				MSE_IF_DEBUG(assert_access_is_unlocked();) return get() < r.get();
			}
			bool operator==(const TXSLTARefCountingPointer& r) const {
				MSE_IF_DEBUG(assert_access_is_unlocked();) return get() == r.get();
			}
			bool operator!=(const TXSLTARefCountingPointer& r) const {
				MSE_IF_DEBUG(assert_access_is_unlocked();) return get() != r.get();
			}

#ifndef MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES
			/* Apparently msvc2015 requires that templated member functions come before regular ones.
			From this webpage regarding compiler error C2668 - https://msdn.microsoft.com/en-us/library/da60x087.aspx:
			"If, in the same class, you have a regular member function and a templated member function with the same
			signature, the templated one must come first. This is a limitation of the current implementation of Visual C++."
			*/
			template <class Y> friend class TXSLTARefCountingPointer;

			/* The lifetime annotation on the parameter of this constructor is premised on the assumption that the 
			lifetimes of type X are the same as, and correspond directly to, the lifetimes of descendant class Y. 
			At the time of authoring we didn't seem to have a practical way of ensuring the validity of this 
			assumption. Todo: add a way to do so. */
			template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<X, Y>::value> MSE_IMPL_EIS >
			TXSLTARefCountingPointer(const TXSLTARefCountingPointer<Y>& r MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				acquire(r.m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}

			template <class Y> bool operator==(const TXSLTARefCountingPointer<Y>& r) const { MSE_IF_DEBUG(assert_access_is_unlocked();) return get() == r.get(); }
			template <class Y> bool operator!=(const TXSLTARefCountingPointer<Y>& r) const { MSE_IF_DEBUG(assert_access_is_unlocked();) return !((*this) == r); }
			template <class Y> bool operator==(const TXSLTARefCountingNotNullPointer<Y>& r) const { MSE_IF_DEBUG(assert_access_is_unlocked();) return get() == r.get(); }
			template <class Y> bool operator!=(const TXSLTARefCountingNotNullPointer<Y>& r) const { MSE_IF_DEBUG(assert_access_is_unlocked();) return !((*this) == r); }
#endif // !MSE_REFCOUNTINGPOINTER_DISABLE_MEMBER_TEMPLATES

			auto operator*() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return TXSLTARefCountingPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, X>(mse::rsv::xslta_ptr_to(*this));
			}
			auto operator*() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				return TXSLTARefCountingPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, X>(mse::rsv::xslta_ptr_to(*this));
			}
			auto operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				typedef TXSLTARefCountingPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(mse::impl::decl_lval<_Myt>())), _Myt, X> TProxyRef;
				typedef TXSLTARefCountingPointerElementProxyPtr<TProxyRef, typename TProxyRef::lender_type, typename TProxyRef::element_type> TElementProxyPtr;
				return TElementProxyPtr(mse::rsv::xslta_ptr_to(*this));
				//X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				//return x_ptr;
			}
			bool unique() const {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
			}

			static TXSLTARefCountingPointer make(X&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTARefCountingPointer(std::forward<X>(src_ref));
			}
			static TXSLTARefCountingPointer make(std::nullptr_t, const X& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTARefCountingPointer(nullptr, src_cref);
			}

		private:
			explicit TXSLTARefCountingPointer(X&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) {
				m_ref_with_target_obj_ptr = new TRefWithTargetObj<X>(std::forward<X>(src_ref));
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}

			void acquire(CRefCounter* c) {
				m_ref_with_target_obj_ptr = c;
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
					if (c) { c->increment(); }
			}

			void release() {
				dorelease(m_ref_with_target_obj_ptr);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}

			struct auto_release {
				auto_release(CRefCounter* c) : m_ref_with_target_obj_ptr(c) {}
				~auto_release() { dorelease(m_ref_with_target_obj_ptr); }
				CRefCounter* m_ref_with_target_obj_ptr;
			};

			void static dorelease(CRefCounter* ref_with_target_obj_ptr) {
				// decrement the count, delete if it is nullptr
				if (ref_with_target_obj_ptr) {
					if (1 == ref_with_target_obj_ptr->use_count()) {
						delete ref_with_target_obj_ptr;
					}
					else {
						ref_with_target_obj_ptr->decrement();
					}
				}
			}

			X* get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				if (!m_ref_with_target_obj_ptr) {
					return nullptr;
				}
				else {
					X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
					return x_ptr;
				}
			}
			X* unchecked_get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				return x_ptr;
			}

#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
			/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
			(intended) compile error. */
			template<class X2 = X, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<X2, X>::value) && (mse::impl::is_potentially_not_xscope<X2>::value)> MSE_IMPL_EIS >
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
			void valid_if_X_is_not_an_xscope_type() const {}

			CRefCounter* m_ref_with_target_obj_ptr;

			MSE_IF_DEBUG(mutable mse::non_thread_safe_shared_mutex m_debug_access_mutex;)
			MSE_IF_DEBUG(X const* m_debug_target_obj_cptr = nullptr;)

#if !defined(NDEBUG)

			/* In debug mode we'll provide facilities for locking access to the pointer while it's being borrowed. */
			typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt> xscope_shared_structure_lock_guard_t;
			static auto s_make_xscope_shared_structure_lock_guard(_Myt const& src_ref) -> xscope_shared_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_shared_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(src_ref));
			}
			//typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt> xscope_shared_const_structure_lock_guard_t;
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_exclusive_structure_lock_guard<_Myt, true/*LockAccessToOriginal*/> xscope_exclusive_structure_lock_guard_t;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_shared_structure_lock_guard<_Myt> xscope_exclusive_structure_lock_guard_t;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			static auto s_make_xscope_exclusive_structure_lock_guard(_Myt& src_ref) -> xscope_exclusive_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_exclusive_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(src_ref));
			}

			auto access_lock() { MSE_IF_DEBUG(m_access_mutex.lock(); m_access_is_prohibited = true;) }
			auto access_unlock() { MSE_IF_DEBUG(m_access_mutex.unlock(); m_access_is_prohibited = false;) }
			auto assert_access_is_unlocked() const {
#if !defined(NDEBUG)
				if (m_access_is_prohibited) {
					MSE_THROW(std::logic_error("attempt to access a TXSLTARefCountingPointer<> while borrowed? - mse::rsv::TXSLTARefCountingPointer"));
				}
#endif // !defined(NDEBUG)
			}

			/* Other owning types in the library provide separate facilities for locking the "structure" of the owned 
			contents and for locking "access" to the owned contents. For consistency we use the "common locking 
			framework", but this owning pointer only uses access locking, so we'll just use a "dummy" structure change 
			mutex when the common framework calls for one. */
			mutable dummy_recursive_shared_timed_mutex m_structure_change_mutex;

#if !defined(NDEBUG)
			/* While a "structure lock" is used to prevent deallocation or relocation of any of the contents, an "access lock"
			is used to prevent any access whatsoever. This is generally used to catch "use-while-borrowed" bugs (in debug builds). */
			/* These shouldn't need to be atomic as this class is not eligible to be shared among threads anyway. */
			bool m_access_is_prohibited = false;
			mse::non_thread_safe_mutex m_access_mutex;
#endif // !defined(NDEBUG)

			friend class mse::us::impl::Txscope_shared_structure_lock_guard<_Myt>;
			friend class mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>;
			template<class TDynamicContainer, bool LockAccessToOriginal> friend class mse::us::impl::Txscope_exclusive_structure_lock_guard;
			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_shared_structure_lock_guard_MemberFunction_impl;
			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_exclusive_structure_lock_guard_MemberFunction_impl;
#endif // !defined(NDEBUG)
				
			template <class Y> friend class TXSLTARefCountingPointer;
			template <class Y> friend class TXSLTARefCountingNotNullPointer;
			template<class _TPointerToLender, class _TLender, class _Ty2, bool _ExclusiveAccess, MSE_IMPL_EI_FORWARD_DECL(mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)>)>
			friend class xslta_accessing_fixed_owning_pointer;
			template <class _TLender, class T2>
			friend class xslta_borrowing_fixed_owning_pointer;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(X, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTARefCountingPointer);

	namespace rsv {
		/* For now we're just defining xslta_accessing_fixed_owning_not_null_pointer<> to be an alias of
		xslta_accessing_fixed_owning_pointer<>. We could save a theoretical null pointer check by using a distinct
		implementation, but it probably wouldn't make much difference. */
		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = typename _TLender::element_type, bool _ExclusiveAccess = false
			, MSE_IMPL_EIP mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)> MSE_IMPL_EIS >
		using xslta_accessing_fixed_owning_not_null_pointer = xslta_accessing_fixed_owning_pointer<_TPointerToLender, _TLender, _Ty, _ExclusiveAccess>;

		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = typename _TLender::element_type
			, MSE_IMPL_EIP mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)> MSE_IMPL_EIS >
		auto make_xslta_accessing_fixed_owning_not_null_pointer(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_accessing_fixed_owning_not_null_pointer<_TPointerToLender, _TLender, _Ty>(src_xs_ptr);
		}

		/* For now we're just defining xslta_borrowing_fixed_owning_not_null_pointer<> to be an alias of
		xslta_borrowing_fixed_owning_pointer<>. We could save a theoretical null pointer check by using a distinct
		implementation, but I'm guessing that in practice it probably wouldn't make much difference. */
		template <class _TLender, class T = typename mse::impl::remove_reference_t<_TLender>::element_type>
		using xslta_borrowing_fixed_owning_not_null_pointer = xslta_borrowing_fixed_owning_pointer<_TLender, T>;
		template <class _TLender, class T = typename mse::impl::remove_reference_t<_TLender>::element_type>
		using xl_bf_owning_not_null_pointer = xslta_borrowing_fixed_owning_not_null_pointer<_TLender, T>; /* provisional shorter alias */

		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xslta_borrowing_fixed_owning_not_null_pointer(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_owning_not_null_pointer<_TLender, _Ty>(src_xs_ptr);
		}
		/* provisional shorter alias */
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xl_bf_owning_not_null_pointer(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return make_xslta_borrowing_fixed_owning_not_null_pointer<_TLender, _Ty>(src_xs_ptr);
		}
#if !defined(MSE_SLTAPOINTER_DISABLED)
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xslta_borrowing_fixed_owning_not_null_pointer(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_owning_not_null_pointer<_TLender, _Ty>(src_xs_ptr);
		}
		/* provisional shorter alias */
		template<class _TLender, class _Ty = typename mse::impl::remove_reference_t<_TLender>::element_type>
		auto make_xl_bf_owning_not_null_pointer(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return make_xslta_borrowing_fixed_owning_not_null_pointer<_TLender, _Ty>(src_xs_ptr);
		}
#endif // !defined(MSE_SLTAPOINTER_DISABLED)

		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTARefCountingNotNullPointerElementProxyRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTARefCountingNotNullPointerElementProxyConstRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyConstRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;

		template<class TElementProxyRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTARefCountingNotNullPointerElementProxyPtr = TXSLTADynamicStrongPointerElementProxyPtr<TElementProxyRef, _TLender, _Ty>;
		template<class TElementProxyConstRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTARefCountingNotNullPointerElementProxyConstPtr = TXSLTADynamicStrongPointerElementProxyConstPtr<TElementProxyConstRef, _TLender, _Ty>;

		template<typename _Ty>
		class TXSLTARefCountingNotNullPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::RefCStrongPointerTagBase, public mse::us::impl::NeverNullTagBase
			, MSE_INHERIT_XSLTA_TAG_BASE_SET_FROM(_Ty, TXSLTARefCountingNotNullPointer<_Ty>)
		{
		public:
			typedef _Ty element_type;
			typedef TXSLTARefCountingNotNullPointer _Myt;
			TXSLTARefCountingNotNullPointer(const TXSLTARefCountingNotNullPointer& src_cref) : m_rcptr(src_cref.m_rcptr) {
				if (!(src_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TXSLTARefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::rsv::TXSLTARefCountingNotNullPointer")); }
			}

			/* The lifetime annotation on the parameter of this constructor is premised on the assumption that the 
			lifetimes of type _Ty are the same as, and correspond directly to, the lifetimes of descendant class Y. 
			At the time of authoring we didn't seem to have a practical way of ensuring the validity of this 
			assumption. Todo: add a way to do so. */
			template <class Y, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<_Ty, Y>::value> MSE_IMPL_EIS >
			TXSLTARefCountingNotNullPointer(const TXSLTARefCountingNotNullPointer<Y>& r MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : m_rcptr(r.m_rcptr) {
				if (!(r.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TXSLTARefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::rsv::TXSLTARefCountingNotNullPointer")); }
			}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXSLTARefCountingNotNullPointer() {}
			explicit operator bool() const { return true; }
			TXSLTARefCountingNotNullPointer<_Ty>& operator=(const TXSLTARefCountingNotNullPointer<_Ty>& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				if (!(_Right_cref.m_rcptr)) { MSE_THROW(std::logic_error("attempt to copy a TXSLTARefCountingNotNullPointer<> that's in a partially destructed (or constructed?) state - mse::rsv::TXSLTARefCountingNotNullPointer")); }
				m_rcptr = (_Right_cref.m_rcptr);
				return (*this);
			}
			bool operator==(const TXSLTARefCountingNotNullPointer& r) const { return m_rcptr == r.m_rcptr; }
			bool operator!=(const TXSLTARefCountingNotNullPointer& r) const { return !((*this) == r); }
			template <class Y> bool operator==(const TXSLTARefCountingPointer<Y>& r) const { return get() == r.get(); }
			template <class Y> bool operator!=(const TXSLTARefCountingPointer<Y>& r) const { return !((*this) == r); }
			template <class Y> bool operator==(const TXSLTARefCountingNotNullPointer<Y>& r) const { return get() == r.get(); }
			template <class Y> bool operator!=(const TXSLTARefCountingNotNullPointer<Y>& r) const { return !((*this) == r); }

			auto operator*() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return TXSLTARefCountingNotNullPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(m_rcptr)), decltype(m_rcptr), _Ty>(mse::rsv::xslta_ptr_to(m_rcptr));
			}
			auto operator*() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return TXSLTARefCountingNotNullPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(m_rcptr)), decltype(m_rcptr), _Ty>(mse::rsv::xslta_ptr_to(m_rcptr));
			}
			auto operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				//if (!m_ref_with_target_obj_ptr) { MSE_THROW(refcounting_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTARefCountingPointer")); }
				typedef TXSLTARefCountingNotNullPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(m_rcptr)), decltype(m_rcptr), _Ty> TProxyRef;
				typedef TXSLTARefCountingNotNullPointerElementProxyPtr<TProxyRef, typename TProxyRef::lender_type, typename TProxyRef::element_type> TElementProxyPtr;
				return TElementProxyPtr(mse::rsv::xslta_ptr_to(m_rcptr));
			}
			bool unique() const {
				return m_rcptr.unique();
			}

			static TXSLTARefCountingNotNullPointer make(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTARefCountingNotNullPointer(std::forward<_Ty>(src_ref));
			}

		private:
			explicit TXSLTARefCountingNotNullPointer(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])"))
				: m_rcptr(MSE_FWD(src_ref)) {
			}

			/* If you want to use this constructor, use not_null_from_nullable() instead. */
			explicit TXSLTARefCountingNotNullPointer(const TXSLTARefCountingPointer<_Ty>& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) : m_rcptr(src_cref) {
				if (!m_rcptr) { MSE_THROW(refcounting_null_dereference_error("attempt to construct a 'not null' pointer from a null pointer value - mse::rsv::TXSLTARefCountingNotNullPointer")); }
			}
			explicit TXSLTARefCountingNotNullPointer(TXSLTARefCountingPointer<_Ty>&& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) : m_rcptr(MSE_FWD(src_cref)) {
				if (!m_rcptr) { MSE_THROW(refcounting_null_dereference_error("attempt to construct a 'not null' pointer from a null pointer value - mse::rsv::TXSLTARefCountingNotNullPointer")); }
			}
			_Ty* unchecked_get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return m_rcptr.unchecked_get();
			}
			_Ty* get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return unchecked_get();
			}

			TXSLTARefCountingPointer<_Ty> m_rcptr;

#if !defined(NDEBUG)

			/* In debug mode we'll provide facilities for locking access to the pointer while it's being borrowed. */
			typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<TXSLTARefCountingPointer<_Ty> > xscope_shared_structure_lock_guard_t;
			static auto s_make_xscope_shared_structure_lock_guard(_Myt const& src_ref) -> xscope_shared_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return TXSLTARefCountingPointer<_Ty>::s_make_xscope_shared_structure_lock_guard(src_ref.m_rcptr);
			}
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_exclusive_structure_lock_guard<TXSLTARefCountingPointer<_Ty>, true/*LockAccessToOriginal*/> xscope_exclusive_structure_lock_guard_t;
			static auto s_make_xscope_exclusive_structure_lock_guard(_Myt& src_ref) -> xscope_exclusive_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return TXSLTARefCountingPointer<_Ty>::s_make_xscope_exclusive_structure_lock_guard(src_ref.m_rcptr);
			}
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<TXSLTARefCountingPointer<_Ty>> xscope_exclusive_structure_lock_guard_t;
			static auto s_make_xscope_exclusive_structure_lock_guard(_Myt& src_ref) -> xscope_exclusive_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return TXSLTARefCountingPointer<_Ty>::s_make_xscope_shared_structure_lock_guard(src_ref.m_rcptr);
			}
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_shared_structure_lock_guard_MemberFunction_impl;
			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_exclusive_structure_lock_guard_MemberFunction_impl;
#endif // !defined(NDEBUG)

			template <class Y> friend class TXSLTARefCountingPointer;
			template <class Y> friend class TXSLTARefCountingNotNullPointer;
			template <class Y> friend class TXSLTARefCountingFixedPointer;
			template<typename _Ty2>
			friend TXSLTARefCountingNotNullPointer<_Ty2> not_null_from_nullable(const TXSLTARefCountingPointer<_Ty2>& src);
			template<class _TPointerToLender, class _TLender, class _Ty2, bool _ExclusiveAccess, MSE_IMPL_EI_FORWARD_DECL(mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_mserefcounting<_TLender>::value*/)>)>
			friend class xslta_accessing_fixed_owning_pointer;
			template <class _TLender, class T2>
			friend class xslta_borrowing_fixed_owning_pointer;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTARefCountingNotNullPointer);

	namespace rsv {

		template <class X> TXSLTARefCountingPointer<X>::TXSLTARefCountingPointer(const TXSLTARefCountingNotNullPointer<X>& r) : TXSLTARefCountingPointer(r.m_rcptr) {}
		template <class X> TXSLTARefCountingPointer<X>::TXSLTARefCountingPointer(TXSLTARefCountingNotNullPointer<X>&& r) : TXSLTARefCountingPointer(MSE_FWD(r.m_rcptr)) {}

		/* TXSLTARefCountingFixedPointer cannot be retargeted or constructed without a target. (This implementation
		that publicly inherits from TXSLTARefCountingNotNullPointer<> is just a placeholder implementation that
		doesn't fully prevent its claimed invariant from being violated.) */
		template<typename _Ty>
		class TXSLTARefCountingFixedPointer : public TXSLTARefCountingNotNullPointer<_Ty> {
		public:
			typedef TXSLTARefCountingNotNullPointer<_Ty> base_class;
			typedef _Ty element_type;
			typedef TXSLTARefCountingFixedPointer _Myt;
			TXSLTARefCountingFixedPointer(const TXSLTARefCountingFixedPointer& src_cref) : TXSLTARefCountingNotNullPointer<_Ty>(src_cref) {}
			TXSLTARefCountingFixedPointer(const TXSLTARefCountingNotNullPointer<_Ty>& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) : TXSLTARefCountingNotNullPointer<_Ty>(src_cref) {}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXSLTARefCountingFixedPointer() {}

			auto& operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return *(this->get());
			}
			auto* operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return this->get();
			}

			static TXSLTARefCountingFixedPointer make(_Ty&& src_ref  MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return base_class::make(std::forward<_Ty>(src_ref));
			}

		private:

			TXSLTARefCountingFixedPointer<_Ty>& operator=(const TXSLTARefCountingFixedPointer<_Ty>& _Right_cref) = delete;

		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTARefCountingFixedPointer);

	namespace rsv {
		template <class X>
		TXSLTARefCountingNotNullPointer<X> make_xslta_refcounting(X x MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_12$)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTARefCountingNotNullPointer<X>::make(std::move(x));
		}
		template <class X>
		TXSLTARefCountingPointer<X> make_xslta_nullable_refcounting(X x MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_12$)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTARefCountingPointer<X>::make(std::move(x));
		}
		template <class X>
		TXSLTARefCountingPointer<X> make_xslta_nullable_refcounting(std::nullptr_t, const X& x MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTARefCountingPointer<X>::make(nullptr, x);
		}

		template<typename _Ty>
		TXSLTARefCountingNotNullPointer<_Ty> not_null_from_nullable(const TXSLTARefCountingPointer<_Ty>& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_11$); labels(alias_11$); return_value(alias_11$) }") {
			return src;
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::rsv::TXSLTARefCountingPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTARefCountingPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTARefCountingPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::rsv::TXSLTARefCountingNotNullPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTARefCountingNotNullPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTARefCountingNotNullPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::rsv::TXSLTARefCountingFixedPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTARefCountingFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTARefCountingFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
}
namespace mse {

#endif // MSESLTA_H_



	namespace us {
		namespace impl {
#ifdef MSEPOINTERBASICS_H
			typedef mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase SingleOwnerStrongPointerTagBase;
#else // MSEPOINTERBASICS_H
			class mse::us::impl::SingleOwnerStrongPointerTagBase {};
#endif // MSEPOINTERBASICS_H
		}
	}

	class single_owner_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template <class _Ty /*, class _Dx = default_delete<_Ty> */>
	class TSingleOwnerPointer : public mse::us::impl::SingleOwnerStrongPointerTagBase { // non-copyable pointer to an object
	public:
		using pointer = typename std::unique_ptr<_Ty>::pointer;
		using element_type = _Ty;

		constexpr TSingleOwnerPointer() noexcept {}

		constexpr TSingleOwnerPointer(std::nullptr_t) noexcept {}

		MSE_CONSTEXPR23 TSingleOwnerPointer& operator=(std::nullptr_t) noexcept {
			m_uq_ptr = nullptr;
			return *this;
		}

		MSE_CONSTEXPR23 TSingleOwnerPointer(TSingleOwnerPointer const& _Right) = delete;
		MSE_CONSTEXPR23 TSingleOwnerPointer(TSingleOwnerPointer&& _Right) noexcept
			: m_uq_ptr(MSE_FWD(_Right).m_uq_ptr) {}

		template <class _Ty2,
			MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<std::unique_ptr<_Ty2>, std::unique_ptr<_Ty> >::value> MSE_IMPL_EIS >
		MSE_CONSTEXPR23 TSingleOwnerPointer(TSingleOwnerPointer<_Ty2>&& _Right) noexcept
			: m_uq_ptr(MSE_FWD(_Right).m_uq_ptr) {}

		MSE_CONSTEXPR23 TSingleOwnerPointer& operator=(TSingleOwnerPointer&& _Right) noexcept {
			m_uq_ptr = MSE_FWD(_Right).m_uq_ptr;
			return *this;
		}

		TSingleOwnerPointer& operator=(const TSingleOwnerPointer&) = delete;

		MSE_CONSTEXPR23 void swap(TSingleOwnerPointer& _Right) noexcept {
			m_uq_ptr.swap(_Right.m_uq_ptr);
		}

		_NODISCARD MSE_CONSTEXPR23 typename std::add_lvalue_reference<_Ty>::type operator*() const /*noexcept(noexcept(*_STD declval<pointer>()))*/ {
			if (!m_uq_ptr) { MSE_THROW(single_owner_null_dereference_error("attempt to dereference null pointer - mse::TSingleOwnerPointer")); }
			return *m_uq_ptr;
		}

		_NODISCARD MSE_CONSTEXPR23 pointer operator->() const /*noexcept*/ {
			if (!m_uq_ptr) { MSE_THROW(single_owner_null_dereference_error("attempt to dereference null pointer - mse::TSingleOwnerPointer")); }
			return std::addressof(*m_uq_ptr);
		}

		/*
		_NODISCARD MSE_CONSTEXPR23 pointer get() const noexcept {
			return std::addressof(*m_uq_ptr);
		}
		*/

		MSE_CONSTEXPR23 explicit operator bool() const noexcept {
			return static_cast<bool>(m_uq_ptr);
		}

		MSE_CONSTEXPR23 /*pointer*/ void release() noexcept {
			return m_uq_ptr.release();
		}

		MSE_CONSTEXPR23 void reset(/*pointer _Ptr = nullptr*/) noexcept {
			m_uq_ptr.reset();
		}

		template <class... Args>
		MSE_CONSTEXPR23 static TSingleOwnerPointer make(Args&&... args) {
			return TSingleOwnerPointer(uq_contruct_tag{}, std::make_unique<_Ty>(std::forward<Args>(args)...));
		}

	private:
		struct uq_contruct_tag {};
		MSE_CONSTEXPR23 TSingleOwnerPointer(uq_contruct_tag, std::unique_ptr<_Ty>&& uq_ptr) : m_uq_ptr(MSE_FWD(uq_ptr)) {}

		template <class /*, class*/ >
		friend class TSingleOwnerPointer;

		std::unique_ptr<_Ty> m_uq_ptr;
	};
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::TSingleOwnerPointer);
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(std::unique_ptr);

	template <class _Ty, class... _Types>
	MSE_CONSTEXPR23 TSingleOwnerPointer<_Ty> make_single_owner(_Types&&... _Args) { // make a TSingleOwnerPointer
		return TSingleOwnerPointer<_Ty>::make(std::forward<_Types>(_Args)...);
	}

#if 0
	template <class _Ty>
	MSE_CONSTEXPR23 TSingleOwnerPointer<_Ty> make_unique_for_overwrite() {
		/* don't know if this implementation is exactly correct, but probably good enough for now */
		return TSingleOwnerPointer<_Ty>::make(_Ty{});
	}
#endif // 0
}

namespace std {
	template<class _Ty>
	struct hash<mse::TSingleOwnerPointer<_Ty> > {	// hash functor
		typedef mse::TSingleOwnerPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TSingleOwnerPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
}

namespace mse {
	namespace rsv {

		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTASingleOwnerPointerElementProxyRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyRef<mse::rsv::xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>, _TPointer, _TLender, _Ty>;
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTASingleOwnerPointerElementProxyConstRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyConstRef<mse::rsv::xslta_borrowing_fixed_owning_pointer<_TLender, _Ty>, _TPointer, _TLender, _Ty>;

		template<class TElementProxyRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTASingleOwnerPointerElementProxyPtr = TXSLTADynamicStrongPointerElementProxyPtr<TElementProxyRef, _TLender, _Ty>;
		template<class TElementProxyConstRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTASingleOwnerPointerElementProxyConstPtr = TXSLTADynamicStrongPointerElementProxyConstPtr<TElementProxyConstRef, _TLender, _Ty>;

		template<typename _Ty>
		class TXSLTASingleOwnerFixedPointer;

		/* TXSLTASingleOwnerPointer behaves somewhat like an std::shared_ptr. */
		template <class X>
		class TXSLTASingleOwnerPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::SingleOwnerStrongPointerTagBase
			, MSE_INHERIT_XSLTA_TAG_BASE_SET_FROM(X, TXSLTASingleOwnerPointer<X>)
		{
		public:
			typedef X element_type;
			typedef TXSLTASingleOwnerPointer _Myt;
			/* If an initialization value is not given, any lifetimes will be "deduced" to be a (minimum) default value. */
			constexpr TXSLTASingleOwnerPointer() noexcept {}
			constexpr TXSLTASingleOwnerPointer(std::nullptr_t) noexcept {}
			/* Constructs a null single_owner pointer, uses the second argument only to deduce lifetime. */
			constexpr TXSLTASingleOwnerPointer(std::nullptr_t, const X& MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) noexcept {}

			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer(X&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) {
				m_uq_ptr = std::make_unique<X>(std::forward<X>(src_ref));
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}

			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer(X const& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) {
				m_uq_ptr = std::make_unique<X>(src_cref);
				MSE_IF_DEBUG(m_debug_target_obj_cptr = get();)
			}

			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer& operator=(std::nullptr_t) noexcept {
				m_uq_ptr = nullptr;
				return *this;
			}

			TXSLTASingleOwnerPointer(TXSLTASingleOwnerPointer const& r) = delete;
			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer(TXSLTASingleOwnerPointer&& r)
				: m_uq_ptr((MSE_IF_DEBUG(r.assert_access_is_unlocked(),) std::move(r.m_uq_ptr))) {}

			/* The lifetime annotation on the parameter of this constructor is premised on the assumption that the
			lifetimes of type X are the same as, and correspond directly to, the lifetimes of convertible class _Ty2.
			At the time of authoring we didn't seem to have a practical way of ensuring the validity of this
			assumption. Todo: add a way to do so. */
			template <class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<std::unique_ptr<_Ty2>, std::unique_ptr<X> >::value> MSE_IMPL_EIS >
			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer(TXSLTASingleOwnerPointer<_Ty2>&& _Right MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) noexcept
				: m_uq_ptr((MSE_IF_DEBUG(_Right.assert_access_is_unlocked(),) MSE_FWD(_Right).m_uq_ptr)) {
			}
			template <class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<std::unique_ptr<_Ty2>, std::unique_ptr<X> >::value> MSE_IMPL_EIS >
			TXSLTASingleOwnerPointer(TXSLTASingleOwnerPointer<_Ty2> const& _Right) = delete;

			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer& operator=(TXSLTASingleOwnerPointer const& _Right MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") = delete;

			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer& operator=(TXSLTASingleOwnerPointer&& _Right MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				MSE_IF_DEBUG(_Right.assert_access_is_unlocked();)
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				m_uq_ptr = MSE_FWD(_Right).m_uq_ptr;
				return *this;
			}

			MSE_CONSTEXPR23 auto operator*() & MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_uq_ptr) { MSE_THROW(single_owner_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTASingleOwnerPointer")); }
				return TXSLTASingleOwnerPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, X>(mse::rsv::xslta_ptr_to(*this));
			}
			MSE_CONSTEXPR23 auto operator*() && MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_uq_ptr) { MSE_THROW(single_owner_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTASingleOwnerPointer")); }
				return TXSLTASingleOwnerPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, X>(mse::rsv::xslta_ptr_to(*this));
			}
			MSE_CONSTEXPR23 auto operator->() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				if (!m_uq_ptr) { MSE_THROW(single_owner_null_dereference_error("attempt to dereference null pointer - mse::rsv::TXSLTASingleOwnerPointer")); }
				typedef TXSLTASingleOwnerPointerElementProxyRef<decltype(mse::rsv::xslta_ptr_to(mse::impl::decl_lval<_Myt>())), _Myt, X> TProxyRef;
				typedef TXSLTASingleOwnerPointerElementProxyPtr<TProxyRef, typename TProxyRef::lender_type, typename TProxyRef::element_type> TElementProxyPtr;
				return TElementProxyPtr(mse::rsv::xslta_ptr_to(*this));
				//X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				//return x_ptr;
			}


			MSE_CONSTEXPR23 explicit operator bool() const noexcept { MSE_IF_DEBUG(assert_access_is_unlocked();) return nullptr != get(); }
			MSE_CONSTEXPR23 void reset() { MSE_IF_DEBUG(assert_access_is_unlocked();) (*this) = TXSLTASingleOwnerPointer<X>(nullptr); }
			MSE_CONSTEXPR23 bool operator==(const TXSLTASingleOwnerPointer& r) const {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				return get() == r.get();
			}
			MSE_CONSTEXPR23 bool operator!=(const TXSLTASingleOwnerPointer& r) const {
				MSE_IF_DEBUG(assert_access_is_unlocked();)
				return get() != r.get();
			}

			MSE_CONSTEXPR23 static TXSLTASingleOwnerPointer make(X&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTASingleOwnerPointer(std::forward<X>(src_ref));
			}
			MSE_CONSTEXPR23 static TXSLTASingleOwnerPointer make(std::nullptr_t, const X& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTASingleOwnerPointer(nullptr, src_cref);
			}

		private:
			struct uq_contruct_tag {};
			MSE_CONSTEXPR23 TXSLTASingleOwnerPointer(uq_contruct_tag, std::unique_ptr<X>&& uq_ptr) : m_uq_ptr(MSE_FWD(uq_ptr)) {}

			MSE_CONSTEXPR23 X* get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return m_uq_ptr.get();
			}
			MSE_CONSTEXPR23 X* unchecked_get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return m_uq_ptr.get();
			}

			std::unique_ptr<X> m_uq_ptr;

			MSE_IF_DEBUG(mutable mse::non_thread_safe_shared_mutex m_debug_access_mutex;)
			MSE_IF_DEBUG(X const* m_debug_target_obj_cptr = nullptr;)

#if !defined(NDEBUG)

			/* In debug mode we'll provide facilities for locking access to the pointer while it's being borrowed. */
			typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt> xscope_shared_structure_lock_guard_t;
			static auto s_make_xscope_shared_structure_lock_guard(_Myt const& src_ref) -> xscope_shared_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_shared_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(src_ref));
			}
			//typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt> xscope_shared_const_structure_lock_guard_t;
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_exclusive_structure_lock_guard<_Myt, true/*LockAccessToOriginal*/> xscope_exclusive_structure_lock_guard_t;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			typedef mse::us::impl::Txscope_shared_structure_lock_guard<_Myt> xscope_exclusive_structure_lock_guard_t;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			static auto s_make_xscope_exclusive_structure_lock_guard(_Myt& src_ref) -> xscope_exclusive_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_exclusive_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(src_ref));
			}

			auto access_lock() { MSE_IF_DEBUG(m_access_mutex.lock(); m_access_is_prohibited = true;) }
			auto access_unlock() { MSE_IF_DEBUG(m_access_mutex.unlock(); m_access_is_prohibited = false;) }
			auto assert_access_is_unlocked() const {
#if !defined(NDEBUG)
				if (m_access_is_prohibited) {
					MSE_THROW(std::logic_error("attempt to access a TXSLTASingleOwnerPointer<> while borrowed? - mse::rsv::TXSLTASingleOwnerPointer"));
				}
#endif // !defined(NDEBUG)
			}

			/* Other owning types in the library provide separate facilities for locking the "structure" of the owned
			contents and for locking "access" to the owned contents. For consistency we use the "common locking
			framework", but this owning pointer only uses access locking, so we'll just use a "dummy" structure change
			mutex when the common framework calls for one. */
			mutable dummy_recursive_shared_timed_mutex m_structure_change_mutex;

#if !defined(NDEBUG)
			/* While a "structure lock" is used to prevent deallocation or relocation of any of the contents, an "access lock"
			is used to prevent any access whatsoever. This is generally used to catch "use-while-borrowed" bugs (in debug builds). */
			/* These shouldn't need to be atomic as this class is not eligible to be shared among threads anyway. */
			bool m_access_is_prohibited = false;
			mse::non_thread_safe_mutex m_access_mutex;
#endif // !defined(NDEBUG)

			template<class TDynamicContainer, bool LockAccessToOriginal> friend class mse::us::impl::Txscope_exclusive_structure_lock_guard;
			friend class mse::us::impl::Txscope_shared_structure_lock_guard<_Myt>;
			friend class mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>;
			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_shared_structure_lock_guard_MemberFunction_impl;
			template<class T2, class EqualTo2> friend struct mse::impl::Has_s_make_xscope_exclusive_structure_lock_guard_MemberFunction_impl;
#endif // !defined(NDEBUG)

			template <class Y> friend class TXSLTASingleOwnerPointer;
			template<class _TPointerToLender, class _TLender, class _Ty2, bool _ExclusiveAccess, MSE_IMPL_EI_FORWARD_DECL(mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_msesingle_owner<_TLender>::value*/)>)>
			friend class xslta_accessing_fixed_owning_pointer;
			template <class _TLender, class T2>
			friend class xslta_borrowing_fixed_owning_pointer;
			template<typename _Ty2>
			friend class TXSLTASingleOwnerFixedPointer;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(X, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTASingleOwnerPointer);

	namespace rsv {
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTASingleOwnerFixedPointerElementProxyRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = typename _TLender::element_type >
		using TXSLTASingleOwnerFixedPointerElementProxyConstRef = mse::rsv::TXSLTADynamicStrongPointerElementProxyConstRef<mse::rsv::xslta_accessing_fixed_owning_pointer<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty>;

		template<class TElementProxyRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTASingleOwnerFixedPointerElementProxyPtr = TXSLTADynamicStrongPointerElementProxyPtr<TElementProxyRef, _TLender, _Ty>;
		template<class TElementProxyConstRef, class _TLender, class _Ty = typename _TLender::element_type>
		using TXSLTASingleOwnerFixedPointerElementProxyConstPtr = TXSLTADynamicStrongPointerElementProxyConstPtr<TElementProxyConstRef, _TLender, _Ty>;

		/* TXSLTASingleOwnerFixedPointer cannot be retargeted or constructed without a target (i.e. it does not support having a null value). */
		template<typename _Ty>
		class TXSLTASingleOwnerFixedPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::RefCStrongPointerTagBase, public mse::us::impl::NeverNullTagBase
			, MSE_INHERIT_XSLTA_TAG_BASE_SET_FROM(_Ty, TXSLTASingleOwnerFixedPointer<_Ty>)
		{
		public:
			typedef _Ty element_type;
			typedef TXSLTASingleOwnerFixedPointer _Myt;

			MSE_CONSTEXPR23 explicit TXSLTASingleOwnerFixedPointer(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])"))
				: m_soptr(MSE_FWD(src_ref)) {
			}

			MSE_CONSTEXPR23 explicit operator bool() const { return true; }
			TXSLTASingleOwnerFixedPointer<_Ty>& operator=(const TXSLTASingleOwnerFixedPointer<_Ty>& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") = delete;
			MSE_CONSTEXPR23 bool operator==(const TXSLTASingleOwnerFixedPointer& r) const { return m_soptr == r.m_soptr; }
			MSE_CONSTEXPR23 bool operator!=(const TXSLTASingleOwnerFixedPointer& r) const { return !((*this) == r); }

			MSE_CONSTEXPR23 auto& operator*() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				assert(this->get());
				return *(this->get());
			}
			MSE_CONSTEXPR23 auto& operator*() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				assert(this->get());
				return *(this->get());
			}
			MSE_CONSTEXPR23 auto* operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				assert(this->get());
				return this->get();
			}

			MSE_CONSTEXPR23 static TXSLTASingleOwnerFixedPointer make(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return TXSLTASingleOwnerFixedPointer(std::forward<_Ty>(src_ref));
			}

		private:
			TXSLTASingleOwnerFixedPointer(const TXSLTASingleOwnerFixedPointer& src_cref) = delete;
			TXSLTASingleOwnerFixedPointer(TXSLTASingleOwnerFixedPointer&& src_ref) : m_soptr(std::move(src_ref.m_soptr)) {}

			TXSLTASingleOwnerFixedPointer(TXSLTASingleOwnerPointer<_Ty> const& r) = delete;
			MSE_CONSTEXPR23 TXSLTASingleOwnerFixedPointer(TXSLTASingleOwnerPointer<_Ty>&& r)
				: m_soptr(std::move(r)) {
				if (!m_soptr) { MSE_THROW(single_owner_null_dereference_error("attempt to construct a 'not null' 'fixed' pointer from a null pointer value - mse::rsv::TXSLTASingleOwnerFixedPointer")); }
			}

			MSE_CONSTEXPR23 _Ty* unchecked_get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return m_soptr.unchecked_get();
			}
			MSE_CONSTEXPR23 _Ty* get() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return unchecked_get();
			}

			TXSLTASingleOwnerPointer<_Ty> m_soptr;

			template <class Y> friend class TXSLTASingleOwnerPointer;
			template <class Y> friend class TXSLTASingleOwnerFixedPointer;
			template<typename _Ty2>
			friend TXSLTASingleOwnerFixedPointer<_Ty2> not_null_from_nullable(TXSLTASingleOwnerPointer<_Ty2>&& src);
			template<class _TPointerToLender, class _TLender, class _Ty2, bool _ExclusiveAccess, MSE_IMPL_EI_FORWARD_DECL(mse::impl::enable_if_t<(true/*mse::impl::HasOrInheritsGetMethod_msesingle_owner<_TLender>::value*/)>)>
			friend class xslta_accessing_fixed_owning_pointer;
			template <class _TLender, class T2>
			friend class xslta_borrowing_fixed_owning_pointer;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTASingleOwnerFixedPointer);

	namespace rsv {
#ifdef MSE_HAS_CXX17

		template <class X>
		TXSLTASingleOwnerFixedPointer<X> make_xslta_fixed_single_owner(X x MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_12$)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTASingleOwnerFixedPointer<X>::make(std::move(x));
		}

#endif // MSE_HAS_CXX17

		template <class X>
		TXSLTASingleOwnerPointer<X> make_xslta_nullable_single_owner(X x MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_12$)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTASingleOwnerPointer<X>::make(std::move(x));
		}
		template <class X>
		TXSLTASingleOwnerPointer<X> make_xslta_nullable_single_owner(std::nullptr_t, const X& x MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_12$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(X, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
			return TXSLTASingleOwnerPointer<X>::make(nullptr, x);
		}

		template<typename _Ty>
		TXSLTASingleOwnerFixedPointer<_Ty> not_null_from_nullable(TXSLTASingleOwnerPointer<_Ty>&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_11$); labels(alias_11$); return_value(alias_11$) }") {
			return MSE_FWD(src);
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::rsv::TXSLTASingleOwnerPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTASingleOwnerPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTASingleOwnerPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::rsv::TXSLTASingleOwnerFixedPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTASingleOwnerFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTASingleOwnerFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty*>()(ptr1));
		}
	};
}
namespace mse {


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class TRefCountingPointer_test {
	public:
#ifdef MSE_SELF_TESTS
		// sensed events
		typedef std::map<std::string, int> Events;
		/*static */Events constructions, destructions;

		struct Trackable
		{
			Trackable(TRefCountingPointer_test* state_ptr, const std::string& id) : m_state_ptr(state_ptr), _id(id) {
				state_ptr->constructions[_id]++;
			}
			~Trackable() { m_state_ptr->destructions[_id]++; }
			TRefCountingPointer_test* m_state_ptr;
			const std::string _id;
		};

		typedef TRefCountingPointer<Trackable> target_t;
#endif // MSE_SELF_TESTS


#define MTXASSERT_EQ(a, b, c) a &= (b==c)
#define MTXASSERT(a, b) a &= static_cast<bool>(b)
		bool testBehaviour()
		{
			bool ok = true;
#ifdef MSE_SELF_TESTS
			static const TRefCountingPointer<Trackable> Nil = target_t(nullptr);

			constructions.clear();
			destructions.clear();

			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t a = mse::make_refcounting<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = mse::make_refcounting<Trackable>(this, "noot"),
					c = mse::make_refcounting<Trackable>(this, "mies"),
					nil = Nil,
					a2 = a;

				MTXASSERT(ok, a2 == a);
				MTXASSERT(ok, nil != a);

				MTXASSERT_EQ(ok, 3ul, constructions.size());
				MTXASSERT_EQ(ok, 1, constructions["aap"]);
				MTXASSERT_EQ(ok, 1, constructions["noot"]);
				MTXASSERT_EQ(ok, 1, constructions["mies"]);
				MTXASSERT_EQ(ok, 0, constructions["broer"]);
				MTXASSERT_EQ(ok, 4ul, constructions.size());

				MTXASSERT_EQ(ok, 0ul, destructions.size());

				hold = b;
			}

			MTXASSERT_EQ(ok, 1ul, destructions.size());
			MTXASSERT_EQ(ok, 0, destructions["aap"]);
			MTXASSERT_EQ(ok, 0, destructions["noot"]);
			MTXASSERT_EQ(ok, 1, destructions["mies"]);
			MTXASSERT_EQ(ok, 3ul, destructions.size());

			hold = Nil;
			MTXASSERT_EQ(ok, 3ul, destructions.size());
			MTXASSERT_EQ(ok, 0, destructions["aap"]);
			MTXASSERT_EQ(ok, 1, destructions["noot"]);
			MTXASSERT_EQ(ok, 1, destructions["mies"]);
			MTXASSERT_EQ(ok, 4ul, constructions.size());
#endif // MSE_SELF_TESTS

			// ok, enuf for now
			return ok;
		}

#ifdef MSE_SELF_TESTS
		struct Linked : Trackable
		{
			Linked(TRefCountingPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountingPointer<Linked> next;
		};
#endif // MSE_SELF_TESTS

		bool testLinked()
		{
			bool ok = true;
#ifdef MSE_SELF_TESTS

			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountingPointer<Linked> node = mse::make_refcounting<Linked>(this, "parent");
			MTXASSERT(ok, (node != nullptr));
			node->next = mse::make_refcounting<Linked>(this, "child");

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, (node != nullptr));

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 1ul, destructions.size());

			node = node->next;
			MTXASSERT(ok, (node == nullptr));

			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 2ul, destructions.size());
#endif // MSE_SELF_TESTS

			return ok;
		}

		void test1() {
#ifdef MSE_SELF_TESTS
			class A {
			public:
				A() {}
				A(const A& _X) : b(_X.b) {}
				A(A&& _X) : b(MSE_FWD(_X).b) {}
				virtual ~A() {}
				A& operator=(A&& _X) { b = MSE_FWD(_X).b; return (*this); }
				A& operator=(const A& _X) { b = _X.b; return (*this); }

				int b = 3;
				std::string s = "some text ";
			};
			class B {
			public:
				static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
				static int foo2(mse::TRefCountingPointer<A> A_refcounting_ptr) { return A_refcounting_ptr->b; }
			};

			A* A_native_ptr = nullptr;
			/* mse::TRefCountingPointer<> is basically a slightly "safer" version of std::shared_ptr. */
			mse::TRefCountingPointer<A> A_refcounting_ptr1;

			{
				A a;

				A_native_ptr = &a;
				A_refcounting_ptr1 = mse::make_refcounting<A>();
				assert(A_native_ptr->b == A_refcounting_ptr1->b);

				mse::TRefCountingPointer<A> A_refcounting_ptr2 = A_refcounting_ptr1;
				A_refcounting_ptr2 = nullptr;
#ifndef MSE_REFCOUNTINGPOINTER_DISABLED
				bool expected_exception = false;
				MSE_TRY {
					int i = A_refcounting_ptr2->b; /* this is gonna throw an exception */
				}
				MSE_CATCH_ANY {
					//std::cerr << "expected exception" << std::endl;
					expected_exception = true;
					/* The exception is triggered by an attempt to dereference a null "refcounting pointer". */
				}
				assert(expected_exception);
#endif // !MSE_REFCOUNTINGPOINTER_DISABLED

				B::foo1(&(*A_refcounting_ptr1));

				if (A_refcounting_ptr2) {
				}
				else if (A_refcounting_ptr2 != A_refcounting_ptr1) {
					A_refcounting_ptr2 = A_refcounting_ptr1;
					assert(A_refcounting_ptr2 == A_refcounting_ptr1);
				}

				mse::TRefCountingConstPointer<A> rcp = A_refcounting_ptr1;
				mse::TRefCountingConstPointer<A> rcp2 = rcp;
				rcp = mse::make_refcounting<A>();
				mse::TRefCountingFixedConstPointer<A> rfcp = mse::make_refcounting<A>();
				{
					int i = rfcp->b;
				}
			}

			int i = A_refcounting_ptr1->b;

			{
				class D : public A {};
				mse::TRefCountingFixedPointer<D> D_refcountingfixed_ptr1 = mse::make_refcounting<D>();
				//mse::TRefCountingFixedPointer<const D> constD_refcountingfixed_ptr1 = D_refcountingfixed_ptr1;
				//mse::TRefCountingPointer<A> A_refcountingfixed_ptr2 = D_refcountingfixed_ptr1;
				//int j = A_refcountingfixed_ptr2->b;
				int k = D_refcountingfixed_ptr1->b;
			}

			{
				/* You can use the "mse::make_pointer_to_member_v2()" function to obtain a safe pointer to a member of
				an object owned by a refcounting pointer. */
				auto s_safe_ptr1 = mse::make_pointer_to_member_v2(A_refcounting_ptr1, &A::s);
				(*s_safe_ptr1) = "some new text";
				auto s_safe_const_ptr1 = mse::make_const_pointer_to_member_v2(A_refcounting_ptr1, &A::s);

				/* Just testing the convertibility of mse::TStrongFixedPointers. */
				auto A_refcfp = mse::make_refcounting<A>();
				auto sfptr1 = mse::us::make_strong<std::string>(A_refcfp->s, A_refcfp);
				mse::us::TStrongFixedPointer<std::string, mse::TRefCountingFixedPointer<A>> sfptr2 = sfptr1;
				mse::us::TStrongFixedConstPointer<std::string, mse::TRefCountingFixedPointer<A>> sfcptr1 = sfptr2;
				mse::us::TStrongFixedConstPointer<std::string, mse::TRefCountingPointer<A>> sfcptr2 = sfcptr1;
				if (sfcptr1 == sfptr1) {
					int q = 7;
				}
				if (sfptr1 == sfcptr1) {
					int q = 7;
				}
				if (sfptr1) {
					int q = 7;
				}
				if (A_refcfp) {
					int q = 5;
				}
			}

			{
				auto xl_refcptr1 = mse::rsv::TXSLTARefCountingPointer<int>::make(5);
				int i1 = *xl_refcptr1;
				*xl_refcptr1 = i1;
				auto xl_af_refcptr1 = mse::rsv::make_xslta_accessing_fixed_owning_pointer(&xl_refcptr1);
				auto i2 = *xl_af_refcptr1;
				*xl_af_refcptr1 = i2;
				int q = 5;
			}
			{
				int i1 = 3;
				int i2 = 5;
				int i3 = 7;
				auto iltaptr4 = mse::rsv::TXSLTAPointer<int>{ &i2 };
				auto iltaptr5 = mse::rsv::TXSLTAPointer<int>{ &i1 };

				mse::rsv::TXSLTARefCountingPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlrefcptr3 = mse::rsv::make_xslta_nullable_refcounting(iltaptr4);

				/* Even when you want to construct a null rsv::TXSLTARefCountingPointer<>, if the element type has an annotated
				lifetime, you would still need to provide (a reference to) an initialization element object from which
				a lower bound lifetime can be inferred. You could just initialize the refcounting pointer with a value, then reset()
				the rsv::TXSLTARefCountingPointer<>. Alternatively, you can pass nullptr as the first constructor parameter,
				and a second (otherwise unused) parameter from which the lower bound lifetime will be inferred. */
				mse::rsv::TXSLTARefCountingPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlrefcptr2(nullptr, iltaptr4);
				//mse::rsv::TXSLTARefCountingPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlrefcptr;    // scpptool would complain
				mse::rsv::TXSLTARefCountingPointer<int> int_xlrefcptr;    // fine, the element type does not have an annotated lifetime

				auto int_xlptr_xlrefcptr5 = mse::rsv::make_xslta_nullable_refcounting(nullptr, iltaptr4);
				auto int_xlptr_xlrefcptr6 = mse::rsv::make_xslta_nullable_refcounting(iltaptr4);
				{
					/* As with rsv::xslta_optional<>, the preferred way of accessing the (owned) target of an rsv::TXSLTARefCountingPointer<>
					is via an associated rsv::xslta_borrowing_fixed_owning_pointer<> (which, while it exists, "borrows" exclusive
					access to the (pointer) value of the given owning pointer and prevents it from being reset() or replaced in a way that 
					might result in the deallocation of the owned target object. */
					auto bfint_xlptr_xlrefcptr6 = mse::rsv::make_xslta_borrowing_fixed_owning_pointer(&int_xlptr_xlrefcptr6);
					auto iltaptr26 = *bfint_xlptr_xlrefcptr6;
					std::swap(iltaptr26, iltaptr4);
				}

				/* While not the preferred method, rsv::TXSLTARefCountingPointer<> does (currently) have limited support for accessing
				its owned target object (pseudo-)directly. */

				/* As with rsv::xslta_optional<>, rsv::TXSLTARefCountingPointer<>'s dereference methods and operators do not
				return a raw reference. They return a "proxy reference" object that (while it exists, prevents the deallocation 
				of the target object and) behaves like a (raw) reference in some situations. For example, like a reference,
				it can be cast to the element type. */
				typename decltype(int_xlptr_xlrefcptr6)::element_type iltaptr6 = (*int_xlptr_xlrefcptr6);
				iltaptr6 = &i2;
				//iltaptr6 = &i3; // scpptool would complain (because i3 does not live long enough)

				/* The returned "proxy reference" object also has limited support for assignment operations. */
				*int_xlptr_xlrefcptr6 = &i1;
				//*int_xlptr_xlrefcptr6 = &i3;    // scpptool would complain (because i3 does not live long enough)

				/* Note that these returned "proxy reference" objects are designed to be used as temporary (rvalue) objects,
				not as (lvalue) declared variables or stored objects. */

				auto const& int_xlptr_xlrefcptr6_cref1 = int_xlptr_xlrefcptr6;
				typename decltype(int_xlptr_xlrefcptr6)::element_type iltaptr3b = *int_xlptr_xlrefcptr6_cref1;

				/* rsv::TXSLTARefCountingFixedPointer<> is a (lifetime annotated) refcounting pointer that doesn't support 
				any operations that would change which object is being targeted/owned (subsequent to initialization). (Basically 
				a "const rsv::TXSLTARefCountingNotNullPointer<>".) Because the owned target object is fixed, its dereference 
				operators just return raw references, so unlike its "dynamic" counterparts, there's no need to involve a 
				"borrowing fixed owning pointer". */
				mse::rsv::TXSLTARefCountingFixedPointer<mse::rsv::TXSLTAPointer<int> > fint_xlptr_xlrefcptr2 = mse::rsv::make_xslta_refcounting<mse::rsv::TXSLTAPointer<int> >(iltaptr4);
				auto fint_xlptr_xlrefcptr16 = mse::rsv::TXSLTARefCountingFixedPointer<mse::rsv::TXSLTAPointer<int> >(mse::rsv::make_xslta_refcounting<mse::rsv::TXSLTAPointer<int> >(iltaptr4));
				auto& iltaptr16_ref = *fint_xlptr_xlrefcptr16;
				std::swap(iltaptr16_ref, iltaptr4);
				std::swap(iltaptr5, iltaptr16_ref);

				{
					int i21 = 11;
					auto iltaptr21 = mse::rsv::TXSLTAPointer<int>{ &i21 };
					auto int_xlptr_xlrefcptr21 = mse::rsv::make_xslta_nullable_refcounting(iltaptr21);
					//int_xlptr_xlrefcptr6 = int_xlptr_xlrefcptr21; /* scpptool would complain */
					/* because the lifetime associated with int_xlptr_xlrefcptr21's lifetime restriction is shorter than that 
					of int_xlptr_xlrefcptr6 */
					int_xlptr_xlrefcptr21 = int_xlptr_xlrefcptr6; // which means the reverse assignment is safe and permitted

					auto int_xlrefcptr21 = mse::rsv::make_xslta_nullable_refcounting(i21);
					int_xlrefcptr = int_xlrefcptr21;    // but this is fine because the element type does not have an annotated lifetime
					int_xlrefcptr21 = int_xlrefcptr;


					auto refc_ptr2 = mse::rsv::make_xslta_nullable_refcounting(mse::rsv::TXSLTAPointer<int>{ &i2 });

					{
						auto const& refc_ptr2_cref = refc_ptr2;

						/* Obtaining an rsv::xslta_borrowing_fixed_owning_pointer<> requires a non-const pointer to the lending owning pointer.
						When only a const pointer is available we can instead use rsv::xslta_accessing_fixed_owning_pointer<> for supported owning_pointer types. */
						auto af_refc_ptr2a = mse::rsv::make_xslta_accessing_fixed_owning_pointer(&refc_ptr2_cref);
						// or
						//auto af_refc_ptr2a = mse::rsv::xslta_accessing_fixed_owning_pointer(&refc_ptr2_cref);

						auto& elem_ref1 = *af_refc_ptr2a;
						int i4 = *elem_ref1;
					}

					{
						//typedef mse::rsv::TXSLTAPointer<int> A;
						class A {
						public:
							A(mse::rsv::TXSLTAPointer<int> const& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[42])")) : m_ptr(src) {}
							mse::rsv::TXSLTAPointer<int> m_ptr;
						} MSE_ATTR_STR("mse::lifetime_label(42)") MSE_ATTR_STR("mse::lifetime_label_for_base_class(42)");

						class D : public A {
						public:
							typedef A base_class;
							D(A const& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[42])")) : base_class(src) {}
						} MSE_ATTR_STR("mse::lifetime_label(42)") MSE_ATTR_STR("mse::lifetime_label_for_base_class(42)");

						auto D_refcounting_ptr1 = mse::rsv::make_xslta_nullable_refcounting<D>(A{ iltaptr4 });
						auto A_refcounting_ptr1 = mse::rsv::make_xslta_nullable_refcounting<A>(iltaptr4);
						A_refcounting_ptr1 = D_refcounting_ptr1;
						A a1 = *D_refcounting_ptr1;
						mse::rsv::TXSLTAPointer<int> int_xlptr11 = a1.m_ptr;
					}
				}
				{
					auto int_xlptr_xlrefcnnptr2 = mse::rsv::make_xslta_refcounting<mse::rsv::TXSLTAPointer<int> >(iltaptr4);
					{
						auto bfint_xlptr_xlrefcnnptr2 = mse::rsv::make_xslta_borrowing_fixed_owning_pointer(&int_xlptr_xlrefcnnptr2);
						auto iltaptr26 = *bfint_xlptr_xlrefcnnptr2;
						std::swap(iltaptr26, iltaptr4);
					}
				}
				{
					auto ptr1 = mse::rsv::make_xslta_nullable_refcounting(int{ 3 });
					{
						auto bf_own_ptr1 = mse::rsv::make_xslta_borrowing_fixed_owning_not_null_pointer(&ptr1);
					}
					auto af_own_ptr1 = mse::rsv::make_xslta_accessing_fixed_owning_not_null_pointer(&ptr1);
					auto ptr2 = ptr1;
				}
				{
					auto ptr1 = mse::rsv::make_xslta_refcounting(int{ 3 });
					{
						auto bf_own_ptr1 = mse::rsv::make_xslta_borrowing_fixed_owning_not_null_pointer(&ptr1);
					}
					auto af_own_ptr1 = mse::rsv::make_xslta_accessing_fixed_owning_not_null_pointer(&ptr1);
					auto ptr2 = ptr1;
				}
				int q = 5;
			}
#endif // MSE_SELF_TESTS
		}
	};

	class TSingleOwnerPointer_test {
	public:
#ifdef MSE_SELF_TESTS
		// helper class for runtime polymorphism demo below
		struct B
		{
			virtual ~B() = default;

			virtual void bar() { std::cout << "B::bar\n"; }
		};

		struct D : B
		{
			D() { std::cout << "D::D\n"; }
			~D() { std::cout << "D::~D\n"; }

			void bar() override { std::cout << "D::bar\n"; }
		};

		// a function consuming a unique_ptr can take it by value or by rvalue reference
		mse::TSingleOwnerPointer<D> pass_through(mse::TSingleOwnerPointer<D> p)
		{
			p->bar();
			return p;
		}

		// helper function for the custom deleter demo below
		void close_file(std::FILE* fp)
		{
			std::fclose(fp);
		}

		// unique_ptr-based linked list demo
		struct List
		{
			struct Node
			{
				int data;
				mse::TSingleOwnerPointer<Node> next;
			};

			mse::TSingleOwnerPointer<Node> head;

			~List()
			{
				// destroy list nodes sequentially in a loop, the default destructor
				// would have invoked its `next`'s destructor recursively, which would
				// cause stack overflow for sufficiently large lists.
				while (head)
				{
					auto next = std::move(head->next);
					head = std::move(next);
				}
			}

			void push(int data)
			{
				head = mse::make_single_owner<Node>(Node{ data, std::move(head) });
				//head = mse::TSingleOwnerPointer<Node>(new Node{ data, std::move(head) });
			}
		};
#endif // MSE_SELF_TESTS

		void test1() {
#ifdef MSE_SELF_TESTS
			std::cout << "1) Unique ownership semantics demo\n";
			{
				// Create a (uniquely owned) resource
				mse::TSingleOwnerPointer<D> p = mse::make_single_owner<D>();

				// Transfer ownership to `pass_through`,
				// which in turn transfers ownership back through the return value
				mse::TSingleOwnerPointer<D> q = pass_through(std::move(p));

				// p is now in a moved-from 'empty' state, equal to nullptr
				assert(!p);
			}

			std::cout << "\n" "2) Runtime polymorphism demo\n";
			{
				// Create a derived resource and point to it via base type
				mse::TSingleOwnerPointer<B> p = mse::make_single_owner<D>();

				// Dynamic dispatch works as expected
				p->bar();
			}

			std::cout << "\n" "6) Linked list demo\n";
			{
				List wall;
				const int enough{ 1'000'000 };
				for (int beer = 0; beer != enough; ++beer)
					wall.push(beer);

				std::cout.imbue(std::locale("en_US.UTF-8"));
				std::cout << enough << " bottles of beer on the wall...\n";
			} // destroys all the beers

			{
				int i1 = 3;
				int i2 = 5;
				int i3 = 7;
				auto iltaptr4 = mse::rsv::TXSLTAPointer<int>{ &i2 };
				auto iltaptr5 = mse::rsv::TXSLTAPointer<int>{ &i1 };

				mse::rsv::TXSLTASingleOwnerPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlsoptr3 = mse::rsv::make_xslta_nullable_single_owner(iltaptr4);

				/* Even when you want to construct a null rsv::TXSLTASingleOwnerPointer<>, if the element type has an annotated
				lifetime, you would still need to provide (a reference to) an initialization element object from which
				a lower bound lifetime can be inferred. You could just initialize the single_owner pointer with a value, then reset()
				the rsv::TXSLTASingleOwnerPointer<>. Alternatively, you can pass nullptr as the first constructor parameter,
				and a second (otherwise unused) parameter from which the lower bound lifetime will be inferred. */
				mse::rsv::TXSLTASingleOwnerPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlsoptr2(nullptr, iltaptr4);
				//mse::rsv::TXSLTASingleOwnerPointer<mse::rsv::TXSLTAPointer<int> > int_xlptr_xlsoptr;    // scpptool would complain
				mse::rsv::TXSLTASingleOwnerPointer<int> int_xlsoptr;    // fine, the element type does not have an annotated lifetime

				auto int_xlptr_xlsoptr5 = mse::rsv::make_xslta_nullable_single_owner(nullptr, iltaptr4);
				auto int_xlptr_xlsoptr6 = mse::rsv::make_xslta_nullable_single_owner(iltaptr4);
				{
					/* As with rsv::xslta_optional<>, the preferred way of accessing the (owned) target of an rsv::TXSLTASingleOwnerPointer<>
					is via an associated rsv::xslta_borrowing_fixed_owning_pointer<> (which, while it exists, "borrows" exclusive
					access to the (pointer) value of the given owning pointer and prevents it from being reset() or replaced in a way that
					might result in the deallocation of the owned target object. */
					auto bfint_xlptr_xlsoptr6 = mse::rsv::make_xslta_borrowing_fixed_owning_pointer(&int_xlptr_xlsoptr6);
					auto& iltaptr26_ref = *bfint_xlptr_xlsoptr6;
					std::swap(iltaptr26_ref, iltaptr4);
				}

				/* While not the preferred method, rsv::TXSLTASingleOwnerPointer<> does (currently) have limited support for accessing
				its owned target object (pseudo-)directly. */

				/* As with rsv::xslta_optional<>, rsv::TXSLTASingleOwnerPointer<>'s dereference methods and operators do not
				return a raw reference. They return a "proxy reference" object that (while it exists, prevents the deallocation
				of the target object and) behaves like a (raw) reference in some situations. For example, like a reference,
				it can be cast to the element type. */
				typename decltype(int_xlptr_xlsoptr6)::element_type iltaptr6 = (*int_xlptr_xlsoptr6);
				iltaptr6 = &i2;
				//iltaptr6 = &i3; // scpptool would complain (because i3 does not live long enough)

				/* The returned "proxy reference" object also has limited support for assignment operations. */
				*int_xlptr_xlsoptr6 = &i1;
				//*int_xlptr_xlsoptr6 = &i3;    // scpptool would complain (because i3 does not live long enough)

				/* Note that these returned "proxy reference" objects are designed to be used as temporary (rvalue) objects,
				not as (lvalue) declared variables or stored objects. */

#ifdef MSE_HAS_CXX17

				/* rsv::TXSLTASingleOwnerFixedPointer<> is a (lifetime annotated) single_owner pointer that doesn't support
				any operations that would change which object is being targeted/owned (subsequent to initialization). Because
				the owned target object is fixed, its dereference operators just return raw references, so unlike its "dynamic"
				counterpart, there's no need to involve a "borrowing fixed owning pointer". */
				auto fint_xlptr_xlsoptr16 = mse::rsv::make_xslta_fixed_single_owner<mse::rsv::TXSLTAPointer<int> >(iltaptr4);
				auto& iltaptr16_ref = *fint_xlptr_xlsoptr16;
				std::swap(iltaptr16_ref, iltaptr4);
				std::swap(iltaptr5, iltaptr16_ref);

#endif // MSE_HAS_CXX17

				{
					int i21 = 11;
					auto iltaptr21 = mse::rsv::TXSLTAPointer<int>{ &i21 };
					auto int_xlptr_xlsoptr21 = mse::rsv::make_xslta_nullable_single_owner(iltaptr21);
					//int_xlptr_xlsoptr6 = int_xlptr_xlsoptr21; /* scpptool would complain */
					/* because the lifetime associated with int_xlptr_xlsoptr21's lifetime restriction is shorter than that
					of int_xlptr_xlsoptr6 */
					int_xlptr_xlsoptr21 = std::move(int_xlptr_xlsoptr6); // which means the reverse assignment is safe and permitted

					auto int_xlsoptr21 = mse::rsv::make_xslta_nullable_single_owner(i21);
					int_xlsoptr = std::move(int_xlsoptr21);    // but this is fine because the element type does not have an annotated lifetime
					int_xlsoptr21 = std::move(int_xlsoptr);
				}
				{
					auto ptr1 = mse::rsv::make_xslta_nullable_single_owner(int{ 3 });
					{
						auto bf_own_ptr1 = mse::rsv::make_xslta_borrowing_fixed_owning_not_null_pointer(&ptr1);
					}
					auto ptr2 = std::move(ptr1);
				}
				int q = 5;
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

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#pragma pop_macro("_NOEXCEPT")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSEREFCOUNTING_H_
