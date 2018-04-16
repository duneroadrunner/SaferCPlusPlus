
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSESCOPE_H_
#define MSESCOPE_H_

//include "mseprimitives.h"
#include "msepointerbasics.h"
#include <utility>
#include <unordered_set>
#include <functional>
#include <cassert>

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

/* Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause relaxed registered pointers to be used to help catch
misuse of scope pointers in debug mode. Additionally defining MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause
mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED
#include "mserelaxedregistered.h"
#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED


#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_SCOPEPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

/* Note that by default, MSE_SCOPEPOINTER_DISABLED is defined in non-debug builds. This is enacted in "msepointerbasics.h". */

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/


namespace mse {

	/* This macro roughly simulates constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) MSE_USING(Derived, Base)

	template<typename _Ty> class TScopeID {};

	class XScopeTagBase {
	public:
		void xscope_tag() const {}
	};

	/* Note that objects not derived from ReferenceableByScopePointerTagBase might still be targeted by a scope pointer via
	make_pointer_to_member(). */
	class ReferenceableByScopePointerTagBase {};

	class ContainsNonOwningScopeReferenceTagBase {};
	class XScopeContainsNonOwningScopeReferenceTagBase : public ContainsNonOwningScopeReferenceTagBase, public XScopeTagBase {};

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

	/* determines if a given type is an instantiation of a given template */
	template<typename T, template<typename> class TT>
	struct is_instantiation_of_msescope : std::false_type { };
	template<typename T, template<typename> class TT>
	struct is_instantiation_of_msescope<TT<T>, TT> : std::true_type { };

	/* The purpose of these template functions are just to produce a compile error on attempts to instantiate
	when certain conditions are not met. */
	template<class _Ty, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _Ty>::value), void>::type>
	void T_valid_if_not_an_xscope_type() {}

	template<class _Ty>
	void T_valid_if_not_an_xscope_type(const _Ty&) {
		T_valid_if_not_an_xscope_type<_Ty>();
	}

#ifdef MSE_SCOPEPOINTER_DISABLED
	//TScopeID
	template<typename _Ty> using TXScopePointer = _Ty*;
	template<typename _Ty> using TXScopeConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeNotNullPointer = _Ty*;
	template<typename _Ty> using TXScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeFixedPointer = _Ty*;
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty*;
	template<typename _TROy> using TXScopeObjBase = _TROy;
	template<typename _TROy> using TXScopeObj = _TROy;
	template<typename _Ty> using TXScopeItemFixedPointer = _Ty*;
	template<typename _Ty> using TXScopeItemFixedConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeCagedItemFixedPointerToRValue = _Ty*;
	template<typename _Ty> using TXScopeCagedItemFixedConstPointerToRValue = const _Ty*;
	//template<typename _TROy> using TXScopeReturnValue = _TROy;

	template<typename _TROy> class TXScopeOwnerPointer;

	template<typename _Ty> auto xscope_ifptr_to(_Ty&& _X) { return std::addressof(_X); }
	template<typename _Ty> auto xscope_ifptr_to(const _Ty& _X) { return std::addressof(_X); }

	//template<typename _Ty> const _Ty& return_value(const _Ty& _X) { return _X; }
	//template<typename _Ty> _Ty&& return_value(_Ty&& _X) { return std::forward<decltype(_X)>(_X); }
	template<typename _TROy> using TNonXScopeObj = _TROy;

#else /*MSE_SCOPEPOINTER_DISABLED*/

#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TXScopePointerBase = mse::TRelaxedRegisteredPointer<_Ty>;
	template<typename _Ty> using TXScopeConstPointerBase = mse::TRelaxedRegisteredConstPointer<_Ty>;
	template<typename _TROz> using TXScopeObjBase = mse::TRelaxedRegisteredObj<_TROz>;

#else // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template<typename _Ty> using TXScopePointerBase = TPointerForLegacy<_Ty, TScopeID<const _Ty>>;
	template<typename _Ty> using TXScopeConstPointerBase = TPointerForLegacy<const _Ty, TScopeID<const _Ty>>;

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
	};

