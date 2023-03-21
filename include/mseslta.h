
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESLTA_H_
#define MSESLTA_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_SLTAPOINTER_RUNTIME_CHECKS_ENABLED
#include "msenorad.h"
#endif // MSE_SLTAPOINTER_RUNTIME_CHECKS_ENABLED

#ifdef MSE_SELF_TESTS
#include <string>
#endif // MSE_SELF_TESTS

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-value"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
//#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("_NOEXCEPT")
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NODISCARD
#define _NODISCARD
#endif /*_NODISCARD*/


/* This macro roughly simulates constructor inheritance. */
#define MSE_SLTA_USING(Derived, Base) MSE_USING(Derived, Base)

#ifdef MSE_SLTAPOINTER_DISABLED
#define MSE_IF_NOT_SLTAPOINTER_DISABLED(x)
#else // MSE_SLTAPOINTER_DISABLED
#define MSE_IF_NOT_SLTAPOINTER_DISABLED(x) x
#endif // MSE_SLTAPOINTER_DISABLED


#define MSE_INHERIT_LTA_ASSIGNMENT_OPERATOR_FROM(base_class, this_class) \
	this_class& operator=(const this_class& _Right_cref) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
		base_class::operator=(_Right_cref); \
		return (*this); \
	} \
	this_class& operator=(this_class&& _Right_cref) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
		base_class::operator=(MSE_FWD(_Right_cref)); \
		return (*this); \
	}

namespace mse {
	namespace us {
		namespace impl {
			typedef ReferenceableByScopePointerTagBase ReferenceableBySLTAPointerTagBase;
			typedef ContainsNonOwningScopeReferenceTagBase ContainsNonOwningSLTAReferenceTagBase;
		}
	}

	namespace impl {
		class TSLTAID {};

		template<typename T>
		struct HasXSLTAReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xslta_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool value = (sizeof(Test<T>(0)) == sizeof(char));
		};

