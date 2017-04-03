
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPOINTERBASICS_H
#define MSEPOINTERBASICS_H

#include <assert.h>
#include <climits>       // ULONG_MAX
#include <limits>       // std::numeric_limits
#include <stdexcept>      // primitives_range_error
#include <memory>

/*compiler specific defines*/
#ifdef _MSC_VER
#if (1700 > _MSC_VER)
#define MSVC2010_COMPATIBLE 1
#endif /*(1700 > _MSC_VER)*/
#if (1900 > _MSC_VER)
#define MSVC2013_COMPATIBLE 1
#endif /*(1900 > _MSC_VER)*/
#if (2000 > _MSC_VER)
#define MSVC2015_COMPATIBLE 1
#endif /*(1900 > _MSC_VER)*/
#else /*_MSC_VER*/
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBLE 1
#if ((5 > __GNUC__) && (!defined(__clang__)))
#define GPP4P8_COMPATIBLE 1
#endif /*((5 > __GNUC__) && (!defined(__clang__)))*/
#endif /*(defined(__GNUC__) || defined(__GNUG__))*/
#endif /*_MSC_VER*/

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_POINTERBASICS_DISABLED
#define MSE_SAFERPTR_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#if defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR
#else // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR constexpr
#endif // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	class primitives_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#define MSE_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

	/* This macro roughly simulates constructor inheritance. Originally it was used when some compilers didn't support
	constructor inheritance, but now we use it because of it's differences with standard constructor inheritance. */
#define MSE_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

#ifdef MSE_SAFERPTR_DISABLED
	template<typename _Ty>
	using TSaferPtr = _Ty*;

	template<typename _Ty>
	using TSaferPtrForLegacy = _Ty*;
