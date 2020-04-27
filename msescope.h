
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

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("_NOEXCEPT")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


namespace mse {

	/* This macro roughly simulates constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) MSE_USING(Derived, Base)

	namespace impl {
		template<typename _Ty> class TScopeID {};
	}

	namespace impl {

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
			static const bool value = ((std::is_base_of<mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, T>::value
					&& std::is_base_of<mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, T>::value)
				|| (std::is_pointer<T>::value && (!mse::impl::is_potentially_not_xscope<T>::value)));
			using type = std::integral_constant<bool, value>;
		};
		template<class T, class EqualTo = T>
		struct IsNonOwningScopePointer : IsNonOwningScopePointer_impl<
			typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

		template<class T, class EqualTo>
		struct IsNonOwningScopeOrIndeterminatePointer_impl
		{
			static const bool value = (IsNonOwningScopePointer<T>::value)
#ifdef MSE_SCOPEPOINTER_DISABLED
				|| (std::is_pointer<T>::value)
#endif // MSE_SCOPEPOINTER_DISABLED
				;
			using type = std::integral_constant<bool, value>;
		};
		template<class T, class EqualTo = T>
		struct IsNonOwningScopeOrIndeterminatePointer : IsNonOwningScopeOrIndeterminatePointer_impl<
			typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

		template <class _Ty, class _Ty2, class = typename std::enable_if<
			(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
			, void>::type>
		static void valid_if_not_rvalue_reference_of_given_type(_Ty2 src) {}
	}

#ifdef MSE_SCOPEPOINTER_DISABLED
	template<typename _Ty> using TXScopeObjPointer = _Ty * ;
	template<typename _Ty> using TXScopeObjConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeObjNotNullPointer = _Ty * ;
	template<typename _Ty> using TXScopeObjNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeObjFixedPointer = _Ty * /*const*/; /* Can't be const qualified because standard
																	   library containers don't support const elements. */
	template<typename _Ty> using TXScopeObjFixedConstPointer = const _Ty* /*const*/;

	namespace us {
		namespace impl {
			template<typename _TROy> using TXScopeObjBase = _TROy;
		}
	}
	template<typename _TROy> using TXScopeObj = _TROy;
	template<typename _Ty> using TXScopeFixedPointer = _Ty * /*const*/;
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty* /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedPointerToRValue = _Ty * /*const*/;
	template<typename _Ty> using TXScopeCagedItemFixedConstPointerToRValue = const _Ty* /*const*/;
	//template<typename _TROy> using TXScopeReturnValue = _TROy;

	template<typename _TROy> class TXScopeOwnerPointer;

	template<typename _Ty> auto xscope_ifptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto xscope_ifptr_to(const _Ty& _X) { return std::addressof(_X); }

	namespace us {
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}

	//template<typename _Ty> const _Ty& return_value(const _Ty& _X) { return _X; }
	//template<typename _Ty> _Ty&& return_value(_Ty&& _X) { return std::forward<decltype(_X)>(_X); }
	template<typename _TROy> using TNonXScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

	template<typename _Ty> class TXScopeObj;

	namespace us {
		namespace impl {

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

			template<typename _TROz> using TXScopeObjBase = mse::TNDNoradObj<_TROz>;
			template<typename _Ty> using TXScopeItemPointerBase = mse::us::impl::TAnyPointerBase<_Ty>;
			template<typename _Ty> using TXScopeItemConstPointerBase = mse::us::impl::TAnyConstPointerBase<_Ty>;

			template<typename _Ty>
			class TXScopeObjPointerBase : public mse::TNDNoradPointer<_Ty> {
			public:
				typedef mse::TNDNoradPointer<_Ty> base_class;
				TXScopeObjPointerBase(const TXScopeObjPointerBase&) = default;
				TXScopeObjPointerBase(TXScopeObjPointerBase&&) = default;
				TXScopeObjPointerBase(TXScopeObj<_Ty>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				TXScopeObj<_Ty>& operator*() const {
					return static_cast<TXScopeObj<_Ty>& >(*(static_cast<const base_class&>(*this)));
				}
				TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty>
			class TXScopeObjConstPointerBase : public mse::TNDNoradConstPointer<_Ty> {
			public:
				typedef mse::TNDNoradConstPointer<_Ty> base_class;
				TXScopeObjConstPointerBase(const TXScopeObjConstPointerBase&) = default;
				TXScopeObjConstPointerBase(TXScopeObjConstPointerBase&&) = default;
				TXScopeObjConstPointerBase(const TXScopeObjPointerBase<_Ty>& src_cref) : base_class(src_cref) {}
				TXScopeObjConstPointerBase(const TXScopeObj<_Ty>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

				const TXScopeObj<_Ty>& operator*() const {
					return static_cast<const TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this)));
				}
				const TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<const TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

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

