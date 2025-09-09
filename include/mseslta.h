
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


#if defined(__clang__)
#define MSE_INHERIT_LTA_ASSIGNMENT_OPERATOR_FROM(base_class, this_class, lifetime_labels) \
	this_class& operator=(const this_class& _Right_cref  __attribute__((annotate("mse::lifetime_labels(_[" lifetime_labels "])")))) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
		base_class::operator=(_Right_cref); \
		return (*this); \
	} \
	this_class& operator=(this_class&& _Right_cref  __attribute__((annotate("mse::lifetime_labels(_[" lifetime_labels "])")))) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
		base_class::operator=(MSE_FWD(_Right_cref)); \
		return (*this); \
	}
#else // defined(__clang__)
#define MSE_INHERIT_LTA_ASSIGNMENT_OPERATOR_FROM(base_class, this_class, lifetime_labels) \
this_class& operator=(const this_class& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(" lifetime_labels ")")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
		base_class::operator=(_Right_cref); \
		return (*this); \
} \
this_class& operator=(this_class&& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(" lifetime_labels ")")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(mse_ilaof_42); this(mse_ilaof_42); return_value(mse_ilaof_42) }") { \
			base_class::operator=(MSE_FWD(_Right_cref)); \
			return (*this); \
	}
#endif // defined(__clang__)

#ifndef MSE_IMPL_DISABLE_LT_SHORT_MACROS
/* provisional */
#define LT(...) MSE_ATTR_STR("mse::lifetime_labels("#__VA_ARGS__")")
#define LTP(...) MSE_ATTR_PARAM_STR("mse::lifetime_labels("#__VA_ARGS__")")
#define LT_RETVAL(...) MSE_ATTR_STR("mse::lifetime_return_value("#__VA_ARGS__")")
#define LT_THIS(...) MSE_ATTR_STR("mse::lifetime_this("#__VA_ARGS__")")
#define LT_ENCOMPASSES(x, y) MSE_ATTR_STR("mse::lifetime_encompasses("#x", "#y")")
#define LT_FIRST_CAN_BE_ASSIGNED_TO_SECOND(x, y) MSE_ATTR_STR("mse::first_can_be_assigned_to_second("#x", "#y")")
#define LT_FROM_TEMPLATE_PARAM_BY_NAME(x, y) MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name("#x", "#y")")
#endif // !MSE_IMPL_DISABLE_LT_SHORT_MACROS

namespace mse {
	namespace us {
		namespace impl {
			class ReferenceableBySLTAPointerTagBase : public ReferenceableByScopePointerTagBase {};
			class ContainsNonOwningSLTAReferenceTagBase : public ContainsNonOwningScopeReferenceTagBase {};
			class XSLTAContainsNonOwningSLTAReferenceTagBase : public ContainsNonOwningSLTAReferenceTagBase, public XSLTATagBase {};
		}
	}

	namespace rsv {
		namespace impl {

			template<typename _Ty> using is_potentially_xslta = mse::impl::is_potentially_xscope<_Ty>;
			template<typename _Ty> using is_potentially_not_xslta = mse::impl::is_potentially_not_xscope<_Ty>;
			template<typename _Ty> struct is_xslta : mse::impl::negation<is_potentially_not_xslta<_Ty> > {};
			/* The purpose of these template functions are just to produce a compile error on attempts to instantiate
			when certain conditions are not met. */
			template<class _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::rsv::impl::is_potentially_not_xslta<_Ty>::value)> MSE_IMPL_EIS >
			void T_valid_if_not_an_xslta_type() {}
			template<class _Ty>
			void T_valid_if_not_an_xslta_type(const _Ty&) {
				T_valid_if_not_an_xslta_type<_Ty>();
			}


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

			template<typename T>
			struct is_nonowning_slta_pointer : std::integral_constant<bool, ((std::is_base_of<mse::us::impl::XSLTATagBase, T>::value
				&& std::is_base_of<mse::us::impl::ContainsNonOwningSLTAReferenceTagBase, T>::value&& std::is_base_of<mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, T>::value)
#ifdef MSE_SLTAPOINTER_DISABLED
				|| (std::is_pointer<T>::value && (!mse::impl::is_potentially_not_xslta<T>::value))
#endif /*MSE_SLTAPOINTER_DISABLED*/
				)> {};
		}

#ifdef MSE_SLTAPOINTER_DISABLED

		/* By default we make xslta pointers simply an alias for native pointers in non-debug builds. */
		template<typename _ncTy> using  TXSLTAConstPointer = _ncTy const*;
		template<typename _Ty> using TXSLTAPointer = _Ty*;

#else // MSE_SLTAPOINTER_DISABLED
		template<typename _Ty> class TXSLTAPointer;
		template<typename _Ty> class TXSLTAConstPointer;
#endif // MSE_SLTAPOINTER_DISABLED

		namespace impl {
			template<typename TPtr, typename TTarget>
			struct is_convertible_to_nonowning_slta_pointer_helper1 : std::integral_constant<bool,
				std::is_convertible<TPtr, mse::rsv::TXSLTAPointer<TTarget>>::value || std::is_convertible<TPtr, mse::rsv::TXSLTAConstPointer<TTarget>>::value> {};
			template<typename TPtr>
			struct is_convertible_to_nonowning_slta_pointer : is_convertible_to_nonowning_slta_pointer_helper1<TPtr
				, mse::impl::remove_reference_t<decltype(*std::declval<TPtr>())> > {};

