
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPRIMITIVES_H
#define MSEPRIMITIVES_H

#include <assert.h>
#include <climits>       // ULONG_MAX
#include <limits>       // std::numeric_limits
#include <stdexcept>      // primitives_range_error

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
#define MSE_PRIMITIVES_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#if defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR
#else // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR constexpr
#endif // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)

#ifdef MSVC2015_COMPATIBLE
#ifndef MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
/* msvc2015's incomplete support for "constexpr" means that range checks that should be done at compile time would
be done at run time, at significant cost. So by default we disable range checks upon assignment. */
#define MSE_PRIMITIVE_ASSIGN_RANGE_CHECK_DISABLED 1
#endif // !MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
#endif // MSVC2015_COMPATIBLE


#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION


#ifndef MSE_CINT_BASE_INTEGER_TYPE
#if SIZE_MAX <= ULONG_MAX
#define MSE_CINT_BASE_INTEGER_TYPE long int
#else // SIZE_MAX <= ULONG_MAX
#define MSE_CINT_BASE_INTEGER_TYPE long long int
#endif // SIZE_MAX <= ULONG_MAX
#endif // !MSE_CINT_BASE_INTEGER_TYPE


namespace mse {

	class primitives_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};

	/* When the mse primitive replacements are "disabled" they lose their default initialization and may cause problems for
	code that relies on it. */
#ifdef MSE_PRIMITIVES_DISABLED
	typedef bool CBool;
	typedef MSE_CINT_BASE_INTEGER_TYPE CInt;
	typedef size_t CSize_t;
	static size_t as_a_size_t(CSize_t n) { return (n); }
