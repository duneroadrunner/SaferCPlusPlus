
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEREFCOUNTING_H_
#define MSEREFCOUNTING_H_

//include "mseprimitives.h"
#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
#include "msescope.h"
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
#include <memory>
#include <iostream>
#include <utility>
#include <cassert>
#include <stdexcept>

/* for the test functions */
#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/


#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_REFCOUNTINGPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

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

#ifdef MSE_REFCOUNTINGPOINTER_DISABLED
	template <class X> using TRefCountingPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingNotNullPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingFixedPointer = std::shared_ptr<X>;
	template <class X> using TRefCountingConstPointer = std::shared_ptr<const X>;
	template <class X> using TRefCountingNotNullConstPointer = std::shared_ptr<const X>;
	template <class X> using TRefCountingFixedConstPointer = std::shared_ptr<const X>;

	template <class X, class... Args>
	TRefCountingFixedPointer<X> make_refcounting(Args&&... args) {
		return std::make_shared<X>(std::forward<Args>(args)...);
	}
#else /*MSE_REFCOUNTINGPOINTER_DISABLED*/

#ifdef MSEPOINTERBASICS_H
	typedef StrongPointerNotAsyncShareableTagBase RefCStrongPointerTagBase;
#else // MSEPOINTERBASICS_H
	class RefCStrongPointerTagBase {};
