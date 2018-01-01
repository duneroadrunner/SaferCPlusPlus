
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

#ifdef NDEBUG
#ifndef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#define MSE_SCOPEPOINTER_DISABLED
#endif // !MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // NDEBUG


namespace mse {

	/* This macro roughly simulates constructor inheritance. */
#define MSE_SCOPE_USING(Derived, Base) MSE_USING(Derived, Base)

	template<typename _Ty>
	class TScopeID {};

	class XScopeTagBase {
	public:
		void xscope_tag() const {}
	};

	class ContainsAccessibleScopeAddressOfOperatorTagBase {};
	//class DoesNotContainAccessibleScopeAddressOfOperatorTagBase {};
	//class XScopeContainsAccessibleAddressOfOperatorTagBase : public ContainsAccessibleScopeAddressOfOperatorTagBase, public XScopeTagBase {};
	//class XScopeDoesNotContainAccessibleAddressOfTagBase : public DoesNotContainAccessibleScopeAddressOfOperatorTagBase, public XScopeTagBase {};

	class ContainsNonOwningScopeReferenceTagBase {};
	//class DoesNotContainNonOwningScopeReferenceTagBase {};
	class XScopeContainsNonOwningScopeReferenceTagBase : public ContainsNonOwningScopeReferenceTagBase, public XScopeTagBase {};
	//class XScopeDoesNotContainNonOwningScopeReferenceTagBase : public DoesNotContainNonOwningScopeReferenceTagBase, public XScopeTagBase {};

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

	/* The purpose of this template function is just to produce a compile error on attempts to instantiate with a scope type. */
	template<class _Ty, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _Ty>::value), void>::type>
	void T_valid_if_not_an_xscope_type() {}