			template<typename _Ty>
			class TXScopeObjPointerBase : public mse::us::impl::TPointerForLegacy<TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> {
			public:
				typedef mse::us::impl::TPointerForLegacy<TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> base_class;
				TXScopeObjPointerBase(const TXScopeObjPointerBase&) = default;
				TXScopeObjPointerBase(TXScopeObjPointerBase&&) = default;
				TXScopeObjPointerBase(TXScopeObj<_Ty>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				TXScopeObj<_Ty>& operator*() const {
					return static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this)));
				}
				TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty>
			class TXScopeObjConstPointerBase : public mse::us::impl::TPointerForLegacy<const TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> {
			public:
				typedef mse::us::impl::TPointerForLegacy<const TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> base_class;
				TXScopeObjConstPointerBase(const TXScopeObjConstPointerBase&) = default;
				TXScopeObjConstPointerBase(TXScopeObjConstPointerBase&&) = default;
				TXScopeObjConstPointerBase(const TXScopeObjPointerBase<_Ty>& src_cref) : base_class(src_cref) {}
				TXScopeObjConstPointerBase(const TXScopeObj<_Ty>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

				const TXScopeObj<_Ty>& operator*() const {
					return static_cast<const TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this)));
				}
				const TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<const TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty> using TXScopeItemPointerBase = mse::us::impl::TPointerForLegacy<_Ty, mse::impl::TScopeID<const _Ty>>;
			template<typename _Ty> using TXScopeItemConstPointerBase = mse::us::impl::TPointerForLegacy<const _Ty, mse::impl::TScopeID<const _Ty>>;

#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

		}
	}

	template<typename _Ty> class TXScopeObj;
	template<typename _Ty> class TXScopeObjNotNullPointer;
	template<typename _Ty> class TXScopeObjNotNullConstPointer;
	template<typename _Ty> class TXScopeObjFixedPointer;
	template<typename _Ty> class TXScopeObjFixedConstPointer;
	template<typename _Ty> class TXScopeOwnerPointer;

	template<typename _Ty> class TXScopeFixedPointer;
	template<typename _Ty> class TXScopeFixedConstPointer;
	template<typename _Ty> class TXScopeCagedItemFixedPointerToRValue;
	template<typename _Ty> class TXScopeCagedItemFixedConstPointerToRValue;
	namespace rsv {
		template<typename _Ty> class TXScopeFixedPointerFParam;
		template<typename _Ty> class TXScopeFixedConstPointerFParam;
	}

	namespace us {
		namespace impl {
			template <typename _Ty, typename _TConstPointer1> class TCommonizedPointer;
			template <typename _Ty, typename _TConstPointer1> class TCommonizedConstPointer;
		}
	}

	/* Use TXScopeObjFixedPointer instead. */
	template<typename _Ty>
	class TXScopeObjPointer : public mse::us::impl::TXScopeObjPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjPointer() {}
	private:
		TXScopeObjPointer() : mse::us::impl::TXScopeObjPointerBase<_Ty>() {}
		TXScopeObjPointer(TXScopeObj<_Ty>& scpobj_ref) : mse::us::impl::TXScopeObjPointerBase<_Ty>(scpobj_ref) {}
		TXScopeObjPointer(const TXScopeObjPointer& src_cref) : mse::us::impl::TXScopeObjPointerBase<_Ty>(
			static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty>&>(src_cref)) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjPointer(const TXScopeObjPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjPointerBase<_Ty>(mse::us::impl::TXScopeObjPointerBase<_Ty2>(src_cref)) {}
		TXScopeObjPointer<_Ty>& operator=(TXScopeObj<_Ty>* ptr) {
			return mse::us::impl::TXScopeObjPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeObjPointer<_Ty>& operator=(const TXScopeObjPointer<_Ty>& _Right_cref) {
			return mse::us::impl::TXScopeObjPointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (bool(*static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty>*>(this)));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator _Ty*() const {
			_Ty* retval = std::addressof(*(*this))/*(*static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty>*>(this))*/;
			return retval;
		}
		MSE_DEPRECATED explicit operator TXScopeObj<_Ty>*() const {
			TXScopeObj<_Ty>* retval = (*static_cast<const mse::us::impl::TXScopeObjPointerBase<_Ty>*>(this));
			return retval;
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObjNotNullPointer<_Ty>;
		friend class us::impl::TCommonizedPointer<_Ty, TXScopeObjPointer<_Ty> >;
		friend class us::impl::TCommonizedConstPointer<const _Ty, TXScopeObjPointer<_Ty> >;
	};

	/* Use TXScopeObjFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeObjConstPointer : public mse::us::impl::TXScopeObjConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjConstPointer() {}
	private:
		TXScopeObjConstPointer() : mse::us::impl::TXScopeObjConstPointerBase<_Ty>() {}
		TXScopeObjConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty>(scpobj_cref) {}
		TXScopeObjConstPointer(const TXScopeObjConstPointer& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty>(
			static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty>&>(src_cref)) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjConstPointer(const TXScopeObjConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty>(src_cref) {}
		TXScopeObjConstPointer(const TXScopeObjPointer<_Ty>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjConstPointer(const TXScopeObjPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeObjConstPointerBase<_Ty>(mse::us::impl::TXScopeItemConstPointerBase<_Ty2>(src_cref)) {}
		TXScopeObjConstPointer<_Ty>& operator=(const TXScopeObj<_Ty>* ptr) {
			return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeObjConstPointer<_Ty>& operator=(const TXScopeObjConstPointer<_Ty>& _Right_cref) {
			return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (bool(*static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty>*>(this)));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty>*>(this));
			return retval;
		}
		MSE_DEPRECATED explicit operator const TXScopeObj<_Ty>*() const {
			const TXScopeObj<_Ty>* retval = (*static_cast<const mse::us::impl::TXScopeObjConstPointerBase<_Ty>*>(this));
			return retval;
		}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObjNotNullConstPointer<_Ty>;
		friend class us::impl::TCommonizedConstPointer<const _Ty, TXScopeObjConstPointer<_Ty> >;
	};

	/* Use TXScopeObjFixedPointer instead. */
	template<typename _Ty>
	class TXScopeObjNotNullPointer : public TXScopeObjPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjNotNullPointer() {}
	private:
		TXScopeObjNotNullPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopeObjPointer<_Ty>(scpobj_ref) {}
		TXScopeObjNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopeObjPointer<_Ty>(ptr) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjNotNullPointer(const TXScopeObjNotNullPointer<_Ty2>& src_cref) : TXScopeObjPointer<_Ty>(src_cref) {}
		TXScopeObjNotNullPointer<_Ty>& operator=(const TXScopeObjPointer<_Ty>& _Right_cref) {
			TXScopeObjPointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		operator bool() const { return (*static_cast<const TXScopeObjPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator _Ty*() const { return TXScopeObjPointer<_Ty>::operator _Ty*(); }
		MSE_DEPRECATED explicit operator TXScopeObj<_Ty>*() const { return TXScopeObjPointer<_Ty>::operator TXScopeObj<_Ty>*(); }

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObjFixedPointer<_Ty>;
	};

	/* Use TXScopeObjFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeObjNotNullConstPointer : public TXScopeObjConstPointer<_Ty>, public mse::us::impl::NeverNullTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjNotNullConstPointer() {}
	private:
		TXScopeObjNotNullConstPointer(const TXScopeObjNotNullConstPointer<_Ty>& src_cref) : TXScopeObjConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjNotNullConstPointer(const TXScopeObjNotNullConstPointer<_Ty2>& src_cref) : TXScopeObjConstPointer<_Ty>(src_cref) {}
		TXScopeObjNotNullConstPointer(const TXScopeObjNotNullPointer<_Ty>& src_cref) : TXScopeObjConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjNotNullConstPointer(const TXScopeObjNotNullPointer<_Ty2>& src_cref) : TXScopeObjConstPointer<_Ty>(src_cref) {}
		TXScopeObjNotNullConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeObjConstPointer<_Ty>(scpobj_cref) {}
		operator bool() const { return (*static_cast<const TXScopeObjConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator const _Ty*() const { return TXScopeObjConstPointer<_Ty>::operator const _Ty*(); }
		MSE_DEPRECATED explicit operator const TXScopeObj<_Ty>*() const { return TXScopeObjConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObjFixedConstPointer<_Ty>;
	};

	/* A TXScopeObjFixedPointer points to a TXScopeObj. Its intended for very limited use. Basically just to pass a TXScopeObj
	by reference as a function parameter. TXScopeObjFixedPointers can be obtained from TXScopeObj's "&" (address of) operator. */
	template<typename _Ty>
	class TXScopeObjFixedPointer : public TXScopeObjNotNullPointer<_Ty> {
	public:
		TXScopeObjFixedPointer(const TXScopeObjFixedPointer& src_cref) : TXScopeObjNotNullPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjFixedPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : TXScopeObjNotNullPointer<_Ty>(src_cref) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjFixedPointer() {}
		operator bool() const { return (*static_cast<const TXScopeObjNotNullPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator _Ty*() const { return TXScopeObjNotNullPointer<_Ty>::operator _Ty*(); }
		MSE_DEPRECATED explicit operator TXScopeObj<_Ty>*() const { return TXScopeObjNotNullPointer<_Ty>::operator TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeObjFixedPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopeObjNotNullPointer<_Ty>(scpobj_ref) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		/* Disabling public move construction prevents some unsafe operations, like some, but not all,
		attempts to use a TXScopeObjFixedPointer<> as a return value. But it also prevents some safe
		operations too, like initialization via '=' (assignment operator). And the set of prevented
		operations might not be consistent across compilers. We're deciding here that the minor safety
		benefits outweigh the minor inconveniences. Note that we can't disable public move construction
		in the other scope pointers as it would interfere with implicit conversions. */
		TXScopeObjFixedPointer(TXScopeObjFixedPointer&& src_ref) : TXScopeObjNotNullPointer<_Ty>(src_ref) {
			int q = 5;
		}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		TXScopeObjFixedPointer<_Ty>& operator=(const TXScopeObjFixedPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeObj<_Ty>;
	};

	template<typename _Ty>
	class TXScopeObjFixedConstPointer : public TXScopeObjNotNullConstPointer<_Ty> {
	public:
		TXScopeObjFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty2>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
		TXScopeObjFixedConstPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeObjFixedConstPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : TXScopeObjNotNullConstPointer<_Ty>(src_cref) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObjFixedConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeObjNotNullConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator const _Ty*() const { return TXScopeObjNotNullConstPointer<_Ty>::operator const _Ty*(); }
		MSE_DEPRECATED explicit operator const TXScopeObj<_Ty>*() const { return TXScopeObjNotNullConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeObjFixedConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeObjNotNullConstPointer<_Ty>(scpobj_cref) {}
		TXScopeObjFixedConstPointer<_Ty>& operator=(const TXScopeObjFixedConstPointer<_Ty>& _Right_cref) = delete;
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
		, public MSE_FIRST_OR_PLACEHOLDER_IF_A_BASE_OF_SECOND(mse::us::impl::XScopeTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy>)
		, public MSE_FIRST_OR_PLACEHOLDER_IF_A_BASE_OF_SECOND(mse::us::impl::ReferenceableByScopePointerTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy>)
	{
	public:
		typedef mse::us::impl::TXScopeObjBase<_TROy> base_class;

		TXScopeObj(const TXScopeObj& _X) : base_class(_X) {}

#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		explicit TXScopeObj(TXScopeObj&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		MSE_SCOPE_USING(TXScopeObj, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObj() {}

		TXScopeObj& operator=(TXScopeObj&& _X) {
			//mse::impl::valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
			base_class::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		TXScopeObj& operator=(const TXScopeObj& _X) { base_class::operator=(_X); return (*this); }
		template<class _Ty2>
		TXScopeObj& operator=(_Ty2&& _X) {
			base_class::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		template<class _Ty2>
		TXScopeObj& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

		const TXScopeFixedPointer<_TROy> operator&() & {
			return TXScopeObjFixedPointer<_TROy>(*this);
		}
		const TXScopeFixedConstPointer<_TROy> operator&() const & {
			return TXScopeObjFixedConstPointer<_TROy>(*this);
		}
		const TXScopeFixedPointer<_TROy> mse_xscope_ifptr() & { return &(*this); }
		const TXScopeFixedConstPointer<_TROy> mse_xscope_ifptr() const & { return &(*this); }

		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() && {
			//return TXScopeFixedConstPointer<_TROy>(TXScopeObjFixedPointer<_TROy>(&(*static_cast<base_class*>(this))));
			return TXScopeFixedConstPointer<_TROy>(TXScopeObjFixedPointer<_TROy>(*this));
		}
		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() const && {
			return TXScopeObjFixedConstPointer<_TROy>(TXScopeObjConstPointer<_TROy>(&(*static_cast<const base_class*>(this))));
		}
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() && { return &(*this); }
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() const && { return &(*this); }

		void xscope_tag() const {}
		//void xscope_contains_accessible_scope_address_of_operator_tag() const {}
		/* This type can be safely used as a function return value if _Ty is also safely returnable. */
		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		MSE_DEFAULT_OPERATOR_NEW_DECLARATION

		template<typename _TROy2>
		friend class TXScopeOwnerPointer;
		//friend class TXScopeOwnerPointer<_TROy>;
	};

	template<typename _Ty>
	auto xscope_ifptr_to(_Ty&& _X) {
		return _X.mse_xscope_ifptr();
	}
	template<typename _Ty>
	auto xscope_ifptr_to(const _Ty& _X) {
		return _X.mse_xscope_ifptr();
	}


	namespace us {
		/* A couple of unsafe functions for internal use. */
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}


	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = TXScopeFixedPointer<typename std::remove_reference<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())>::type>;
	}

	/* While TXScopeObjFixedPointer<> points to a TXScopeObj<>, TXScopeFixedPointer<> is intended to be able to point to a
	TXScopeObj<>, any member of a TXScopeObj<>, or various other items with scope lifetime that, for various reasons, aren't
	declared as TXScopeObj<>. */
	template<typename _Ty>
	class TXScopeFixedPointer : public mse::us::impl::TXScopeItemPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
	public:
		typedef mse::us::impl::TXScopeItemPointerBase<_Ty> base_class;
		TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeFixedPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : base_class(static_cast<const base_class&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeFixedPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeFixedPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeFixedPointer(&(*src_cref)) {}

		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator _Ty*() const { return std::addressof(*(*this))/*base_class::operator _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeFixedPointer(_Ty* ptr) : base_class(ptr) {}
		TXScopeFixedPointer(const base_class& ptr) : base_class(ptr) {}
		TXScopeFixedPointer<_Ty>& operator=(const TXScopeFixedPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty2, _TMemberObjectPointer>;
		/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeObjFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);

		template<class _Ty2> friend TXScopeFixedPointer<_Ty2> us::unsafe_make_xscope_pointer_to(_Ty2& ref);
	};

	template<typename _Ty>
	class TXScopeFixedConstPointer : public mse::us::impl::TXScopeItemConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase {
	public:
		typedef mse::us::impl::TXScopeItemConstPointerBase<_Ty> base_class;
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty>& src_cref) : base_class(static_cast<const base_class&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeFixedConstPointer(const TXScopeObjFixedPointer<_Ty>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeObjFixedPointer<_Ty2>& src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeFixedConstPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeFixedConstPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {}

		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		MSE_DEPRECATED explicit operator const _Ty*() const { return std::addressof(*(*this))/*base_class::operator const _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeFixedConstPointer(const _Ty* ptr) : base_class(ptr) {}
		TXScopeFixedConstPointer(const base_class& ptr) : base_class(ptr) {}
		TXScopeFixedConstPointer<_Ty>& operator=(const TXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
		/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty2> &lease_pointer);

		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty2> &lease_pointer);
		template<class _Ty2> friend TXScopeFixedConstPointer<_Ty2> us::unsafe_make_xscope_const_pointer_to(const _Ty2& cref);
	};

	/* TXScopeCagedItemFixedPointerToRValue<> holds a TXScopeFixedPointer<> that points to an r-value which can only be
	accessed when converted to a rsv::TXScopeFixedPointerFParam<>. */
	template<typename _Ty>
	class TXScopeCagedItemFixedPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		void xscope_tag() const {}

	private:
		TXScopeCagedItemFixedPointerToRValue(const TXScopeCagedItemFixedPointerToRValue&) = delete;
		TXScopeCagedItemFixedPointerToRValue(TXScopeCagedItemFixedPointerToRValue&&) = default;
		TXScopeCagedItemFixedPointerToRValue(const TXScopeFixedPointer<_Ty>& ptr) : m_xscope_ptr(ptr) {}

		auto uncaged_pointer() const {
			return m_xscope_ptr;
		}

		TXScopeCagedItemFixedPointerToRValue<_Ty>& operator=(const TXScopeCagedItemFixedPointerToRValue<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		TXScopeFixedPointer<_Ty> m_xscope_ptr;

		friend class TXScopeObj<_Ty>;
		template<class _Ty2> friend class rsv::TXScopeFixedPointerFParam;
		template<class _Ty2> friend class rsv::TXScopeFixedConstPointerFParam;
		template<class _Ty2> friend class TXScopeStrongPointerStore;
	};

	template<typename _Ty>
	class TXScopeCagedItemFixedConstPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		void xscope_tag() const {}

	private:
		TXScopeCagedItemFixedConstPointerToRValue(const TXScopeCagedItemFixedConstPointerToRValue& src_cref) = delete;
		TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_cref) = default;
		TXScopeCagedItemFixedConstPointerToRValue(const TXScopeFixedConstPointer<_Ty>& ptr) : m_xscope_ptr(ptr) {}
#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		TXScopeCagedItemFixedConstPointerToRValue(TXScopeCagedItemFixedConstPointerToRValue&& src_ref) : m_xscope_ptr(src_ref) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		auto uncaged_pointer() const { return m_xscope_ptr; }

		TXScopeCagedItemFixedConstPointerToRValue<_Ty>& operator=(const TXScopeCagedItemFixedConstPointerToRValue<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		TXScopeFixedConstPointer<_Ty> m_xscope_ptr;

		friend class TXScopeObj<_Ty>;
		template<class _Ty2> friend class rsv::TXScopeFixedConstPointerFParam;
		template<typename _Ty2> friend auto pointer_to(_Ty2&& _X) -> decltype(&std::forward<_Ty2>(_X));
		template<class _Ty2> friend class TXScopeStrongConstPointerStore;
	};
}

namespace std {
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
	struct hash<mse::TXScopeObjFixedPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeObjFixedPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeObjFixedPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
	template<class _Ty>
	struct hash<mse::TXScopeObjFixedConstPointer<_Ty> > {	// hash functor
		typedef mse::TXScopeObjFixedConstPointer<_Ty> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeObjFixedConstPointer<_Ty>& _Keyval) const _NOEXCEPT {
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
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TNonXScopeObj() {
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

#define MSE_SCOPE_IMPL_OBJ_INHERIT_ASSIGNMENT_OPERATOR(class_name) \
		auto& operator=(class_name&& _X) { base_class::operator=(std::forward<decltype(_X)>(_X)); return (*this); } \
		auto& operator=(const class_name& _X) { base_class::operator=(_X); return (*this); } \
		template<class _Ty2> auto& operator=(_Ty2&& _X) { base_class::operator=(std::forward<decltype(_X)>(_X)); return (*this); } \
		template<class _Ty2> auto& operator=(const _Ty2& _X) { base_class::operator=(_X); return (*this); }

#define MSE_SCOPE_IMPL_OBJ_SPECIALIZATION_DEFINITIONS1(class_name) \
		class_name(const class_name&) = default; \
		class_name(class_name&&) = default; \
		MSE_SCOPE_IMPL_OBJ_INHERIT_ASSIGNMENT_OPERATOR(class_name);

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(class_name) \
			class_name(std::nullptr_t) {} \
			class_name() {}
#else // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(class_name)
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)

	/* Note that because we explicitly define some (private) constructors, default copy and move constructors
	and assignment operators won't be generated, so we have to define those as well. */
#define MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
		template<typename _Ty> \
		class TXScopeObj<specified_type> : public TXScopeObj<mapped_type> { \
		public: \
			typedef TXScopeObj<mapped_type> base_class; \
			MSE_USING(TXScopeObj, base_class); \
			MSE_SCOPE_IMPL_OBJ_SPECIALIZATION_DEFINITIONS1(TXScopeObj); \
		private: \
			MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_PRIVATE_CONSTRUCTORS1(TXScopeObj); \
		};

		/* To achieve compatibility with the us::unsafe_make_xscope_pointer() functions, these specializations make use of
		reinterpret_cast<>s in certain situations. The safety of these reinterpret_cast<>s rely on the 'mapped_type'
		being safely "reinterpretable" as a 'specified_type'. */
#define MSE_SCOPE_IMPL_PTR_SPECIALIZATION(specified_type, mapped_type) \
		template<typename _Ty> \
		class TXScopeFixedPointer<specified_type> : public mse::us::impl::TXScopeItemPointerBase<specified_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemPointerBase<specified_type> base_class; \
			TXScopeFixedPointer(const TXScopeFixedPointer<mapped_type>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default; \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeFixedPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeOwnerPointer<specified_type2> & src_cref) : TXScopeFixedPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {} \
			operator bool() const { return true; } \
			void xscope_tag() const {} \
		private: \
			TXScopeFixedPointer(specified_type * ptr) : base_class(ptr) {} \
			TXScopeFixedPointer(mapped_type * ptr) : base_class(reinterpret_cast<specified_type *>(ptr)) {} \
			TXScopeFixedPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedPointer<specified_type>& operator=(const TXScopeFixedPointer<specified_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->mse::impl::make_xscope_pointer_to_member_v2_return_type1<specified_type2, _TMemberObjectPointer>; \
			template<class specified_type2> friend TXScopeFixedPointer<specified_type2> us::unsafe_make_xscope_pointer_to(specified_type2 & ref); \
		}; \
		template<typename _Ty> \
		class TXScopeFixedConstPointer<specified_type> : public TXScopeFixedConstPointer<mapped_type> { \
		public: \
			typedef mse::us::impl::TXScopeItemConstPointerBase<specified_type> base_class; \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<mapped_type>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<specified_type>& src_cref) = default; \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<specified_type> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type>&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<specified_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemConstPointerBase<specified_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<specified_type> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type>&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<specified_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<specified_type2>&>(src_cref)) {} \
			template<class specified_type2, class = typename std::enable_if<std::is_convertible<specified_type2*, specified_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeOwnerPointer<specified_type2> & src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {} \
			operator bool() const { return true; } \
			void xscope_tag() const {} \
		private: \
			TXScopeFixedConstPointer(typename std::add_const<specified_type>::type * ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer(typename std::add_const<mapped_type>::type * ptr) : base_class(reinterpret_cast<const specified_type *>(ptr)) {} \
			TXScopeFixedConstPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer<specified_type>& operator=(const TXScopeFixedConstPointer<specified_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			template<class specified_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<specified_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */ \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedPointer<specified_type2> & lease_pointer); \
			template<class _TTargetType, class specified_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<specified_type2> & lease_pointer); \
			template<class specified_type2> friend TXScopeFixedConstPointer<specified_type2> us::unsafe_make_xscope_const_pointer_to(const specified_type2 & cref); \
		};

#define MSE_SCOPE_IMPL_PTR_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
			MSE_SCOPE_IMPL_PTR_SPECIALIZATION(specified_type, mapped_type)

#define MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type) \
		MSE_SCOPE_IMPL_PTR_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type); \
		MSE_SCOPE_IMPL_OBJ_NATIVE_POINTER_SPECIALIZATION(specified_type, mapped_type);

	MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(_Ty*, mse::us::impl::TPointerForLegacy<_Ty>);
	MSE_SCOPE_IMPL_NATIVE_POINTER_SPECIALIZATION(_Ty* const, const mse::us::impl::TPointerForLegacy<_Ty>);

#ifdef MSEPRIMITIVES_H

#define MSE_SCOPE_IMPL_OBJ_INTEGRAL_SPECIALIZATION(integral_type) \
		template<> \
		class TXScopeObj<integral_type> : public TXScopeObj<mse::TInt<integral_type>> { \
		public: \
			typedef TXScopeObj<mse::TInt<integral_type>> base_class; \
			MSE_USING(TXScopeObj, base_class); \
		};

		/* To achieve compatibility with the us::unsafe_make_xscope_pointer() functions, these specializations make use of
		reinterpret_cast<>s in certain situations. The safety of these reinterpret_cast<>s rely on 'mse::TInt<integral_type>'
		being safely "reinterpretable" as an 'integral_type'. */
#define MSE_SCOPE_IMPL_PTR_INTEGRAL_SPECIALIZATION(integral_type) \
		template<> \
		class TXScopeFixedPointer<integral_type> : public mse::us::impl::TXScopeItemPointerBase<integral_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemPointerBase<integral_type> base_class; \
			TXScopeFixedPointer(const TXScopeFixedPointer<mse::TInt<integral_type>>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) = default; \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeFixedPointer<integral_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type2>&>(src_cref)) {} \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<integral_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeObjFixedPointer<integral_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type2>&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedPointer(const TXScopeOwnerPointer<integral_type2> & src_cref) : TXScopeFixedPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {} \
			operator bool() const { return true; } \
			void xscope_tag() const {} \
		private: \
			TXScopeFixedPointer(integral_type * ptr) : base_class(ptr) {} \
			TXScopeFixedPointer(mse::TInt<integral_type> * ptr) : base_class(reinterpret_cast<integral_type *>(ptr)) {} \
			TXScopeFixedPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedPointer<integral_type>& operator=(const TXScopeFixedPointer<integral_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class integral_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<integral_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->mse::impl::make_xscope_pointer_to_member_v2_return_type1<integral_type2, _TMemberObjectPointer>; \
			template<class integral_type2> friend TXScopeFixedPointer<integral_type2> us::unsafe_make_xscope_pointer_to(integral_type2 & ref); \
		}; \
		template<> \
		class TXScopeFixedConstPointer<integral_type> : public mse::us::impl::TXScopeItemConstPointerBase<integral_type>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase, public mse::us::impl::NeverNullTagBase { \
		public: \
			typedef mse::us::impl::TXScopeItemConstPointerBase<integral_type> base_class; \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<mse::TInt<integral_type>>& src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<integral_type>& src_cref) = default; \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeFixedConstPointer<integral_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<integral_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<integral_type> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type>&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeFixedPointer<integral_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<integral_type> & src_cref) : base_class(reinterpret_cast<const base_class&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeObjFixedConstPointer<integral_type2> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemConstPointerBase<integral_type2>&>(src_cref)) {} \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<integral_type> & src_cref) : base_class(reinterpret_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type>&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeObjFixedPointer<integral_type2> & src_cref) : base_class(static_cast<const mse::us::impl::TXScopeItemPointerBase<integral_type2>&>(src_cref)) {} \
			template<class integral_type2, class = typename std::enable_if<std::is_convertible<integral_type2*, integral_type*>::value, void>::type> \
			TXScopeFixedConstPointer(const TXScopeOwnerPointer<integral_type2> & src_cref) : TXScopeFixedConstPointer(&(*src_cref)) {} \
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {} \
			operator bool() const { return true; } \
			void xscope_tag() const {} \
		private: \
			TXScopeFixedConstPointer(typename std::add_const<integral_type>::type * ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer(typename std::add_const<mse::TInt<integral_type>>::type * ptr) : base_class(reinterpret_cast<const integral_type *>(ptr)) {} \
			TXScopeFixedConstPointer(const base_class & ptr) : base_class(ptr) {} \
			TXScopeFixedConstPointer<integral_type>& operator=(const TXScopeFixedConstPointer<integral_type> & _Right_cref) = delete; \
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION; \
			template<class integral_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<integral_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			template<class integral_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<integral_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			template<class integral_type2, class _TMemberObjectPointer> \
			friend auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<integral_type2> & lease_pointer, const _TMemberObjectPointer & member_object_ptr) \
				->TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>; \
			/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */ \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<integral_type2> & lease_pointer); \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedPointer<integral_type2> & lease_pointer); \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeObjFixedConstPointer<integral_type2> & lease_pointer); \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<integral_type2> & lease_pointer); \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedPointer<integral_type2> & lease_pointer); \
			template<class _TTargetType, class integral_type2> \
			friend TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType & target, const TXScopeFixedConstPointer<integral_type2> & lease_pointer); \
			template<class integral_type2> friend TXScopeFixedConstPointer<integral_type2> us::unsafe_make_xscope_const_pointer_to(const integral_type2 & cref); \
		};

