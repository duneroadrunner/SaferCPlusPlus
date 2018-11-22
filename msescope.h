
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

#include "msepointerbasics.h"
#include "mseprimitives.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#include "msenorad.h"
#include "mseany.h"
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-value"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif /*__GNUC__*/
#endif /*__clang__*/

/* Note that by default, MSE_SCOPEPOINTER_DISABLED is defined in non-debug builds. This is enacted in "msepointerbasics.h". */

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


namespace mse {

	/* This macro roughly simulates constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) MSE_USING(Derived, Base)

	namespace impl {
		template<typename _Ty> class TScopeID {};
	}

	namespace us {
		namespace impl {
			/* moved to msepointerbasics.h */
			//class XScopeTagBase { public: void xscope_tag() const {} };

			/* Note that objects not derived from ReferenceableByScopePointerTagBase might still be targeted by a scope pointer via
			make_pointer_to_member_v2(). */
			class ReferenceableByScopePointerTagBase {};

			class ContainsNonOwningScopeReferenceTagBase {};
			class XScopeContainsNonOwningScopeReferenceTagBase : public ContainsNonOwningScopeReferenceTagBase, public XScopeTagBase {};
		}
	}

	namespace impl {
		template<typename _Ty>
		class TPlaceHolder_msescope {};
		template<typename _Ty>
		class TPlaceHolder2_msescope {};

		template<typename T>
		struct HasXScopeReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xscope_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
		};

		/*
		template<typename T>
		struct HasXScopeNotReturnableTagMethod
		{
			template<typename U, void(U::*)() const> struct SFINAE {};
			template<typename U> static char Test(SFINAE<U, &U::xscope_not_returnable_tag>*);
			template<typename U> static int Test(...);
			static const bool Has = (sizeof(Test<T>(0)) == sizeof(char));
		};
		*/

		template<class T, class EqualTo>
		struct IsNonOwningScopePointer_impl
		{
			static const bool value = (std::is_base_of<mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, T>::value
				&& std::is_base_of<mse::us::impl::StrongPointerNotAsyncShareableTagBase, T>::value);
			using type = std::integral_constant<bool, value>;
		};
		template<class T, class EqualTo = T>
		struct IsNonOwningScopePointer : IsNonOwningScopePointer_impl<
			typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

		/* determines if a given type is an instantiation of a given template */
		template<typename T, template<typename> class TT>
		struct is_instantiation_of_msescope : std::false_type { };
		template<typename T, template<typename> class TT>
		struct is_instantiation_of_msescope<TT<T>, TT> : std::true_type { };

		template <class _Ty, class _Ty2, class = typename std::enable_if<
			(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
			, void>::type>
		static void valid_if_not_rvalue_reference_of_given_type(_Ty2 src) {}
	}