#endif // MSEPOINTERBASICS_H

	class refcounting_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template<typename _Ty> class TRefCountingNotNullPointer;
	template<typename _Ty> class TRefCountingFixedPointer;
	template<typename _Ty> class TRefCountingNotNullConstPointer;
	template<typename _Ty> class TRefCountingFixedConstPointer;

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
			return const_cast<void *>(static_cast<const void *>(std::addressof(m_object)));
		}
	};

	/* Some code originally came from this stackoverflow post:
	http://stackoverflow.com/questions/6593770/creating-a-non-thread-safe-shared-ptr */

	template <class X> class TRefCountingConstPointer;

	/* TRefCountingPointer behaves similar to an std::shared_ptr. Some differences being that it foregoes any thread safety
	mechanisms, it does not accept raw pointer assignment or construction (use make_refcounting<>() instead), and it will throw
	an exception on attempted nullptr dereference. And it's faster. */
	template <class X>
	class TRefCountingPointer : public RefCStrongPointerTagBase {
	public:
		TRefCountingPointer() : m_ref_with_target_obj_ptr(nullptr) {}
		TRefCountingPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
		~TRefCountingPointer() {
			release();

			/* This is just a no-op function that will cause a compile error when X is not an eligible type. */
			valid_if_X_is_not_an_xscope_type();
		}
		TRefCountingPointer(const TRefCountingPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		operator bool() const { return nullptr != get(); }
		void clear() { (*this) = TRefCountingPointer<X>(nullptr); }
		TRefCountingPointer& operator=(const TRefCountingPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		bool operator<(const TRefCountingPointer& r) const {
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
		template <class Y> TRefCountingPointer(const TRefCountingPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		template <class Y> TRefCountingPointer& operator=(const TRefCountingPointer<Y>& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		template <class Y> bool operator<(const TRefCountingPointer<Y>& r) const {
			return get() < r.get();
		}
		template <class Y> bool operator==(const TRefCountingPointer<Y>& r) const {
			return get() == r.get();
		}
		template <class Y> bool operator!=(const TRefCountingPointer<Y>& r) const {
			return get() != r.get();
		}
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

	protected:
		X* get() const {
			if (!m_ref_with_target_obj_ptr) {
				return nullptr;
			}
			else {
				X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				return x_ptr;
			}
		}

	private:
		explicit TRefCountingPointer(TRefWithTargetObj<X>* p/* = nullptr*/) {
			m_ref_with_target_obj_ptr = p;
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
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

#ifndef MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class X2 = X, class = typename std::enable_if<(std::is_same<X2, X>::value) && (!std::is_base_of<XScopeTagBase, X2>::value), void>::type>
#endif // !MSE_REFCOUNTING_NO_XSCOPE_DEPENDENCE
		void valid_if_X_is_not_an_xscope_type() const {}

		CRefCounter* m_ref_with_target_obj_ptr;

		friend class TRefCountingNotNullPointer<X>;
		friend class TRefCountingConstPointer<X>;
	};

	template<typename _Ty>
	class TRefCountingNotNullPointer : public TRefCountingPointer<_Ty> {
	public:
		TRefCountingNotNullPointer(const TRefCountingNotNullPointer& src_cref) : TRefCountingPointer<_Ty>(src_cref) {}
		TRefCountingNotNullPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		virtual ~TRefCountingNotNullPointer() {}
		TRefCountingNotNullPointer<_Ty>& operator=(const TRefCountingNotNullPointer<_Ty>& _Right_cref) {
			TRefCountingPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountingPointer<_Ty>::operator _Ty*(); }

	private:
		explicit TRefCountingNotNullPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountingPointer<_Ty>(p) {}

		//TRefCountingNotNullPointer<_Ty>* operator&() { return this; }
		//const TRefCountingNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingFixedPointer<_Ty>;
	};

	/* TRefCountingFixedPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRefCountingFixedPointer : public TRefCountingNotNullPointer<_Ty> {
	public:
		TRefCountingFixedPointer(const TRefCountingFixedPointer& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		TRefCountingFixedPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		TRefCountingFixedPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingNotNullPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TRefCountingNotNullPointer<_Ty>::operator _Ty*(); }

		template <class... Args>
		static TRefCountingFixedPointer make(Args&&... args) {
			auto new_ptr = new TRefWithTargetObj<_Ty>(std::forward<Args>(args)...);
			TRefCountingFixedPointer retval(new_ptr);
			return retval;
		}

	private:
		explicit TRefCountingFixedPointer(TRefWithTargetObj<_Ty>* p/* = nullptr*/) : TRefCountingNotNullPointer<_Ty>(p) {}
		TRefCountingFixedPointer<_Ty>& operator=(const TRefCountingFixedPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingFixedPointer<_Ty>* operator&() { return this; }
		//const TRefCountingFixedPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingConstPointer<_Ty>;
	};

	template <class X, class... Args>
	TRefCountingFixedPointer<X> make_refcounting(Args&&... args) {
		return TRefCountingFixedPointer<X>::make(std::forward<Args>(args)...);
	}


	template <class X>
	class TRefCountingConstPointer : public RefCStrongPointerTagBase {
	public:
		TRefCountingConstPointer() : m_ref_with_target_obj_ptr(nullptr) {}
		TRefCountingConstPointer(std::nullptr_t) : m_ref_with_target_obj_ptr(nullptr) {}
		~TRefCountingConstPointer() {
			release();
		}
		TRefCountingConstPointer(const TRefCountingConstPointer& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		TRefCountingConstPointer(const TRefCountingPointer<X>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		operator bool() const { return nullptr != get(); }
		void clear() { (*this) = TRefCountingConstPointer<X>(nullptr); }
		TRefCountingConstPointer& operator=(const TRefCountingConstPointer& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		TRefCountingConstPointer& operator=(const TRefCountingPointer<X>& r) {
			/*if (this != &r) */ {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		bool operator<(const TRefCountingConstPointer& r) const {
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
		template <class Y> TRefCountingConstPointer(const TRefCountingConstPointer<Y>& r) {
			acquire(r.m_ref_with_target_obj_ptr);
		}
		template <class Y> TRefCountingConstPointer& operator=(const TRefCountingConstPointer<Y>& r) {
			if (this != &r) {
				auto_release keep(m_ref_with_target_obj_ptr);
				acquire(r.m_ref_with_target_obj_ptr);
			}
			return *this;
		}
		template <class Y> bool operator<(const TRefCountingConstPointer<Y>& r) const {
			return get() < r.get();
		}
		template <class Y> bool operator==(const TRefCountingConstPointer<Y>& r) const {
			return get() == r.get();
		}
		template <class Y> bool operator!=(const TRefCountingConstPointer<Y>& r) const {
			return get() != r.get();
		}
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
		const X* get() const {
			if (!m_ref_with_target_obj_ptr) {
				return nullptr;
			}
			else {
				X* x_ptr = static_cast<X*>(m_ref_with_target_obj_ptr->target_obj_address());
				return x_ptr;
			}
		}
		bool unique() const {
			return (m_ref_with_target_obj_ptr ? (m_ref_with_target_obj_ptr->use_count() == 1) : true);
		}

	private:
		explicit TRefCountingConstPointer(TRefWithTargetObj<X>* p/* = nullptr*/) {
			m_ref_with_target_obj_ptr = p;
		}

		void acquire(CRefCounter* c) {
			m_ref_with_target_obj_ptr = c;
			if (c) { c->increment(); }
		}

		void release() {
			dorelease(m_ref_with_target_obj_ptr);
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

		CRefCounter* m_ref_with_target_obj_ptr;

		friend class TRefCountingNotNullConstPointer<X>;
	};

	template<typename _Ty>
	class TRefCountingNotNullConstPointer : public TRefCountingConstPointer<_Ty> {
	public:
		TRefCountingNotNullConstPointer(const TRefCountingNotNullConstPointer& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {}
		TRefCountingNotNullConstPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {}
		TRefCountingNotNullConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		TRefCountingNotNullConstPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingConstPointer<_Ty>(src_cref) {
			*src_cref; // to ensure that src_cref points to a valid target
		}
		virtual ~TRefCountingNotNullConstPointer() {}
		TRefCountingNotNullConstPointer<_Ty>& operator=(const TRefCountingNotNullConstPointer<_Ty>& _Right_cref) {
			TRefCountingConstPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}

		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRefCountingConstPointer<_Ty>::operator _Ty*(); }

	private:
		//TRefCountingNotNullConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingNotNullConstPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingFixedConstPointer<_Ty>;
	};

	/* TRefCountingFixedConstPointer cannot be retargeted or constructed without a target. This pointer is recommended for passing
	parameters by reference. */
	template<typename _Ty>
	class TRefCountingFixedConstPointer : public TRefCountingNotNullConstPointer<_Ty> {
	public:
		TRefCountingFixedConstPointer(const TRefCountingFixedConstPointer& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingFixedPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingNotNullConstPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingNotNullPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		TRefCountingFixedConstPointer(const TRefCountingPointer<_Ty>& src_cref) : TRefCountingNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TRefCountingFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TRefCountingNotNullConstPointer<_Ty>::operator _Ty*(); }

	private:
		TRefCountingFixedConstPointer<_Ty>& operator=(const TRefCountingFixedConstPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingFixedConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingFixedConstPointer<_Ty>* operator&() const { return this; }
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

#ifdef MSEPOINTERBASICS_H
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, TRefCountingPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TRefCountingPointer<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, TRefCountingPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingConstPointer<_Ty>> make_pointer_to_member(const _TTargetType& target, const TRefCountingConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingConstPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingConstPointer<_Ty>>::make(target, lease_pointer);
	}

	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, TRefCountingNotNullPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TRefCountingNotNullPointer<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, TRefCountingNotNullPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullConstPointer<_Ty>> make_pointer_to_member(const _TTargetType& target, const TRefCountingNotNullConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullConstPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingNotNullPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingNotNullConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingNotNullConstPointer<_Ty>>::make(target, lease_pointer);
	}

	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, TRefCountingFixedPointer<_Ty>> make_pointer_to_member(_TTargetType& target, const TRefCountingFixedPointer<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, TRefCountingFixedPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingFixedConstPointer<_Ty>> make_pointer_to_member(const _TTargetType& target, const TRefCountingFixedConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingFixedConstPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingFixedPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingFixedPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingFixedPointer<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, TRefCountingFixedConstPointer<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const TRefCountingFixedConstPointer<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, TRefCountingFixedConstPointer<_Ty>>::make(target, lease_pointer);
	}


	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedPointer<_TTarget, TRefCountingPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingNotNullPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedPointer<_TTarget, TRefCountingNotNullPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingNotNullConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingNotNullConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingNotNullPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingNotNullPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingNotNullConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingNotNullConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedPointer<_TTarget, TRefCountingFixedPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TRefCountingFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingFixedConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingFixedPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TRefCountingFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, TRefCountingFixedConstPointer<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)

	template<class _TTargetType, class _Ty>
	TStrongFixedPointer<_TTargetType, std::shared_ptr<_Ty>> make_pointer_to_member(_TTargetType& target, const std::shared_ptr<_Ty> &lease_pointer) {
		return TStrongFixedPointer<_TTargetType, std::shared_ptr<_Ty>>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TStrongFixedConstPointer<_TTargetType, std::shared_ptr<_Ty>> make_const_pointer_to_member(const _TTargetType& target, const std::shared_ptr<_Ty> &lease_pointer) {
		return TStrongFixedConstPointer<_TTargetType, std::shared_ptr<_Ty>>::make(target, lease_pointer);
	}

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const std::shared_ptr<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedPointer<_TTarget, std::shared_ptr<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const std::shared_ptr<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return TStrongFixedConstPointer<_TTarget, std::shared_ptr<_Ty>>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
#endif // MSEPOINTERBASICS_H


#ifdef MSESCOPE_H_
	template<typename _Ty> using TXScopeRefCountingStore = TXScopeStrongPointerStore<TRefCountingPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingConstStore = TXScopeStrongConstPointerStore<TRefCountingConstPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingNotNullStore = TXScopeStrongNotNullPointerStore<TRefCountingNotNullPointer<_Ty> >;
	template<typename _Ty> using TXScopeRefCountingNotNullConstStore = TXScopeStrongNotNullConstPointerStore<TRefCountingNotNullConstPointer<_Ty> >;

	template<typename _Ty>
	TXScopeRefCountingStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeRefCountingConstStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingConstPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingConstStore<_Ty>(stored_ptr);
	}
#if !defined(MSE_REFCOUNTINGPOINTER_DISABLED)
	template<typename _Ty>
	TXScopeRefCountingNotNullStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingNotNullPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingNotNullStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeRefCountingNotNullConstStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingNotNullConstPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingNotNullConstStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeRefCountingNotNullStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingFixedPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingNotNullStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeRefCountingNotNullConstStore<_Ty> make_xscope_strong_pointer_store(const TRefCountingFixedConstPointer<_Ty>& stored_ptr) {
		return TXScopeRefCountingNotNullConstStore<_Ty>(stored_ptr);
	}
#endif // !defined(MSE_REFCOUNTINGPOINTER_DISABLED)

#endif // MSESCOPE_H_


	/* shorter aliases */
	template<typename _Ty> using refcp = TRefCountingPointer<_Ty>;
	template<typename _Ty> using refccp = TRefCountingConstPointer<_Ty>;
	template<typename _Ty> using refcnnp = TRefCountingNotNullPointer<_Ty>;
	template<typename _Ty> using refcnncp = TRefCountingNotNullConstPointer<_Ty>;
	template<typename _Ty> using refcfp = TRefCountingFixedPointer<_Ty>;
	template<typename _Ty> using refcfcp = TRefCountingFixedConstPointer<_Ty>;

	template <class X, class... Args>
	auto mkrc(Args&&... args) {
		return make_refcounting<X>(std::forward<Args>(args)...);
	}

	/* These functions assume the type is the same as the lone parameter, so you don't need to explicitly specify it. */
	template <typename _TLoneParam>
	auto mkrclp(const _TLoneParam& lone_param) {
		return make_refcounting<_TLoneParam>(lone_param);
	}
	template <typename _TLoneParam>
	auto mkrclp(_TLoneParam&& lone_param) {
		return make_refcounting<_TLoneParam>(std::forward<decltype(lone_param)>(lone_param));
	}

	/* deprecated aliases */
	template<class _TTargetType, class _TLeaseType> using strfp = TStrongFixedPointer<_TTargetType, _TLeaseType>;
	template<class _TTargetType, class _TLeaseType> using strfcp = TStrongFixedConstPointer<_TTargetType, _TLeaseType>;


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class TRefCountingPointer_test {
	public:
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

			target_t a = make_refcounting<Trackable>(this, "aap");

			MTXASSERT_EQ(ok, 1ul, constructions.size());
			MTXASSERT_EQ(ok, 1, constructions["aap"]);
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			MTXASSERT_EQ(ok, 0, constructions["noot"]);
			MTXASSERT_EQ(ok, 2ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			target_t hold;
			{
				target_t b = make_refcounting<Trackable>(this, "noot"),
					c = make_refcounting<Trackable>(this, "mies"),
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

		struct Linked : Trackable
		{
			Linked(TRefCountingPointer_test* state_ptr, const std::string&t) :Trackable(state_ptr, t) {}
			TRefCountingPointer<Linked> next;
		};

		bool testLinked()
		{
			bool ok = true;
#ifdef MSE_SELF_TESTS

			constructions.clear();
			destructions.clear();
			MTXASSERT_EQ(ok, 0ul, constructions.size());
			MTXASSERT_EQ(ok, 0ul, destructions.size());

			TRefCountingPointer<Linked> node = make_refcounting<Linked>(this, "parent");
			MTXASSERT(ok, (node != nullptr));
			node->next = make_refcounting<Linked>(this, "child");

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
				A(A&& _X) : b(std::forward<decltype(_X.b)>(_X.b)) {}
				virtual ~A() {}
				A& operator=(A&& _X) { b = std::forward<decltype(_X.b)>(_X.b); return (*this); }
				A& operator=(const A& _X) { b = _X.b; return (*this); }

				int b = 3;
				std::string s = "some text ";
			};
			class B {
			public:
				static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
				static int foo2(mse::TRefCountingPointer<A> A_refcounting_ptr) { return A_refcounting_ptr->b; }
			protected:
				~B() {}
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
				try {
					int i = A_refcounting_ptr2->b; /* this is gonna throw an exception */
				}
				catch (...) {
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
				mse::TRefCountingPointer<A> A_refcountingfixed_ptr2 = D_refcountingfixed_ptr1;
				int j = A_refcountingfixed_ptr2->b;
				int k = D_refcountingfixed_ptr1->b;
			}

			{
				/* You can use the "mse::make_pointer_to_member()" function to obtain a safe pointer to a member of
				an object owned by a refcounting pointer. */
				auto s_safe_ptr1 = mse::make_pointer_to_member(A_refcounting_ptr1->s, A_refcounting_ptr1);
				(*s_safe_ptr1) = "some new text";
				auto s_safe_const_ptr1 = mse::make_const_pointer_to_member(A_refcounting_ptr1->s, A_refcounting_ptr1);

				/* Just testing the convertibility of mse::TStrongFixedPointers. */
				auto A_refcfp = mse::make_refcounting<A>();
				auto sfptr1 = mse::make_strong<std::string>(A_refcfp->s, A_refcfp);
				mse::TStrongFixedPointer<std::string, mse::TRefCountingPointer<A>> sfptr2 = sfptr1;
				mse::TStrongFixedConstPointer<std::string, mse::TRefCountingFixedPointer<A>> sfcptr1 = sfptr1;
				mse::TStrongFixedConstPointer<std::string, mse::TRefCountingPointer<A>> sfcptr2 = sfcptr1;
				if (sfcptr1 == sfptr1) {
					int q = 7;
				}
				if (sfptr1 == sfcptr1) {
					int q = 7;
				}
				if (sfptr1) {
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

#undef MSE_THROW

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSEREFCOUNTING_H_