#define MSE_SCOPE_IMPL_INTEGRAL_SPECIALIZATION(integral_type) \
		MSE_SCOPE_IMPL_PTR_INTEGRAL_SPECIALIZATION(integral_type); \
		MSE_SCOPE_IMPL_OBJ_INTEGRAL_SPECIALIZATION(integral_type); \
		MSE_SCOPE_IMPL_PTR_INTEGRAL_SPECIALIZATION(typename std::add_const<integral_type>::type); \
		MSE_SCOPE_IMPL_OBJ_INTEGRAL_SPECIALIZATION(typename std::add_const<integral_type>::type);

	MSE_SCOPE_IMPL_INTEGRAL_SPECIALIZATION(int);
	MSE_SCOPE_IMPL_INTEGRAL_SPECIALIZATION(size_t);

#endif /*MSEPRIMITIVES_H*/

	/* end of template specializations */

#endif /*MSE_SCOPEPOINTER_DISABLED*/

	template<typename _Ty> using TXScopeItemFixedPointer = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TXScopeItemFixedConstPointer = TXScopeFixedConstPointer<_Ty>;

#ifdef MSE_MSTDARRAY_DISABLED

	/* When mstd::array is "disabled" it is just aliased to std::array. But since std::array (and std::vector, etc.)
	iterators are dependent types, they do not participate in overload resolution. So the xscope_pointer() overload for
	those iterators actually needs to be a "universal" (template) overload that accepts any type. The reason it needs
	to be here (rather than in the msemstdarray.h file) is that if scope pointers are disabled, then it's possible that
	both scope pointers and std::array iterators could manifest as raw pointers and so would need to be handled (and
	(heuristically) disambiguated) by the same overload implementation. */

	namespace impl {

		template<class T, class EqualTo>
		struct HasOrInheritsIteratorCategoryMemberType_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype(std::declval<typename U::iterator_category>(), std::declval<typename V::iterator_category>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct HasOrInheritsIteratorCategoryMemberType : HasOrInheritsIteratorCategoryMemberType_impl<
			typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};
		template<class _Ty>
		struct is_non_pointer_iterator : std::integral_constant<bool, HasOrInheritsIteratorCategoryMemberType<_Ty>::value> {};

	}
	template<class _Ty>
	auto xscope_const_pointer(const _Ty& param) {
		return mse::us::unsafe_make_xscope_const_pointer_to(*param);
	}
	template<class _Ty>
	auto xscope_pointer(const _Ty& param) {
		return mse::us::unsafe_make_xscope_pointer_to(*param);
	}