			template<typename TPtr, typename TTarget>
			struct is_convertible_to_nonowning_slta_or_indeterminate_pointer_helper1 : std::integral_constant<bool,
				is_convertible_to_nonowning_slta_pointer<TPtr>::value
			> {};
			template<typename TPtr>
			struct is_convertible_to_nonowning_slta_or_indeterminate_pointer : is_convertible_to_nonowning_slta_or_indeterminate_pointer_helper1
				<TPtr, mse::impl::remove_reference_t<decltype(*std::declval<TPtr>())> > {};
		}
	}


	namespace rsv {

#ifdef MSE_SLTAPOINTER_DISABLED
		/* By default we make xslta pointers simply an alias for native pointers in non-debug builds. */
#else // MSE_SLTAPOINTER_DISABLED

		template<typename _ncTy>
		class TXSLTAConstPointer;

		/* TXSLTAPointer is just a lifetime annotated wrapper for native pointers. */
		template<typename _Ty>
		class TXSLTAPointer : public mse::us::impl::XSLTAContainsNonOwningSLTAReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
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
			TXSLTAPointer& operator=(const TXSLTAPointer& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
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
			explicit operator bool() const {
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
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTAPointer);
	namespace rsv {

		template<typename _Ty>
		using xl_ptr = TXSLTAPointer< _Ty>; /* provisional shorter alias */

		/* TXSLTAPointer<T> does implicitly convert to TXSLTAPointer<const T>. But some may prefer to
		think of the pointer giving "const" access to the object rather than giving access to a "const object". */
		template<typename _ncTy>
		class TXSLTAConstPointer : public mse::us::impl::XSLTAContainsNonOwningSLTAReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
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
			TXSLTAConstPointer& operator=(const TXSLTAConstPointer& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
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
			explicit operator bool() const {
				return (m_ptr != nullptr);
			}

			explicit operator _Ty* () const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
				return /*mse::return_value*/(m_ptr);
			}

		private:
			TXSLTAConstPointer(std::nullptr_t) {}

			_Ty* m_ptr MSE_ATTR_STR("mse::lifetime_label(99)");
		} MSE_ATTR_STR("mse::lifetime_label(99)");

		template<typename _Ty>
		using xl_const_ptr = TXSLTAConstPointer< _Ty>; /* provisional shorter alias */

#endif // MSE_SLTAPOINTER_DISABLED

		template<typename _Ty>
		auto xslta_ptr_to(_Ty&& _X) {
			return TXSLTAPointer<_Ty>(std::addressof(_X));
		}
		template<typename _Ty>
		auto xslta_ptr_to(_Ty& _X) {
			return TXSLTAPointer<_Ty>(std::addressof(_X));
		}
		template<typename _Ty>
		auto xslta_ptr_to(const _Ty& _X) {
			return TXSLTAConstPointer<_Ty>(std::addressof(_X));
		}


		template<typename _Ty> class TXSLTAOwnerPointer;
		template <class X, class... Args>
		TXSLTAOwnerPointer<X> make_xslta_owner(Args&&... args);
	}
}

namespace std {
#if !defined(MSE_SLTAPOINTER_DISABLED)
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
#endif // !defined(MSE_SLTAPOINTER_DISABLED)
}

namespace mse {
	namespace rsv {
		/* TXSLTAOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
		TXSLTAOwnerPointer takes an rvalue expression of the object type. */
		template<typename _Ty>
		class TXSLTAOwnerPointer : public mse::us::impl::XSLTATagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase
			, mse::us::impl::ReferenceableBySLTAPointerTagBase
			, mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty, TXSLTAOwnerPointer<_Ty> >
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
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::rsv::TXSLTAOwnerPointer);
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
		namespace us {
			/* (Unsafely) obtain a slta pointer to any object. */
			template<typename _Ty>
			TXSLTAPointer<_Ty> unsafe_make_xslta_pointer_to(_Ty& ref) {
				return TXSLTAPointer<_Ty>(std::addressof(ref));
			}
			template<typename _Ty>
			TXSLTAConstPointer<_Ty> unsafe_make_xslta_const_pointer_to(const _Ty& cref) {
				return TXSLTAConstPointer<_Ty>(std::addressof(cref));
			}
			template<typename _Ty>
			TXSLTAConstPointer<_Ty> unsafe_make_xslta_pointer_to(const _Ty& cref) {
				return unsafe_make_xslta_const_pointer_to(cref);
			}
		}

		/* Obtain a slta pointer to any object. Requires static verification. */
		template<typename _Ty>
		TXSLTAPointer<_Ty> make_xslta_pointer_to(_Ty& ref) {
			return mse::rsv::us::unsafe_make_xslta_pointer_to(ref);
		}
		template<typename _Ty>
		TXSLTAConstPointer<_Ty> make_xslta_const_pointer_to(const _Ty& cref) {
			return mse::rsv::us::unsafe_make_xslta_const_pointer_to(cref);
		}
		template<typename _Ty>
		TXSLTAConstPointer<_Ty> make_xslta_pointer_to(const _Ty& cref) {
			return make_xslta_const_pointer_to(cref);
		}
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