		/*
		template<typename T>
		struct HasXSLTANotReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xslta_not_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool value = (sizeof(Test<T>(0)) == sizeof(char));
		};
		*/

#if 0
		template<typename T>
		struct is_nonowning_slta_pointer : std::integral_constant<bool, ((std::is_base_of<mse::us::impl::XSLTAContainsNonOwningSLTAReferenceTagBase, T>::value
			&& std::is_base_of<mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, T>::value)
#ifdef MSE_SLTAPOINTER_DISABLED
			|| (std::is_pointer<T>::value && (!mse::impl::is_potentially_not_xslta<T>::value))
#endif /*MSE_SLTAPOINTER_DISABLED*/
			)> {};
#endif /*0*/
	}


	namespace rsv {

		template<typename _ncTy>
		class TXSLTAConstPointer;

		/* TXSLTAPointer is just a lifetime annotated wrapper for native pointers. */
		template<typename _Ty>
		class TXSLTAPointer : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
		public:
			TXSLTAPointer(_Ty* ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) : m_ptr(ptr) {}
			TXSLTAPointer(const TXSLTAPointer& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_ptr(src.m_ptr) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value || std::is_same<const _Ty2, _Ty>::value> MSE_IMPL_EIS >
			TXSLTAPointer(const TXSLTAPointer<_Ty2 >& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_ptr(src_cref.m_ptr) {}

			_Ty& operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(*m_ptr);
			}
			_Ty* operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(m_ptr);
			}
			TXSLTAPointer& operator=(_Ty* ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = ptr;
				return /*mse::return_value*/(*this);
			}
			TXSLTAPointer& operator=(const TXSLTAPointer& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = _Right_cref.m_ptr;
				return /*mse::return_value*/(*this);
			}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value || std::is_same<const _Ty2, _Ty>::value> MSE_IMPL_EIS >
			TXSLTAPointer& operator=(const TXSLTAPointer<_Ty2 >& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = _Right_cref.m_ptr;
				return /*mse::return_value*/(*this);
			}
			bool operator==(const TXSLTAPointer& _Right_cref) const { return (_Right_cref.m_ptr == m_ptr); }
#ifndef MSE_HAS_CXX20
			//bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
			bool operator!=(const _Ty* _Right_cref) const { return (!(m_ptr == _Right_cref)); }
			bool operator!=(const TXSLTAPointer& _Right_cref) const { return (!((*this) == _Right_cref.m_ptr)); }
#endif // !MSE_HAS_CXX20

			bool operator!() const { return (!m_ptr); }
			operator bool() const {
				return (m_ptr != nullptr);
			}

			explicit operator _Ty* () const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(m_ptr);
			}

		private:
			TXSLTAPointer(std::nullptr_t) {}

			_Ty* m_ptr MSE_ATTR_STR("mse::lifetime_label(99)");

			template<typename _ncTy>
			friend class TXSLTAConstPointer;
		} MSE_ATTR_STR("mse::lifetime_label(99)");

		/* TXSLTAPointer<T> does implicitly convert to TXSLTAPointer<const T>. But some may prefer to
		think of the pointer giving "const" access to the object rather than giving access to a "const object". */
		template<typename _ncTy>
		class TXSLTAConstPointer : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
		public:
			typedef const _ncTy _Ty;
			TXSLTAConstPointer(_Ty* ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) : m_ptr(ptr) {}
			TXSLTAConstPointer(const TXSLTAConstPointer& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_ptr(src.m_ptr) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value || std::is_same<const _Ty2, _Ty>::value> MSE_IMPL_EIS >
			TXSLTAConstPointer(const TXSLTAConstPointer<_Ty2 >& src_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_ptr(src_cref.m_ptr) {}
			TXSLTAConstPointer(const TXSLTAPointer<_ncTy>& src MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_ptr(src.m_ptr) {}

			_Ty& operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(*m_ptr);
			}
			_Ty* operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(m_ptr);
			}
			TXSLTAConstPointer& operator=(_Ty* ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = ptr;
				return /*mse::return_value*/(*this);
			}
			TXSLTAConstPointer& operator=(const TXSLTAConstPointer& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = _Right_cref.m_ptr;
				return /*mse::return_value*/(*this);
			}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2*, _Ty*>::value || std::is_same<const _Ty2, _Ty>::value> MSE_IMPL_EIS >
			TXSLTAConstPointer& operator=(const TXSLTAConstPointer<_Ty2 >& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				m_ptr = _Right_cref.m_ptr;
				return /*mse::return_value*/(*this);
			}
			bool operator==(const TXSLTAConstPointer& _Right_cref) const { return (_Right_cref.m_ptr == m_ptr); }
#ifndef MSE_HAS_CXX20
			//bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
			bool operator!=(const _Ty* _Right_cref) const { return (!(m_ptr == _Right_cref)); }
			bool operator!=(const TXSLTAConstPointer& _Right_cref) const { return (!((*this) == _Right_cref.m_ptr)); }
#endif // !MSE_HAS_CXX20

			bool operator!() const { return (!m_ptr); }
			operator bool() const {
				return (m_ptr != nullptr);
			}

			explicit operator _Ty* () const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(m_ptr);
			}

		private:
			TXSLTAConstPointer(std::nullptr_t) {}

			_Ty* m_ptr MSE_ATTR_STR("mse::lifetime_label(99)");
		} MSE_ATTR_STR("mse::lifetime_label(99)");


		template<typename _Ty> class TXSLTAOwnerPointer;
		template <class X, class... Args>
		TXSLTAOwnerPointer<X> make_xslta_owner(Args&&... args);
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::rsv::TXSLTAPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTAPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTAPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::rsv::TXSLTAConstPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTAConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTAConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {
	namespace rsv {
		/* TXSLTAOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
		TXSLTAOwnerPointer takes an rvalue expression of the object type. */
		template<typename _Ty>
		class TXSLTAOwnerPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase
			, mse::us::impl::ReferenceableBySLTAPointerTagBase
			, mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningSLTAReferenceTagBase, _Ty, TXSLTAOwnerPointer<_Ty> >
		{
		public:
			TXSLTAOwnerPointer(TXSLTAOwnerPointer&& src_ref) = default;
			TXSLTAOwnerPointer(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)")) : m_ptr(new _Ty(std::forward<_Ty>(src_ref))) {}

			_Ty& operator*() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return (*m_ptr);
			}
			_Ty&& operator*() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::move(*m_ptr);
			}
			_Ty* operator->() const& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(*m_ptr);
			}
			void operator->() const&& = delete;

			static TXSLTAOwnerPointer make(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }")
			{
				return TXSLTAOwnerPointer(std::forward<_Ty>(src_ref));
			}

		private:
			TXSLTAOwnerPointer(const TXSLTAOwnerPointer& src_cref) = delete;
			TXSLTAOwnerPointer& operator=(const TXSLTAOwnerPointer& _Right_cref) = delete;
			void* operator new(size_t size) { return ::operator new(size); }

			std::unique_ptr<_Ty > m_ptr = nullptr;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template <class X>
		TXSLTAOwnerPointer<X> make_xslta_owner(X&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(X, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_labels(alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }")
		{
			return TXSLTAOwnerPointer<X>::make(std::forward<X>(src_ref));
		}
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::rsv::TXSLTAOwnerPointer<_Ty> > {	// hash functor
		typedef mse::rsv::TXSLTAOwnerPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::rsv::TXSLTAOwnerPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {
	namespace rsv {
		;
	}
}

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("_NOEXCEPT")
#pragma pop_macro("MSE_THROW")
#pragma pop_macro("_NODISCARD")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

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

#endif // MSESLTA_H_