#endif // MSE_MSTDARRAY_DISABLED

#if defined(MSE_SCOPEPOINTER_DISABLED) && defined(MSE_MSTDARRAY_DISABLED)
#define MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS
#endif // defined(MSE_SCOPEPOINTER_DISABLED) && defined(MSE_MSTDARRAY_DISABLED)

#ifndef MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS
	/* When mstd::arrays, etc. are disabled, a "universal" overload of xscope_pointer() is provided for their iterators. 
	That overload already handles raw pointers (which may be potentially ambiguous in that situation), so we shouldn't
	provide another one. */
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
#ifndef MSE_SCOPEPOINTER_DISABLED
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeObjFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeObjFixedPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_pointer(const mse::TXScopeObjFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
	template <typename _Ty>
	auto xscope_const_pointer(const mse::TXScopeObjFixedConstPointer<_Ty>& xsptr) {
		return xsptr;
	}
#endif //!MSE_SCOPEPOINTER_DISABLED
#else // !MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS

#endif // !MSE_SCOPE_POINTERS_AND_ITERATORS_MAY_BOTH_BE_RAW_POINTERS

	namespace us {
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

			mse::TXScopeObjFixedPointer<_TTargetType> xscope_ptr() const & {
				return mse::us::unsafe_make_xscope_pointer_to(*(*this));
			}
			mse::TXScopeObjFixedPointer<_TTargetType> xscope_ptr() const && = delete;
			operator mse::TXScopeObjFixedPointer<_TTargetType>() const & {
				return xscope_ptr();
			}
			operator mse::TXScopeObjFixedPointer<_TTargetType>() const && = delete;

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

		template <class _TTargetType, class _TLeaseType>
		class TXScopeStrongFixedConstPointer : public TStrongFixedConstPointer<_TTargetType, _TLeaseType>, public mse::us::impl::XScopeTagBase {
		public:
			typedef TStrongFixedConstPointer<_TTargetType, _TLeaseType> base_class;

			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedConstPointer&) = default;
			template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType>&>(src_cref)) {}
			template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
			TXScopeStrongFixedConstPointer(const TXScopeStrongFixedPointer<_TTargetType, _TLeaseType2>& src_cref) : base_class(static_cast<const TStrongFixedConstPointer<_TTargetType, _TLeaseType2>&>(src_cref)) {}

			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedConstPointer make(const _TTargetType2& target, const _TLeaseType2& lease) {
				return base_class::make(target, lease);
			}
			template <class _TTargetType2, class _TLeaseType2>
			static TXScopeStrongFixedConstPointer make(const _TTargetType2& target, _TLeaseType2&& lease) {
				return base_class::make(target, std::forward<decltype(lease)>(lease));
			}

			mse::TXScopeObjFixedConstPointer<_TTargetType> xscope_ptr() const & {
				return mse::us::unsafe_make_xscope_const_pointer_to(*(*this));
			}
			mse::TXScopeObjFixedConstPointer<_TTargetType> xscope_ptr() const && = delete;
			operator mse::TXScopeObjFixedConstPointer<_TTargetType>() const & {
				return xscope_ptr();
			}
			operator mse::TXScopeObjFixedConstPointer<_TTargetType>() const && = delete;

		protected:
			TXScopeStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
				: base_class(target, lease) {}
			TXScopeStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeaseType&& lease)
				: base_class(target, std::forward<decltype(lease)>(lease)) {}
		private:
			TXScopeStrongFixedConstPointer(const base_class& src_cref) : base_class(src_cref) {}

			TXScopeStrongFixedConstPointer & operator=(const TXScopeStrongFixedConstPointer& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template <class _TTargetType, class _TLeaseType>
		TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType> make_xscope_const_strong(const _TTargetType& target, const _TLeaseType& lease) {
			return TXScopeStrongFixedConstPointer<_TTargetType, _TLeaseType>::make(target, lease);
		}
		template <class _TTargetType, class _TLeaseType>
		auto make_xscope_const_strong(const _TTargetType& target, _TLeaseType&& lease) -> TXScopeStrongFixedConstPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type> {
			return TXScopeStrongFixedConstPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type>::make(target, std::forward<decltype(lease)>(lease));
		}
	}
	template <class _TTargetType, class _TLeaseType>
	using TXScopeStrongFixedPointer MSE_DEPRECATED = us::TXScopeStrongFixedPointer<_TTargetType, _TLeaseType>;
	template <class _TTargetType, class _TLeaseType>
	MSE_DEPRECATED auto make_xscope_strong(_TTargetType& target, const _TLeaseType& lease) { return us::make_xscope_strong(target, lease); }
	template <class _TTargetType, class _TLeaseType>
	MSE_DEPRECATED auto make_xscope_strong(_TTargetType& target, _TLeaseType&& lease) { return us::make_xscope_strong(target, std::forward<decltype(lease)>(lease)); }


	namespace impl {
		template<typename _Ty, class... Args>
		auto make_xscope_helper(std::true_type, Args&&... args) {
			return _Ty(std::forward<Args>(args)...);
		}
		template<typename _Ty, class... Args>
		auto make_xscope_helper(std::false_type, Args&&... args) {
			return TXScopeObj<_Ty>(std::forward<Args>(args)...);
		}
	}
	template <class X, class... Args>
	auto make_xscope(Args&&... args) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), std::forward<Args>(args)...);
	}
	template <class X>
	auto make_xscope(const X& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), arg);
	}
	template <class X>
	auto make_xscope(X&& arg) {
		typedef typename std::remove_reference<X>::type nrX;
		return impl::make_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TXScopeObj>::type(), std::forward<decltype(arg)>(arg));
	}

	namespace us {
		namespace impl {
#ifdef MSE_SCOPEPOINTER_DISABLED
			template<typename _Ty> using TXScopeFixedPointerFParamBase = mse::us::impl::TPointer<_Ty>;
			template<typename _Ty> using TXScopeFixedConstPointerFParamBase = mse::us::impl::TPointer<const _Ty>;
#else /*MSE_SCOPEPOINTER_DISABLED*/
			template<typename _Ty> using TXScopeFixedPointerFParamBase = TXScopeFixedPointer<_Ty>;
			template<typename _Ty> using TXScopeFixedConstPointerFParamBase = TXScopeFixedConstPointer<_Ty>;
#endif /*MSE_SCOPEPOINTER_DISABLED*/
		}
	}

	namespace rsv {
		/* TXScopeFixedPointerFParam<> is just a version of TXScopeFixedPointer<> which may only be used for
		function parameter declations. It has the extra ability to accept (caged) scope pointers to r-value scope objects
		(i.e. supports temporaries by scope reference). */

		template<typename _Ty>
		class TXScopeFixedPointerFParam : public mse::us::impl::TXScopeFixedPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeFixedPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeFixedPointerFParam, base_class);

			TXScopeFixedPointerFParam(const TXScopeFixedPointerFParam& src_cref) = default;

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeFixedPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif //!MSE_SCOPEPOINTER_DISABLED

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}

		private:
			TXScopeFixedPointerFParam<_Ty>& operator=(const TXScopeFixedPointerFParam<_Ty>& _Right_cref) = delete;
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};

		template<typename _Ty>
		class TXScopeFixedConstPointerFParam : public mse::us::impl::TXScopeFixedConstPointerFParamBase<_Ty> {
		public:
			typedef mse::us::impl::TXScopeFixedConstPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeFixedConstPointerFParam, base_class);

			TXScopeFixedConstPointerFParam(const TXScopeFixedConstPointerFParam<_Ty>& src_cref) = default;
			TXScopeFixedConstPointerFParam(const TXScopeFixedPointerFParam<_Ty>& src_cref) : base_class(src_cref) {}

