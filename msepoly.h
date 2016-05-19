
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPOLY_H_
#define MSEPOLY_H_

//include "mseprimitives.h"
#include "mserefcounting.h"
#include "msescope.h"
#include <memory>
#include <iostream>
#include <utility>
#include <cassert>

/* for the test functions */
#include <map>
#include <string>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_POLYPOINTER_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

#ifdef MSE_POLYPOINTER_DISABLED
#else /*MSE_POLYPOINTER_DISABLED*/
#endif /*MSE_POLYPOINTER_DISABLED*/

	template<typename _Ty> class TRefCountingOrXScopeConstPointer;

	template<typename _Ty>
	class TRefCountingOrXScopeFixedPointer : public TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>> {
	public:
		TRefCountingOrXScopeFixedPointer(const TRefCountingOrXScopeFixedPointer& src_cref) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(src_cref) {}
		TRefCountingOrXScopeFixedPointer(const TRefCountingPointer<_Ty>& src_cref) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(*src_cref, src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedPointer(const TRefCountingPointer<_Ty2>& src_cref) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(*src_cref, src_cref) {}
		TRefCountingOrXScopeFixedPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		virtual ~TRefCountingOrXScopeFixedPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>::operator _Ty*(); }

	protected:
		TRefCountingOrXScopeFixedPointer(_Ty* ptr) : TStrongFixedPointer<_Ty, TRefCountingPointer<_Ty>>(*ptr, TRefCountingPointer<_Ty>()) {}
	private:
		TRefCountingOrXScopeFixedPointer<_Ty>& operator=(const TRefCountingOrXScopeFixedPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingOrXScopeFixedPointer<_Ty>* operator&() { return this; }
		//const TRefCountingOrXScopeFixedPointer<_Ty>* operator&() const { return this; }

		friend class TRefCountingOrXScopeConstPointer<_Ty>;
	};

	template<typename _Ty>
	class TRefCountingOrXScopeFixedConstPointer : public TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>> {
	public:
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingOrXScopeFixedConstPointer& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(src_cref) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingOrXScopeFixedPointer<_Ty>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(src_cref) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingConstPointer<_Ty>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingConstPointer<_Ty2>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, src_cref) {}
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingPointer<_Ty>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, src_cref) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TRefCountingPointer<_Ty2>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>(src_cref)) {}
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedConstPointer<_Ty2>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
		TRefCountingOrXScopeFixedConstPointer(const TXScopeFixedPointer<_Ty2>& src_cref) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*src_cref, TRefCountingPointer<_Ty>()) {}
		virtual ~TRefCountingOrXScopeFixedConstPointer() {}
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const { return TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>::operator _Ty*(); }

	protected:
		TRefCountingOrXScopeFixedConstPointer(_Ty* ptr) : TStrongFixedConstPointer<_Ty, TRefCountingConstPointer<_Ty>>(*ptr, TRefCountingConstPointer<_Ty>()) {}
	private:
		TRefCountingOrXScopeFixedConstPointer<_Ty>& operator=(const TRefCountingOrXScopeFixedConstPointer<_Ty>& _Right_cref) = delete;

		//TRefCountingOrXScopeFixedConstPointer<_Ty>* operator&() { return this; }
		//const TRefCountingOrXScopeFixedConstPointer<_Ty>* operator&() const { return this; }
	};


	template<typename _Ty>
	class TRefCountingOrXScopeOrRawFixedPointer : public TRefCountingOrXScopeFixedPointer<_Ty> {
	public:
		MSE_SCOPE_USING(TRefCountingOrXScopeOrRawFixedPointer, TRefCountingOrXScopeFixedPointer<_Ty>);
		TRefCountingOrXScopeOrRawFixedPointer(_Ty* ptr) : TRefCountingOrXScopeFixedPointer<_Ty>(ptr) {}
	};

	template<typename _Ty>
	class TRefCountingOrXScopeOrRawFixedConstPointer : public TRefCountingOrXScopeFixedConstPointer<_Ty> {
	public:
		MSE_SCOPE_USING(TRefCountingOrXScopeOrRawFixedConstPointer, TRefCountingOrXScopeFixedConstPointer<_Ty>);
		TRefCountingOrXScopeOrRawFixedConstPointer(_Ty* ptr) : TRefCountingOrXScopeFixedConstPointer<_Ty>(ptr) {}
	};

}

#endif // MSEPOLY_H_