#ifdef MSE_SCOPEPOINTER_DISABLED
	//mse::impl::TScopeID
	template<typename _Ty> using TXScopePointer = _Ty * ;
	template<typename _Ty> using TXScopeConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeNotNullPointer = _Ty * ;
	template<typename _Ty> using TXScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																	   library containers don't support const elements. */
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty* /*const*/;

	namespace us {
		namespace impl {
			template<typename _TROy> using TXScopeObjBase = _TROy;
		}
	}
	template<typename _TROy> using TXScopeObj = _TROy;
	template<typename _Ty> using TXScopeItemFixedPointer = _Ty * /*const*/;
	template<typename _Ty> using TXScopeItemFixedConstPointer = const _Ty* /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedPointerToRValue = _Ty * /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedConstPointerToRValue = const _Ty* /*const*/;
	//template<typename _TROy> using TXScopeReturnValue = _TROy;

	template<typename _TROy> class TXScopeOwnerPointer;

	template<typename _Ty> auto xscope_ifptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto xscope_ifptr_to(const _Ty& _X) { return std::addressof(_X); }

	//template<typename _Ty> const _Ty& return_value(const _Ty& _X) { return _X; }
	//template<typename _Ty> _Ty&& return_value(_Ty&& _X) { return std::forward<decltype(_X)>(_X); }
	template<typename _TROy> using TNonXScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

	namespace us {
		namespace impl {

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

			template<typename _TROz> using TXScopeObjBase = mse::TWNoradObj<_TROz>;
			template<typename _Ty> using TXScopePointerBase = mse::us::impl::TAnyPointerBase<_Ty>;
			template<typename _Ty> using TXScopeConstPointerBase = mse::us::impl::TAnyConstPointerBase<_Ty>;
			template<typename _Ty> using Tscope_obj_base_ptr = mse::TWNoradFixedPointer<_Ty>;
			template<typename _Ty> using Tscope_obj_base_const_ptr = mse::TWNoradConstPointer<_Ty>;

#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

			template<typename _TROz>
			class TXScopeObjBase : public _TROz {
			public:
				MSE_SCOPE_USING(TXScopeObjBase, _TROz);
				TXScopeObjBase(const TXScopeObjBase& _X) : _TROz(_X) {}
				TXScopeObjBase(TXScopeObjBase&& _X) : _TROz(std::forward<decltype(_X)>(_X)) {}

				TXScopeObjBase& operator=(TXScopeObjBase&& _X) { _TROz::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
				TXScopeObjBase& operator=(const TXScopeObjBase& _X) { _TROz::operator=(_X); return (*this); }
				template<class _Ty2>
				TXScopeObjBase& operator=(_Ty2&& _X) { _TROz::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
				template<class _Ty2>
				TXScopeObjBase& operator=(const _Ty2& _X) { _TROz::operator=(_X); return (*this); }

				auto operator&() {
					return this;
				}
				auto operator&() const {
					return this;
				}
			};

			template<typename _Ty> using TXScopePointerBase = mse::us::impl::TPointerForLegacy<_Ty, mse::impl::TScopeID<const _Ty>>;
			template<typename _Ty> using TXScopeConstPointerBase = mse::us::impl::TPointerForLegacy<const _Ty, mse::impl::TScopeID<const _Ty>>;
			template<typename _Ty> using Tscope_obj_base_ptr = TXScopeObjBase<_Ty>*;
			template<typename _Ty> using Tscope_obj_base_const_ptr = TXScopeObjBase<_Ty> const*;

#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

		}
	}

	template<typename _Ty> class TXScopeObj;
	template<typename _Ty> class TXScopeNotNullPointer;
	template<typename _Ty> class TXScopeNotNullConstPointer;
	template<typename _Ty> class TXScopeFixedPointer;
	template<typename _Ty> class TXScopeFixedConstPointer;
	template<typename _Ty> class TXScopeOwnerPointer;

	template<typename _Ty> class TXScopeItemFixedPointer;
	template<typename _Ty> class TXScopeItemFixedConstPointer;
	template<typename _Ty> class TXScopeCagedItemFixedPointerToRValue;
	template<typename _Ty> class TXScopeCagedItemFixedConstPointerToRValue;
	namespace rsv {
		template<typename _Ty> class TXScopeItemFixedPointerFParam;
		template<typename _Ty> class TXScopeItemFixedConstPointerFParam;
	}

	namespace us {
		namespace impl {
			template <typename _Ty, typename _TConstPointer1> class TCommonizedPointer;
			template <typename _Ty, typename _TConstPointer1> class TCommonizedConstPointer;
		}
	}

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopePointer : public mse::us::impl::TXScopePointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		typedef mse::us::impl::Tscope_obj_base_ptr<_Ty> scope_obj_base_ptr_t;
		virtual ~TXScopePointer() {}
	private:
		TXScopePointer() : mse::us::impl::TXScopePointerBase<_Ty>() {}
		TXScopePointer(scope_obj_base_ptr_t ptr) : mse::us::impl::TXScopePointerBase<_Ty>(ptr) {}
		TXScopePointer(const TXScopePointer& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(
			static_cast<const mse::us::impl::TXScopePointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePointer(const TXScopePointer<_Ty2>& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(mse::us::impl::TXScopePointerBase<_Ty2>(src_cref)) {}
		TXScopePointer<_Ty>& operator=(TXScopeObj<_Ty>* ptr) {
			return mse::us::impl::TXScopePointerBase<_Ty>::operator=(ptr);
		}
		TXScopePointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			return mse::us::impl::TXScopePointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (bool(*static_cast<const mse::us::impl::TXScopePointerBase<_Ty>*>(this)));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
			_Ty* retval = std::addressof(*(*this))/*(*static_cast<const mse::us::impl::TXScopePointerBase<_Ty>*>(this))*/;
			return retval;
		}
		explicit operator TXScopeObj<_Ty>*() const {
			TXScopeObj<_Ty>* retval = (*static_cast<const mse::us::impl::TXScopePointerBase<_Ty>*>(this));
			return retval;
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeNotNullPointer<_Ty>;
		friend class us::impl::TCommonizedPointer<_Ty, TXScopePointer<_Ty> >;
		friend class us::impl::TCommonizedConstPointer<const _Ty, TXScopePointer<_Ty> >;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeConstPointer : public mse::us::impl::TXScopeConstPointerBase<const _Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		typedef mse::us::impl::Tscope_obj_base_const_ptr<_Ty> scope_obj_base_const_ptr_t;
		virtual ~TXScopeConstPointer() {}
	private:
		TXScopeConstPointer() : mse::us::impl::TXScopeConstPointerBase<const _Ty>() {}
		TXScopeConstPointer(scope_obj_base_const_ptr_t ptr) : mse::us::impl::TXScopeConstPointerBase<const _Ty>(ptr) {}
		TXScopeConstPointer(const TXScopeConstPointer& src_cref) : mse::us::impl::TXScopeConstPointerBase<const _Ty>(static_cast<const mse::us::impl::TXScopeConstPointerBase<const _Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopeConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<const _Ty>(src_cref) {}
		TXScopeConstPointer(const TXScopePointer<_Ty>& src_cref) : mse::us::impl::TXScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopePointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<const _Ty>(mse::us::impl::TXScopeConstPointerBase<_Ty2>(src_cref)) {}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeObj<_Ty>* ptr) {
			return mse::us::impl::TXScopeConstPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeConstPointer<_Ty>& _Right_cref) {
			return mse::us::impl::TXScopeConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (bool(*static_cast<const mse::us::impl::TXScopeConstPointerBase<const _Ty>*>(this)));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const mse::us::impl::TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}
		explicit operator const TXScopeObj<_Ty>*() const {
			const TXScopeObj<_Ty>* retval = (*static_cast<const mse::us::impl::TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeNotNullConstPointer<_Ty>;
		friend class us::impl::TCommonizedConstPointer<const _Ty, TXScopeConstPointer<_Ty> >;
	};

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullPointer : public TXScopePointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		virtual ~TXScopeNotNullPointer() {}
	private:
		TXScopeNotNullPointer(typename TXScopePointer<_Ty>::scope_obj_base_ptr_t src_cref) : TXScopePointer<_Ty>(src_cref) {}
		TXScopeNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopePointer<_Ty>(ptr) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopePointer<_Ty>(src_cref) {}
		TXScopeNotNullPointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			TXScopePointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		operator bool() const { return (*static_cast<const TXScopePointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopePointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopePointer<_Ty>::operator TXScopeObj<_Ty>*(); }

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeFixedPointer<_Ty>;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullConstPointer : public TXScopeConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		virtual ~TXScopeNotNullConstPointer() {}
	private:
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		operator bool() const { return (*static_cast<const TXScopeConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		TXScopeNotNullConstPointer(typename TXScopeConstPointer<_Ty>::scope_obj_base_const_ptr_t ptr) : TXScopeConstPointer<_Ty>(ptr) {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeFixedConstPointer<_Ty>;
	};

	/* A TXScopeFixedPointer points to a TXScopeObj. Its intended for very limited use. Basically just to pass a TXScopeObj
	by reference as a function parameter. TXScopeFixedPointers can be obtained from TXScopeObj's "&" (address of) operator. */
	template<typename _Ty>
	class TXScopeFixedPointer : public TXScopeNotNullPointer<_Ty> {
	public:
		TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		virtual ~TXScopeFixedPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopeNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopeNotNullPointer<_Ty>::operator TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeFixedPointer(typename TXScopePointer<_Ty>::scope_obj_base_ptr_t ptr) : TXScopeNotNullPointer<_Ty>(ptr) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		/* Disabling public move construction prevents some unsafe operations, like some, but not all,
		attempts to use a TXScopeFixedPointer<> as a return value. But it also prevents some safe
		operations too, like initialization via '=' (assignment operator). And the set of prevented
		operations might not be consistent across compilers. We're deciding here that the minor safety
		benefits outweigh the minor inconveniences. Note that we can't disable public move construction
		in the other scope pointers as it would interfere with implicit conversions. */
		TXScopeFixedPointer(TXScopeFixedPointer&& src_ref) : TXScopeNotNullPointer<_Ty>(src_ref) {
			int q = 5;
		}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		TXScopeFixedPointer<_Ty>& operator=(const TXScopeFixedPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObj<_Ty>;
	};

	template<typename _Ty>
	class TXScopeFixedConstPointer : public TXScopeNotNullConstPointer<_Ty> {
	public:
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		virtual ~TXScopeFixedConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeNotNullConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeFixedConstPointer(typename TXScopeConstPointer<_Ty>::scope_obj_base_const_ptr_t ptr) : TXScopeNotNullConstPointer<_Ty>(ptr) {}
		TXScopeFixedConstPointer<_Ty>& operator=(const TXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObj<_Ty>;
	};

	/* TXScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
	that are either allocated on the stack, or whose "owning" pointer is allocated on the stack. Unfortunately it's not
	really possible to completely prevent misuse. For example, std::list<TXScopeObj<mse::CInt>> is an improper, and
	dangerous, use of TXScopeObj<>. So we provide the option of using an mse::us::TFLRegisteredObj as TXScopeObj's base
	class to enforce safety and to help catch misuse. Defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause
	mse::us::TFLRegisteredObj to be used in non-debug modes as well. */
	template<typename _TROy>
	class TXScopeObj : public mse::us::impl::TXScopeObjBase<_TROy>
		, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _TROy>::value, mse::impl::TPlaceHolder_msescope<TXScopeObj<_TROy> >, mse::us::impl::XScopeTagBase>::type
		, public std::conditional<std::is_base_of<mse::us::impl::ReferenceableByScopePointerTagBase, _TROy>::value, mse::impl::TPlaceHolder2_msescope<TXScopeObj<_TROy> >, mse::us::impl::ReferenceableByScopePointerTagBase>::type
	{
	public:
		TXScopeObj(const TXScopeObj& _X) : mse::us::impl::TXScopeObjBase<_TROy>(_X) {}

#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		explicit TXScopeObj(TXScopeObj&& _X) : mse::us::impl::TXScopeObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		MSE_SCOPE_USING(TXScopeObj, mse::us::impl::TXScopeObjBase<_TROy>);
		virtual ~TXScopeObj() {}

		TXScopeObj& operator=(TXScopeObj&& _X) {
			mse::impl::valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
			mse::us::impl::TXScopeObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		TXScopeObj& operator=(const TXScopeObj& _X) { mse::us::impl::TXScopeObjBase<_TROy>::operator=(_X); return (*this); }
		template<class _Ty2>
		TXScopeObj& operator=(_Ty2&& _X) {
			mse::us::impl::TXScopeObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		template<class _Ty2>
		TXScopeObj& operator=(const _Ty2& _X) { mse::us::impl::TXScopeObjBase<_TROy>::operator=(_X); return (*this); }

		const TXScopeFixedPointer<_TROy> operator&() & {
			return &(*static_cast<mse::us::impl::TXScopeObjBase<_TROy>*>(this));
		}
		const TXScopeFixedConstPointer<_TROy> operator&() const & {
			return &(*static_cast<const mse::us::impl::TXScopeObjBase<_TROy>*>(this));
		}
		const TXScopeItemFixedPointer<_TROy> mse_xscope_ifptr() & { return &(*this); }
		const TXScopeItemFixedConstPointer<_TROy> mse_xscope_ifptr() const & { return &(*this); }

		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() && {
			return TXScopeItemFixedConstPointer<_TROy>(TXScopeFixedPointer<_TROy>(&(*static_cast<mse::us::impl::TXScopeObjBase<_TROy>*>(this))));
		}
		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() const && {
			return TXScopeFixedConstPointer<_TROy>(TXScopeConstPointer<_TROy>(&(*static_cast<const mse::us::impl::TXScopeObjBase<_TROy>*>(this))));
		}
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() && { return &(*this); }
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() const && { return &(*this); }

		void xscope_tag() const {}
		//void xscope_contains_accessible_scope_address_of_operator_tag() const {}
		/* This type can be safely used as a function return value if _Ty is also safely returnable. */
		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		void* operator new(size_t size) { return ::operator new(size); }

		friend class TXScopeOwnerPointer<_TROy>;
	};

	template<typename _Ty>
	auto xscope_ifptr_to(_Ty&& _X) {
		return _X.mse_xscope_ifptr();
	}
	template<typename _Ty>
	auto xscope_ifptr_to(const _Ty& _X) {
		return _X.mse_xscope_ifptr();
	}


	template <class _TTargetType, class _TLeaseType> class TXScopeStrongFixedConstPointer;

	template <class _TTargetType, class _TLeaseType>
	class TXScopeStrongFixedPointer : public TStrongFixedPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
	public:
		typedef TStrongFixedPointer<_TTargetType, _TLeaseType> base_class;

		TXScopeStrongFixedPointer(const TXScopeStrongFixedPointer&) = default;
		template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
		TXScopeStrongFixedPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

		template <class _TTargetType2, class _TLeaseType2>
		static TXScopeStrongFixedPointer make(_TTargetType2& target, const _TLeaseType2& lease) {
			return base_class::make(target, lease);
		}
		template <class _TTargetType2, class _TLeaseType2>
		static TXScopeStrongFixedPointer make(_TTargetType2& target, _TLeaseType2&& lease) {
			return base_class::make(target, std::forward<decltype(lease)>(lease));
		}

	protected:
		TXScopeStrongFixedPointer(_TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
			: base_class(target, lease) {}
		TXScopeStrongFixedPointer(_TTargetType& target/* often a struct member */, _TLeaseType&& lease)
			: base_class(target, std::forward<decltype(lease)>(lease)) {}
	private:
		TXScopeStrongFixedPointer(const base_class& src_cref) : base_class(src_cref) {}
		
		TXScopeStrongFixedPointer & operator=(const TXScopeStrongFixedPointer& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType>;
	};

	template <class _TTargetType, class _TLeaseType>
	TXScopeStrongFixedPointer<_TTargetType, _TLeaseType> make_xscope_strong(_TTargetType& target, const _TLeaseType& lease) {
		return TXScopeStrongFixedPointer<_TTargetType, _TLeaseType>::make(target, lease);
	}
	template <class _TTargetType, class _TLeaseType>
	auto make_xscope_strong(_TTargetType& target, _TLeaseType&& lease) -> TXScopeStrongFixedPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type> {
		return TXScopeStrongFixedPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type>::make(target, std::forward<decltype(lease)>(lease));
	}


	namespace us {
		/* A couple of unsafe functions for internal use. */
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}

	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = TXScopeItemFixedPointer<typename std::remove_reference<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())>::type>;
	}

	/* While TXScopeFixedPointer<> points to a TXScopeObj<>, TXScopeItemFixedPointer<> is intended to be able to point to a
	TXScopeObj<>, any member of a TXScopeObj<>, or various other items with scope lifetime that, for various reasons, aren't
	declared as TXScopeObj<>. */
	template<typename _Ty>
	class TXScopeItemFixedPointer : public mse::us::impl::TXScopePointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}

		virtual ~TXScopeItemFixedPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return std::addressof(*(*this))/*mse::us::impl::TXScopePointerBase<_Ty>::operator _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedPointer(_Ty* ptr) : mse::us::impl::TXScopePointerBase<_Ty>(ptr) {}
		TXScopeItemFixedPointer(const mse::us::impl::TXScopePointerBase<_Ty>& ptr) : mse::us::impl::TXScopePointerBase<_Ty>(ptr) {}
		TXScopeItemFixedPointer<_Ty>& operator=(const TXScopeItemFixedPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty2, _TMemberObjectPointer>;
		/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty2> &lease_pointer);

		template<class _Ty2> friend TXScopeItemFixedPointer<_Ty2> us::unsafe_make_xscope_pointer_to(_Ty2& ref);
	};

	template<typename _Ty>
	class TXScopeItemFixedConstPointer : public mse::us::impl::TXScopeConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty>& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopePointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}

		virtual ~TXScopeItemFixedConstPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::addressof(*(*this))/*mse::us::impl::TXScopeConstPointerBase<_Ty>::operator const _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedConstPointer(const _Ty* ptr) : mse::us::impl::TXScopeConstPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedConstPointer(const mse::us::impl::TXScopeConstPointerBase<_Ty>& ptr) : mse::us::impl::TXScopeConstPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedConstPointer<_Ty>& operator=(const TXScopeItemFixedConstPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>;
		/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2> &lease_pointer);

		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer);
		template<class _Ty2> friend TXScopeItemFixedConstPointer<_Ty2> us::unsafe_make_xscope_const_pointer_to(const _Ty2& cref);
	};

	/* TXScopeCagedItemFixedPointerToRValue<> holds a TXScopeItemFixedPointer<> that points to an r-value which can only be
	accessed when converted to a rsv::TXScopeItemFixedPointerFParam<>. */
	template<typename _Ty>
	class TXScopeCagedItemFixedPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		void xscope_tag() const {}

	private:
		TXScopeCagedItemFixedPointerToRValue(const TXScopeCagedItemFixedPointerToRValue&) = delete;
		TXScopeCagedItemFixedPointerToRValue(TXScopeCagedItemFixedPointerToRValue&&) = default;
		TXScopeCagedItemFixedPointerToRValue(const TXScopeItemFixedPointer<_Ty>& ptr) : m_xscope_ptr(ptr) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		TXScopeCagedItemFixedPointerToRValue(TXScopeCagedItemFixedPointerToRValue&& src_ref) : m_xscope_ptr(src_ref) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		auto uncaged_pointer() const {
			return m_xscope_ptr;
		}

		TXScopeCagedItemFixedPointerToRValue<_Ty>& operator=(const TXScopeCagedItemFixedPointerToRValue<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		TXScopeItemFixedPointer<_Ty> m_xscope_ptr;

		friend class TXScopeObj<_Ty>;
		template<class _Ty2> friend class rsv::TXScopeItemFixedPointerFParam;
		template<class _Ty2> friend class rsv::TXScopeItemFixedConstPointerFParam;
	};

	template<typename _Ty>
	class TXScopeCagedItemFixedConstPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase {
	public:
		void xscope_tag() const {}

	private:
		TXScopeCagedItemFixedConstPointerToRValue(const TXScopeCagedItemFixedConstPointerToRValue& src_cref) = delete;
		TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_cref) = default;
		TXScopeCagedItemFixedConstPointerToRValue(const TXScopeItemFixedConstPointer<_Ty>& ptr) : m_xscope_ptr(ptr) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_ref) : m_xscope_ptr(src_ref) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		auto uncaged_pointer() const { return m_xscope_ptr; }

		TXScopeCagedItemFixedConstPointerToRValue<_Ty>& operator=(const TXScopeCagedItemFixedConstPointerToRValue<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		TXScopeItemFixedConstPointer<_Ty> m_xscope_ptr;

		friend class TXScopeObj<_Ty>;
		template<class _Ty2> friend class rsv::TXScopeItemFixedConstPointerFParam;
		template<typename _Ty2> friend auto pointer_to(_Ty2&& _X) -> decltype(&std::forward<_Ty2>(_X));
	};
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopeItemFixedPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeItemFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeItemFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopeFixedPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};

	template<class _Ty>
	struct hash<mse::TXScopeItemFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeItemFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeItemFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
	template<class _Ty>
	struct hash<mse::TXScopeFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
			const _Ty* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _Ty *>()(ptr1));
		}
	};
}

namespace mse {

	template<typename _TROy>
	class TNonXScopeObj : public _TROy {
	public:
		MSE_USING(TNonXScopeObj, _TROy);
		TNonXScopeObj(const TNonXScopeObj& _X) : _TROy(_X) {}
		TNonXScopeObj(TNonXScopeObj&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TNonXScopeObj() {
			mse::impl::T_valid_if_not_an_xscope_type<_TROy>();
		}

		TNonXScopeObj& operator=(TNonXScopeObj&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		TNonXScopeObj& operator=(const TNonXScopeObj& _X) { _TROy::operator=(_X); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(_Ty2&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		//TNonXScopeObj& operator=(_Ty2&& _X) { static_cast<_TROy&>(*this) = (std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
	};


	/* template specializations */

	template<typename _Ty>
	class TXScopeObj<_Ty*> : public TXScopeObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
	template<typename _Ty>
	class TXScopeObj<_Ty* const> : public TXScopeObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
	template<typename _Ty>
	class TXScopeObj<const _Ty *> : public TXScopeObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
	template<typename _Ty>
	class TXScopeObj<const _Ty * const> : public TXScopeObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};

#ifdef MSEPRIMITIVES_H
	template<>
	class TXScopeObj<int> : public TXScopeObj<mse::TInt<int>> {
	public:
		typedef TXScopeObj<mse::TInt<int>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
	template<>
	class TXScopeObj<const int> : public TXScopeObj<const mse::TInt<int>> {
	public:
		typedef TXScopeObj<const mse::TInt<int>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};

	template<>
	class TXScopeObj<size_t> : public TXScopeObj<mse::TInt<size_t>> {
	public:
		typedef TXScopeObj<mse::TInt<size_t>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
	template<>
	class TXScopeObj<const size_t> : public TXScopeObj<const mse::TInt<size_t>> {
	public:
		typedef TXScopeObj<const mse::TInt<size_t>> base_class;
		MSE_USING(TXScopeObj, base_class);
	};
#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#endif /*MSE_SCOPEPOINTER_DISABLED*/
	namespace us {
		namespace impl {
#ifdef MSE_SCOPEPOINTER_DISABLED
			template<typename _Ty> using TXScopeItemFixedPointerFParamBase = mse::us::impl::TPointer<_Ty>;
			template<typename _Ty> using TXScopeItemFixedConstPointerFParamBase = mse::us::impl::TPointer<const _Ty>;
#else /*MSE_SCOPEPOINTER_DISABLED*/
			template<typename _Ty> using TXScopeItemFixedPointerFParamBase = TXScopeItemFixedPointer<_Ty>;
			template<typename _Ty> using TXScopeItemFixedConstPointerFParamBase = TXScopeItemFixedConstPointer<_Ty>;
#endif /*MSE_SCOPEPOINTER_DISABLED*/
		}
	}

	namespace rsv {
		/* TXScopeItemFixedPointerFParam<> is just a version of TXScopeItemFixedPointer<> which may only be used for
		function parameter declations. It has the extra ability to accept (caged) scope pointers to r-value scope objects
		(i.e. supports temporaries by scope reference). */

		template<typename _Ty>
		class TXScopeItemFixedPointerFParam : public mse::us::impl::TXScopeItemFixedPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeItemFixedPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeItemFixedPointerFParam, base_class);

			TXScopeItemFixedPointerFParam(const TXScopeItemFixedPointerFParam& src_cref) = default;

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeItemFixedPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif //!MSE_SCOPEPOINTER_DISABLED

			virtual ~TXScopeItemFixedPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}

		private:
			TXScopeItemFixedPointerFParam<_Ty>& operator=(const TXScopeItemFixedPointerFParam<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template<typename _Ty>
		class TXScopeItemFixedConstPointerFParam : public mse::us::impl::TXScopeItemFixedConstPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeItemFixedConstPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeItemFixedConstPointerFParam, base_class);

			TXScopeItemFixedConstPointerFParam(const TXScopeItemFixedConstPointerFParam<_Ty>& src_cref) = default;
			TXScopeItemFixedConstPointerFParam(const TXScopeItemFixedPointerFParam<_Ty>& src_cref) : base_class(src_cref) {}

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeItemFixedConstPointerFParam(const TXScopeCagedItemFixedConstPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeItemFixedConstPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif //!MSE_SCOPEPOINTER_DISABLED

			virtual ~TXScopeItemFixedConstPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}

		private:
			TXScopeItemFixedConstPointerFParam<_Ty>& operator=(const TXScopeItemFixedConstPointerFParam<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
	}

	namespace rsv {
		/* rsv::TFParam<> is just a transparent template wrapper for function parameter declarations. In most cases
		use of this wrapper is not necessary, but in some cases it enables functionality only available to variables
		that are function parameters. Specifically, it allows functions to support scope pointer/references to
		temporary objects. For safety reasons, by default, scope pointer/references to temporaries are actually
		"functionally disabled" types distinct from regular scope pointer/reference types. Because it's safe to do so
		in the case of function parameters, the rsv::TFParam<> wrapper enables certain scope pointer/reference types
		(like TXScopeItemFixedPointer<>, and "random access section" types) to be constructed from their
		"functionally disabled" counterparts.
		*/
		template<typename _Ty>
		class TFParam : public _Ty {
		public:
			typedef _Ty base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty> using TXScopeFParam = TFParam<_Ty>;

		namespace impl {
			namespace fparam {
				template<typename _Ty>
				const auto& as_an_fparam_helper1(std::false_type, const _Ty& param) {
					return param;
				}
				template<typename _Ty>
				auto as_an_fparam_helper1(std::true_type, const _Ty& param) -> typename TFParam<typename std::remove_reference<_Ty>::type>::base_class {
					return TFParam<typename std::remove_reference<_Ty>::type>(param);
				}

				template<typename _Ty>
				auto as_an_fparam_helper1(std::false_type, _Ty&& param) {
					return std::forward<_Ty>(param);
				}
				template<typename _Ty>
				auto as_an_fparam_helper1(std::true_type, _Ty&& param) -> typename TFParam<typename std::remove_reference<_Ty>::type>::base_class {
					return TFParam<typename std::remove_reference<_Ty>::type>(std::forward<_Ty>(param));
				}
			}
		}

		template<typename _Ty>
		auto as_an_fparam(const _Ty& param) -> decltype(impl::fparam::as_an_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), param)) {
			return impl::fparam::as_an_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), param);
		}
		template<typename _Ty>
		auto as_an_fparam(_Ty&& param) {
			return impl::fparam::as_an_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_as_an_fparam(const _Ty& param) -> decltype(as_an_fparam(param)) {
			return as_an_fparam(param);
		}
		template<typename _Ty>
		auto xscope_as_an_fparam(_Ty&& param) {
			return as_an_fparam(std::forward<_Ty>(param));
		}

		/* Template specializations of TFParam<>. There are a number of them. */

#ifndef MSE_SCOPEPOINTER_DISABLED
		template<typename _Ty>
		class TFParam<mse::TXScopeItemFixedConstPointer<_Ty> > : public TXScopeItemFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeItemFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<const mse::TXScopeItemFixedConstPointer<_Ty> > : public TXScopeItemFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeItemFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<mse::TXScopeCagedItemFixedConstPointerToRValue<_Ty> > : public TXScopeItemFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeItemFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
#endif //!MSE_SCOPEPOINTER_DISABLED

		template<typename _Ty>
		class TFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		/* These specializations for native arrays aren't actually meant to be used. They're just needed because when you call
		as_an_fparam() on a native array, msvc2017 will try to instantiate a TFParam<> with the native array even though it is
		determined at compile that it will never be used. clang6, for example, doesn't have the same issue. */
		template<typename _Ty, size_t _Size>
		class TFParam<const _Ty[_Size]> : public mse::us::impl::TPointerForLegacy<const _Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(const _Ty(&param)[_Size]) : base_class(param) {}
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
		template<typename _Ty, size_t _Size>
		class TFParam<_Ty[_Size]> : public mse::us::impl::TPointerForLegacy<_Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(_Ty(&param)[_Size]) : base_class(param) {}
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};


		/* rsv::TReturnableFParam<> is just a transparent template wrapper for function parameter declarations. Like
		us::FParam<>, in most cases use of this wrapper is not necessary, but in some cases it enables functionality
		only available to variables that are function parameters. Specifically, rsv::TReturnableFParam<> "marks"
		scope pointer/reference parameters as safe to use as the return value of the function, whereas by default,
		scope pointer/references are not considered safe to use as a return value. Note that unlike us::FParam<>,
		rsv::TReturnableFParam<> does not enable the function to accept scope pointer/reference temporaries.
		*/
		template<typename _Ty>
		class TReturnableFParam : public _Ty {
		public:
			typedef _Ty base_class;
			typedef _Ty returnable_fparam_contained_type;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

			void returnable_once_tag() const {}
			void xscope_returnable_tag() const {}

		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty> using TXScopeReturnableFParam = TReturnableFParam<_Ty>;


		template<typename _Ty>
		auto returnable_fparam_as_base_type(TReturnableFParam<_Ty>&& _X) {
			return std::forward<_Ty>(_X);
		}
		template<typename _Ty>
		auto returnable_fparam_as_base_type(const TReturnableFParam<_Ty>& _X) -> const typename TReturnableFParam<_Ty>::base_class& {
			return _X;
		}


		namespace impl {
			namespace returnable_fparam {
				template<typename _Ty>
				const auto& as_a_returnable_fparam_helper1(std::false_type, const _Ty& param) {
					return param;
				}
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, const _Ty& param) -> TReturnableFParam<typename std::remove_reference<_Ty>::type> {
					return TReturnableFParam<typename std::remove_reference<_Ty>::type>(param);
				}

				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::false_type, _Ty&& param) {
					return std::forward<_Ty>(param);
				}
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, _Ty&& param) -> TReturnableFParam<typename std::remove_reference<_Ty>::type> {
					return TReturnableFParam<typename std::remove_reference<_Ty>::type>(std::forward<_Ty>(param));
				}
			}
		}

		template<typename _Ty>
		auto as_a_returnable_fparam(const _Ty& param) -> decltype(impl::returnable_fparam::as_a_returnable_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), param)) {
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), param);
		}
		template<typename _Ty>
		auto as_a_returnable_fparam(_Ty&& param) {
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(const _Ty& param) -> decltype(as_a_returnable_fparam(param)) {
			return as_a_returnable_fparam(param);
		}
		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(_Ty&& param) {
			return as_a_returnable_fparam(std::forward<_Ty>(param));
		}

		/* Template specializations of TReturnableFParam<>. */

		template<typename _Ty>
		class TReturnableFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TReturnableFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty> {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}

	/* If a rsv::TReturnableFParam<> wrapped reference is used to make a pointer to a member of its target object, then the
	created pointer to member can inherit the "returnability" of the original wrapped reference. */
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const rsv::TReturnableFParam<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		const _Ty& lease_pointer_base_ref = lease_pointer;
		typedef decltype(make_xscope_pointer_to_member_v2(lease_pointer_base_ref, member_object_ptr)) base_return_type;
		return rsv::TReturnableFParam<base_return_type>(make_xscope_pointer_to_member_v2(lease_pointer_base_ref, member_object_ptr));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const rsv::TReturnableFParam<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		const _Ty& lease_pointer_base_ref = lease_pointer;
		typedef decltype(make_xscope_const_pointer_to_member_v2(lease_pointer_base_ref, member_object_ptr)) base_return_type;
		return rsv::TReturnableFParam<base_return_type>(make_xscope_const_pointer_to_member_v2(lease_pointer_base_ref, member_object_ptr));
	}

	template<typename _TROy>
	class TReturnValue : public _TROy {
	public:
		typedef _TROy base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, _TROy);
		virtual ~TReturnValue() {
			/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
			valid_if_TROy_is_marked_as_returnable_or_not_xscope_type();
		}

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		/* If _TROy is not recognized as safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class = typename std::enable_if<(!std::is_base_of<mse::us::impl::XScopeTagBase, _TROy>::value)
			|| (!std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TROy>::value)
			|| ((std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_TROy>::Has>())
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/
				), void>::type>
		void valid_if_TROy_is_marked_as_returnable_or_not_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _TROy>
	class TXScopeReturnValue : public TReturnValue<_TROy>
		, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _TROy>::value, mse::impl::TPlaceHolder_msescope<TXScopeReturnValue<_TROy> >, mse::us::impl::XScopeTagBase>::type
	{
	public:
		typedef TReturnValue<_TROy> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TXScopeReturnValue, base_class);

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	namespace impl {
		template<typename _Ty>
		static void z__returnable_noop(const _Ty&) {}

		template<typename _Ty>
		auto return_value_helper12(const _Ty& _X) {
			return rsv::returnable_fparam_as_base_type(_X);
		}
		template<typename _Ty>
		auto return_value_helper12(_Ty&& _X) {
			return rsv::returnable_fparam_as_base_type(std::forward<decltype(_X)>(_X));
		}

		template<typename _Ty>
		const auto& return_value_helper11(std::false_type, const _Ty& _X) {
			z__returnable_noop<mse::TReturnValue<_Ty> >(_X);
			return _X;
		}
		template<typename _Ty>
		const auto& return_value_helper11(std::true_type, const _Ty& _X) {
			return return_value_helper12(_X);
		}
	}

	template<typename _Ty>
	const auto& return_value(const _Ty& _X) {
		typedef typename std::remove_reference<_Ty>::type _Ty_noref;
		return impl::return_value_helper11(typename std::conditional<
			impl::is_instantiation_of_msescope<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of_msescope<_Ty_noref, rsv::TXScopeReturnableFParam>::value
			, std::true_type, std::false_type>::type(), _X);
	}

	namespace impl {
		template<typename _Ty>
		auto return_value_helper11(std::false_type, _Ty&& _X) {
			z__returnable_noop<mse::TReturnValue<typename std::remove_reference<_Ty>::type> >(_X);
			return std::forward<decltype(_X)>(_X);
		}
		template<typename _Ty>
		auto return_value_helper11(std::true_type, _Ty&& _X) {
			return return_value_helper12(std::forward<decltype(_X)>(_X));
		}
	}
	template<typename _Ty>
	auto return_value(_Ty&& _X) {
		typedef typename std::remove_reference<_Ty>::type _Ty_noref;
		return impl::return_value_helper11(typename std::conditional<
			impl::is_instantiation_of_msescope<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of_msescope<_Ty_noref, rsv::TXScopeReturnableFParam>::value
			, std::true_type, std::false_type>::type(), std::forward<decltype(_X)>(_X));
	}

	/* Template specializations of TReturnValue<>. */

	template<typename _Ty>
	class TReturnValue<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		virtual ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, class = typename std::enable_if<!(std::is_same<_Ty2, _Ty>::value)>::type>
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _Ty>
	class TReturnValue<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		virtual ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

	private:
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		template<class _Ty2 = _Ty, class = typename std::enable_if<!(std::is_same<_Ty2, _Ty>::value)>::type>
#endif /*!defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};


	/* deprecated aliases */
	template<typename _TROy> using TReturnable = TReturnValue<_TROy>;
	template<typename _TROy> using TXScopeReturnable = TXScopeReturnValue<_TROy>;
	template<typename _Ty> const auto& returnable(const _Ty& _X) { return return_value(_X); }
	template<typename _Ty> auto&& returnable(_Ty&& _X) { return return_value(std::forward<decltype(_X)>(_X)); }


	/* TMemberObj is a transparent wrapper that can be used to wrap class/struct members to ensure that they are not scope
	types. This might be particularly relevant when the member type is, or is derived from, a template parameter. */
	template<typename _TROy> using TMemberObj = TNonXScopeObj<_TROy>;

	/* TBaseClass is a transparent wrapper that can be used to wrap base classes to ensure that they are not scope
	types. This might be particularly relevant when the base class is, or is derived from, a template parameter. */
	template<typename _TROy> using TBaseClass = TNonXScopeObj<_TROy>;

	/* TXScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
	TXScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TXScopeObj<_Ty>.
	TXScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
	enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
	*/
	template<typename _Ty>
	class TXScopeOwnerPointer : public mse::us::impl::XScopeTagBase, public mse::us::impl::StrongPointerNotAsyncShareableTagBase
		, public std::conditional<std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty>::value, mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::TPlaceHolder_msescope<TXScopeOwnerPointer<_Ty> > >::type
	{
	public:
		TXScopeOwnerPointer(TXScopeOwnerPointer<_Ty>&& src_ref) = default;

		template <class... Args>
		TXScopeOwnerPointer(Args&&... args) {
			/* In the case where there is exactly one argument and its type is derived from this type, we want to
			act like a move constructor here. We use a helper function to check for this case and act accordingly. */
			constructor_helper1(std::forward<Args>(args)...);
		}

		TXScopeObj<_Ty>& operator*() const {
			return (*m_ptr);
		}
		TXScopeObj<_Ty>* operator->() const {
			return std::addressof(*m_ptr);
		}

#ifdef MSE_SCOPEPOINTER_DISABLED
		operator _Ty*() const {
			return std::addressof(*(*this));
		}
		operator const _Ty*() const {
			return std::addressof(*(*this));
		}
#endif /*MSE_SCOPEPOINTER_DISABLED*/

		template <class... Args>
		static TXScopeOwnerPointer make(Args&&... args) {
			return TXScopeOwnerPointer(std::forward<Args>(args)...);
		}

		void xscope_tag() const {}
		/* This type can be safely used as a function return value if _TROy is also safely returnable. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		/* construction helper functions */
		template <class... Args>
		void initialize(Args&&... args) {
			/* We can't use std::make_unique<> because TXScopeObj<>'s "operator new()" is private and inaccessible to
			std::make_unique<> (which is not a friend of TXScopeObj<> like we are). */
			auto new_ptr = new TXScopeObj<_Ty>(std::forward<Args>(args)...);
			m_ptr.reset(new_ptr);
		}
		template <class _TSoleArg>
		void constructor_helper2(std::true_type, _TSoleArg&& sole_arg) {
			/* The sole parameter is derived from, or of this type, so we're going to consider the constructor
			a move constructor. */
			m_ptr = std::forward<decltype(sole_arg.m_ptr)>(sole_arg.m_ptr);
		}
		template <class _TSoleArg>
		void constructor_helper2(std::false_type, _TSoleArg&& sole_arg) {
			/* The sole parameter is not derived from, or of this type, so the constructor is not a move
			constructor. */
			initialize(std::forward<decltype(sole_arg)>(sole_arg));
		}
		template <class... Args>
		void constructor_helper1(Args&&... args) {
			initialize(std::forward<Args>(args)...);
		}
		template <class _TSoleArg>
		void constructor_helper1(_TSoleArg&& sole_arg) {
			/* The constructor was given exactly one parameter. If the parameter is derived from, or of this type,
			then we're going to consider the constructor a move constructor. */
			constructor_helper2(typename std::is_base_of<TXScopeOwnerPointer, _TSoleArg>::type(), std::forward<decltype(sole_arg)>(sole_arg));
		}

		TXScopeOwnerPointer(const TXScopeOwnerPointer<_Ty>& src_cref) = delete;
		TXScopeOwnerPointer<_Ty>& operator=(const TXScopeOwnerPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		std::unique_ptr<TXScopeObj<_Ty> > m_ptr = nullptr;
	};

	template <class X, class... Args>
	TXScopeOwnerPointer<X> make_xscope_owner(Args&&... args) {
		return TXScopeOwnerPointer<X>::make(std::forward<Args>(args)...);
	}
}

namespace std {
	template<class _Ty>
	struct hash<mse::TXScopeOwnerPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeOwnerPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeOwnerPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
		/* (Unsafely) obtain a scope pointer to any object. */
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref) {
			return TXScopeItemFixedPointer<_Ty>(std::addressof(ref));
		}
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref) {
			return TXScopeItemFixedConstPointer<_Ty>(std::addressof(cref));
		}
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_pointer_to(const _Ty& cref) {
			return unsafe_make_xscope_const_pointer_to(cref);
		}
	}

	namespace us {
		template<typename _TROy>
		class TXScopeUserDeclaredReturnable : public _TROy {
		public:
			MSE_USING(TXScopeUserDeclaredReturnable, _TROy);
			TXScopeUserDeclaredReturnable(const TXScopeUserDeclaredReturnable& _X) : _TROy(_X) {}
			TXScopeUserDeclaredReturnable(TXScopeUserDeclaredReturnable&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
			virtual ~TXScopeUserDeclaredReturnable() {
				/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
				valid_if_TROy_is_not_marked_as_unreturn_value();
				valid_if_TROy_is_an_xscope_type();
			}

			template<class _Ty2>
			TXScopeUserDeclaredReturnable& operator=(_Ty2&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			template<class _Ty2>
			TXScopeUserDeclaredReturnable& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

		private:

			/* If _TROy is "marked" as not safe to use as a function return value, then the following member function
			will not instantiate, causing an (intended) compile error. */
			template<class = typename std::enable_if<(!std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TROy>::value)
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/, void>::type>
				void valid_if_TROy_is_not_marked_as_unreturn_value() const {}

			template<class = typename std::enable_if<std::is_base_of<mse::us::impl::XScopeTagBase, _TROy>::value, void>::type>
			void valid_if_TROy_is_an_xscope_type() const {}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
	}


	template<class _TTargetType, class _Ty>
	TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifndef MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopePointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeConstPointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeConstPointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeConstPointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(TXScopeItemFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(TXScopeItemFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(TXScopeItemFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(TXScopeItemFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#endif // !MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER


	/* TXScopeStrongPointerStore et al are types that store a strong pointer (like a refcounting pointer), and let you
	obtain a corresponding scope pointer. */
	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongPointerStore : public mse::us::impl::XScopeTagBase
		, public std::conditional<std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::TPlaceHolder_msescope<TXScopeStrongPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongPointerStore(const TXScopeStrongPointerStore&) = delete;
		TXScopeStrongPointerStore(TXScopeStrongPointerStore&&) = default;

		TXScopeStrongPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {
			*stored_ptr; /* Just verifying that stored_ptr points to a valid target. */
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongConstPointerStore : public mse::us::impl::XScopeTagBase
		, public std::conditional<std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::TPlaceHolder_msescope<TXScopeStrongConstPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongConstPointerStore(const TXScopeStrongConstPointerStore&) = delete;
		TXScopeStrongConstPointerStore(TXScopeStrongConstPointerStore&&) = default;

		TXScopeStrongConstPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {
			*stored_ptr; /* Just verifying that stored_ptr points to a valid target. */
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<_TStrongPointer> >
	class TXScopeStrongNotNullPointerStore : public mse::us::impl::XScopeTagBase
		, public std::conditional<std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::TPlaceHolder_msescope<TXScopeStrongNotNullPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongNotNullPointerStore(const TXScopeStrongNotNullPointerStore&) = delete;
		TXScopeStrongNotNullPointerStore(TXScopeStrongNotNullPointerStore&&) = default;

		TXScopeStrongNotNullPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<_TStrongPointer> >
	class TXScopeStrongNotNullConstPointerStore : public mse::us::impl::XScopeTagBase
		, public std::conditional<std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, mse::us::impl::ContainsNonOwningScopeReferenceTagBase, mse::impl::TPlaceHolder_msescope<TXScopeStrongNotNullConstPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongNotNullConstPointerStore(const TXScopeStrongNotNullConstPointerStore&) = delete;
		TXScopeStrongNotNullConstPointerStore(TXScopeStrongNotNullConstPointerStore&&) = default;

		TXScopeStrongNotNullConstPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer>
	TXScopeStrongPointerStore<_TStrongPointer> make_xscope_strong_pointer_store(const _TStrongPointer& stored_ptr) {
		return TXScopeStrongPointerStore<_TStrongPointer>(stored_ptr);
	}


	template<typename _Ty> using TXScopeXScopeItemFixedStore = TXScopeStrongNotNullPointerStore<TXScopeItemFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeItemFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeItemFixedConstPointer<_Ty> >;
	template<typename _Ty>
	TXScopeXScopeItemFixedStore<_Ty> make_xscope_strong_pointer_store(const TXScopeItemFixedPointer<_Ty>& stored_ptr) {
		return TXScopeXScopeItemFixedStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeXScopeItemFixedConstStore<_Ty> make_xscope_strong_pointer_store(const TXScopeItemFixedConstPointer<_Ty>& stored_ptr) {
		return TXScopeXScopeItemFixedConstStore<_Ty>(stored_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty> using TXScopeXScopeFixedStore = TXScopeStrongNotNullPointerStore<TXScopeFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeFixedConstPointer<_Ty> >;
	template<typename _Ty>
	TXScopeXScopeFixedStore<_Ty> make_xscope_strong_pointer_store(const TXScopeFixedPointer<_Ty>& stored_ptr) {
		return TXScopeXScopeFixedStore<_Ty>(stored_ptr);
	}
	template<typename _Ty>
	TXScopeXScopeFixedConstStore<_Ty> make_xscope_strong_pointer_store(const TXScopeFixedConstPointer<_Ty>& stored_ptr) {
		return TXScopeXScopeFixedConstStore<_Ty>(stored_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	/* The purpose of the xscope_chosen_pointer() function is simply to take two scope pointers as input parameters and return one of
	them. Which of the pointers is returned is determined by a "decider" function that is passed, as the first parameter, to
	xscope_chosen_pointer(). The "decider" function needs to return a bool and take the two scope pointers as its first two parameters.
	The reason this xscope_chosen_pointer() function is needed is that (non-owning) scope pointers are, in general, not allowed to be
	used as a function return value. (Because you might accidentally return a pointer to a local scope object (which is bad)
	instead of one of the pointers given as an input parameter (which is fine).) So the xscope_chosen_pointer() template is the
	sanctioned way of creating a function that returns a non-owning scope pointer. */
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const auto& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedConstPointer<_Ty>& a, const TXScopeItemFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const auto& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedPointer<_Ty>& a, const TXScopeItemFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const auto& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const auto& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<typename _Ty>
	const auto& xscope_chosen_pointer(bool choose_the_second, const TXScopeItemFixedConstPointer<_Ty>& a, const TXScopeItemFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	const auto& xscope_chosen_pointer(bool choose_the_second, const TXScopeItemFixedPointer<_Ty>& a, const TXScopeItemFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty>
	const auto& xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	const auto& xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	/* Just the generalization of xscope_chosen_pointer(). */
	template<typename _Ty>
	const auto& chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	const auto& xscope_chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return choose_the_second ? b : a;
	}

	/* shorter aliases */
	template<typename _Ty> using sfp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using sfcp = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using so = TXScopeObj<_TROy>;
	template<typename _Ty> using sifp = TXScopeItemFixedPointer<_Ty>;
	template<typename _Ty> using sifcp = TXScopeItemFixedConstPointer<_Ty>;

	/* deprecated aliases */
	template<typename _Ty> using scpfp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using scpfcp = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using scpo = TXScopeObj<_TROy>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfp = TSyncWeakFixedPointer<_TTargetType, _TXScopePointerType>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfcp = TSyncWeakFixedConstPointer<_TTargetType, _TXScopePointerType>;

	template<typename _Ty> using TScopeFixedPointer = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TScopeFixedConstPointer = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using TScopeObj = TXScopeObj<_TROy>;
	template<typename _Ty> using TScopeOwnerPointer = TXScopeOwnerPointer<_Ty>;


	namespace self_test {
		class CXScpPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class A {
				public:
					A(int x) : b(x) {}
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
					static int foo2(mse::TXScopeItemFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
				protected:
					~B() {}
				};

				A* A_native_ptr = nullptr;

				{
					A a(7);
					mse::TXScopeObj<A> scope_a(7);
					/* mse::TXScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
					in almost all cases. */

					assert(a.b == scope_a.b);
					A_native_ptr = &a;

					mse::TXScopeItemFixedPointer<A> A_scope_ptr1(&scope_a);
					assert(A_native_ptr->b == A_scope_ptr1->b);
					mse::TXScopeItemFixedPointer<A> A_scope_ptr2 = &scope_a;

					/* mse::TXScopeItemFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<A*>(A_scope_ptr1));

					if (!A_scope_ptr2) {
						assert(false);
					}
					else if (!(A_scope_ptr2 != A_scope_ptr1)) {
						int q = B::foo2(A_scope_ptr2);
					}
					else {
						assert(false);
					}

					mse::us::impl::TPointerForLegacy<A> pfl_ptr1 = &a;
					if (!(pfl_ptr1 != nullptr)) {
						assert(false);
					}
					mse::us::impl::TPointerForLegacy<A> pfl_ptr2 = nullptr;
					if (!(pfl_ptr1 != pfl_ptr2)) {
						assert(false);
					}

					A a2 = a;
					mse::TXScopeObj<A> scope_a2 = scope_a;
					scope_a2 = a;
					scope_a2 = scope_a;

					mse::TXScopeItemFixedConstPointer<A> rcp = A_scope_ptr1;
					mse::TXScopeItemFixedConstPointer<A> rcp2 = rcp;
					const mse::TXScopeObj<A> cscope_a(11);
					mse::TXScopeItemFixedConstPointer<A> rfcp = &cscope_a;

					mse::TXScopeOwnerPointer<A> A_scpoptr(11);
					//B::foo2(A_scpoptr);
					B::foo2(&*A_scpoptr);
					if (A_scpoptr->b == (&*A_scpoptr)->b) {
					}
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					mse::TXScopeObj<GE> scope_gd;
					mse::TXScopeItemFixedPointer<GE> GE_scope_ifptr1 = &scope_gd;
					mse::TXScopeItemFixedPointer<E> E_scope_ifptr5 = GE_scope_ifptr1;
					mse::TXScopeFixedPointer<E> E_scope_fptr2(&scope_gd);
					mse::TXScopeItemFixedPointer<E> E_scope_ifptr2(&scope_gd);
					mse::TXScopeItemFixedConstPointer<E> E_scope_fcptr2 = &scope_gd;
				}

				{
					class A {
					public:
						A(int x) : b(x) {}
						virtual ~A() {}

						int b = 3;
						std::string s = "some text ";
					};
					class B {
					public:
						static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
						static int foo2(mse::TXScopeItemFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
						static int foo3(mse::TXScopeItemFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
					protected:
						~B() {}
					};

					mse::TXScopeObj<A> a_scpobj(5);
					int res1 = (&a_scpobj)->b;
					int res2 = B::foo2(&a_scpobj);
					int res3 = B::foo3(&a_scpobj);
					mse::TXScopeOwnerPointer<A> a_scpoptr(7);
					//int res4 = B::foo2(a_scpoptr);
					int res4b = B::foo2(&(*a_scpoptr));

					/* You can use the "mse::make_xscope_pointer_to_member_v2()" function to obtain a safe pointer to a member of
					an xscope object. */
					auto s_safe_ptr1 = mse::make_xscope_pointer_to_member_v2((&a_scpobj), &A::s);
					(*s_safe_ptr1) = "some new text";
					auto s_safe_const_ptr1 = mse::make_xscope_const_pointer_to_member_v2((&a_scpobj), &A::s);
				}

				{
					A a(7);
					mse::TXScopeObj<A> scope_a(7);
					/* mse::TXScopeObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
					in almost all cases. */

					assert(a.b == scope_a.b);
					A_native_ptr = &a;

					mse::TXScopeFixedPointer<A> A_scope_ptr1 = &scope_a;
					assert(A_native_ptr->b == A_scope_ptr1->b);
					mse::TXScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

					/* mse::TXScopeFixedPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
					B::foo1(static_cast<A*>(A_scope_ptr1));

					if (!A_scope_ptr2) {
						assert(false);
					}
					else if (!(A_scope_ptr2 != A_scope_ptr1)) {
						int q = B::foo2(A_scope_ptr2);
					}
					else {
						assert(false);
					}

					A a2 = a;
					mse::TXScopeObj<A> scope_a2 = scope_a;
					scope_a2 = a;
					scope_a2 = scope_a;

					mse::TXScopeFixedConstPointer<A> rcp = A_scope_ptr1;
					mse::TXScopeFixedConstPointer<A> rcp2 = rcp;
					const mse::TXScopeObj<A> cscope_a(11);
					mse::TXScopeFixedConstPointer<A> rfcp = &cscope_a;

					mse::TXScopeOwnerPointer<A> A_scpoptr(11);
					//B::foo2(A_scpoptr);
					B::foo2(&*A_scpoptr);
					if (A_scpoptr->b == (&*A_scpoptr)->b) {
					}
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					mse::TXScopeObj<GE> scope_gd;
					mse::TXScopeFixedPointer<GE> GE_scope_ifptr1 = &scope_gd;
					mse::TXScopeFixedPointer<E> E_scope_ptr5(GE_scope_ifptr1);
					mse::TXScopeFixedPointer<E> E_scope_ifptr2(&scope_gd);
					mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_gd;
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

}

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSESCOPE_H_