#ifndef MSE_SCOPEPOINTER_DISABLED
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(TXScopeCagedItemFixedConstPointerToRValue<_Ty2>&& src_cref) : base_class(src_cref.uncaged_pointer()) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(TXScopeCagedItemFixedPointerToRValue<_Ty2>&& src_cref) : base_class(src_cref.uncaged_pointer()) {}

#ifndef MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedConstPointerToRValue<_Ty2>& src_cref) = delete;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) = delete;
#else // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedConstPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2*, _Ty*>::value, void>::type>
			TXScopeFixedConstPointerFParam(const TXScopeCagedItemFixedPointerToRValue<_Ty2>& src_cref) : base_class(src_cref.uncaged_pointer()) {}
#endif // !MSE_TXSCOPECAGEDITEMFIXEDCONSTPOINTER_LEGACY_COMPATIBILITY1

#endif //!MSE_SCOPEPOINTER_DISABLED

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointerFParam() {}

			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}

		private:
			TXScopeFixedConstPointerFParam<_Ty>& operator=(const TXScopeFixedConstPointerFParam<_Ty>& _Right_cref) = delete;
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
		(like TXScopeFixedPointer<>, and "random access section" types) to be constructed from their
		"functionally disabled" counterparts.
		*/
		template<typename _Ty>
		class TFParam : public _Ty, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::value, mse::impl::TPlaceHolder<TFParam<_Ty> >, mse::us::impl::XScopeTagBase>::type {
		public:
			typedef _Ty base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Ty>
		TFParam(_Ty)->TFParam<_Ty>;
		template<class _Ty>
		TFParam(TFParam<_Ty>)->TFParam<_Ty>;
#endif /* MSE_HAS_CXX17 */

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
		auto as_an_fparam(const _Ty& param) -> decltype(impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param)) {
			return impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<typename std::remove_reference<_Ty>::type>::type(), param);
		}
		template<typename _Ty>
		auto as_an_fparam(_Ty&& param) {
			return impl::fparam::as_an_fparam_helper1(typename mse::impl::is_potentially_xscope<typename std::remove_reference<_Ty>::type>::type(), std::forward<_Ty>(param));
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
		class TFParam<mse::TXScopeFixedConstPointer<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<const mse::TXScopeFixedConstPointer<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<mse::TXScopeCagedItemFixedConstPointerToRValue<_Ty> > : public TXScopeFixedConstPointerFParam<_Ty> {
		public:
			typedef TXScopeFixedConstPointerFParam<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
			void xscope_not_returnable_tag() const {}
			void xscope_tag() const {}
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
#endif //!MSE_SCOPEPOINTER_DISABLED

		template<typename _Ty>
		class TFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TFParam<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TFParam<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TFParam(std::nullptr_t) {}
			TFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		/* These specializations for native arrays aren't actually meant to be used. They're just needed because when you call
		as_an_fparam() on a native array, msvc2017 will try to instantiate a TFParam<> with the native array even though it is
		determined at compile that it will never be used. clang6, for example, doesn't have the same issue. */
		template<typename _Ty, size_t _Size>
		class TFParam<const _Ty[_Size]> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(const _Ty(&param)[_Size]) : base_class(param) {}
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
		template<typename _Ty, size_t _Size>
		class TFParam<_Ty[_Size]> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam);
			TFParam(_Ty(&param)[_Size]) : base_class(param) {}
		private:
			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};


		/* Some forward declarations needed to support "cyclic friending". */
		template<typename _Ty>
		class TReturnableFParam;

		namespace impl {
			namespace returnable_fparam {
				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, const _Ty& param)->TReturnableFParam<typename std::remove_reference<_Ty>::type>;

				template<typename _Ty>
				auto as_a_returnable_fparam_helper1(std::true_type, _Ty&& param)->TReturnableFParam<typename std::remove_reference<_Ty>::type>;
			}
		}

		/* rsv::TReturnableFParam<> is just a transparent template wrapper for function parameter declarations. Like
		us::FParam<>, in most cases use of this wrapper is not necessary, but in some cases it enables functionality
		only available to variables that are function parameters. Specifically, rsv::TReturnableFParam<> "marks"
		scope pointer/reference parameters as safe to use as the return value of the function, whereas by default,
		scope pointer/references are not considered safe to use as a return value. Note that unlike us::FParam<>,
		rsv::TReturnableFParam<> does not enable the function to accept scope pointer/reference temporaries.
		*/
		template<typename _Ty>
		class TReturnableFParam : public _Ty, public std::conditional<std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::value, mse::impl::TPlaceHolder<TReturnableFParam<_Ty> >, mse::us::impl::XScopeTagBase>::type {
		public:
			typedef _Ty base_class;
			typedef _Ty returnable_fparam_contained_type;
			MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam);

			void returnable_once_tag() const {}
			void xscope_returnable_tag() const {}

		private:
			MSE_USING(TReturnableFParam, base_class);

			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);

			template<typename _Ty2>
			friend auto impl::returnable_fparam::as_a_returnable_fparam_helper1(std::true_type, const _Ty2& param)->TReturnableFParam<typename std::remove_reference<_Ty2>::type>;
			template<typename _Ty2>
			friend auto impl::returnable_fparam::as_a_returnable_fparam_helper1(std::true_type, _Ty2&& param)->TReturnableFParam<typename std::remove_reference<_Ty2>::type>;
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Ty>
		TReturnableFParam(_Ty)->TReturnableFParam<_Ty>;
		template<class _Ty>
		TReturnableFParam(TReturnableFParam<_Ty>)->TReturnableFParam<_Ty>;