#else /*MSE_PRIMITIVES_DISABLED*/

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#define MSE_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

	/* This class is just meant to act like the "bool" type, except that it has a default intialization value (false). */
	class CBool {
	public:
		// Constructs zero.
		CBool() : m_val(false) {}

		// Copy constructor
		CBool(const CBool &x) : m_val(x.m_val) { note_value_assignment(); };

		// Assignment operator
		CBool& operator=(const CBool &x) { note_value_assignment(); m_val = x.m_val; return (*this); }

		// Constructors from primitive boolean types
		CBool(bool   x) { note_value_assignment(); m_val = x; }

		// Casts to primitive boolean types
		operator bool() const { assert_initialized(); return m_val; }

		CBool& operator |=(const CBool &x) { assert_initialized(); m_val |= x.m_val; return (*this); }
		CBool& operator &=(const CBool &x) { assert_initialized(); m_val &= x.m_val; return (*this); }
		CBool& operator ^=(const CBool &x) { assert_initialized(); m_val ^= x.m_val; return (*this); }

		bool m_val;

#ifdef MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_CHECK_USE_BEFORE_SET
	};


	template<typename _TDestination, typename _TSource>
	MSE_CONSTEXPR static bool sg_can_exceed_upper_bound() {
		return (
			((std::numeric_limits<_TSource>::is_signed == std::numeric_limits<_TDestination>::is_signed)
				&& (std::numeric_limits<_TSource>::digits > std::numeric_limits<_TDestination>::digits))
			|| ((std::numeric_limits<_TSource>::is_signed != std::numeric_limits<_TDestination>::is_signed)
				&& ((std::numeric_limits<_TSource>::is_signed && (std::numeric_limits<_TSource>::digits > (1 + std::numeric_limits<_TDestination>::digits)))
					|| ((!std::numeric_limits<_TSource>::is_signed) && ((1 + std::numeric_limits<_TSource>::digits) > std::numeric_limits<_TDestination>::digits))
					)
				)
			);
	}
	template<typename _TDestination, typename _TSource>
	MSE_CONSTEXPR static bool sg_can_exceed_lower_bound() {
		return (
			(std::numeric_limits<_TSource>::is_signed && (!std::numeric_limits<_TDestination>::is_signed))
			|| (std::numeric_limits<_TSource>::is_signed && (std::numeric_limits<_TSource>::digits > std::numeric_limits<_TDestination>::digits))
			);
	}

	template<typename _TDestination, typename _TSource>
	void g_assign_check_range(const _TSource &x) {
#ifndef MSE_PRIMITIVE_ASSIGN_RANGE_CHECK_DISABLED
		/* This probably needs to be cleaned up. But at the moment this should be mostly compile time complexity. And
		as is it avoids "signed/unsigned" mismatch warnings. */
		MSE_CONSTEXPR const bool rhs_can_exceed_upper_bound = sg_can_exceed_upper_bound<_TDestination, _TSource>();
		MSE_CONSTEXPR const bool rhs_can_exceed_lower_bound = sg_can_exceed_lower_bound<_TDestination, _TSource>();
		MSE_CONSTEXPR const bool can_exceed_bounds = rhs_can_exceed_upper_bound || rhs_can_exceed_lower_bound;
		if (can_exceed_bounds) {
			if (rhs_can_exceed_upper_bound) {
				if (x > _TSource(std::numeric_limits<_TDestination>::max())) {
					MSE_THROW(primitives_range_error("range error - value to be assigned is out of range of the target (integer) type"));
				}
			}
			if (rhs_can_exceed_lower_bound) {
				/* We're assuming that std::numeric_limits<>::lowest() will never be greater than zero. */
				if (0 > x) {
					if (0 == std::numeric_limits<_TDestination>::lowest()) {
						MSE_THROW(primitives_range_error("range error - value to be assigned is out of range of the target (integer) type"));
					}
					else if (x < _TSource(std::numeric_limits<_TDestination>::lowest())) {
						MSE_THROW(primitives_range_error("range error - value to be assigned is out of range of the target (integer) type"));
					}
				}
			}
		}
#endif // !MSE_PRIMITIVE_ASSIGN_RANGE_CHECK_DISABLED
	}

	/* The CInt and CSize_t classes are meant to substitute for standard "int" and "size_t" types. The differences between
	the standard types and these classes are that the classes have a default intialization value (zero), and the
	classes, as much as possible, try to prevent the problematic behaviour of (possibly negative) signed integers
	being cast (inadvertently) to the unsigned size_t type. For example, the expression (0 > (int)5 - (size_t)7) evaluates
	(unintuitively) to false, whereas the expression (0 > (CInt)5 - (CSize_t)7) evaluates to true. Also, the classes do
	some range checking. For example, the code "CSize_t s = -2;" will throw an exception. */
	template<typename _Ty>
	class TIntBase1 {
	public:
		// Constructs zero.
		TIntBase1() : m_val(0) {}

		// Copy constructor
		TIntBase1(const TIntBase1 &x) : m_val(x.m_val) { note_value_assignment(); };

		// Constructors from primitive integer types
		explicit TIntBase1(_Ty   x) { note_value_assignment(); m_val = x; }

		template<typename _Tz>
		void assign_check_range(const _Tz &x) {
			note_value_assignment();
			g_assign_check_range<_Ty, _Tz>(x);
		}

		_Ty m_val;

#ifdef MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		void assert_initialized() const { assert(m_initialized); }
		bool m_initialized = false;
#else // MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
		void assert_initialized() const {}
#endif // MSE_CHECK_USE_BEFORE_SET
	};

	class CInt : public TIntBase1<MSE_CINT_BASE_INTEGER_TYPE> {
	public:
		typedef MSE_CINT_BASE_INTEGER_TYPE _Ty;
		typedef TIntBase1<_Ty> _Myt;

		// Constructs zero.
		CInt() : _Myt() {}

		// Copy constructor
		CInt(const CInt &x) : _Myt(x) {};
		CInt(const _Myt &x) : _Myt(x) {};

		// Assignment operator
		CInt& operator=(const CInt &x) { (*this).note_value_assignment(); m_val = x.m_val; return (*this); }
		//CInt& operator=(const _Ty &x) { (*this).note_value_assignment(); m_val = x; return (*this); }

		CInt& operator=(long long x) { assign_check_range<long long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CInt& operator=(long x) { assign_check_range<long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CInt& operator=(int x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CInt& operator=(short x) { assign_check_range<short>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CInt& operator=(char x) { assign_check_range<char>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CInt& operator=(size_t x) { assign_check_range<size_t>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CInt& operator=(CSize_t x) { assign_check_range<size_t>(x.as_a_size_t()); m_val = x.as_a_size_t(); return (*this); }
		/* We would have liked to have assignment operators for the unsigned primitive integer types, but one of them could
		potentially clash with the size_t assignment operator. */
		//CInt& operator=(unsigned long long x) { assign_check_range<unsigned long long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CInt& operator=(unsigned long x) { assign_check_range<unsigned long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CInt& operator=(unsigned int x) { assign_check_range<unsigned int>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CInt& operator=(unsigned short x) { assign_check_range<unsigned short>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CInt& operator=(unsigned char x) { assign_check_range<unsigned char>(x); m_val = static_cast<_Ty>(x); return (*this); }

		// Constructors from primitive integer types
		//CInt(_Ty   x) { m_val = x; }
		CInt(long long  x) { assign_check_range<long long>(x); m_val = static_cast<_Ty>(x); }
		CInt(long  x) { assign_check_range< long>(x); m_val = static_cast<_Ty>(x); }
		CInt(int   x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); }
		CInt(short x) { assign_check_range<short>(x); m_val = static_cast<_Ty>(x); }
		CInt(char x) { assign_check_range<char>(x); m_val = static_cast<_Ty>(x); }
		CInt(size_t   x) { assign_check_range<size_t>(x); m_val = static_cast<_Ty>(x); }
		//CInt(CSize_t   x) { assign_check_range<size_t>(x.as_a_size_t()); m_val = x.as_a_size_t(); }
		/* We would have liked to have constructors for the unsigned primitive integer types, but one of them could
		potentially clash with the size_t constructor. */
		//CInt(unsigned long long  x) { assign_check_range<unsigned long long>(x); m_val = static_cast<_Ty>(x); }
		//CInt(unsigned long  x) { assign_check_range<unsigned long>(x); m_val = static_cast<_Ty>(x); }
		//CInt(unsigned int   x) { assign_check_range<unsigned int>(x); m_val = static_cast<_Ty>(x); }
		//CInt(unsigned short x) { assign_check_range<unsigned short>(x); m_val = static_cast<_Ty>(x); }
		//CInt(unsigned char x) { assign_check_range<unsigned char>(x); m_val = static_cast<_Ty>(x); }

		// Casts to primitive integer types
		operator _Ty() const { (*this).assert_initialized(); return m_val; }

		CInt operator ~() const { (*this).assert_initialized(); return CInt(~m_val); }
		CInt& operator |=(const CInt &x) { (*this).assert_initialized(); m_val |= x.m_val; return (*this); }
		CInt& operator &=(const CInt &x) { (*this).assert_initialized(); m_val &= x.m_val; return (*this); }
		CInt& operator ^=(const CInt &x) { (*this).assert_initialized(); m_val ^= x.m_val; return (*this); }

		CInt operator -() const { (*this).assert_initialized(); return CInt(-m_val); }
		CInt& operator +=(const CInt &x) { (*this).assert_initialized(); m_val += x.m_val; return (*this); }
		CInt& operator -=(const CInt &x) { (*this).assert_initialized(); m_val -= x.m_val; return (*this); }
		CInt& operator *=(const CInt &x) { (*this).assert_initialized(); m_val *= x.m_val; return (*this); }
		CInt& operator /=(const CInt &x) { (*this).assert_initialized(); m_val /= x.m_val; return (*this); }
		CInt& operator %=(const CInt &x) { (*this).assert_initialized(); m_val %= x.m_val; return (*this); }
		CInt& operator >>=(const CInt &x) { (*this).assert_initialized(); m_val >>= x.m_val; return (*this); }
		CInt& operator <<=(const CInt &x) { (*this).assert_initialized(); m_val <<= x.m_val; return (*this); }

		CInt operator +(const CInt &x) const { (*this).assert_initialized(); return CInt(m_val + x.m_val); }
		CInt operator +(long long x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(long x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(int x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(short x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(char x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(size_t x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		//CInt operator +(CSize_t x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }

		CInt operator -(const CInt &x) const { (*this).assert_initialized(); return CInt(m_val - x.m_val); }
		CInt operator -(long long x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(long x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(int x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(short x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(char x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(size_t x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		//CInt operator -(CSize_t x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }

		CInt operator *(const CInt &x) const { (*this).assert_initialized(); return CInt(m_val * x.m_val); }
		CInt operator *(long long x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(long x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(int x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(short x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(char x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(size_t x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		//CInt operator *(CSize_t x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }

		CInt operator /(const CInt &x) const { (*this).assert_initialized(); return CInt(m_val / x.m_val); }
		CInt operator /(long long x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(long x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(int x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(short x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(char x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(size_t x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		//CInt operator /(CSize_t x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }

		bool operator <(const CInt &x) const { (*this).assert_initialized(); return (m_val < x.m_val); }
		bool operator <(long long x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(long x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(int x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(short x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(char x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(size_t x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		//bool operator <(CSize_t x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }

		bool operator >(const CInt &x) const { (*this).assert_initialized(); return (m_val > x.m_val); }
		bool operator >(long long x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(long x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(int x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(short x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(char x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(size_t x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		//bool operator >(CSize_t x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }

		bool operator <=(const CInt &x) const { (*this).assert_initialized(); return (m_val <= x.m_val); }
		bool operator <=(long long x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(long x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(int x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(short x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(char x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(size_t x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		//bool operator <=(CSize_t x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }

		bool operator >=(const CInt &x) const { (*this).assert_initialized(); return (m_val >= x.m_val); }
		bool operator >=(long long x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(long x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(int x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(short x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(char x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(size_t x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		//bool operator >=(CSize_t x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }

		bool operator ==(const CInt &x) const { (*this).assert_initialized(); return (m_val == x.m_val); }
		bool operator ==(long long x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(long x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(int x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(short x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(char x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(size_t x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		//bool operator ==(CSize_t x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }

		bool operator !=(const CInt &x) const { (*this).assert_initialized(); return (m_val != x.m_val); }
		bool operator !=(long long x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(long x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(int x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(short x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(char x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(size_t x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		//bool operator !=(CSize_t x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }

		// INCREMENT/DECREMENT OPERATORS
		CInt& operator ++() { (*this).assert_initialized(); m_val++; return (*this); }
		CInt operator ++(int) {
			(*this).assert_initialized();
			CInt tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		CInt& operator --() {
			(*this).assert_initialized();
			if (0 <= std::numeric_limits<_Ty>::lowest()) {
				(*this).assert_initialized();
				(*this) = (*this) - 1; return (*this);
			}
			else {
				(*this).assert_initialized();
				m_val--; return (*this);
			}
		}
		CInt operator --(int) {
			(*this).assert_initialized();
			CInt tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		//_Ty m_val;
	};
}

namespace std {
#ifndef _THROW0
#define _THROW0()
#endif // !_THROW0
#ifndef _STCONS
#define _STCONS(ty, name, val)	static constexpr ty name = (ty)(val)
#endif // !_STCONS

	template<> class numeric_limits<mse::CInt> {	// limits for type int
	public:
		typedef MSE_CINT_BASE_INTEGER_TYPE _Ty;

		static constexpr _Ty(min)() _THROW0()
		{	// return minimum value
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::min();
		}
		static constexpr _Ty(max)() _THROW0()
		{	// return maximum value
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::max();
		}
		static constexpr _Ty lowest() _THROW0()
		{	// return most negative value
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::lowest();
		}
		static constexpr _Ty epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::epsilon();
		}
		static constexpr _Ty round_error() _THROW0()
		{	// return largest rounding error
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::round_error();
		}
		static constexpr _Ty denorm_min() _THROW0()
		{	// return minimum denormalized value
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::denorm_min();
		}
		static constexpr _Ty infinity() _THROW0()
		{	// return positive infinity
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::infinity();
		}
		static constexpr _Ty quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::quiet_NaN();
		}
		static constexpr _Ty signaling_NaN() _THROW0()
		{	// return signaling NaN
			return numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::signaling_NaN();
		}
		_STCONS(float_denorm_style, has_denorm, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::has_denorm);
		_STCONS(bool, has_denorm_loss, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::has_denorm_loss);
		_STCONS(bool, has_infinity, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::has_infinity);
		_STCONS(bool, has_quiet_NaN, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::has_quiet_NaN);
		_STCONS(bool, has_signaling_NaN, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::has_signaling_NaN);
		_STCONS(bool, is_bounded, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_bounded);
		_STCONS(bool, is_exact, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_exact);
		_STCONS(bool, is_iec559, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_iec559);
		_STCONS(bool, is_integer, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_integer);
		_STCONS(bool, is_modulo, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_modulo);
		_STCONS(bool, is_signed, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_signed);
		_STCONS(bool, is_specialized, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::is_specialized);
		_STCONS(bool, tinyness_before, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::tinyness_before);
		_STCONS(bool, traps, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::traps);
		_STCONS(float_round_style, round_style, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::round_style);
		_STCONS(int, digits, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::digits);
		_STCONS(int, digits10, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::digits10);
		_STCONS(int, max_digits10, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::max_digits10);
		_STCONS(int, max_exponent, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::max_exponent);
		_STCONS(int, max_exponent10, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::max_exponent10);
		_STCONS(int, min_exponent, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::min_exponent);
		_STCONS(int, min_exponent10, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::min_exponent10);
		_STCONS(int, radix, numeric_limits<MSE_CINT_BASE_INTEGER_TYPE>::radix);
	};
}

namespace mse {
	class CSize_t;
	static size_t as_a_size_t(CSize_t n);

	/* Note that CSize_t does not have a default conversion to size_t. This is by design. Use the as_a_size_t() member
	function to get a size_t when necessary. */
	class CSize_t : public TIntBase1<size_t> {
	public:
		typedef size_t _Ty;
		typedef int _T_signed_primitive_integer_type;
		typedef TIntBase1<_Ty> _Myt;

		// Constructs zero.
		CSize_t() : _Myt() {}

		// Copy constructor
		CSize_t(const CSize_t &x) : _Myt(x) {};
		CSize_t(const _Myt &x) : _Myt(x) {};

		// Assignment operator
		CSize_t& operator=(const CSize_t &x) { m_val = x.m_val; return (*this); }
		//CSize_t& operator=(const _Ty &x) { m_val = x; return (*this); }

		CSize_t& operator=(long long x) { assign_check_range<long long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(long x) { assign_check_range<long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(int x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(short x) { assign_check_range<short>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(char x) { assign_check_range<char>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(size_t x) { assign_check_range<size_t>(x); m_val = static_cast<_Ty>(x); return (*this); }
		CSize_t& operator=(CInt x) { assign_check_range<MSE_CINT_BASE_INTEGER_TYPE>(x); m_val = static_cast<_Ty>(x); return (*this); }
		/* We would have liked to have assignment operators for the unsigned primitive integer types, but one of them could
		potentially clash with the size_t assignment operator. */
		//CSize_t& operator=(unsigned long long x) { assign_check_range<unsigned long long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CSize_t& operator=(unsigned long x) { assign_check_range<unsigned long>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CSize_t& operator=(unsigned int x) { assign_check_range<unsigned int>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CSize_t& operator=(unsigned short x) { assign_check_range<unsigned short>(x); m_val = static_cast<_Ty>(x); return (*this); }
		//CSize_t& operator=(unsigned char x) { assign_check_range<unsigned char>(x); m_val = static_cast<_Ty>(x); return (*this); }

		// Constructors from primitive integer types
		//explicit CSize_t(_Ty   x) { m_val = x; }
		explicit CSize_t(long long  x) { assign_check_range<long long>(x); m_val = static_cast<_Ty>(x); }
		explicit CSize_t(long  x) { assign_check_range< long>(x); m_val = static_cast<_Ty>(x); }
		explicit CSize_t(int   x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); }
		explicit CSize_t(short x) { assign_check_range<short>(x); m_val = static_cast<_Ty>(x); }
		explicit CSize_t(char x) { assign_check_range<char>(x); m_val = static_cast<_Ty>(x); }
		CSize_t(size_t   x) { assign_check_range<size_t>(x); m_val = static_cast<_Ty>(x); }
		/*explicit */CSize_t(CInt   x) { assign_check_range<MSE_CINT_BASE_INTEGER_TYPE>(x); m_val = static_cast<_Ty>(x); }
		/* We would have liked to have constructors for the unsigned primitive integer types, but one of them could
		potentially clash with the size_t constructor. */
		//explicit CSize_t(unsigned long long  x) { assign_check_range<unsigned long long>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned long  x) { assign_check_range<unsigned long>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned int   x) { assign_check_range<unsigned int>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned short x) { assign_check_range<unsigned short>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned char x) { assign_check_range<unsigned char>(x); m_val = static_cast<_Ty>(x); }

		// Casts to primitive integer types
		operator CInt() const { (*this).assert_initialized(); return CInt(m_val); }
#ifndef MSVC2010_COMPATIBLE
		explicit operator size_t() const { (*this).assert_initialized(); return (m_val); }
#endif /*MSVC2010_COMPATIBLE*/
		//size_t as_a_size_t() const { (*this).assert_initialized(); return m_val; }

		CSize_t operator ~() const { (*this).assert_initialized(); return (~m_val); }
		CSize_t& operator |=(const CSize_t &x) { (*this).assert_initialized(); m_val |= x.m_val; return (*this); }
		CSize_t& operator &=(const CSize_t &x) { (*this).assert_initialized(); m_val &= x.m_val; return (*this); }
		CSize_t& operator ^=(const CSize_t &x) { (*this).assert_initialized(); m_val ^= x.m_val; return (*this); }

		CInt operator -() const { (*this).assert_initialized(); /* Should unsigned types even support this opperator? */
			return (-(CInt(m_val)));
		}
		CSize_t& operator +=(const CSize_t &x) { (*this).assert_initialized(); m_val += x.m_val; return (*this); }
		CSize_t& operator -=(const CSize_t &x) {
			(*this).assert_initialized();
			//assert(0 <= std::numeric_limits<_Ty>::lowest());
			if (x.m_val > m_val) {
				MSE_THROW(primitives_range_error("range error - value to be assigned is out of range of the target (integer) type"));
			}
			m_val -= x.m_val; return (*this);
		}
		CSize_t& operator *=(const CSize_t &x) { (*this).assert_initialized(); m_val *= x.m_val; return (*this); }
		CSize_t& operator /=(const CSize_t &x) { (*this).assert_initialized(); m_val /= x.m_val; return (*this); }
		CSize_t& operator %=(const CSize_t &x) { (*this).assert_initialized(); m_val %= x.m_val; return (*this); }
		CSize_t& operator >>=(const CSize_t &x) { (*this).assert_initialized(); m_val >>= x.m_val; return (*this); }
		CSize_t& operator <<=(const CSize_t &x) { (*this).assert_initialized(); m_val <<= x.m_val; return (*this); }

		CSize_t operator +(const CSize_t &x) const { (*this).assert_initialized(); return (m_val + x.m_val); }
		CInt operator +(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) + x); }
		CInt operator +(long long x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(long x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(int x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(short x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CInt operator +(char x) const { (*this).assert_initialized(); return ((*this) + CInt(x)); }
		CSize_t operator +(size_t x) const { (*this).assert_initialized(); return ((*this) + CSize_t(x)); }

		CInt operator -(const CSize_t &x) const { (*this).assert_initialized(); return (CInt(m_val) - CInt(x.m_val)); }
		CInt operator -(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) - x); }
		CInt operator -(long long x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(long x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(int x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(short x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(char x) const { (*this).assert_initialized(); return ((*this) - CInt(x)); }
		CInt operator -(size_t x) const { (*this).assert_initialized(); return ((*this) - CSize_t(x)); }

		CSize_t operator *(const CSize_t &x) const { (*this).assert_initialized(); return (m_val * x.m_val); }
		CInt operator *(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) * x); }
		CInt operator *(long long x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(long x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(int x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(short x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CInt operator *(char x) const { (*this).assert_initialized(); return ((*this) * CInt(x)); }
		CSize_t operator *(size_t x) const { (*this).assert_initialized(); return ((*this) * CSize_t(x)); }

		CSize_t operator /(const CSize_t &x) const { (*this).assert_initialized(); return (m_val / x.m_val); }
		CInt operator /(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) / x); }
		CInt operator /(long long x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(long x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(int x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(short x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CInt operator /(char x) const { (*this).assert_initialized(); return ((*this) / CInt(x)); }
		CSize_t operator /(size_t x) const { (*this).assert_initialized(); return ((*this) / CSize_t(x)); }

		bool operator <(const CSize_t &x) const { (*this).assert_initialized(); return (m_val < x.m_val); }
		bool operator <(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) < x); }
		bool operator <(long long x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(long x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(int x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(short x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(char x) const { (*this).assert_initialized(); return ((*this) < CInt(x)); }
		bool operator <(size_t x) const { (*this).assert_initialized(); return ((*this) < CSize_t(x)); }

		bool operator >(const CSize_t &x) const { (*this).assert_initialized(); return (m_val > x.m_val); }
		bool operator >(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) > x); }
		bool operator >(long long x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(long x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(int x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(short x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(char x) const { (*this).assert_initialized(); return ((*this) > CInt(x)); }
		bool operator >(size_t x) const { (*this).assert_initialized(); return ((*this) > CSize_t(x)); }

		bool operator <=(const CSize_t &x) const { (*this).assert_initialized(); return (m_val <= x.m_val); }
		bool operator <=(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) <= x); }
		bool operator <=(long long x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(long x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(int x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(short x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(char x) const { (*this).assert_initialized(); return ((*this) <= CInt(x)); }
		bool operator <=(size_t x) const { (*this).assert_initialized(); return ((*this) <= CSize_t(x)); }

		bool operator >=(const CSize_t &x) const { (*this).assert_initialized(); return (m_val >= x.m_val); }
		bool operator >=(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) >= x); }
		bool operator >=(long long x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(long x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(int x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(short x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(char x) const { (*this).assert_initialized(); return ((*this) >= CInt(x)); }
		bool operator >=(size_t x) const { (*this).assert_initialized(); return ((*this) >= CSize_t(x)); }

		bool operator ==(const CSize_t &x) const { (*this).assert_initialized(); return (m_val == x.m_val); }
		bool operator ==(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) == x); }
		bool operator ==(long long x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(long x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(int x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(short x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(char x) const { (*this).assert_initialized(); return ((*this) == CInt(x)); }
		bool operator ==(size_t x) const { (*this).assert_initialized(); return ((*this) == CSize_t(x)); }

		bool operator !=(const CSize_t &x) const { (*this).assert_initialized(); return (m_val != x.m_val); }
		bool operator !=(const CInt &x) const { (*this).assert_initialized(); return (CInt(m_val) != x); }
		bool operator !=(long long x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(long x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(int x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(short x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(char x) const { (*this).assert_initialized(); return ((*this) != CInt(x)); }
		bool operator !=(size_t x) const { (*this).assert_initialized(); return ((*this) != CSize_t(x)); }

		// INCREMENT/DECREMENT OPERATORS
		CSize_t& operator ++() { (*this).assert_initialized(); m_val++; return (*this); }
		CSize_t operator ++(int) { (*this).assert_initialized();
			CSize_t tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		CSize_t& operator --() { (*this).assert_initialized();
			if (0 <= std::numeric_limits<_Ty>::lowest()) { (*this).assert_initialized();
				(*this) = (*this) - 1; return (*this);
			}
			else { (*this).assert_initialized();
				m_val--; return (*this);
			}
		}
		CSize_t operator --(int) { (*this).assert_initialized();
			CSize_t tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		//_Ty m_val;

		friend size_t as_a_size_t(CSize_t n);
	};
	size_t as_a_size_t(CSize_t n) { n.assert_initialized(); return n.m_val; }
}

namespace std {
#ifndef _THROW0
#define _THROW0()
#endif // !_THROW0
#ifndef _STCONS
#define _STCONS(ty, name, val)	static constexpr ty name = (ty)(val)
#endif // !_STCONS

	template<> class numeric_limits<mse::CSize_t> {	// limits for type int
	public:
		typedef size_t _Ty;

		static constexpr _Ty(min)() _THROW0()
		{	// return minimum value
			return numeric_limits<size_t>::min();
		}
		static constexpr _Ty(max)() _THROW0()
		{	// return maximum value
			return numeric_limits<size_t>::max();
		}
		static constexpr _Ty lowest() _THROW0()
		{	// return most negative value
			return numeric_limits<size_t>::lowest();
		}
		static constexpr _Ty epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return numeric_limits<size_t>::epsilon();
		}
		static constexpr _Ty round_error() _THROW0()
		{	// return largest rounding error
			return numeric_limits<size_t>::round_error();
		}
		static constexpr _Ty denorm_min() _THROW0()
		{	// return minimum denormalized value
			return numeric_limits<size_t>::denorm_min();
		}
		static constexpr _Ty infinity() _THROW0()
		{	// return positive infinity
			return numeric_limits<size_t>::infinity();
		}
		static constexpr _Ty quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return numeric_limits<size_t>::quiet_NaN();
		}
		static constexpr _Ty signaling_NaN() _THROW0()
		{	// return signaling NaN
			return numeric_limits<size_t>::signaling_NaN();
		}
		_STCONS(float_denorm_style, has_denorm, numeric_limits<size_t>::has_denorm);
		_STCONS(bool, has_denorm_loss, numeric_limits<size_t>::has_denorm_loss);
		_STCONS(bool, has_infinity, numeric_limits<size_t>::has_infinity);
		_STCONS(bool, has_quiet_NaN, numeric_limits<size_t>::has_quiet_NaN);
		_STCONS(bool, has_signaling_NaN, numeric_limits<size_t>::has_signaling_NaN);
		_STCONS(bool, is_bounded, numeric_limits<size_t>::is_bounded);
		_STCONS(bool, is_exact, numeric_limits<size_t>::is_exact);
		_STCONS(bool, is_iec559, numeric_limits<size_t>::is_iec559);
		_STCONS(bool, is_integer, numeric_limits<size_t>::is_integer);
		_STCONS(bool, is_modulo, numeric_limits<size_t>::is_modulo);
		_STCONS(bool, is_signed, numeric_limits<size_t>::is_signed);
		_STCONS(bool, is_specialized, numeric_limits<size_t>::is_specialized);
		_STCONS(bool, tinyness_before, numeric_limits<size_t>::tinyness_before);
		_STCONS(bool, traps, numeric_limits<size_t>::traps);
		_STCONS(float_round_style, round_style, numeric_limits<size_t>::round_style);
		_STCONS(int, digits, numeric_limits<size_t>::digits);
		_STCONS(int, digits10, numeric_limits<size_t>::digits10);
		_STCONS(int, max_digits10, numeric_limits<size_t>::max_digits10);
		_STCONS(int, max_exponent, numeric_limits<size_t>::max_exponent);
		_STCONS(int, max_exponent10, numeric_limits<size_t>::max_exponent10);
		_STCONS(int, min_exponent, numeric_limits<size_t>::min_exponent);
		_STCONS(int, min_exponent10, numeric_limits<size_t>::min_exponent10);
		_STCONS(int, radix, numeric_limits<size_t>::radix);
	};
}

namespace mse {

	inline CInt operator+(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); rhs.assert_initialized(); return CSize_t(lhs) + rhs; }
	inline CSize_t operator+(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) + rhs; }
	inline CInt operator+(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) + rhs; }
	inline CInt operator+(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) + as_a_size_t(rhs); }
	inline CInt operator+(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs + as_a_size_t(rhs); }
	inline CInt operator-(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) - rhs; }
	inline CInt operator-(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) - rhs; }
	inline CInt operator-(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) - rhs; }
	inline CInt operator-(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) - as_a_size_t(rhs); }
	inline CInt operator-(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs - as_a_size_t(rhs); }
	inline CInt operator*(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) * rhs; }
	inline CSize_t operator*(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) * rhs; }
	inline CInt operator*(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) * rhs; }
	inline CInt operator*(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) * as_a_size_t(rhs); }
	inline CInt operator*(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs * as_a_size_t(rhs); }
	inline CInt operator/(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) / rhs; }
	inline CSize_t operator/(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) / rhs; }
	inline CInt operator/(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) / rhs; }
	inline CInt operator/(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) / as_a_size_t(rhs); }
	inline CInt operator/(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs / as_a_size_t(rhs); }

	inline bool operator<(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) < rhs; }
	inline bool operator<(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) < rhs; }
	inline bool operator<(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) < rhs; }
	inline bool operator<(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) < as_a_size_t(rhs); }
	inline bool operator<(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) < rhs; }
	inline bool operator<(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) < as_a_size_t(rhs); }
	inline bool operator<(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs < as_a_size_t(rhs); }
	inline bool operator>(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) > rhs; }
	inline bool operator>(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) > rhs; }
	inline bool operator>(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) > rhs; }
	inline bool operator>(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) > as_a_size_t(rhs); }
	inline bool operator>(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) > rhs; }
	inline bool operator>(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) > as_a_size_t(rhs); }
	inline bool operator>(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs > as_a_size_t(rhs); }
	inline bool operator<=(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) <= rhs; }
	inline bool operator<=(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) <= rhs; }
	inline bool operator<=(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) <= rhs; }
	inline bool operator<=(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) <= as_a_size_t(rhs); }
	inline bool operator<=(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) <= rhs; }
	inline bool operator<=(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) <= as_a_size_t(rhs); }
	inline bool operator<=(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs <= as_a_size_t(rhs); }
	inline bool operator>=(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) >= rhs; }
	inline bool operator>=(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) >= rhs; }
	inline bool operator>=(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) >= rhs; }
	inline bool operator>=(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) >= as_a_size_t(rhs); }
	inline bool operator>=(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) >= rhs; }
	inline bool operator>=(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) >= as_a_size_t(rhs); }
	inline bool operator>=(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs >= as_a_size_t(rhs); }
	inline bool operator==(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) == rhs; }
	inline bool operator==(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) == rhs; }
	inline bool operator==(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) == rhs; }
	inline bool operator==(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) == as_a_size_t(rhs); }
	inline bool operator==(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) == rhs; }
	inline bool operator==(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) == as_a_size_t(rhs); }
	inline bool operator==(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs == as_a_size_t(rhs); }
	inline bool operator!=(size_t lhs, const CInt &rhs) { rhs.assert_initialized(); return CSize_t(lhs) != rhs; }
	inline bool operator!=(size_t lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CSize_t(lhs) != rhs; }
	inline bool operator!=(int lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) != rhs; }
	inline bool operator!=(int lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) != as_a_size_t(rhs); }
	inline bool operator!=(long long lhs, const CInt &rhs) { rhs.assert_initialized(); return CInt(lhs) != rhs; }
	inline bool operator!=(long long lhs, const CSize_t &rhs) { rhs.assert_initialized(); return CInt(lhs) != as_a_size_t(rhs); }
	inline bool operator!=(const CInt &lhs, const CSize_t &rhs) { rhs.assert_initialized(); return lhs != as_a_size_t(rhs); }
#endif /*MSE_PRIMITIVES_DISABLED*/

	static void s_type_test1() {
#ifdef MSE_SELF_TESTS
		CInt i1(3);
		CInt i2 = 5;
		CInt i3;
		i3 = 7;
		CInt i4 = i1 + i2;
		i4 = i1 + 17;
		i4 = 19 + i1;
		i4 += i2;
		i4 -= 23;
		i4++;
		CBool b1 = (i1 < i2);
		b1 = (i1 < 17);
		b1 = (19 < i1);
		b1 = (i1 == i2);
		b1 = (i1 == 17);
		b1 = (19 == i1);

		CSize_t szt1(3);
		CSize_t szt2 = 5;
		CSize_t szt3;
		szt3 = 7;
		CSize_t szt4 = szt1 + szt2;
		szt4 = szt1 + 17;
		szt4 = 19 + szt1;
		CInt i11 = 19 + szt1;
		szt4 += szt2;
		szt4 -= 23;
		szt4++;
#ifndef MSVC2010_COMPATIBLE
		size_t szt5 = (size_t)szt4;
#endif /*MSVC2010_COMPATIBLE*/
		bool b3 = (szt1 < szt2);
		b3 = (szt1 < 17);
		b3 = (19 < szt1);
		CBool b2 = (19 < szt1);
		b3 = (szt1 == szt2);
		b3 = (szt1 == 17);
		b3 = (19 == szt1);
		CBool b4 = (b1 < b2);
		b4 = (b1 == b2);
		b4 = (b1 > b3);
		b4 = (b3 >= b1);
		b4 = (b3 == b1);
		b4 = (b1 && b2);
		b4 = (b1 || b3);
		b4 = (b3 && b1);
		b4 |= b1;
		b4 &= b3;
#endif // MSE_SELF_TESTS
	}
}

#undef MSE_THROW

#endif /*ndef MSEPRIMITIVES_H*/