#else /*MSE_SAFERPTR_DISABLED*/

	class CSaferPtrBase {
	public:
		/* setToNull() needs to be available even when the smart pointer is const, because the object it points to may become
		invalid (deleted). */
		virtual void setToNull() const = 0;
	};

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_TSAFERPTR_CHECK_USE_BEFORE_SET
#define MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_TSAFERPTR_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

	/* TSaferPtr behaves similar to, and is largely compatible with, native pointers. It's a bit safer in that it initializes to
	nullptr by default and checks for attempted dereference of null pointers. */
	template<typename _Ty>
	class TSaferPtr : public CSaferPtrBase {
	public:
		TSaferPtr() : m_ptr(nullptr) {}
		TSaferPtr(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
		TSaferPtr(const TSaferPtr<_Ty>& src) : m_ptr(src.m_ptr) { note_value_assignment(); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TSaferPtr(const TSaferPtr<_Ty2>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
		virtual ~TSaferPtr() {}

		virtual void setToNull() const { m_ptr = nullptr; }

		void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
		_Ty* raw_pointer() const { return m_ptr; }
		_Ty* get() const { return m_ptr; }
		_Ty& operator*() const {
			assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTR_CHECKS
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtr"));
			}
#endif /*MSE_DISABLE_TSAFERPTR_CHECKS*/
			return (*m_ptr);
		}
		_Ty* operator->() const {
			assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTR_CHECKS
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtr"));
			}
#endif /*MSE_DISABLE_TSAFERPTR_CHECKS*/
			return m_ptr;
		}
		TSaferPtr<_Ty>& operator=(_Ty* ptr) {
			note_value_assignment();
			m_ptr = ptr;
			return (*this);
		}
		TSaferPtr<_Ty>& operator=(const TSaferPtr<_Ty>& _Right_cref) {
			note_value_assignment();
			m_ptr = _Right_cref.m_ptr;
			return (*this);
		}
		bool operator==(const _Ty* _Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }
		bool operator==(const TSaferPtr<_Ty> &_Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
		bool operator!=(const TSaferPtr<_Ty> &_Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }

		bool operator!() const { assert_initialized(); return (!m_ptr); }
		operator bool() const {
			assert_initialized();
			return (m_ptr != nullptr);
		}

		explicit operator _Ty*() const {
			assert_initialized();
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
		been deleted) even in cases when this smart pointer is const. */
		mutable _Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
	};

	/* TSaferPtrForLegacy is similar to TSaferPtr, but more readily converts to a native pointer implicitly. So when replacing
	native pointers with safer pointers in legacy code, fewer code changes (explicit casts) may be required when using this
	template. */
	template<typename _Ty>
	class TSaferPtrForLegacy : public CSaferPtrBase {
	public:
		TSaferPtrForLegacy() : m_ptr(nullptr) {}
		TSaferPtrForLegacy(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TSaferPtrForLegacy(const TSaferPtrForLegacy<_Ty2>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
		virtual ~TSaferPtrForLegacy() {}

		virtual void setToNull() const { m_ptr = nullptr; }

		void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
		_Ty* raw_pointer() const { return m_ptr; }
		_Ty* get() const { return m_ptr; }
		_Ty& operator*() const {
			assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtrForLegacy"));
			}
#endif /*MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS*/
			return (*m_ptr);
		}
		_Ty* operator->() const {
			assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtrForLegacy"));
			}
#endif /*MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS*/
			return m_ptr;
		}
		TSaferPtrForLegacy<_Ty>& operator=(_Ty* ptr) {
			note_value_assignment();
			m_ptr = ptr;
			return (*this);
		}
		//operator bool() const { return m_ptr; }

		operator _Ty*() const {
			assert_initialized();
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
		been deleted) even in cases when this smart pointer is const. */
		mutable _Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
	};
#endif /*MSE_SAFERPTR_DISABLED*/

	template<typename _Ty>
	class TPointerID {};

	/* TPointer is just a wrapper for native pointers that can act as a base class. */
	template<typename _Ty, typename _TID = TPointerID<_Ty>>
	class TPointer {
	public:
		TPointer() : m_ptr(nullptr) {}
		TPointer(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
		TPointer(const TPointer<_Ty, _TID>& src) : m_ptr(src.m_ptr) { note_value_assignment(); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TPointer(const TPointer<_Ty2, _TID>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
		virtual ~TPointer() {}

		void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
		_Ty* raw_pointer() const { return m_ptr; }
		_Ty* get() const { return m_ptr; }
		_Ty& operator*() const {
			assert_initialized();
#ifndef NDEBUG
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointer"));
			}
#endif // !NDEBUG
			return (*m_ptr);
		}
		_Ty* operator->() const {
			assert_initialized();
#ifndef NDEBUG
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointer"));
			}
#endif // !NDEBUG
			return m_ptr;
		}
		TPointer<_Ty, _TID>& operator=(_Ty* ptr) {
			note_value_assignment();
			m_ptr = ptr;
			return (*this);
		}
		TPointer<_Ty, _TID>& operator=(const TPointer<_Ty, _TID>& _Right_cref) {
			note_value_assignment();
			m_ptr = _Right_cref.m_ptr;
			return (*this);
		}
		bool operator==(const _Ty* _Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }
		bool operator==(const TPointer<_Ty, _TID> &_Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
		bool operator!=(const TPointer<_Ty, _TID> &_Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }

		bool operator!() const { assert_initialized(); return (!m_ptr); }
		operator bool() const {
			assert_initialized();
			return (m_ptr != nullptr);
		}

		explicit operator _Ty*() const {
			assert_initialized();
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		_Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
	};

	template<typename _Ty, typename _TID = TPointerID<_Ty>>
	class TPointerForLegacy {
	public:
		TPointerForLegacy() : m_ptr(nullptr) {}
		TPointerForLegacy(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
		TPointerForLegacy(const TPointerForLegacy<_Ty2, _TID>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
		virtual ~TPointerForLegacy() {}

		void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
		_Ty* raw_pointer() const { return m_ptr; }
		_Ty* get() const { return m_ptr; }
		_Ty& operator*() const {
			assert_initialized();
#ifndef NDEBUG
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointerForLegacy"));
			}
#endif // !NDEBUG
			return (*m_ptr);
		}
		_Ty* operator->() const {
			assert_initialized();
#ifndef NDEBUG
			if (nullptr == m_ptr) {
				MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointerForLegacy"));
			}
#endif // !NDEBUG
			return m_ptr;
		}
		TPointerForLegacy<_Ty, _TID>& operator=(_Ty* ptr) {
			note_value_assignment();
			m_ptr = ptr;
			return (*this);
		}

		operator _Ty*() const {
			assert_initialized();
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		_Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
	};


	template <class _TTargetType, class _TLeasePointerType> class TSyncWeakFixedConstPointer;

	/* If, for example, you want a safe pointer to a member of a registered pointer target, you can use a
	TSyncWeakFixedPointer to store a copy of the registered pointer along with the pointer targeting the
	member. */
	template <class _TTargetType, class _TLeasePointerType>
	class TSyncWeakFixedPointer {
	public:
		TSyncWeakFixedPointer(const TSyncWeakFixedPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TSyncWeakFixedPointer(const TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		_TTargetType& operator*() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		_TTargetType* operator->() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TSyncWeakFixedPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;
		bool operator!=(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;

		bool operator!() const { return (!m_target_pointer); }
		operator bool() const {
			return (m_target_pointer != nullptr);
		}

		explicit operator _TTargetType*() const {
			if (nullptr == m_target_pointer) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_target_pointer;
		}
		_TLeasePointerType lease_pointer() const { return (*this).m_lease_pointer; }

		template <class _TTargetType2, class _TLeasePointerType2>
		static TSyncWeakFixedPointer make(_TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TSyncWeakFixedPointer(target, lease_pointer);
		}

	private:
		TSyncWeakFixedPointer(_TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TSyncWeakFixedPointer& operator=(const TSyncWeakFixedPointer& _Right_cref) = delete;

		_TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
		friend class TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType>;
	};

	template <class _TTargetType, class _TLeasePointerType>
	TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType> make_syncweak(_TTargetType& target, const _TLeasePointerType& lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::make(target, lease_pointer);
	}

	template <class _TTargetType, class _TLeasePointerType>
	class TSyncWeakFixedConstPointer {
	public:
		TSyncWeakFixedConstPointer(const TSyncWeakFixedConstPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TSyncWeakFixedConstPointer(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		TSyncWeakFixedConstPointer(const TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>&src) : m_target_pointer(src.m_target_pointer), m_lease_pointer(src.m_lease_pointer) {}
		const _TTargetType& operator*() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		const _TTargetType* operator->() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedConstPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TSyncWeakFixedConstPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }

		bool operator!() const { return (!m_target_pointer); }
		operator bool() const {
			return (m_target_pointer != nullptr);
		}

		explicit operator const _TTargetType*() const {
			if (nullptr == m_target_pointer) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_target_pointer;
		}
		_TLeasePointerType lease_pointer() const { return (*this).m_lease_pointer; }

		template <class _TTargetType2, class _TLeasePointerType2>
		static TSyncWeakFixedConstPointer make(const _TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TSyncWeakFixedConstPointer(target, lease_pointer);
		}

	private:
		TSyncWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TSyncWeakFixedConstPointer& operator=(const TSyncWeakFixedConstPointer& _Right_cref) = delete;

		const _TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
	};

	template <class _TTargetType, class _TLeasePointerType>
	bool TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator==(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (_Right_cref == m_target_pointer); }
	template <class _TTargetType, class _TLeasePointerType>
	bool TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator!=(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (!((*this) == _Right_cref)); }

	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, _Ty> make_pointer_to_member(_TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedConstPointer<_TTargetType, _Ty> make_pointer_to_member(const _TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedConstPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedConstPointer<_TTargetType, _Ty> make_const_pointer_to_member(const _TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedConstPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}
}

#undef MSE_THROW

#endif /*ndef MSEPOINTERBASICS_H*/
