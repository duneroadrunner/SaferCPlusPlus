
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

	namespace us {
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
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
			class TXScopePointerBase : public mse::TNDNoradPointer<_Ty> {
			public:
				typedef mse::TNDNoradPointer<_Ty> base_class;
				TXScopePointerBase(const TXScopePointerBase&) = default;
				TXScopePointerBase(TXScopePointerBase&&) = default;
				TXScopePointerBase(TXScopeObj<_Ty>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				TXScopeObj<_Ty>& operator*() const {
					return static_cast<TXScopeObj<_Ty>& >(*(static_cast<const base_class&>(*this)));
				}
				TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty>
			class TXScopeConstPointerBase : public mse::TNDNoradConstPointer<_Ty> {
			public:
				typedef mse::TNDNoradConstPointer<_Ty> base_class;
				TXScopeConstPointerBase(const TXScopeConstPointerBase&) = default;
				TXScopeConstPointerBase(TXScopeConstPointerBase&&) = default;
				TXScopeConstPointerBase(const TXScopePointerBase<_Ty>& src_cref) : base_class(src_cref) {}
				TXScopeConstPointerBase(const TXScopeObj<_Ty>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

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
			class TXScopePointerBase : public mse::us::impl::TPointerForLegacy<TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> {
			public:
				typedef mse::us::impl::TPointerForLegacy<TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> base_class;
				TXScopePointerBase(const TXScopePointerBase&) = default;
				TXScopePointerBase(TXScopePointerBase&&) = default;
				TXScopePointerBase(TXScopeObj<_Ty>& scpobj_ref) : base_class(&(static_cast<TXScopeObjBase<_Ty>&>(scpobj_ref))) {}

				TXScopeObj<_Ty>& operator*() const {
					return static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this)));
				}
				TXScopeObj<_Ty>* operator->() const {
					return std::addressof(static_cast<TXScopeObj<_Ty>&>(*(static_cast<const base_class&>(*this))));
				}
			};

			template<typename _Ty>
			class TXScopeConstPointerBase : public mse::us::impl::TPointerForLegacy<const TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> {
			public:
				typedef mse::us::impl::TPointerForLegacy<const TXScopeObjBase<_Ty>, mse::impl::TScopeID<const _Ty>> base_class;
				TXScopeConstPointerBase(const TXScopeConstPointerBase&) = default;
				TXScopeConstPointerBase(TXScopeConstPointerBase&&) = default;
				TXScopeConstPointerBase(const TXScopePointerBase<_Ty>& src_cref) : base_class(src_cref) {}
				TXScopeConstPointerBase(const TXScopeObj<_Ty>& scpobj_cref) : base_class(&(static_cast<const TXScopeObjBase<_Ty>&>(scpobj_cref))) {}

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
	class TXScopePointer : public mse::us::impl::TXScopePointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopePointer() {}
	private:
		TXScopePointer() : mse::us::impl::TXScopePointerBase<_Ty>() {}
		TXScopePointer(TXScopeObj<_Ty>& scpobj_ref) : mse::us::impl::TXScopePointerBase<_Ty>(scpobj_ref) {}
		TXScopePointer(const TXScopePointer& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(
			static_cast<const mse::us::impl::TXScopePointerBase<_Ty>&>(src_cref)) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopePointer(const TXScopePointer<_Ty2>& src_cref) : mse::us::impl::TXScopePointerBase<_Ty>(mse::us::impl::TXScopePointerBase<_Ty2>(src_cref)) {}
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
	class TXScopeConstPointer : public mse::us::impl::TXScopeConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeConstPointer() {}
	private:
		TXScopeConstPointer() : mse::us::impl::TXScopeConstPointerBase<_Ty>() {}
		TXScopeConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(scpobj_cref) {}
		TXScopeConstPointer(const TXScopeConstPointer& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(
			static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty>&>(src_cref)) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeConstPointer(const TXScopeConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(src_cref) {}
		TXScopeConstPointer(const TXScopePointer<_Ty>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeConstPointer(const TXScopePointer<_Ty2>& src_cref) : mse::us::impl::TXScopeConstPointerBase<_Ty>(mse::us::impl::TXScopeItemConstPointerBase<_Ty2>(src_cref)) {}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeObj<_Ty>* ptr) {
			return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeConstPointer<_Ty>& _Right_cref) {
			return mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = (bool(*static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty>*>(this)));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty>*>(this));
			return retval;
		}
		explicit operator const TXScopeObj<_Ty>*() const {
			const TXScopeObj<_Ty>* retval = (*static_cast<const mse::us::impl::TXScopeConstPointerBase<_Ty>*>(this));
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
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeNotNullPointer() {}
	private:
		TXScopeNotNullPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopePointer<_Ty>(scpobj_ref) {}
		TXScopeNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopePointer<_Ty>(ptr) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeNotNullPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopePointer<_Ty>(src_cref) {}
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
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeNotNullConstPointer() {}
	private:
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		TXScopeNotNullConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeConstPointer<_Ty>(scpobj_cref) {}
		operator bool() const { return (*static_cast<const TXScopeConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		friend class TXScopeFixedConstPointer<_Ty>;
	};

	/* A TXScopeFixedPointer points to a TXScopeObj. Its intended for very limited use. Basically just to pass a TXScopeObj
	by reference as a function parameter. TXScopeFixedPointers can be obtained from TXScopeObj's "&" (address of) operator. */
	template<typename _Ty>
	class TXScopeFixedPointer : public TXScopeNotNullPointer<_Ty> {
	public:
		TXScopeFixedPointer(const TXScopeFixedPointer& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullPointer<_Ty>(src_cref) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopeNotNullPointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopeNotNullPointer<_Ty>::operator TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeFixedPointer(TXScopeObj<_Ty>& scpobj_ref) : TXScopeNotNullPointer<_Ty>(scpobj_ref) {}
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
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeNotNullConstPointer<_Ty>(src_cref) {}
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeFixedConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeNotNullConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeNotNullConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeNotNullConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		void xscope_tag() const {}

	private:
		TXScopeFixedConstPointer(const TXScopeObj<_Ty>& scpobj_cref) : TXScopeNotNullConstPointer<_Ty>(scpobj_cref) {}
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
		, public MSE_FIRST_OR_PLACEHOLDER_IF_A_BASE_OF_SECOND(mse::us::impl::XScopeTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy>)
		, public MSE_FIRST_OR_PLACEHOLDER_IF_A_BASE_OF_SECOND(mse::us::impl::ReferenceableByScopePointerTagBase, mse::us::impl::TXScopeObjBase<_TROy>, TXScopeObj<_TROy>)
	{
	public:
		TXScopeObj(const TXScopeObj& _X) : mse::us::impl::TXScopeObjBase<_TROy>(_X) {}

#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		explicit TXScopeObj(TXScopeObj&& _X) : mse::us::impl::TXScopeObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		MSE_SCOPE_USING(TXScopeObj, mse::us::impl::TXScopeObjBase<_TROy>);
		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeObj() {}

		TXScopeObj& operator=(TXScopeObj&& _X) {
			//mse::impl::valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
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
			return *this;
		}
		const TXScopeFixedConstPointer<_TROy> operator&() const & {
			return *this;
		}
		const TXScopeItemFixedPointer<_TROy> mse_xscope_ifptr() & { return &(*this); }
		const TXScopeItemFixedConstPointer<_TROy> mse_xscope_ifptr() const & { return &(*this); }

		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() && {
			//return TXScopeItemFixedConstPointer<_TROy>(TXScopeFixedPointer<_TROy>(&(*static_cast<mse::us::impl::TXScopeObjBase<_TROy>*>(this))));
			return TXScopeItemFixedConstPointer<_TROy>(TXScopeFixedPointer<_TROy>(*this));
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
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (mse::impl::is_potentially_not_xscope<_Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		void* operator new(size_t size) { return ::operator new(size); }

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
	class TXScopeItemFixedPointer : public mse::us::impl::TXScopeItemPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopeItemPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}

		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeItemFixedPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return std::addressof(*(*this))/*mse::us::impl::TXScopeItemPointerBase<_Ty>::operator _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedPointer(_Ty* ptr) : mse::us::impl::TXScopeItemPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedPointer(const mse::us::impl::TXScopeItemPointerBase<_Ty>& ptr) : mse::us::impl::TXScopeItemPointerBase<_Ty>(ptr) {}
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
	class TXScopeItemFixedConstPointer : public mse::us::impl::TXScopeItemConstPointerBase<_Ty>, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
	public:
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty>& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty>&>(src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(static_cast<const mse::us::impl::TXScopeItemPointerBase<_Ty2>&>(src_cref)) {}

		//TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}

		MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeItemFixedConstPointer() {}

		operator bool() const { return true; }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return std::addressof(*(*this))/*mse::us::impl::TXScopeItemConstPointerBase<_Ty>::operator const _Ty*()*/; }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedConstPointer(const _Ty* ptr) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedConstPointer(const mse::us::impl::TXScopeItemConstPointerBase<_Ty>& ptr) : mse::us::impl::TXScopeItemConstPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedConstPointer<_Ty>& operator=(const TXScopeItemFixedConstPointer<_Ty>& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
		template<class _Ty2, class _TMemberObjectPointer>
		friend auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty2> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
			->TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>;
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
	class TXScopeCagedItemFixedPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
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
		template<class _Ty2> friend class TXScopeStrongPointerStore;
	};

	template<typename _Ty>
	class TXScopeCagedItemFixedConstPointerToRValue : public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::StrongPointerAsyncNotShareableAndNotPassableTagBase {
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
		template<class _Ty2> friend class TXScopeStrongConstPointerStore;
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

	template<typename _Ty>
	class TXScopeObj<_Ty*> : public TXScopeObj<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeObj<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	private:
		TXScopeObj(std::nullptr_t) {}
		TXScopeObj() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	};
	template<typename _Ty>
	class TXScopeObj<const _Ty*> : public TXScopeObj<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeObj<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	private:
		TXScopeObj(std::nullptr_t) {}
		TXScopeObj() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	};

	template<typename _Ty>
	class TXScopeObj<_Ty* const> : public TXScopeObj<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeObj<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	private:
		TXScopeObj(std::nullptr_t) {}
		TXScopeObj() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	};
	template<typename _Ty>
	class TXScopeObj<const _Ty * const> : public TXScopeObj<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeObj<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeObj, base_class);
#if !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	private:
		TXScopeObj(std::nullptr_t) {}
		TXScopeObj() {}
#endif // !defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	};

	template<typename _Ty>
	class TXScopeItemFixedPointer<_Ty*> : public TXScopeItemFixedPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeItemFixedPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeItemFixedPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedPointer<_Ty* const> : public TXScopeItemFixedPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeItemFixedPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeItemFixedPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedPointer<const _Ty*> : public TXScopeItemFixedPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeItemFixedPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeItemFixedPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedPointer<const _Ty* const> : public TXScopeItemFixedPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeItemFixedPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeItemFixedPointer, base_class);
	};

	template<typename _Ty>
	class TXScopeItemFixedConstPointer<_Ty*> : public TXScopeItemFixedConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeItemFixedConstPointer<mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeItemFixedConstPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedConstPointer<_Ty* const> : public TXScopeItemFixedConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> {
	public:
		typedef TXScopeItemFixedConstPointer<const mse::us::impl::TPointerForLegacy<_Ty>> base_class;
		MSE_USING(TXScopeItemFixedConstPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedConstPointer<const _Ty*> : public TXScopeItemFixedConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeItemFixedConstPointer<mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeItemFixedConstPointer, base_class);
	};
	template<typename _Ty>
	class TXScopeItemFixedConstPointer<const _Ty* const> : public TXScopeItemFixedConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> {
	public:
		typedef TXScopeItemFixedConstPointer<const mse::us::impl::TPointerForLegacy<const _Ty>> base_class;
		MSE_USING(TXScopeItemFixedConstPointer, base_class);
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

			mse::TXScopeFixedPointer<_TTargetType> xscope_ptr() const & {
				return mse::us::unsafe_make_xscope_pointer_to(*(*this));
			}
			mse::TXScopeFixedPointer<_TTargetType> xscope_ptr() const && = delete;
			operator mse::TXScopeFixedPointer<_TTargetType>() const & {
				return xscope_ptr();
			}
			operator mse::TXScopeFixedPointer<_TTargetType>() const && = delete;

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

			mse::TXScopeFixedConstPointer<_TTargetType> xscope_ptr() const & {
				return mse::us::unsafe_make_xscope_const_pointer_to(*(*this));
			}
			mse::TXScopeFixedConstPointer<_TTargetType> xscope_ptr() const && = delete;
			operator mse::TXScopeFixedConstPointer<_TTargetType>() const & {
				return xscope_ptr();
			}
			operator mse::TXScopeFixedConstPointer<_TTargetType>() const && = delete;

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

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeItemFixedPointerFParam() {}

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

			MSE_IMPL_DESTRUCTOR_PREFIX1 ~TXScopeItemFixedConstPointerFParam() {}

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

	namespace rsv {
		/* Obtain a scope pointer to any object. Requires static verification. */
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> make_xscope_pointer_to(_Ty& ref) {
			return mse::us::unsafe_make_xscope_pointer_to(ref);
		}
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> make_xscope_const_pointer_to(const _Ty& cref) {
			return mse::us::unsafe_make_xscope_const_pointer_to(cref);
		}
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> make_xscope_pointer_to(const _Ty& cref) {
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
	MSE_DEPRECATED TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_xscope_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return TXScopeItemFixedConstPointer<_TTargetType>(std::addressof(target));
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifndef MSE_SCOPE_DISABLE_MAKE_POINTER_TO_MEMBER
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeItemFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}

#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedPointer<_TTargetType> make_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	MSE_DEPRECATED TXScopeItemFixedConstPointer<_TTargetType> make_const_pointer_to_member(const _TTargetType& target, const TXScopeFixedConstPointer<_Ty> &lease_pointer) {
		return make_xscope_const_pointer_to_member(target, lease_pointer);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifdef MSE_SCOPEPOINTER_DISABLED
	namespace impl {
		/* This template type alias is only used because msvc2017(v15.9.0) crashes if the type expression is used directly. */
		template<class _Ty2, class _TMemberObjectPointer>
		using make_xscope_pointer_to_member_v2_return_type1 = TXScopeItemFixedPointer<typename std::remove_reference<decltype(std::declval<_Ty2>().*std::declval<_TMemberObjectPointer>())>::type>;
	}
#endif // MSE_SCOPEPOINTER_DISABLED

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemPointerBase<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::us::impl::TXScopeItemConstPointerBase<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type>(
			mse::us::make_xscope_strong((*lease_pointer).*member_object_ptr, lease_pointer));
	}
#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> mse::impl::make_xscope_pointer_to_member_v2_return_type1<_Ty, _TMemberObjectPointer> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
		mse::impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_const<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type>::type> {
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

		operator mse::TXScopeItemFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, class = typename std::enable_if<!std::is_same<mse::TXScopeItemFixedConstPointer<target_t2>, mse::TXScopeItemFixedPointer<target_t> >::value, void>::type>
		explicit operator mse::TXScopeItemFixedConstPointer<target_t2>() const & {
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

		operator mse::TXScopeItemFixedConstPointer<target_t>() const & {
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

		operator mse::TXScopeItemFixedPointer<target_t>() const & {
			return m_stored_ptr;
		}
		/*
		template<class target_t2 = target_t, class = typename std::enable_if<!std::is_same<mse::TXScopeItemFixedConstPointer<target_t2>, mse::TXScopeItemFixedPointer<target_t> >::value, void>::type>
		explicit operator mse::TXScopeItemFixedConstPointer<target_t2>() const & {
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

		operator mse::TXScopeItemFixedConstPointer<target_t>() const & {
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

	template<typename _Ty> using TXScopeXScopeItemFixedStore = TXScopeStrongNotNullPointerStore<TXScopeItemFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeItemFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeItemFixedConstPointer<_Ty> >;
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty> using TXScopeXScopeFixedStore = TXScopeStrongNotNullPointerStore<TXScopeFixedPointer<_Ty> >;
	template<typename _Ty> using TXScopeXScopeFixedConstStore = TXScopeStrongNotNullConstPointerStore<TXScopeFixedConstPointer<_Ty> >;
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
		auto make_xscope_non_capture_lambda(const _TLambda& lambda) {
			return mse::make_xscope(lambda);
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
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedConstPointer<_Ty>& a, const TXScopeItemFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedPointer<_Ty>& a, const TXScopeItemFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	auto xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeItemFixedConstPointer<_Ty>& a, const TXScopeItemFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeItemFixedPointer<_Ty>& a, const TXScopeItemFixedPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b) {
		return choose_the_second ? b : a;
	}
	template<typename _Ty>
	auto xscope_chosen_pointer(bool choose_the_second, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b) {
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
	template<typename _Ty> using sfp = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using sfcp = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using so = TXScopeObj<_TROy>;
	template<typename _Ty> using sifp = TXScopeItemFixedPointer<_Ty>;
	template<typename _Ty> using sifcp = TXScopeItemFixedConstPointer<_Ty>;

	/* deprecated aliases */
	template<typename _Ty> using scpfp MSE_DEPRECATED = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using scpfcp MSE_DEPRECATED = TXScopeFixedConstPointer<_Ty>;
	template<typename _TROy> using scpo MSE_DEPRECATED = TXScopeObj<_TROy>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfp MSE_DEPRECATED = TSyncWeakFixedPointer<_TTargetType, _TXScopePointerType>;
	template<class _TTargetType, class _TXScopePointerType> using scpwkfcp MSE_DEPRECATED = TSyncWeakFixedConstPointer<_TTargetType, _TXScopePointerType>;

	template<typename _Ty> using TScopeFixedPointer MSE_DEPRECATED = TXScopeFixedPointer<_Ty>;
	template<typename _Ty> using TScopeFixedConstPointer MSE_DEPRECATED = TXScopeFixedConstPointer<_Ty>;
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
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeItemFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeItemFixedPointer<E> E_scope_ifptr5 = GE_scope_ifptr1;
					mse::TXScopeItemFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeItemFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
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
					mse::TXScopeObj<GE> scope_ge;
					mse::TXScopeItemFixedPointer<GE> GE_scope_ifptr1 = &scope_ge;
					mse::TXScopeItemFixedPointer<E> E_scope_ptr5(GE_scope_ifptr1);
					mse::TXScopeItemFixedPointer<E> E_scope_ifptr2(&scope_ge);
					mse::TXScopeItemFixedConstPointer<E> E_scope_fcptr2 = &scope_ge;
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