#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

	template <class _Ty, class _Ty2, class = typename std::enable_if<
		(!std::is_same<_Ty&&, _Ty2>::value) || (!std::is_rvalue_reference<_Ty2>::value)
		, void>::type>
	static void valid_if_not_rvalue_reference_of_given_type(_Ty2 src) {}

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
	namespace us {
		template<typename _Ty> class TXScopeItemFixedPointerFParam;
		template<typename _Ty> class TXScopeItemFixedConstPointerFParam;
	}

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopePointer : public TXScopePointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
	public:
	private:
		TXScopePointer() : TXScopePointerBase<_Ty>() {}
		TXScopePointer(TXScopeObj<_Ty>* ptr) : TXScopePointerBase<_Ty>(ptr) {}
		TXScopePointer(const TXScopePointer& src_cref) : TXScopePointerBase<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopePointer(const TXScopePointer<_Ty2>& src_cref) : TXScopePointerBase<_Ty>(TXScopePointerBase<_Ty2>(src_cref)) {}
		virtual ~TXScopePointer() {}
		TXScopePointer<_Ty>& operator=(TXScopeObj<_Ty>* ptr) {
			return TXScopePointerBase<_Ty>::operator=(ptr);
		}
		TXScopePointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			return TXScopePointerBase<_Ty>::operator=(_Right_cref);
		}
		operator bool() const {
			bool retval = ((*static_cast<const TXScopePointerBase<_Ty>*>(this)) != nullptr);
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const {
			_Ty* retval = (*static_cast<const TXScopePointerBase<_Ty>*>(this));
			return retval;
		}
		explicit operator TXScopeObj<_Ty>*() const {
			TXScopeObj<_Ty>* retval = (*static_cast<const TXScopePointerBase<_Ty>*>(this));
			return retval;
		}

		TXScopePointer<_Ty>* operator&() { return this; }
		const TXScopePointer<_Ty>* operator&() const { return this; }

		friend class TXScopeNotNullPointer<_Ty>;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeConstPointer : public TXScopeConstPointerBase<const _Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
	public:
	private:
		TXScopeConstPointer() : TXScopeConstPointerBase<const _Ty>() {}
		TXScopeConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeConstPointerBase<const _Ty>(ptr) {}
		TXScopeConstPointer(const TXScopeConstPointer& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopeConstPointer<_Ty2>& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		TXScopeConstPointer(const TXScopePointer<_Ty>& src_cref) : TXScopeConstPointerBase<const _Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeConstPointer(const TXScopePointer<_Ty2>& src_cref) : TXScopeConstPointerBase<const _Ty>(TXScopeConstPointerBase<_Ty2>(src_cref)) {}
		virtual ~TXScopeConstPointer() {}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeObj<_Ty>* ptr) {
			return TXScopeConstPointerBase<_Ty>::operator=(ptr);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopeConstPointer<_Ty>& _Right_cref) {
			return TXScopeConstPointerBase<_Ty>::operator=(_Right_cref);
		}
		TXScopeConstPointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) { return (*this).operator=(TXScopeConstPointer(_Right_cref)); }
		operator bool() const {
			bool retval = (*static_cast<const TXScopeConstPointerBase<_Ty>*>(this));
			return retval;
		}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const {
			const _Ty* retval = (*static_cast<const TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}
		explicit operator const TXScopeObj<_Ty>*() const {
			const TXScopeObj<_Ty>* retval = (*static_cast<const TXScopeConstPointerBase<const _Ty>*>(this));
			return retval;
		}

		TXScopeConstPointer<_Ty>* operator&() { return this; }
		const TXScopeConstPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeNotNullConstPointer<_Ty>;
	};

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullPointer : public TXScopePointer<_Ty> {
	public:
	private:
		TXScopeNotNullPointer(TXScopeObj<_Ty>* ptr) : TXScopePointer<_Ty>(ptr) {}
		TXScopeNotNullPointer(const TXScopeNotNullPointer& src_cref) : TXScopePointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopePointer<_Ty>(src_cref) {}
		virtual ~TXScopeNotNullPointer() {}
		TXScopeNotNullPointer<_Ty>& operator=(const TXScopePointer<_Ty>& _Right_cref) {
			TXScopePointer<_Ty>::operator=(_Right_cref);
			return (*this);
		}
		operator bool() const { return (*static_cast<const TXScopePointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopePointer<_Ty>::operator _Ty*(); }
		explicit operator TXScopeObj<_Ty>*() const { return TXScopePointer<_Ty>::operator TXScopeObj<_Ty>*(); }

		TXScopeNotNullPointer<_Ty>* operator&() { return this; }
		const TXScopeNotNullPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeFixedPointer<_Ty>;
	};

	/* Use TXScopeFixedConstPointer instead. */
	template<typename _Ty>
	class TXScopeNotNullConstPointer : public TXScopeConstPointer<_Ty> {
	public:
	private:
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullConstPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeNotNullConstPointer(const TXScopeNotNullPointer<_Ty2>& src_cref) : TXScopeConstPointer<_Ty>(src_cref) {}
		virtual ~TXScopeNotNullConstPointer() {}
		operator bool() const { return (*static_cast<const TXScopeConstPointer<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeConstPointer<_Ty>::operator const _Ty*(); }
		explicit operator const TXScopeObj<_Ty>*() const { return TXScopeConstPointer<_Ty>::operator const TXScopeObj<_Ty>*(); }
		TXScopeNotNullConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeConstPointer<_Ty>(ptr) {}

		TXScopeNotNullConstPointer<_Ty>* operator&() { return this; }
		const TXScopeNotNullConstPointer<_Ty>* operator&() const { return this; }

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
		TXScopeFixedPointer(TXScopeObj<_Ty>* ptr) : TXScopeNotNullPointer<_Ty>(ptr) {}
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
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeFixedPointer<_Ty>* operator&() { return this; }
		const TXScopeFixedPointer<_Ty>* operator&() const { return this; }

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
		TXScopeFixedConstPointer(const TXScopeObj<_Ty>* ptr) : TXScopeNotNullConstPointer<_Ty>(ptr) {}
		TXScopeFixedConstPointer<_Ty>& operator=(const TXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeFixedConstPointer<_Ty>* operator&() { return this; }
		const TXScopeFixedConstPointer<_Ty>* operator&() const { return this; }

		friend class TXScopeObj<_Ty>;
	};

	/* TXScopeObj is intended as a transparent wrapper for other classes/objects with "scope lifespans". That is, objects
	that are either allocated on the stack, or whose "owning" pointer is allocated on the stack. Unfortunately it's not
	really possible to completely prevent misuse. For example, std::list<TXScopeObj<mse::CInt>> is an improper, and
	dangerous, use of TXScopeObj<>. So we provide the option of using an mse::TRelaxedRegisteredObj as TXScopeObj's base
	class to enforce safety and to help catch misuse. Defining MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED will cause
	mse::TRelaxedRegisteredObj to be used in non-debug modes as well. */
	template<typename _TROy>
	class TXScopeObj : public TXScopeObjBase<_TROy>
		, public std::conditional<std::is_base_of<XScopeTagBase, _TROy>::value, TPlaceHolder_msescope<TXScopeObj<_TROy> >, XScopeTagBase>::type
		, public std::conditional<std::is_base_of<ReferenceableByScopePointerTagBase, _TROy>::value, TPlaceHolder2_msescope<TXScopeObj<_TROy> >, ReferenceableByScopePointerTagBase>::type
	{
	public:
		TXScopeObj(const TXScopeObj& _X) : TXScopeObjBase<_TROy>(_X) {}

#ifdef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
		explicit TXScopeObj(TXScopeObj&& _X) : TXScopeObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {}
#endif // !MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS

		MSE_SCOPE_USING(TXScopeObj, TXScopeObjBase<_TROy>);
		virtual ~TXScopeObj() {}

		TXScopeObj& operator=(TXScopeObj&& _X) {
			valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
			TXScopeObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		TXScopeObj& operator=(const TXScopeObj& _X) { TXScopeObjBase<_TROy>::operator=(_X); return (*this); }
		template<class _Ty2>
		TXScopeObj& operator=(_Ty2&& _X) {
			TXScopeObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
		template<class _Ty2>
		TXScopeObj& operator=(const _Ty2& _X) { TXScopeObjBase<_TROy>::operator=(_X); return (*this); }

		const TXScopeFixedPointer<_TROy> operator&() & {
			return this;
		}
		const TXScopeFixedConstPointer<_TROy> operator&() const & {
			return this;
		}
		const TXScopeItemFixedPointer<_TROy> mse_xscope_ifptr() & { return &(*this); }
		const TXScopeItemFixedConstPointer<_TROy> mse_xscope_ifptr() const & { return &(*this); }

		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() && {
			return TXScopeItemFixedConstPointer<_TROy>(TXScopeFixedPointer<_TROy>(this));
		}
		TXScopeCagedItemFixedConstPointerToRValue<_TROy> operator&() const && {
			return TXScopeFixedConstPointer<_TROy>(TXScopeConstPointer<_TROy>(this));
		}
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() && { return &(*this); }
		const TXScopeCagedItemFixedConstPointerToRValue<_TROy> mse_xscope_ifptr() const && { return &(*this); }

		void xscope_tag() const {}
		//void xscope_contains_accessible_scope_address_of_operator_tag() const {}
		/* This type can be safely used as a function return value if _Ty is also safely returnable. */
		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
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

	namespace us {
		/* A couple of unsafe functions for internal use. */
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref);
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref);
	}

	/* While TXScopeFixedPointer<> points to a TXScopeObj<>, TXScopeItemFixedPointer<> is intended to be able to point to a
	TXScopeObj<>, any member of a TXScopeObj<>, or various other items with scope lifetime that, for various reasons, aren't
	declared as TXScopeObj<>. */
	template<typename _Ty>
	class TXScopeItemFixedPointer : public TXScopePointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
	public:
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : TXScopePointerBase<_Ty>(static_cast<const TXScopePointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopeItemFixedPointer(std::addressof(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeItemFixedPointer(static_cast<const TXScopeFixedPointer<_Ty>&>(src_cref)) {}

		//TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedPointer(&(*src_cref)) {}

		//template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		//TXScopeItemFixedPointer(const TXScopeWeakFixedPointer<_Ty, _Ty2>& src_cref) : TXScopeItemFixedPointer(std::addressof(*src_cref)) {}

		virtual ~TXScopeItemFixedPointer() {}

		//operator bool() const { return (*static_cast<const TXScopePointerBase<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TXScopePointerBase<_Ty>::operator _Ty*(); }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedPointer(_Ty* ptr) : TXScopePointerBase<_Ty>(ptr) {}
		TXScopeItemFixedPointer<_Ty>& operator=(const TXScopeItemFixedPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeItemFixedPointer<_Ty>* operator&() { return this; }
		const TXScopeItemFixedPointer<_Ty>* operator&() const { return this; }

		/* These versions of make_xscope_pointer_to_member() are actually now deprecated. */
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty2> &lease_pointer);

		template<class _Ty2> friend TXScopeItemFixedPointer<_Ty2> us::unsafe_make_xscope_pointer_to(_Ty2& ref);
	};

	template<typename _Ty>
	class TXScopeItemFixedConstPointer : public TXScopeConstPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
	public:
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty>& src_cref) = default;
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedConstPointer<_Ty2>& src_cref) : TXScopeConstPointerBase<_Ty>(static_cast<const TXScopeConstPointerBase<_Ty2>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty>& src_cref) : TXScopeConstPointerBase<_Ty>(src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeItemFixedPointer<_Ty2>& src_cref) : TXScopeConstPointerBase<_Ty>(TXScopeConstPointerBase<_Ty2>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : TXScopeItemFixedConstPointer(std::addressof(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TXScopeItemFixedConstPointer(static_cast<const TXScopeFixedConstPointer<_Ty>&>(src_cref)) {}

		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TXScopeItemFixedConstPointer(std::addressof(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TXScopeItemFixedConstPointer(static_cast<const TXScopeFixedPointer<_Ty>&>(src_cref)) {}

		//TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TXScopeItemFixedConstPointer(const TXScopeOwnerPointer<_Ty2>& src_cref) : TXScopeItemFixedConstPointer(&(*src_cref)) {}

		virtual ~TXScopeItemFixedConstPointer() {}

		//operator bool() const { return (*static_cast<const TXScopeConstPointerBase<_Ty>*>(this)); }
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator const _Ty*() const { return TXScopeConstPointerBase<_Ty>::operator const _Ty*(); }
		void xscope_tag() const {}

	private:
		TXScopeItemFixedConstPointer(const _Ty* ptr) : TXScopeConstPointerBase<_Ty>(ptr) {}
		TXScopeItemFixedConstPointer<_Ty>& operator=(const TXScopeItemFixedConstPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeItemFixedConstPointer<_Ty>* operator&() { return this; }
		const TXScopeItemFixedConstPointer<_Ty>* operator&() const { return this; }

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
	accessed when converted to a us::TXScopeItemFixedPointerFParam<>. */
	template<typename _Ty>
	class TXScopeCagedItemFixedPointerToRValue : public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
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
		template<class _Ty2> friend class us::TXScopeItemFixedPointerFParam;
		template<class _Ty2> friend class us::TXScopeItemFixedConstPointerFParam;
	};

	template<typename _Ty>
	class TXScopeCagedItemFixedConstPointerToRValue : public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerNotAsyncShareableTagBase {
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
		template<class _Ty2> friend class us::TXScopeItemFixedConstPointerFParam;
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
			mse::T_valid_if_not_an_xscope_type<_TROy>();
		}

		TNonXScopeObj& operator=(TNonXScopeObj&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		TNonXScopeObj& operator=(const TNonXScopeObj& _X) { _TROy::operator=(_X); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(_Ty2&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		//TNonXScopeObj& operator=(_Ty2&& _X) { static_cast<_TROy&>(*this) = (std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TNonXScopeObj& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

		auto operator&() {
			return &(static_cast<_TROy&>(*this));
		}
		auto operator&() const {
			return &(static_cast<const _TROy&>(*this));
		}
	};

#endif /*MSE_SCOPEPOINTER_DISABLED*/

	namespace us {
		/* TXScopeItemFixedPointerFParam<> is just a version of TXScopeItemFixedPointer<> which may only be used for
		function parameter declations. It has the extra ability to accept (caged) scope pointers to r-value scope objects
		(i.e. supports temporaries by scope reference). */

#ifdef MSE_SCOPEPOINTER_DISABLED
		template<typename _Ty> using TXScopeItemFixedPointerFParamBase = TPointer<_Ty>;
		template<typename _Ty> using TXScopeItemFixedConstPointerFParamBase = TPointer<const _Ty>;
		template<typename _Ty> using TXScopeReturnableItemFixedPointerFParamBase = TPointer<_Ty>;
		template<typename _Ty> using TXScopeReturnableItemFixedConstPointerFParamBase = TPointer<const _Ty>;
#else /*MSE_SCOPEPOINTER_DISABLED*/
		template<typename _Ty> using TXScopeItemFixedPointerFParamBase = TXScopeItemFixedPointer<_Ty>;
		template<typename _Ty> using TXScopeItemFixedConstPointerFParamBase = TXScopeItemFixedConstPointer<_Ty>;
		template<typename _Ty> using TXScopeReturnableItemFixedPointerFParamBase = TXScopeItemFixedPointer<_Ty>;
		template<typename _Ty> using TXScopeReturnableItemFixedConstPointerFParamBase = TXScopeItemFixedConstPointer<_Ty>;
#endif /*MSE_SCOPEPOINTER_DISABLED*/

		template<typename _Ty>
		class TXScopeItemFixedPointerFParam : public TXScopeItemFixedPointerFParamBase<_Ty> {
		public:
			typedef TXScopeItemFixedPointerFParamBase<_Ty> base_class;
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
			void* operator new(size_t size) { return ::operator new(size); }

			TXScopeItemFixedPointerFParam<_Ty>* operator&() { return this; }
			const TXScopeItemFixedPointerFParam<_Ty>* operator&() const { return this; }
		};

		template<typename _Ty>
		class TXScopeItemFixedConstPointerFParam : public TXScopeItemFixedConstPointerFParamBase<_Ty> {
		public:
			typedef TXScopeItemFixedConstPointerFParamBase<_Ty> base_class;
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
			void* operator new(size_t size) { return ::operator new(size); }

			TXScopeItemFixedConstPointerFParam<_Ty>* operator&() { return this; }
			const TXScopeItemFixedConstPointerFParam<_Ty>* operator&() const { return this; }
		};

		/* TXScopeReturnableItemFixedPointerFParam<> is just a version of TXScopeItemFixedPointer<> which may only be used
		for function parameter declations. Because it may only be used for function parameters, and it doesn't support
		pointers to temporaries, an object of this type can be use as a function return value. To be clear,
		TXScopeReturnableItemFixedPointerFParam<> may not be used as a return type. But an object of this type may be used
		as return value (wrapped in the mse::return_value() function). The function return type should be "auto". */
		template<typename _Ty>
		class TXScopeReturnableItemFixedPointerFParam : public TXScopeReturnableItemFixedPointerFParamBase<_Ty> {
		public:
			typedef TXScopeReturnableItemFixedPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeReturnableItemFixedPointerFParam, base_class);

			TXScopeReturnableItemFixedPointerFParam(const TXScopeReturnableItemFixedPointerFParam& src_cref) = default;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeReturnableItemFixedPointerFParam(const TXScopeReturnableItemFixedPointerFParam<_Ty2>& src_cref) : TXScopeReturnableItemFixedPointerFParamBase<_Ty>(static_cast<const TXScopeItemFixedPointer<_Ty2>&>(src_cref)) {}

			virtual ~TXScopeReturnableItemFixedPointerFParam() {}

			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
			void xscope_tag() const {}

		private:
			TXScopeReturnableItemFixedPointerFParam<_Ty>& operator=(const TXScopeReturnableItemFixedPointerFParam<_Ty>& _Right_cref) = delete;
			void* operator new(size_t size) { return ::operator new(size); }

			TXScopeReturnableItemFixedPointerFParam<_Ty>* operator&() { return this; }
			const TXScopeReturnableItemFixedPointerFParam<_Ty>* operator&() const { return this; }
		};

		template<typename _Ty>
		class TXScopeReturnableItemFixedConstPointerFParam : public TXScopeReturnableItemFixedConstPointerFParamBase<_Ty> {
		public:
			typedef TXScopeReturnableItemFixedConstPointerFParamBase<_Ty> base_class;
			MSE_SCOPE_USING(TXScopeReturnableItemFixedConstPointerFParam, base_class);

			TXScopeReturnableItemFixedConstPointerFParam(const TXScopeReturnableItemFixedConstPointerFParam<_Ty>& src_cref) = default;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeReturnableItemFixedConstPointerFParam(const TXScopeReturnableItemFixedConstPointerFParam<_Ty2>& src_cref) : TXScopeReturnableItemFixedConstPointerFParamBase<_Ty>(static_cast<const TXScopeItemFixedConstPointer<_Ty2>&>(src_cref)) {}

			TXScopeReturnableItemFixedConstPointerFParam(const TXScopeReturnableItemFixedPointerFParam<_Ty>& src_cref) : TXScopeReturnableItemFixedConstPointerFParamBase<_Ty>(src_cref) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TXScopeReturnableItemFixedConstPointerFParam(const TXScopeReturnableItemFixedPointerFParam<_Ty2>& src_cref) : TXScopeReturnableItemFixedConstPointerFParamBase<_Ty>(TXScopeReturnableItemFixedConstPointerFParamBase<_Ty2>(src_cref)) {}

			virtual ~TXScopeReturnableItemFixedConstPointerFParam() {}

			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */
			void xscope_tag() const {}

		private:
			TXScopeReturnableItemFixedConstPointerFParam<_Ty>& operator=(const TXScopeReturnableItemFixedConstPointerFParam<_Ty>& _Right_cref) = delete;
			void* operator new(size_t size) { return ::operator new(size); }

			TXScopeReturnableItemFixedConstPointerFParam<_Ty>* operator&() { return this; }
			const TXScopeReturnableItemFixedConstPointerFParam<_Ty>* operator&() const { return this; }
		};
	}

	namespace us {
		/* us::TFParam<> is just a transparent template wrapper for function parameter declarations. In most cases
		use of this wrapper is not necessary, but in some cases it enables functionality only available to variables
		that are function parameters. Specifically, it allows functions to support scope pointer/references to
		temporary objects. For safety reasons, by default, scope pointer/references to temporaries are actually
		"functionally disabled" types distinct from regular scope pointer/reference types. Because it's safe to do so
		in the case of function parameters, the us::TFParam<> wrapper enables certain scope pointer/reference types
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
				auto value_from_fparam_helper1(std::false_type, const _Ty& param) {
					return param;
				}
				template<typename _Ty>
				auto value_from_fparam_helper1(std::true_type, const _Ty& param) -> typename TFParam<typename std::remove_reference<_Ty>::type>::base_class {
					return TFParam<typename std::remove_reference<_Ty>::type>(param);
				}

				template<typename _Ty>
				auto value_from_fparam_helper1(std::false_type, _Ty&& param) {
					return std::forward<_Ty>(param);
				}
				template<typename _Ty>
				auto value_from_fparam_helper1(std::true_type, _Ty&& param) -> typename TFParam<typename std::remove_reference<_Ty>::type>::base_class {
					return TFParam<typename std::remove_reference<_Ty>::type>(std::forward<_Ty>(param));
				}
			}
		}

		template<typename _Ty>
		auto value_from_fparam(const _Ty& param) {
			return impl::fparam::value_from_fparam_helper1(typename std::is_base_of<XScopeTagBase, _Ty>::type(), param);
		}
		template<typename _Ty>
		auto value_from_fparam(_Ty&& param) {
			return impl::fparam::value_from_fparam_helper1(typename std::is_base_of<XScopeTagBase, _Ty>::type(), std::forward<_Ty>(param));
		}

		template<typename _Ty>
		auto xscope_value_from_fparam(const _Ty& param) {
			return value_from_fparam(param);
		}
		template<typename _Ty>
		auto xscope_value_from_fparam(_Ty&& param) {
			return value_from_fparam(std::forward<_Ty>(param));
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
		class TFParam<_Ty*> : public TPointer<_Ty> {
		public:
			typedef TPointer<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};

		template<typename _Ty>
		class TFParam<_Ty* const> : public TPointer<_Ty> {
		public:
			typedef TPointer<_Ty> base_class;
			MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(TFParam, base_class);
		private:
			MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(base_class);
		};
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

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		/* If _TROy is not "marked" as safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TROy>::value) || (
			(!std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TROy>::value)
			/*&& (!std::integral_constant<bool, HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/
			&& (std::integral_constant<bool, HasXScopeReturnableTagMethod<_TROy>::Has>())
			), void>::type>
		void valid_if_TROy_is_marked_as_returnable_or_not_xscope_type() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _TROy>
	class TXScopeReturnValue : public TReturnValue<_TROy>
		, public std::conditional<std::is_base_of<XScopeTagBase, _TROy>::value, TPlaceHolder_msescope<TXScopeReturnValue<_TROy> >, XScopeTagBase>::type
	{
	public:
		typedef TReturnValue<_TROy> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TXScopeReturnValue, base_class);

		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:
		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _Ty>
	TXScopeItemFixedPointer<_Ty> return_value_helper2(const us::TXScopeReturnableItemFixedPointerFParam<_Ty>& _X) { return _X; }
	template<typename _Ty>
	TXScopeItemFixedPointer<_Ty> return_value_helper2(us::TXScopeReturnableItemFixedPointerFParam<_Ty>&& _X) { return std::forward<decltype(_X)>(_X); }
	template<typename _Ty>
	TXScopeItemFixedConstPointer<_Ty> return_value_helper2(const us::TXScopeReturnableItemFixedConstPointerFParam<_Ty>& _X) { return _X; }
	template<typename _Ty>
	TXScopeItemFixedConstPointer<_Ty> return_value_helper2(us::TXScopeReturnableItemFixedConstPointerFParam<_Ty>&& _X) { return std::forward<decltype(_X)>(_X); }

	template<typename _Ty>
	static void z__returnable_noop(const _Ty&) {}
	template<typename _Ty>
	const auto& return_value_helper1(std::false_type, const _Ty& _X) {
		z__returnable_noop<mse::TReturnValue<_Ty> >(_X);
		return _X;
	}
	template<typename _Ty>
	const auto& return_value_helper1(std::true_type, const _Ty& _X) {
		return return_value_helper2(_X);
	}
	template<typename _Ty>
	const auto& return_value(const _Ty& _X) {
		return return_value_helper1(typename std::conditional<
			is_instantiation_of_msescope<_Ty, us::TXScopeReturnableItemFixedPointerFParam>::value
			|| is_instantiation_of_msescope<_Ty, us::TXScopeReturnableItemFixedConstPointerFParam>::value
			, std::true_type, std::false_type>::type(), _X);
	}

	template<typename _Ty>
	auto return_value_helper1(std::false_type, _Ty&& _X) {
		z__returnable_noop<mse::TReturnValue<typename std::remove_reference<_Ty>::type> >(_X);
		return std::forward<decltype(_X)>(_X);
	}
	template<typename _Ty>
	auto return_value_helper1(std::true_type, _Ty&& _X) {
		return return_value_helper2(std::forward<decltype(_X)>(_X));
	}
	template<typename _Ty>
	auto return_value(_Ty&& _X) {
		return return_value_helper1(typename std::conditional<
			is_instantiation_of_msescope<typename std::remove_reference<_Ty>::type, us::TXScopeReturnableItemFixedPointerFParam>::value
			|| is_instantiation_of_msescope<typename std::remove_reference<_Ty>::type, us::TXScopeReturnableItemFixedConstPointerFParam>::value
			, std::true_type, std::false_type>::type(), std::forward<decltype(_X)>(_X));
	}

	/* Template specializations of TReturnValue<>. */

	template<typename _Ty>
	class TReturnValue<_Ty*> : public TPointer<_Ty> {
	public:
		typedef TPointer<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		virtual ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

	private:
#if !(defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED))
		template<class _Ty2 = _Ty, class = typename std::enable_if<!(std::is_same<_Ty2, _Ty>::value)>::type>
#endif /*!(defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED))*/
		void valid_if_safe_pointers_are_disabled() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

	template<typename _Ty>
	class TReturnValue<_Ty* const> : public TPointer<_Ty> {
	public:
		typedef TPointer<_Ty> base_class;
		MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(TReturnValue, base_class);
		virtual ~TReturnValue() {
			valid_if_safe_pointers_are_disabled();
		}

#if defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */
#endif /*defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

	private:
#if !(defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED))
		template<class _Ty2 = _Ty, class = typename std::enable_if<!(std::is_same<_Ty2, _Ty>::value)>::type>
#endif /*!(defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED))*/
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

	/* TXScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
	TXScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TXScopeObj<_Ty>.
	TXScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
	enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
	*/
	template<typename _Ty>
	class TXScopeOwnerPointer : public XScopeTagBase, public StrongPointerNotAsyncShareableTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _Ty>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeOwnerPointer<_Ty> > >::type
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
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
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
		/* A couple of unsafe functions for internal use. */
		template<typename _Ty>
		TXScopeItemFixedPointer<_Ty> unsafe_make_xscope_pointer_to(_Ty& ref) {
			return TXScopeItemFixedPointer<_Ty>(std::addressof(ref));
		}
		template<typename _Ty>
		TXScopeItemFixedConstPointer<_Ty> unsafe_make_xscope_const_pointer_to(const _Ty& cref) {
			return TXScopeItemFixedConstPointer<_Ty>(std::addressof(cref));
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
			template<class = typename std::enable_if<(!std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TROy>::value)
				/*&& (!std::integral_constant<bool, HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/, void>::type>
				void valid_if_TROy_is_not_marked_as_unreturn_value() const {}

			template<class = typename std::enable_if<std::is_base_of<XScopeTagBase, _TROy>::value, void>::type>
			void valid_if_TROy_is_an_xscope_type() const {}

			TXScopeUserDeclaredReturnable* operator&() {
				return this;
			}
			const TXScopeUserDeclaredReturnable* operator&() const {
				return this;
			}
		};
	}


	template <class _TTargetType, class _TLeasePointerType> class TXScopeWeakFixedConstPointer;

	/* If, for example, you want a safe pointer to a member of a scope pointer target, you can use a
	TXScopeWeakFixedPointer to store a copy of the scope pointer along with the pointer targeting the
	member. */
	template <class _TTargetType, class _TLeasePointerType>
	class TXScopeWeakFixedPointer : public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		TXScopeWeakFixedPointer(const TXScopeWeakFixedPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TXScopeWeakFixedPointer(const TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		_TTargetType& operator*() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		TXScopeWeakFixedPointer(const TXScopeFixedPointer<_TTargetType>& target) : TXScopeWeakFixedPointer(make_xscopeweak(*target, target)) {}
		_TTargetType* operator->() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TXScopeWeakFixedPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TXScopeWeakFixedPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;
		bool operator!=(const TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;

		bool operator!() const { return (!m_target_pointer); }
		operator bool() const {
			return (m_target_pointer != nullptr);
		}

		explicit operator _TTargetType*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == m_target_pointer) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return m_target_pointer;
		}
		_TLeasePointerType lease_pointer() const { return (*this).m_lease_pointer; }

		template <class _TTargetType2, class _TLeasePointerType2>
		static TXScopeWeakFixedPointer make(_TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TXScopeWeakFixedPointer(target, lease_pointer);
		}

		void xscope_tag() const {}

	private:
		TXScopeWeakFixedPointer(_TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TXScopeWeakFixedPointer& operator=(const TXScopeWeakFixedPointer& _Right_cref) = delete;

		_TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
		friend class TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType>;
	};

	template <class _TTargetType, class _TLeasePointerType>
	TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType> make_xscopeweak(_TTargetType& target, const _TLeasePointerType& lease_pointer) {
		return TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType>::make(target, lease_pointer);
	}

	template <class _TTargetType, class _TLeasePointerType>
	class TXScopeWeakFixedConstPointer : public XScopeContainsNonOwningScopeReferenceTagBase {
	public:
		TXScopeWeakFixedConstPointer(const TXScopeWeakFixedConstPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TXScopeWeakFixedConstPointer(const TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		TXScopeWeakFixedConstPointer(const TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType>&src) : m_target_pointer(src.m_target_pointer), m_lease_pointer(src.m_lease_pointer) {}
		const _TTargetType& operator*() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		const _TTargetType* operator->() const {
			const auto &test_cref = *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TXScopeWeakFixedConstPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TXScopeWeakFixedConstPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }

		bool operator!() const { return (!m_target_pointer); }
		operator bool() const {
			return (m_target_pointer != nullptr);
		}

		explicit operator const _TTargetType*() const {
#ifdef NATIVE_PTR_DEBUG_HELPER1
			if (nullptr == m_target_pointer) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
			return m_target_pointer;
		}
		_TLeasePointerType lease_pointer() const { return (*this).m_lease_pointer; }

		template <class _TTargetType2, class _TLeasePointerType2>
		static TXScopeWeakFixedConstPointer make(const _TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TXScopeWeakFixedConstPointer(target, lease_pointer);
		}

		void xscope_tag() const {}

	private:
		TXScopeWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TXScopeWeakFixedConstPointer& operator=(const TXScopeWeakFixedConstPointer& _Right_cref) = delete;

		const _TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
	};

	template <class _TTargetType, class _TLeasePointerType>
	bool TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator==(const TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (_Right_cref == m_target_pointer); }
	template <class _TTargetType, class _TLeasePointerType>
	bool TXScopeWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator!=(const TXScopeWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (!((*this) == _Right_cref)); }
}

namespace std {
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TXScopeWeakFixedPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TXScopeWeakFixedPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeWeakFixedPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TXScopeWeakFixedConstPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TXScopeWeakFixedConstPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TXScopeWeakFixedConstPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
}

namespace mse {
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

	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
	template<class _Ty, class _TMemberObjectPointer>
	auto make_xscope_const_pointer_to_member_v2(const TXScopeItemFixedConstPointer<_Ty> &lease_pointer, const _TMemberObjectPointer& member_object_ptr)
		-> TXScopeItemFixedConstPointer<typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type> {
		make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		/* Originally, this function itself was a friend of TXScopeItemFixedConstPointer<>, but that seemed to confuse msvc2017 (but not
		g++ or clang) in some cases. */
		//typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		//return TXScopeItemFixedConstPointer<_TTarget>(std::addressof((*lease_pointer).*member_object_ptr));
		return mse::us::unsafe_make_xscope_const_pointer_to((*lease_pointer).*member_object_ptr);
	}
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
	template<typename _TStrongPointer, class = is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongPointerStore : public XScopeTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeStrongPointerStore<_TStrongPointer> > >::type
	{
	public:
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
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongConstPointerStore : public XScopeTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeStrongConstPointerStore<_TStrongPointer> > >::type
	{
	public:
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
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongNotNullPointerStore : public XScopeTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeStrongNotNullPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongNotNullPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		_TStrongPointer m_stored_ptr;
	};

	template<typename _TStrongPointer, class = is_valid_if_strong_pointer<_TStrongPointer> >
	class TXScopeStrongNotNullConstPointerStore : public XScopeTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TStrongPointer>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeStrongNotNullConstPointerStore<_TStrongPointer> > >::type
	{
	public:
		TXScopeStrongNotNullConstPointerStore(const _TStrongPointer& stored_ptr) : m_stored_ptr(stored_ptr) {}
		auto xscope_ptr() const & {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		void xscope_ptr() const && = delete;
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _TStrongPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TStrongPointer>::value) && (
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
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

	/* Just the generalization xscope_chosen_pointer(). */
	template<typename _Ty>
	const auto& chosen(bool choose_the_second, const _Ty& a, const _Ty& b) {
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

				TPointerForLegacy<A> pfl_ptr1 = &a;
				if (!(pfl_ptr1 != nullptr)) {
					assert(false);
				}
				TPointerForLegacy<A> pfl_ptr2 = nullptr;
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

				/* You can use the "mse::make_xscope_pointer_to_member()" function to obtain a safe pointer to a member of
				an xscope object. */
				auto s_safe_ptr1 = mse::make_xscope_pointer_to_member((a_scpobj.s), (&a_scpobj));
				(*s_safe_ptr1) = "some new text";
				auto s_safe_const_ptr1 = mse::make_xscope_const_pointer_to_member((a_scpobj.s), (&a_scpobj));

				/* Just testing the convertibility of mse::TXScopeWeakFixedPointers. */
				auto A_xscope_fixed_ptr1 = &a_scpobj;
				auto xscpwfptr1 = mse::make_xscopeweak<std::string>(A_xscope_fixed_ptr1->s, A_xscope_fixed_ptr1);
				mse::TXScopeWeakFixedPointer<std::string, mse::TXScopeFixedConstPointer<A>> xscpwfptr2 = xscpwfptr1;
				mse::TXScopeWeakFixedConstPointer<std::string, mse::TXScopeFixedPointer<A>> xscpwfcptr1 = xscpwfptr1;
				mse::TXScopeWeakFixedConstPointer<std::string, mse::TXScopeFixedConstPointer<A>> xscpwfcptr2 = xscpwfcptr1;
				if (xscpwfcptr1 == xscpwfptr1) {
					int q = 7;
				}
				if (xscpwfptr1 == xscpwfcptr1) {
					int q = 7;
				}
				if (xscpwfptr1) {
					int q = 7;
				}
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