#ifdef MSE_SCOPEPOINTER_DISABLED
	template<typename _Ty> using TXScopePointer = _Ty*;
	template<typename _Ty> using TXScopeConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeNotNullPointer = _Ty*;
	template<typename _Ty> using TXScopeNotNullConstPointer = const _Ty*;
	template<typename _Ty> using TXScopeFixedPointer = _Ty*;
	template<typename _Ty> using TXScopeFixedConstPointer = const _Ty*;
	template<typename _TROy> using TXScopeObj = _TROy;
	template<typename _Ty> using TXScopeItemFixedPointer = _Ty*;
	template<typename _Ty> using TXScopeItemFixedConstPointer = const _Ty*;

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

	/* Use TXScopeFixedPointer instead. */
	template<typename _Ty>
	class TXScopePointer : public TXScopePointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerTagBase {
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
	class TXScopeConstPointer : public TXScopeConstPointerBase<const _Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerTagBase {
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
#ifndef MSE_SCOPE_DISABLE_MOVE_RESTRICTIONS
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
		, public std::conditional<std::is_base_of<ContainsAccessibleScopeAddressOfOperatorTagBase, _TROy>::value, TPlaceHolder2_msescope<TXScopeObj<_TROy> >, ContainsAccessibleScopeAddressOfOperatorTagBase>::type
	{
	public:
		TXScopeObj(const TXScopeObj& _X) : TXScopeObjBase<_TROy>(_X) {}
		explicit TXScopeObj(TXScopeObj&& _X) : TXScopeObjBase<_TROy>(std::forward<decltype(_X)>(_X)) {
#ifndef __clang__
			/* The idea is that we want to disallow (by causing a compile error) move construction from another TXScopeObj,
			but allow move construction from an object of the base type, _TROy. But clang3.8 seems to require this move
			constructor to be compilable, even when move constructing from an object of the base type. g++ and msvc don't
			seem to have this issue. */
			valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
#endif /*!__clang__*/
		}
		MSE_SCOPE_USING(TXScopeObj, TXScopeObjBase<_TROy>);
		virtual ~TXScopeObj() {}

		TXScopeObj& operator=(TXScopeObj&& _X) {
			valid_if_not_rvalue_reference_of_given_type<TXScopeObj, decltype(_X)>(_X);
			TXScopeObjBase<_TROy>::operator=(std::forward<decltype(_X)>(_X));
			return (*this);
		}
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
		void xscope_tag() const {}
		//void xscope_contains_accessible_scope_address_of_operator_tag() const {}
		/* This type can be safely used as a function return value if _Ty is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
		template<class _Ty2 = _TROy, class = typename std::enable_if<(std::is_same<_Ty2, _TROy>::value) && (
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		/* While there are legitimate cases where one could use a scope pointer to an r-value
		 * scope object, for the sake of safety, we're going to discourage explicit use of
		 * r-value scope objects in general. */
		const TXScopeFixedPointer<_TROy> operator&() && {
			return this;
		}
		const TXScopeFixedConstPointer<_TROy> operator&() const && {
			return this;
		}

		void* operator new(size_t size) { return ::operator new(size); }

		friend class TXScopeOwnerPointer<_TROy>;
	};

	/* TXScopeOwnerPointer is meant to be much like boost::scoped_ptr<>. Instead of taking a native pointer,
	TXScopeOwnerPointer just forwards it's constructor arguments to the constructor of the TXScopeObj<_Ty>.
	TXScopeOwnerPointers are meant to be allocated on the stack only. Unfortunately there's really no way to
	enforce this, which makes this data type less intrinsically safe than say, "reference counting" pointers.
	*/
	template<typename _Ty>
	class TXScopeOwnerPointer : public XScopeTagBase, public StrongPointerTagBase
		, public std::conditional<std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _Ty>::value, ContainsNonOwningScopeReferenceTagBase, TPlaceHolder_msescope<TXScopeOwnerPointer<_Ty> > >::type
	{
	public:
		TXScopeOwnerPointer(TXScopeOwnerPointer<_Ty>&& src_ref) = default;

		template <class... Args>
		TXScopeOwnerPointer(Args&&... args) {
			TXScopeObj<_Ty>* new_ptr = new TXScopeObj<_Ty>(std::forward<Args>(args)...);
			m_ptr = new_ptr;
		}
		virtual ~TXScopeOwnerPointer() {
			assert(m_ptr);
			delete m_ptr;

			/* At the moment TXScopeOwnerPointer<> doesn't support types that are already scope types. */
			T_valid_if_not_an_xscope_type<_Ty>();
		}

		TXScopeObj<_Ty>& operator*() const {
			return (*m_ptr);
		}
		TXScopeObj<_Ty>* operator->() const {
			return m_ptr;
		}

		template <class... Args>
		static TXScopeOwnerPointer make(Args&&... args) {
			return TXScopeOwnerPointer(std::forward<Args>(args)...);
		}

		void xscope_tag() const {}
		/* This type can be safely used as a function return value if _TROy is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (
			(std::integral_constant<bool, HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<XScopeTagBase, _Ty2>::value)
			), void>::type>
			void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

	private:
		TXScopeOwnerPointer(const TXScopeOwnerPointer<_Ty>& src_cref) = delete;
		TXScopeOwnerPointer<_Ty>& operator=(const TXScopeOwnerPointer<_Ty>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeObj<_Ty>* m_ptr = nullptr;
	};

	template <class X, class... Args>
	TXScopeOwnerPointer<X> make_xscope_owner(Args&&... args) {
		return TXScopeOwnerPointer<X>::make(std::forward<Args>(args)...);
	}

	template<typename _Ty>
	class TXScopeItemFixedPointer;
	template<typename _Ty>
	class TXScopeItemFixedConstPointer;

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
	class TXScopeItemFixedPointer : public TXScopePointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerTagBase {
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

		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeFixedPointer<_Ty2> &lease_pointer);
		template<class _TTargetType, class _Ty2>
		friend TXScopeItemFixedPointer<_TTargetType> make_xscope_pointer_to_member(_TTargetType& target, const TXScopeItemFixedPointer<_Ty2> &lease_pointer);
		template<class _Ty2> friend TXScopeItemFixedPointer<_Ty2> us::unsafe_make_xscope_pointer_to(_Ty2& ref);
	};

	template<typename _Ty>
	class TXScopeItemFixedConstPointer : public TXScopeConstPointerBase<_Ty>, public XScopeContainsNonOwningScopeReferenceTagBase, public StrongPointerTagBase {
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

#endif /*MSE_SCOPEPOINTER_DISABLED*/

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


	template<typename _TROy>
	class TXScopeReturnable : public _TROy {
	public:
		MSE_USING(TXScopeReturnable, _TROy);
		TXScopeReturnable(const TXScopeReturnable& _X) : _TROy(_X) {}
		TXScopeReturnable(TXScopeReturnable&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
		virtual ~TXScopeReturnable() {
			/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
			valid_if_TROy_is_marked_as_returnable();
			valid_if_TROy_is_an_xscope_type();
		}

		template<class _Ty2>
		TXScopeReturnable& operator=(_Ty2&& _X) { _TROy::operator=(std::forward<decltype(_X)>(_X)); return (*this); }
		template<class _Ty2>
		TXScopeReturnable& operator=(const _Ty2& _X) { _TROy::operator=(_X); return (*this); }

		void xscope_returnable_tag() const {} /* Indication that this type is eligible to be used as a function return value. */

	private:

		/* If _TROy is not "marked" as safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class = typename std::enable_if<(!std::is_base_of<ContainsNonOwningScopeReferenceTagBase, _TROy>::value)
			/*&& (!std::integral_constant<bool, HasXScopeNotReturnableTagMethod<_TROy>::Has>())*/
			&& (std::integral_constant<bool, HasXScopeReturnableTagMethod<_TROy>::Has>()), void>::type>
		void valid_if_TROy_is_marked_as_returnable() const {}

		template<class = typename std::enable_if<std::is_base_of<XScopeTagBase, _TROy>::value, void>::type>
		void valid_if_TROy_is_an_xscope_type() const {}

		TXScopeReturnable* operator&() {
			return this;
		}
		const TXScopeReturnable* operator&() const {
			return this;
		}
	};

	namespace us {
		template<typename _TROy>
		class TXScopeUserDeclaredReturnable : public _TROy {
		public:
			MSE_USING(TXScopeUserDeclaredReturnable, _TROy);
			TXScopeUserDeclaredReturnable(const TXScopeUserDeclaredReturnable& _X) : _TROy(_X) {}
			TXScopeUserDeclaredReturnable(TXScopeUserDeclaredReturnable&& _X) : _TROy(std::forward<decltype(_X)>(_X)) {}
			virtual ~TXScopeUserDeclaredReturnable() {
				/* This is just a no-op function that will cause a compile error when _TROy is a prohibited type. */
				valid_if_TROy_is_not_marked_as_unreturnable();
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
			void valid_if_TROy_is_not_marked_as_unreturnable() const {}

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
		auto xscope_ptr() const {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
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
		auto xscope_ptr() const {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
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
		auto xscope_ptr() const {
			return mse::us::unsafe_make_xscope_pointer_to(*m_stored_ptr);
		}
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
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
		auto xscope_ptr() const {
			return mse::us::unsafe_make_xscope_const_pointer_to(*m_stored_ptr);
		}
		const _TStrongPointer& stored_ptr() const { return m_stored_ptr; }

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		/* There appears to be a bug in the msvc 2015 compiler that can be worked around by adding a redundant
		component to the enable_if<> condition. */
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
	const TXScopeItemFixedConstPointer<_Ty>& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedConstPointer<_Ty>& a, const TXScopeItemFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const TXScopeItemFixedPointer<_Ty>& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeItemFixedPointer<_Ty>& a, const TXScopeItemFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const TXScopeFixedConstPointer<_Ty>& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedConstPointer<_Ty>& a, const TXScopeFixedConstPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
	template<typename _TBoolFunction, typename _Ty, class... Args>
	const TXScopeFixedPointer<_Ty>& xscope_chosen_pointer(_TBoolFunction function1, const TXScopeFixedPointer<_Ty>& a, const TXScopeFixedPointer<_Ty>& b, Args&&... args) {
		return function1(a, b, std::forward<Args>(args)...) ? b : a;
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

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
				B::foo2(A_scpoptr);
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
				int res4 = B::foo2(a_scpoptr);
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
				B::foo2(A_scpoptr);
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