#endif /* MSE_HAS_CXX17 */

		template<typename _Ty> using TXScopeReturnableFParam = TReturnableFParam<_Ty>;


		template<typename _Ty>
		auto returnable_fparam_as_base_type(TReturnableFParam<_Ty>&& _X) {
			return std::forward<_Ty>(_X);
		}
		template<typename _Ty>
		auto returnable_fparam_as_base_type(const TReturnableFParam<_Ty>& _X) -> const typename TReturnableFParam<_Ty>::base_class & {
			return static_cast<const typename TReturnableFParam<_Ty>::base_class&>(_X);
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
		auto as_a_returnable_fparam(const _Ty& param)
			-> decltype(impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param)) {
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), param);
		}
		template<typename _Ty>
		auto as_a_returnable_fparam(_Ty&& param) {
			return impl::returnable_fparam::as_a_returnable_fparam_helper1(typename mse::impl::is_potentially_xscope<_Ty>::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(const _Ty& param) -> decltype(as_a_returnable_fparam(param)) {
			return as_a_returnable_fparam(param);
		}
		template<typename _Ty>
		auto xscope_as_a_returnable_fparam(_Ty&& param) {
			return as_a_returnable_fparam(std::forward<_Ty>(param));
		}

#define MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_function) \
		template <typename _Ty, class... _Args> \
		auto make_xscope_function(const rsv::TReturnableFParam<_Ty>& param, _Args&&... _Ax) \
			-> decltype(mse::rsv::as_a_returnable_fparam(make_xscope_function(std::declval<const _Ty&>(), std::forward<_Args>(_Ax)...))) { \
			const _Ty& param_base_ref = param; \
			return mse::rsv::as_a_returnable_fparam(make_xscope_function(param_base_ref, std::forward<_Args>(_Ax)...)); \
		} \
		template <typename _Ty, class... _Args> \
		auto make_xscope_function(rsv::TReturnableFParam<_Ty>&& param, _Args&&... _Ax) { \
			return mse::rsv::as_a_returnable_fparam(make_xscope_function(std::forward<_Ty>(param), std::forward<_Args>(_Ax)...)); \
		}

		MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(xscope_pointer)

		/* Template specializations of TReturnableFParam<>. */

		template<typename _Ty>
		class TReturnableFParam<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TReturnableFParam<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TReturnableFParam<_Ty* const> : public mse::us::impl::TPointerForLegacy<_Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
		template<typename _Ty>
		class TReturnableFParam<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty>, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase, public mse::us::impl::XScopeTagBase {
		public:
			typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TReturnableFParam, base_class);

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
			void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			TReturnableFParam(std::nullptr_t) {}
			TReturnableFParam() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
	}

	/* If a rsv::TReturnableFParam<> wrapped reference is used to make a pointer to a member of its target object, then the
	created pointer to member can inherit the "returnability" of the original wrapped reference. */
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_pointer_to_member_v2)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_const_pointer_to_member_v2)

	template<class _Ty>
	auto xscope_pointer(const rsv::TReturnableFParam<_Ty>& lone_param) {
		const _Ty& lone_param_base_ref = lone_param;
		typedef decltype(xscope_pointer(lone_param_base_ref)) base_return_type;
		return rsv::TReturnableFParam<base_return_type>(xscope_pointer(lone_param_base_ref));
	}
	template<class _Ty>
	auto xscope_const_pointer(const rsv::TReturnableFParam<_Ty>& lone_param) {
		const _Ty& lone_param_base_ref = lone_param;
		typedef decltype(xscope_const_pointer(lone_param_base_ref)) base_return_type;
		return rsv::TReturnableFParam<base_return_type>(xscope_const_pointer(lone_param_base_ref));
	}

	template<typename _TROy>
	class TReturnValue : public _TROy {
	public:
		typedef _TROy base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, _TROy);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
			/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
			valid_if_TROy_is_marked_as_returnable_or_not_xscope_type();
		}

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		/* If _TROy is not recognized as safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class = typename std::enable_if<(mse::impl::is_potentially_not_xscope<_TROy>::value)
			|| (mse::impl::potentially_does_not_contain_non_owning_scope_reference<_TROy>::value)
			|| (true
				/*&& (std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_TROy>::Has>())*/
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/
				), void>::type>
		void valid_if_TROy_is_marked_as_returnable_or_not_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _TROy>
	class TXScopeReturnValue : public TReturnValue<_TROy>
		, public MSE_FIRST_OR_PLACEHOLDER_IF_A_BASE_OF_SECOND(mse::us::impl::XScopeTagBase, TReturnValue<_TROy>, TXScopeReturnValue<_TROy>)
	{
	public:
		typedef TReturnValue<_TROy> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TXScopeReturnValue, base_class);

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	namespace impl {
		template<typename _Ty>
		static void z__returnable_noop(const _Ty&) {}

		template<typename _Ty>
		const auto& return_value_helper12(const _Ty& _X) {
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
			impl::is_instantiation_of<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of<_Ty_noref, rsv::TXScopeReturnableFParam>::value
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
			impl::is_instantiation_of<_Ty_noref, rsv::TReturnableFParam>::value
			|| impl::is_instantiation_of<_Ty_noref, rsv::TXScopeReturnableFParam>::value
			, std::true_type, std::false_type>::type(), std::forward<decltype(_X)>(_X));
	}

	/* Template specializations of TReturnValue<>. */

	template<typename _Ty>
	class TReturnValue<_Ty*> : public mse::us::impl::TPointerForLegacy<_Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
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
	class TReturnValue<const _Ty*> : public mse::us::impl::TPointerForLegacy<const _Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
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
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
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
	class TReturnValue<const _Ty* const> : public mse::us::impl::TPointerForLegacy<const _Ty> {
	public:
		typedef mse::us::impl::TPointerForLegacy<const _Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TReturnValue() {
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
	template<typename _TROy> using TReturnable MSE_DEPRECATED = TReturnValue<_TROy>;
	template<typename _TROy> using TXScopeReturnable MSE_DEPRECATED = TXScopeReturnValue<_TROy>;
	template<typename _Ty> MSE_DEPRECATED const auto& returnable(const _Ty& _X) { return return_value(_X); }
	template<typename _Ty> MSE_DEPRECATED auto&& returnable(_Ty&& _X) { return return_value(std::forward<decltype(_X)>(_X)); }


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
	class TXScopeOwnerPointer : public mse::us::impl::XScopeTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase
		, mse::us::impl::ReferenceableByScopePointerTagBase
		, MSE_FIRST_OR_PLACEHOLDER_IF_NOT_A_BASE_OF_SECOND(mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty, TXScopeOwnerPointer<_Ty>)
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
		explicit operator const _Ty*() const {
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
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
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
#ifdef MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY
			/* You would probably only consider defining MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY for extra safety if for
			some reason you couldn't rely on the availability of a tool (like scpptool) to statically enforce the safety of
			these moves. */
#ifdef MSE_HAS_CXX17
			static_assert(false, "The MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY preprocessor symbol is defined, which prohibits the use of TXScopeOwnerPointer<>'s move constructor. ");
#endif // MSE_HAS_CXX17
#endif // MSE_RESTRICT_TXSCOPEOWNERPOINTER_MOVABILITY
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
		TXScopeFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref) {
			return TXScopeFixedPointer<_Ty>(std::addressof(ref));
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref) {
			return TXScopeFixedConstPointer<_Ty>(std::addressof(cref));
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> unsafe_make_xscope_pointer_to(const _Ty& cref) {
			return unsafe_make_xscope_const_pointer_to(cref);
		}
	}

	namespace rsv {
		/* Obtain a scope pointer to any object. Requires static verification. */
		template<typename _Ty>
		TXScopeFixedPointer<_Ty> make_xscope_pointer_to(_Ty& ref) {
			return mse::us::unsafe_make_xscope_pointer_to(ref);
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> make_xscope_const_pointer_to(const _Ty& cref) {
			return mse::us::unsafe_make_xscope_const_pointer_to(cref);
		}
		template<typename _Ty>
		TXScopeFixedConstPointer<_Ty> make_xscope_pointer_to(const _Ty& cref) {
			return make_xscope_const_pointer_to(cref);
		}
	}

	namespace us {
		template<typename _TROy>
		class TXScopeUserDeclaredReturnable : public _TROy {
		public:
			MSE_USING(TXScopeUserDeclaredReturnable, _TROy);
			TXScopeUserDeclaredReturnable(const TXScopeUserDeclaredReturnable& _X) : _TROy(_X) {}
			TXScopeUserDeclaredReturnable(TXScopeUserDeclaredReturnable&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeUserDeclaredReturnable() {
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
			template<class = typename std::enable_if<(mse::impl::potentially_does_not_contain_non_owning_scope_reference<_TROy>::value)
				/*&& (!std::integral_constant<bool, mse::impl::HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/, void>::type>
				void valid_if_TROy_is_not_marked_as_unreturn_value() const {}

			template<class = typename std::enable_if<mse::impl::is_potentially_xscope<_TROy>::value, void>::type>
			void valid_if_TROy_is_an_xscope_type() const {}

			MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
		};
	}


	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeObjFixedPointer<_Ty> &lease_pointer) {
		return TXScopeFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeFixedConstPointer<_TTargetType>(std::addressof(target));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifndef MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeObjFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeObjFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifdef MSE_SCOPEPOINTER_DISABLED
	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = TXScopeFixedPointer<typename std::remove_reference<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())>::type>;
	}
#endif // MSE_SCOPEPOINTER_DISABLED

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemPointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeFixedPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeFixedPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(TXScopeFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(TXScopeFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(TXScopeFixedPointer<_Ty>(lease_pointer), member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(TXScopeFixedConstPointer<_Ty>(lease_pointer), member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

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
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_pointer_to_member_v2(const TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeObjFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_const_pointer_to_member_v2(const TXScopeObjFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		return make_xscope_const_pointer_to_member_v2(lease_pointer, member_object_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#endif // !MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER


	/* TXScopeStrongPointerStore et al are types that store a strong pointer (like a refcounting pointer), and let you
	obtain a corresponding scope pointer. */
	template<typename _TStrongPointer>
	class TXScopeStrongPointerStore : public mse::us::impl::XScopeTagBase
		, public MSE_FIRST_OR_PLACEHOLDER_IF_NOT_A_BASE_OF_SECOND(mse::us::impl::ContainsNonOwningScopeReferenceTagBase, typename std::remove_reference<_TStrongPointer>::type, TXScopeStrongPointerStore<typename std::remove_reference<_TStrongPointer>::type>)
	{
	private:
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongPointerStore _Myt;
		typedef typename std::remove_reference<decltype(*m_stored_ptr)>::type target_t;

		TXScopeStrongPointerStore(const TXScopeStrongPointerStore&) = delete;
		TXScopeStrongPointerStore(TXScopeStrongPointerStore&&) = default;

		TXScopeStrongPointerStore(_TStrongPointerNR&& stored_ptr) : m_stored_ptr(std::forward<_TStrongPointerNR>(stored_ptr)) {
			*m_stored_ptr; /* Just verifying that stored_ptr points to a valid target. */
		}
		TXScopeStrongPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {
			*stored_ptr; /* Just verifying that stored_ptr points to a valid target. */
		}
		~TXScopeStrongPointerStore() {
			mse::impl::is_valid_if_strong_pointer<_TStrongPointerNR>::no_op();
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		auto xscope_ptr() const && {
			return mse::TXScopeCagedItemFixedPointerToRValue<target_t>(mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr));
		}
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, class = typename std::enable_if<!std::is_same<mse::TXScopeFixedConstPointer<target_t2>, mse::TXScopeFixedPointer<target_t> >::value, void>::type>
		explicit operator mse::TXScopeFixedConstPointer<target_t2>() const & {
			return m_stored_ptr;
		}
		*/
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
	};

	template<typename _TStrongPointer>
	class TXScopeStrongConstPointerStore : public mse::us::impl::XScopeTagBase
		, public MSE_FIRST_OR_PLACEHOLDER_IF_NOT_A_BASE_OF_SECOND(mse::us::impl::ContainsNonOwningScopeReferenceTagBase, typename std::remove_reference<_TStrongPointer>::type, TXScopeStrongConstPointerStore<typename std::remove_reference<_TStrongPointer>::type>)
	{
	private:
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;
		static void dummy_foo(const decltype(*std::declval<_TStrongPointerNR>())&) {}

	public:
		typedef TXScopeStrongConstPointerStore _Myt;
		typedef typename std::remove_reference<decltype(*m_stored_ptr)>::type target_t;

		TXScopeStrongConstPointerStore(const TXScopeStrongConstPointerStore&) = delete;
		TXScopeStrongConstPointerStore(TXScopeStrongConstPointerStore&&) = default;

		TXScopeStrongConstPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {
			dummy_foo(*stored_ptr); /* Just verifying that stored_ptr points to a valid target. */
		}
		~TXScopeStrongConstPointerStore() {
			mse::impl::is_valid_if_strong_pointer<_TStrongPointerNR>::no_op();
		}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		auto xscope_ptr() const && {
			return mse::TXScopeCagedItemFixedConstPointerToRValue<typename std::remove_const<typename std::remove_reference<decltype(*m_stored_ptr)>::type>::type>(mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr));
		}
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedConstPointer<target_t>() const & {
			return m_stored_ptr;
		}
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<typename std::remove_reference<_TStrongPointer>::type> >
	class TXScopeStrongNotNullPointerStore : public mse::us::impl::XScopeTagBase
		, public MSE_FIRST_OR_PLACEHOLDER_IF_NOT_A_BASE_OF_SECOND(mse::us::impl::ContainsNonOwningScopeReferenceTagBase, typename std::remove_reference<_TStrongPointer>::type, TXScopeStrongNotNullPointerStore<typename std::remove_reference<_TStrongPointer>::type>)
	{
	private:
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongNotNullPointerStore _Myt;
		typedef typename std::remove_reference<decltype(*m_stored_ptr)>::type target_t;

		TXScopeStrongNotNullPointerStore(const TXScopeStrongNotNullPointerStore&) = delete;
		TXScopeStrongNotNullPointerStore(TXScopeStrongNotNullPointerStore&&) = default;

		TXScopeStrongNotNullPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, class = typename std::enable_if<!std::is_same<mse::TXScopeFixedConstPointer<target_t2>, mse::TXScopeFixedPointer<target_t> >::value, void>::type>
		explicit operator mse::TXScopeFixedConstPointer<target_t2>() const & {
			return m_stored_ptr;
		}
		*/
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
	};

	template<typename _TStrongPointer, class = mse::impl::is_valid_if_strong_and_never_null_pointer<typename std::remove_reference<_TStrongPointer>::type> >
	class TXScopeStrongNotNullConstPointerStore : public mse::us::impl::XScopeTagBase
		, public MSE_FIRST_OR_PLACEHOLDER_IF_NOT_A_BASE_OF_SECOND(mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _TStrongPointer, TXScopeStrongNotNullConstPointerStore<typename std::remove_reference<_TStrongPointer>::type>)
	{
	private:
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		_TStrongPointerNR m_stored_ptr;

	public:
		typedef TXScopeStrongNotNullConstPointerStore _Myt;
		typedef typename std::remove_reference<decltype(*m_stored_ptr)>::type target_t;

		TXScopeStrongNotNullConstPointerStore(const TXScopeStrongNotNullConstPointerStore&) = delete;
		TXScopeStrongNotNullConstPointerStore(TXScopeStrongNotNullConstPointerStore&&) = default;

		TXScopeStrongNotNullConstPointerStore(const _TStrongPointerNR& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointerNR& stored_ptr() const { return m_stored_ptr; }

		operator mse::TXScopeFixedConstPointer<target_t>() const & {
			return m_stored_ptr;
		}
		auto& operator*() const {
			return *m_stored_ptr;
		}
		auto* operator->() const {
			return std::addressof(*m_stored_ptr);
		}
		bool operator==(const _Myt& rhs) const {
			return (rhs.m_stored_ptr == m_stored_ptr);
		}

		void async_not_shareable_and_not_passable_tag() const {}
		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointerNR, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointerNR>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
	};

	namespace impl {
		namespace ns_xscope_strong_pointer_store {

			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::true_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongNotNullConstPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::true_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongNotNullConstPointerStore<_TStrongPointer>(std::forward<decltype(stored_ptr)>(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::false_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongConstPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_const_pointer_store_helper1(std::false_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongConstPointerStore<_TStrongPointer>(std::forward<decltype(stored_ptr)>(stored_ptr));
			}

		}
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_const_pointer_store(const _TStrongPointer& stored_ptr) {
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), stored_ptr);
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_const_pointer_store(_TStrongPointer&& stored_ptr) {
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_const_pointer_store_helper1<_TStrongPointerNR>(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), std::forward<decltype(stored_ptr)>(stored_ptr));
	}

	namespace impl {
		namespace ns_xscope_strong_pointer_store {

			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::true_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongNotNullPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::true_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongNotNullPointerStore<_TStrongPointer>(std::forward<decltype(stored_ptr)>(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::false_type, const _TStrongPointer& stored_ptr) {
				return TXScopeStrongPointerStore<_TStrongPointer>(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper1(std::false_type, _TStrongPointer&& stored_ptr) {
				return TXScopeStrongPointerStore<_TStrongPointer>(std::forward<decltype(stored_ptr)>(stored_ptr));
			}

			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::true_type, const _TStrongPointer& stored_ptr) {
				return make_xscope_strong_const_pointer_store(stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::true_type, _TStrongPointer&& stored_ptr) {
				return make_xscope_strong_const_pointer_store(std::forward<decltype(stored_ptr)>(stored_ptr));
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::false_type, const _TStrongPointer& stored_ptr) {
				typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
				return make_xscope_strong_pointer_store_helper1(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), stored_ptr);
			}
			template<typename _TStrongPointer>
			auto make_xscope_strong_pointer_store_helper2(std::false_type, _TStrongPointer&& stored_ptr) {
				typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
				return make_xscope_strong_pointer_store_helper1(typename std::is_base_of<mse::us::impl::NeverNullTagBase, _TStrongPointerNR>::type(), std::forward<decltype(stored_ptr)>(stored_ptr));
			}
		}
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_pointer_store(const _TStrongPointer& stored_ptr) {
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		typedef typename std::remove_reference<decltype(*stored_ptr)>::type _TTargetNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_pointer_store_helper2<_TStrongPointerNR>(typename std::is_const<_TTargetNR>::type(), stored_ptr);
	}
	template<typename _TStrongPointer>
	auto make_xscope_strong_pointer_store(_TStrongPointer&& stored_ptr) {
		typedef typename std::remove_reference<_TStrongPointer>::type _TStrongPointerNR;
		typedef typename std::remove_reference<decltype(*stored_ptr)>::type _TTargetNR;
		return impl::ns_xscope_strong_pointer_store::make_xscope_strong_pointer_store_helper2<_TStrongPointerNR>(typename std::is_const<_TTargetNR>::type(), std::forward<decltype(stored_ptr)>(stored_ptr));
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_strong_const_pointer_store)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_strong_pointer_store)

	template<typename _Ty> using TXScopeXScopeItemFixedStore = TXScopeStrongNotNullPointerStore<TXScopeFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeItemFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeFixedConstPointer<_Ty> >;
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty> using TXScopeXScopeFixedStore = TXScopeStrongNotNullPointerStore<TXScopeObjFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeObjFixedConstPointer<_Ty> >;
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	namespace rsv {
		namespace impl {
			template<typename _Ty>
			class TContainsNonOwningScopeReferenceWrapper : public _Ty, public mse::us::impl::ContainsNonOwningScopeReferenceTagBase {
				typedef _Ty base_class;
			public:
				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TContainsNonOwningScopeReferenceWrapper, _Ty);
				MSE_IMPL_DESTRUCTOR_PREFIX1 ~TContainsNonOwningScopeReferenceWrapper() {}
			};
		}
		template<typename _TLambda>
		auto make_xscope_reference_or_pointer_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(mse::rsv::impl::TContainsNonOwningScopeReferenceWrapper<_TLambda>(lambda));
		}
		template<typename _TLambda>
		auto make_xscope_non_reference_or_pointer_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(lambda);
		}
		template<typename _TLambda>
		auto make_xscope_capture_lambda(const _TLambda& lambda) {
			return make_xscope_non_reference_or_pointer_capture_lambda(lambda);
		}
		template<typename _TLambda>
		auto make_xscope_non_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(lambda);
		}
	}

	namespace us {
		namespace impl {
			/* The new() operator of scope objects is (often) private. The implementation of some elements (like those that
			use std::any<> or std::function<> type-erasure) may require access to the new() operator. This is just a
			transparent wrapper that doesn't "hide" its new() operator and can be used to wrap scope objects that do. */
			template<typename _TROy>
			class TNewableXScopeObj : public _TROy {
			public:
				typedef _TROy base_class;
				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TNewableXScopeObj, base_class);

				MSE_DEFAULT_OPERATOR_NEW_DECLARATION
			};
			namespace impl {
				template<typename _Ty>
				auto make_newable_xscope_helper(std::false_type, const _Ty& arg) {
					/* Objects that don't derive from mse::us::impl::XScopeTagBase generally don't hide their new() operators
					and may not be usable as a base class. */
					return arg;
				}
				template<typename _Ty>
				auto make_newable_xscope_helper(std::false_type, _Ty&& arg) {
					/* Objects that don't derive from mse::us::impl::XScopeTagBase generally don't hide their new() operators
					and may not be usable as a base class. */
					return std::forward<decltype(arg)>(arg);
				}

				template<typename _Ty>
				auto make_newable_xscope_helper(std::true_type, const _Ty& arg) {
					return TNewableXScopeObj<_Ty>(arg);
				}
				template<typename _Ty>
				auto make_newable_xscope_helper(std::true_type, _Ty&& arg) {
					return TNewableXScopeObj<_Ty>(std::forward<decltype(arg)>(arg));
				}
			}
			template <class X>
			auto make_newable_xscope(const X& arg) {
				typedef typename std::remove_reference<X>::type nrX;
				return impl::make_newable_xscope_helper<nrX>(typename std::is_base_of<mse::us::impl::XScopeTagBase, nrX>::type(), arg);
			}
			template <class X>
			auto make_newable_xscope(X&& arg) {
				typedef typename std::remove_reference<X>::type nrX;
				return impl::make_newable_xscope_helper<nrX>(typename mse::impl::is_instantiation_of<nrX, TNewableXScopeObj>::type(), std::forward<decltype(arg)>(arg));
			}
		}
	}


	/* The purpose of the xscope_chosen_pointer() function is simply to take two scope pointers as input parameters and return (a copy
	of) one of them. Which of the pointers is returned is determined by a "decider" function that is passed, as the first parameter, to
	xscope_chosen_pointer(). The "decider" function needs to return a bool and take the two scope pointers as its first two parameters.
	The reason this xscope_chosen_pointer() function is needed is that (non-owning) scope pointers are, in general, not allowed to be
	used as a function return value. (Because you might accidentally return a pointer to a local scope object (which is bad)
	instead of one of the pointers given as an input parameter (which is fine).) So the xscope_chosen_pointer() template is the
	sanctioned way of creating a function that returns a non-owning scope pointer. */
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeObjFixedConstPointer<_Ty>& a, const TXScopeObjFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeObjFixedPointer<_Ty>& a, const TXScopeObjFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeObjFixedConstPointer<_Ty>& a, const TXScopeObjFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeObjFixedPointer<_Ty>& a, const TXScopeObjFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	/* Just the generalization of xscope_chosen_pointer(). */
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto chosen(const _TBoolFunction& function1, const _Ty& a, const _Ty& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen(const _TBoolFunction& function1, const _Ty& a, const _Ty& b, Args&&... args) {
		return chosen(function1, a, b, std::forward<Args>(args)...);
	}
	template<typename _Ty>
	auto chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
		return chosen(choose_the_second, a, b);
	}

	/* shorter aliases */
	template<typename _Ty> using sfp = TXScopeObjFixedPointer<_Ty>;
	template<typename _Ty> using sfcp = TXScopeObjFixedConstPointer<_Ty>;
	template<typename _TROy> using so = TXScopeObj<_TROy>;
	template<typename _Ty> using sifp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using sifcp = TXScopeFixedConstPointer<_Ty>;

	/* deprecated aliases */
	template<typename _Ty> using scpfp MSE_DEPRECATED = TXScopeObjFixedPointer<_Ty>;
	template<typename _Ty> using scpfcp MSE_DEPRECATED = TXScopeObjFixedConstPointer<_Ty>;
	template<typename _TROy> using scpo MSE_DEPRECATED = TXScopeObj<_TROy>;
	template<class _TTargetType, class _TXScopeObjPointerType> using scpwkfp MSE_DEPRECATED = TSyncWeakFixedPointer<_TTargetType, _TXScopeObjPointerType>;
	template<class _TTargetType, class _TXScopeObjPointerType> using scpwkfcp MSE_DEPRECATED = TSyncWeakFixedConstPointer<_TTargetType, _TXScopeObjPointerType>;

	template<typename _Ty> using TScopeFixedPointer MSE_DEPRECATED = TXScopeObjFixedPointer<_Ty>;
	template<typename _Ty> using TScopeFixedConstPointer MSE_DEPRECATED = TXScopeObjFixedConstPointer<_Ty>;
	template<typename _TROy> using TScopeObj MSE_DEPRECATED = TXScopeObj<_TROy>;
	template<typename _Ty> using TScopeOwnerPointer MSE_DEPRECATED = TXScopeOwnerPointer<_Ty>;


	namespace self_test {
		class CXScpPtrTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				class A {
				public:
					A(int x) : b(x) {}
					A(const A& _X) : b(_X.b) {}
					A(A&& _X) : b(std::forward<decltype(_X)>(_X).b) {}
					virtual ~A() {}
					A& operator=(A&& _X) { b = std::forward<decltype(_X)>(_X).b; return (*this); }
					A& operator=(const A& _X) { b = _X.b; return (*this); }

					int b = 3;
				};
				class B {
				public:
					static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
					static int foo2(mse::TXScopeFixedPointer<A> A_scope_ptr) { return A_scope_ptr->b; }
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

					mse::TXScopeFixedPointer<A> A_scope_ptr1(&scope_a);
					assert(A_native_ptr->b == A_scope_ptr1->b);
					mse::TXScopeFixedPointer<A> A_scope_ptr2 = &scope_a;

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
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeFixedPointer<E> E_scope_ifptr5 = GE_scope_ifptr1;
					mse::TXScopeFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
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
						static int foo2(mse::TXScopeFixedPointer<A> A_scpfptr) { return A_scpfptr->b; }
						static int foo3(mse::TXScopeFixedConstPointer<A> A_scpfcptr) { return A_scpfcptr->b; }
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
					int a(7);
					mse::TXScopeObj<int> scope_a(7);
					/* Use of scalar types (that can't be used as base class types) with TXScopeObj<> is not well supported. So,
					for example, rather than mse::TXScopeObj<int>, mse::TXScopeObj<mse::CInt> would be preferred. */

					auto int_native_ptr = &a;

					mse::TXScopeFixedPointer<int> int_scope_ptr1 = &scope_a;
					mse::TXScopeFixedPointer<int> int_scope_ptr2 = &scope_a;

					if (!int_scope_ptr2) {
						assert(false);
					}
					else if (!(int_scope_ptr2 != int_scope_ptr1)) {
						int q = 5;
					}
					else {
						assert(false);
					}

					int a2 = a;
					mse::TXScopeObj<int> scope_a2 = scope_a;
					scope_a2 = a;
					scope_a2 = scope_a;

					mse::TXScopeFixedConstPointer<int> rcp = int_scope_ptr1;
					mse::TXScopeFixedConstPointer<int> rcp2 = rcp;
					const mse::TXScopeObj<int> cscope_a(11);
					mse::TXScopeFixedConstPointer<int> rfcp = &cscope_a;

					mse::TXScopeOwnerPointer<int> int_scpoptr(11);
					auto int_scpptr = &*int_scpoptr;
				}

				{
					/* Polymorphic conversions. */
					class E {
					public:
						int m_b = 5;
					};

					/* Polymorphic conversions that would not be supported by mse::TRegisteredPointer. */
					class GE : public E {};
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeFixedPointer<E> E_scope_ptr5(GE_scope_ifptr1);
					mse::TXScopeFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
				}

#endif // MSE_SELF_TESTS
			}
		};
	}
}

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("_NOEXCEPT")
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

#endif // MSESCOPE_H_
