
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPRIMITIVES_H
#define MSEPRIMITIVES_H

#include <assert.h>
#include <limits>       // std::numeric_limits
#include <stdexcept>      // std::out_of_range
#include <unordered_set>

/*compiler specific defines*/
#ifdef _MSC_VER
#if (1700 > _MSC_VER)
#define MSVC2010_COMPATIBILE 1
#endif /*(1700 > _MSC_VER)*/
#else /*_MSC_VER*/
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBILE 1
#define GPP4P8_COMPATIBILE 1
#endif
#endif /*_MSC_VER*/


namespace mse {

	/* Some older compilers (i.e. msvc2013) do not yet support inherited constructors, so we provide this macro hack as a
	substitute. So instead of writng "using CBaseClass::CBaseClass;" you can instead write
	"MSE_USING(CDerivedClass, CBaseClass)" */
#define MSE_USING(Derived, Base)                                 \
    template<typename ...Args,                               \
             typename = typename std::enable_if              \
             <                                               \
                std::is_constructible<Base, Args...>::value  \
			 			 			 			              >::type>                                        \
    Derived(Args &&...args)                                  \
        : Base(std::forward<Args>(args)...) { }              \


	/* When the mse primitive replacements are "disabled" they lose their default initialization and may cause problems for
	code that relies on it. */
#ifdef MSE_PRIMITIVES_DISABLED
	typedef bool CBool;
	typedef int CInt;
	typedef size_t CSize_t;
	static size_t as_a_size_t(CSize_t n) { return (n); }
#else /*MSE_PRIMITIVES_DISABLED*/
	/* This class is just meant to act like the "bool" type, except that it has a default intialization value (false). */
	class CBool {
	public:
		// Constructs zero.
		CBool() : m_val(false) {}

		// Copy constructor
		CBool(const CBool &x) : m_val(x.m_val) {};

		// Assignment operator
		CBool& operator=(const CBool &x) { m_val = x.m_val; return (*this); }

		// Constructors from primitive boolean types
		CBool(bool   x) { m_val = x; }

		// Casts to primitive boolean types
		operator bool() const { return m_val; }

		CBool& operator |=(const CBool &x) { m_val |= x.m_val; return (*this); }
		CBool& operator &=(const CBool &x) { m_val &= x.m_val; return (*this); }
		CBool& operator ^=(const CBool &x) { m_val ^= x.m_val; return (*this); }

		bool m_val;
	};

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
		TIntBase1(const TIntBase1 &x) : m_val(x.m_val) {};

		// Constructors from primitive integer types
		explicit TIntBase1(_Ty   x) { m_val = x; }

		template<typename _Tz>
		void assign_check_range(const _Tz &x) {
			/* Note that this function is going to cause "signed/unsigned mismatch" warnings during compile. But if you
			carefully consider each case, the default conversion works for our purposes. */
			/*constexpr*/ bool rhs_can_exceed_upper_bound = (std::numeric_limits<_Tz>::max() > std::numeric_limits<_Ty>::max());
			bool rhs_can_exceed_lower_bound = false;
			/* We're assuming that std::numeric_limits<>::lowest() will never be greater than zero. */
			if ((0 != std::numeric_limits<_Tz>::lowest()) && (0 == std::numeric_limits<_Ty>::lowest())) {
				rhs_can_exceed_lower_bound = true;
			}
			else if ((0 != std::numeric_limits<_Tz>::lowest()) && (0 != std::numeric_limits<_Ty>::lowest())) {
				rhs_can_exceed_lower_bound = (std::numeric_limits<_Tz>::lowest() < std::numeric_limits<_Ty>::lowest());
			}
			if (rhs_can_exceed_upper_bound || rhs_can_exceed_lower_bound) {
				if (rhs_can_exceed_upper_bound) {
					if (x > std::numeric_limits<_Ty>::max()) {
						throw(std::out_of_range("out of range error - value to be assigned is out of range of the target (integer) type"));
					}
				}
				if (rhs_can_exceed_lower_bound) {
					bool lb_exceeded = false;
					/* We're assuming that std::numeric_limits<>::lowest() will never be greater than zero. */
					if (0 > x) {
						if (0 == std::numeric_limits<_Ty>::lowest()) {
							lb_exceeded = true;
						}
						else {
							lb_exceeded = (x < std::numeric_limits<_Ty>::lowest());
						}
					}
					if (lb_exceeded) {
						throw(std::out_of_range("out of range error - value to be assigned is out of range of the target (integer) type"));
					}
				}
			}
		}

		_Ty m_val;
	};

	class CInt : public TIntBase1<int> {
	public:
		typedef int _Ty;
		typedef TIntBase1<_Ty> _Myt;

		// Constructs zero.
		CInt() : _Myt() {}

		// Copy constructor
		CInt(const CInt &x) : _Myt(x) {};
		CInt(const _Myt &x) : _Myt(x) {};

		// Assignment operator
		CInt& operator=(const CInt &x) { m_val = x.m_val; return (*this); }
		//CInt& operator=(const _Ty &x) { m_val = x; return (*this); }

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
		operator _Ty() const { return m_val; }

		CInt operator ~() const { return CInt(~m_val); }
		CInt& operator |=(const CInt &x) { m_val |= x.m_val; return (*this); }
		CInt& operator &=(const CInt &x) { m_val &= x.m_val; return (*this); }
		CInt& operator ^=(const CInt &x) { m_val ^= x.m_val; return (*this); }

		CInt operator -() const { return CInt(-m_val); }
		CInt& operator +=(const CInt &x) { m_val += x.m_val; return (*this); }
		CInt& operator -=(const CInt &x) {
			if (0 <= std::numeric_limits<_Ty>::lowest()) {
				if (x.m_val > m_val) { /*check this*/
					throw(std::out_of_range("out of range error - value to be assigned is out of range of the target (integer) type"));
				}
			}
			m_val -= x.m_val; return (*this);
		}
		CInt& operator *=(const CInt &x) { m_val *= x.m_val; return (*this); }
		CInt& operator /=(const CInt &x) { m_val /= x.m_val; return (*this); }
		CInt& operator %=(const CInt &x) { m_val %= x.m_val; return (*this); }
		CInt& operator >>=(const CInt &x) { m_val >>= x.m_val; return (*this); }
		CInt& operator <<=(const CInt &x) { m_val <<= x.m_val; return (*this); }

		CInt operator +(const CInt &x) const { return CInt(m_val + x.m_val); }
		CInt operator +(long long x) const { return ((*this) + CInt(x)); }
		CInt operator +(long x) const { return ((*this) + CInt(x)); }
		CInt operator +(int x) const { return ((*this) + CInt(x)); }
		CInt operator +(short x) const { return ((*this) + CInt(x)); }
		CInt operator +(char x) const { return ((*this) + CInt(x)); }
		CInt operator +(size_t x) const { return ((*this) + CInt(x)); }
		//CInt operator +(CSize_t x) const { return ((*this) + CInt(x)); }

		CInt operator -(const CInt &x) const { return CInt(m_val - x.m_val); }
		CInt operator -(long long x) const { return ((*this) - CInt(x)); }
		CInt operator -(long x) const { return ((*this) - CInt(x)); }
		CInt operator -(int x) const { return ((*this) - CInt(x)); }
		CInt operator -(short x) const { return ((*this) - CInt(x)); }
		CInt operator -(char x) const { return ((*this) - CInt(x)); }
		CInt operator -(size_t x) const { return ((*this) - CInt(x)); }
		//CInt operator -(CSize_t x) const { return ((*this) - CInt(x)); }

		CInt operator *(const CInt &x) const { return CInt(m_val * x.m_val); }
		CInt operator *(long long x) const { return ((*this) * CInt(x)); }
		CInt operator *(long x) const { return ((*this) * CInt(x)); }
		CInt operator *(int x) const { return ((*this) * CInt(x)); }
		CInt operator *(short x) const { return ((*this) * CInt(x)); }
		CInt operator *(char x) const { return ((*this) * CInt(x)); }
		CInt operator *(size_t x) const { return ((*this) * CInt(x)); }
		//CInt operator *(CSize_t x) const { return ((*this) * CInt(x)); }

		CInt operator /(const CInt &x) const { return CInt(m_val / x.m_val); }
		CInt operator /(long long x) const { return ((*this) / CInt(x)); }
		CInt operator /(long x) const { return ((*this) / CInt(x)); }
		CInt operator /(int x) const { return ((*this) / CInt(x)); }
		CInt operator /(short x) const { return ((*this) / CInt(x)); }
		CInt operator /(char x) const { return ((*this) / CInt(x)); }
		CInt operator /(size_t x) const { return ((*this) / CInt(x)); }
		//CInt operator /(CSize_t x) const { return ((*this) / CInt(x)); }

		bool operator <(const CInt &x) const { return (m_val < x.m_val); }
		bool operator <(long long x) const { return ((*this) < CInt(x)); }
		bool operator <(long x) const { return ((*this) < CInt(x)); }
		bool operator <(int x) const { return ((*this) < CInt(x)); }
		bool operator <(short x) const { return ((*this) < CInt(x)); }
		bool operator <(char x) const { return ((*this) < CInt(x)); }
		bool operator <(size_t x) const { return ((*this) < CInt(x)); }
		//bool operator <(CSize_t x) const { return ((*this) < CInt(x)); }

		bool operator >(const CInt &x) const { return (m_val > x.m_val); }
		bool operator >(long long x) const { return ((*this) > CInt(x)); }
		bool operator >(long x) const { return ((*this) > CInt(x)); }
		bool operator >(int x) const { return ((*this) > CInt(x)); }
		bool operator >(short x) const { return ((*this) > CInt(x)); }
		bool operator >(char x) const { return ((*this) > CInt(x)); }
		bool operator >(size_t x) const { return ((*this) > CInt(x)); }
		//bool operator >(CSize_t x) const { return ((*this) > CInt(x)); }

		bool operator <=(const CInt &x) const { return (m_val <= x.m_val); }
		bool operator <=(long long x) const { return ((*this) <= CInt(x)); }
		bool operator <=(long x) const { return ((*this) <= CInt(x)); }
		bool operator <=(int x) const { return ((*this) <= CInt(x)); }
		bool operator <=(short x) const { return ((*this) <= CInt(x)); }
		bool operator <=(char x) const { return ((*this) <= CInt(x)); }
		bool operator <=(size_t x) const { return ((*this) <= CInt(x)); }
		//bool operator <=(CSize_t x) const { return ((*this) <= CInt(x)); }

		bool operator >=(const CInt &x) const { return (m_val >= x.m_val); }
		bool operator >=(long long x) const { return ((*this) >= CInt(x)); }
		bool operator >=(long x) const { return ((*this) >= CInt(x)); }
		bool operator >=(int x) const { return ((*this) >= CInt(x)); }
		bool operator >=(short x) const { return ((*this) >= CInt(x)); }
		bool operator >=(char x) const { return ((*this) >= CInt(x)); }
		bool operator >=(size_t x) const { return ((*this) >= CInt(x)); }
		//bool operator >=(CSize_t x) const { return ((*this) >= CInt(x)); }

		bool operator ==(const CInt &x) const { return (m_val == x.m_val); }
		bool operator ==(long long x) const { return ((*this) == CInt(x)); }
		bool operator ==(long x) const { return ((*this) == CInt(x)); }
		bool operator ==(int x) const { return ((*this) == CInt(x)); }
		bool operator ==(short x) const { return ((*this) == CInt(x)); }
		bool operator ==(char x) const { return ((*this) == CInt(x)); }
		bool operator ==(size_t x) const { return ((*this) == CInt(x)); }
		//bool operator ==(CSize_t x) const { return ((*this) == CInt(x)); }

		bool operator !=(const CInt &x) const { return (m_val != x.m_val); }
		bool operator !=(long long x) const { return ((*this) != CInt(x)); }
		bool operator !=(long x) const { return ((*this) != CInt(x)); }
		bool operator !=(int x) const { return ((*this) != CInt(x)); }
		bool operator !=(short x) const { return ((*this) != CInt(x)); }
		bool operator !=(char x) const { return ((*this) != CInt(x)); }
		bool operator !=(size_t x) const { return ((*this) != CInt(x)); }
		//bool operator !=(CSize_t x) const { return ((*this) != CInt(x)); }

		// INCREMENT/DECREMENT OPERATORS
		CInt& operator ++() { m_val++; return (*this); }
		CInt operator ++(int) {
			CInt tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		CInt& operator --() {
			if (0 <= std::numeric_limits<_Ty>::lowest()) {
				(*this) = (*this) - 1; return (*this);
			}
			else {
				m_val--; return (*this);
			}
		}
		CInt operator --(int) {
			CInt tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		//_Ty m_val;
	};

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
		CSize_t& operator=(CInt x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); return (*this); }
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
		/*explicit */CSize_t(CInt   x) { assign_check_range<int>(x); m_val = static_cast<_Ty>(x); }
		/* We would have liked to have constructors for the unsigned primitive integer types, but one of them could
		potentially clash with the size_t constructor. */
		//explicit CSize_t(unsigned long long  x) { assign_check_range<unsigned long long>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned long  x) { assign_check_range<unsigned long>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned int   x) { assign_check_range<unsigned int>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned short x) { assign_check_range<unsigned short>(x); m_val = static_cast<_Ty>(x); }
		//explicit CSize_t(unsigned char x) { assign_check_range<unsigned char>(x); m_val = static_cast<_Ty>(x); }

		// Casts to primitive integer types
		operator CInt() const { return CInt(m_val); }
#ifndef MSVC2010_COMPATIBILE
		explicit operator size_t() const { return (m_val); }
#endif /*MSVC2010_COMPATIBILE*/
		//size_t as_a_size_t() const { return m_val; }

		CSize_t operator ~() const { return (~m_val); }
		CSize_t& operator |=(const CSize_t &x) { m_val |= x.m_val; return (*this); }
		CSize_t& operator &=(const CSize_t &x) { m_val &= x.m_val; return (*this); }
		CSize_t& operator ^=(const CSize_t &x) { m_val ^= x.m_val; return (*this); }

		CInt operator -() const { /* Should unsigned types even support this opperator? */
			return (-(CInt(m_val)));
		}
		CSize_t& operator +=(const CSize_t &x) { m_val += x.m_val; return (*this); }
		CSize_t& operator -=(const CSize_t &x) {
			if (0 <= std::numeric_limits<_Ty>::lowest()) {
				if (x.m_val > m_val) { /*check this*/
					throw(std::out_of_range("out of range error - value to be assigned is out of range of the target (integer) type"));
				}
			}
			m_val -= x.m_val; return (*this);
		}
		CSize_t& operator *=(const CSize_t &x) { m_val *= x.m_val; return (*this); }
		CSize_t& operator /=(const CSize_t &x) { m_val /= x.m_val; return (*this); }
		CSize_t& operator %=(const CSize_t &x) { m_val %= x.m_val; return (*this); }
		CSize_t& operator >>=(const CSize_t &x) { m_val >>= x.m_val; return (*this); }
		CSize_t& operator <<=(const CSize_t &x) { m_val <<= x.m_val; return (*this); }

		CSize_t operator +(const CSize_t &x) const { return (m_val + x.m_val); }
		CInt operator +(const CInt &x) const { return (CInt(m_val) + x); }
		CInt operator +(long long x) const { return ((*this) + CInt(x)); }
		CInt operator +(long x) const { return ((*this) + CInt(x)); }
		CInt operator +(int x) const { return ((*this) + CInt(x)); }
		CInt operator +(short x) const { return ((*this) + CInt(x)); }
		CInt operator +(char x) const { return ((*this) + CInt(x)); }
		CSize_t operator +(size_t x) const { return ((*this) + CSize_t(x)); }

		CInt operator -(const CSize_t &x) const { return (CInt(m_val) - CInt(x.m_val)); }
		CInt operator -(const CInt &x) const { return (CInt(m_val) - x); }
		CInt operator -(long long x) const { return ((*this) - CInt(x)); }
		CInt operator -(long x) const { return ((*this) - CInt(x)); }
		CInt operator -(int x) const { return ((*this) - CInt(x)); }
		CInt operator -(short x) const { return ((*this) - CInt(x)); }
		CInt operator -(char x) const { return ((*this) - CInt(x)); }
		CInt operator -(size_t x) const { return ((*this) - CSize_t(x)); }

		CSize_t operator *(const CSize_t &x) const { return (m_val * x.m_val); }
		CInt operator *(const CInt &x) const { return (CInt(m_val) * x); }
		CInt operator *(long long x) const { return ((*this) * CInt(x)); }
		CInt operator *(long x) const { return ((*this) * CInt(x)); }
		CInt operator *(int x) const { return ((*this) * CInt(x)); }
		CInt operator *(short x) const { return ((*this) * CInt(x)); }
		CInt operator *(char x) const { return ((*this) * CInt(x)); }
		CSize_t operator *(size_t x) const { return ((*this) * CSize_t(x)); }

		CSize_t operator /(const CSize_t &x) const { return (m_val / x.m_val); }
		CInt operator /(const CInt &x) const { return (CInt(m_val) / x); }
		CInt operator /(long long x) const { return ((*this) / CInt(x)); }
		CInt operator /(long x) const { return ((*this) / CInt(x)); }
		CInt operator /(int x) const { return ((*this) / CInt(x)); }
		CInt operator /(short x) const { return ((*this) / CInt(x)); }
		CInt operator /(char x) const { return ((*this) / CInt(x)); }
		CSize_t operator /(size_t x) const { return ((*this) / CSize_t(x)); }

		bool operator <(const CSize_t &x) const { return (m_val < x.m_val); }
		bool operator <(const CInt &x) const { return (CInt(m_val) < x); }
		bool operator <(long long x) const { return ((*this) < CInt(x)); }
		bool operator <(long x) const { return ((*this) < CInt(x)); }
		bool operator <(int x) const { return ((*this) < CInt(x)); }
		bool operator <(short x) const { return ((*this) < CInt(x)); }
		bool operator <(char x) const { return ((*this) < CInt(x)); }
		bool operator <(size_t x) const { return ((*this) < CSize_t(x)); }

		bool operator >(const CSize_t &x) const { return (m_val > x.m_val); }
		bool operator >(const CInt &x) const { return (CInt(m_val) > x); }
		bool operator >(long long x) const { return ((*this) > CInt(x)); }
		bool operator >(long x) const { return ((*this) > CInt(x)); }
		bool operator >(int x) const { return ((*this) > CInt(x)); }
		bool operator >(short x) const { return ((*this) > CInt(x)); }
		bool operator >(char x) const { return ((*this) > CInt(x)); }
		bool operator >(size_t x) const { return ((*this) > CSize_t(x)); }

		bool operator <=(const CSize_t &x) const { return (m_val <= x.m_val); }
		bool operator <=(const CInt &x) const { return (CInt(m_val) <= x); }
		bool operator <=(long long x) const { return ((*this) <= CInt(x)); }
		bool operator <=(long x) const { return ((*this) <= CInt(x)); }
		bool operator <=(int x) const { return ((*this) <= CInt(x)); }
		bool operator <=(short x) const { return ((*this) <= CInt(x)); }
		bool operator <=(char x) const { return ((*this) <= CInt(x)); }
		bool operator <=(size_t x) const { return ((*this) <= CSize_t(x)); }

		bool operator >=(const CSize_t &x) const { return (m_val >= x.m_val); }
		bool operator >=(const CInt &x) const { return (CInt(m_val) >= x); }
		bool operator >=(long long x) const { return ((*this) >= CInt(x)); }
		bool operator >=(long x) const { return ((*this) >= CInt(x)); }
		bool operator >=(int x) const { return ((*this) >= CInt(x)); }
		bool operator >=(short x) const { return ((*this) >= CInt(x)); }
		bool operator >=(char x) const { return ((*this) >= CInt(x)); }
		bool operator >=(size_t x) const { return ((*this) >= CSize_t(x)); }

		bool operator ==(const CSize_t &x) const { return (m_val == x.m_val); }
		bool operator ==(const CInt &x) const { return (CInt(m_val) == x); }
		bool operator ==(long long x) const { return ((*this) == CInt(x)); }
		bool operator ==(long x) const { return ((*this) == CInt(x)); }
		bool operator ==(int x) const { return ((*this) == CInt(x)); }
		bool operator ==(short x) const { return ((*this) == CInt(x)); }
		bool operator ==(char x) const { return ((*this) == CInt(x)); }
		bool operator ==(size_t x) const { return ((*this) == CSize_t(x)); }

		bool operator !=(const CSize_t &x) const { return (m_val != x.m_val); }
		bool operator !=(const CInt &x) const { return (CInt(m_val) != x); }
		bool operator !=(long long x) const { return ((*this) != CInt(x)); }
		bool operator !=(long x) const { return ((*this) != CInt(x)); }
		bool operator !=(int x) const { return ((*this) != CInt(x)); }
		bool operator !=(short x) const { return ((*this) != CInt(x)); }
		bool operator !=(char x) const { return ((*this) != CInt(x)); }
		bool operator !=(size_t x) const { return ((*this) != CSize_t(x)); }

		// INCREMENT/DECREMENT OPERATORS
		CSize_t& operator ++() { m_val++; return (*this); }
		CSize_t operator ++(int) {
			CSize_t tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		CSize_t& operator --() {
			if (0 <= std::numeric_limits<_Ty>::lowest()) {
				(*this) = (*this) - 1; return (*this);
			}
			else {
				m_val--; return (*this);
			}
		}
		CSize_t operator --(int) {
			CSize_t tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		//_Ty m_val;
	};
	static size_t as_a_size_t(CSize_t n) { return CInt(n); }

	inline CInt operator+(size_t lhs, const CInt &rhs) { return CSize_t(lhs) + rhs; }
	inline CSize_t operator+(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) + rhs; }
	inline CInt operator+(int lhs, const CInt &rhs) { return CInt(lhs) + rhs; }
	inline CInt operator+(int lhs, const CSize_t &rhs) { return CInt(lhs) + as_a_size_t(rhs); }
	inline CInt operator+(const CInt &lhs, const CSize_t &rhs) { return lhs + as_a_size_t(rhs); }
	inline CInt operator-(size_t lhs, const CInt &rhs) { return CSize_t(lhs) - rhs; }
	inline CInt operator-(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) - rhs; }
	inline CInt operator-(int lhs, const CInt &rhs) { return CInt(lhs) - rhs; }
	inline CInt operator-(int lhs, const CSize_t &rhs) { return CInt(lhs) - as_a_size_t(rhs); }
	inline CInt operator-(const CInt &lhs, const CSize_t &rhs) { return lhs - as_a_size_t(rhs); }
	inline CInt operator*(size_t lhs, const CInt &rhs) { return CSize_t(lhs) * rhs; }
	inline CSize_t operator*(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) * rhs; }
	inline CInt operator*(int lhs, const CInt &rhs) { return CInt(lhs) * rhs; }
	inline CInt operator*(int lhs, const CSize_t &rhs) { return CInt(lhs) * as_a_size_t(rhs); }
	inline CInt operator*(const CInt &lhs, const CSize_t &rhs) { return lhs * as_a_size_t(rhs); }
	inline CInt operator/(size_t lhs, const CInt &rhs) { return CSize_t(lhs) / rhs; }
	inline CSize_t operator/(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) / rhs; }
	inline CInt operator/(int lhs, const CInt &rhs) { return CInt(lhs) / rhs; }
	inline CInt operator/(int lhs, const CSize_t &rhs) { return CInt(lhs) / as_a_size_t(rhs); }
	inline CInt operator/(const CInt &lhs, const CSize_t &rhs) { return lhs / as_a_size_t(rhs); }

	inline bool operator<(size_t lhs, const CInt &rhs) { return CSize_t(lhs) < rhs; }
	inline bool operator<(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) < rhs; }
	inline bool operator<(int lhs, const CInt &rhs) { return CInt(lhs) < rhs; }
	inline bool operator<(int lhs, const CSize_t &rhs) { return CInt(lhs) < as_a_size_t(rhs); }
	inline bool operator<(long long lhs, const CInt &rhs) { return CInt(lhs) < rhs; }
	inline bool operator<(long long lhs, const CSize_t &rhs) { return CInt(lhs) < as_a_size_t(rhs); }
	inline bool operator<(const CInt &lhs, const CSize_t &rhs) { return lhs < as_a_size_t(rhs); }
	inline bool operator>(size_t lhs, const CInt &rhs) { return CSize_t(lhs) > rhs; }
	inline bool operator>(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) > rhs; }
	inline bool operator>(int lhs, const CInt &rhs) { return CInt(lhs) > rhs; }
	inline bool operator>(int lhs, const CSize_t &rhs) { return CInt(lhs) > as_a_size_t(rhs); }
	inline bool operator>(long long lhs, const CInt &rhs) { return CInt(lhs) > rhs; }
	inline bool operator>(long long lhs, const CSize_t &rhs) { return CInt(lhs) > as_a_size_t(rhs); }
	inline bool operator>(const CInt &lhs, const CSize_t &rhs) { return lhs > as_a_size_t(rhs); }
	inline bool operator<=(size_t lhs, const CInt &rhs) { return CSize_t(lhs) <= rhs; }
	inline bool operator<=(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) <= rhs; }
	inline bool operator<=(int lhs, const CInt &rhs) { return CInt(lhs) <= rhs; }
	inline bool operator<=(int lhs, const CSize_t &rhs) { return CInt(lhs) <= as_a_size_t(rhs); }
	inline bool operator<=(long long lhs, const CInt &rhs) { return CInt(lhs) <= rhs; }
	inline bool operator<=(long long lhs, const CSize_t &rhs) { return CInt(lhs) <= as_a_size_t(rhs); }
	inline bool operator<=(const CInt &lhs, const CSize_t &rhs) { return lhs <= as_a_size_t(rhs); }
	inline bool operator>=(size_t lhs, const CInt &rhs) { return CSize_t(lhs) >= rhs; }
	inline bool operator>=(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) >= rhs; }
	inline bool operator>=(int lhs, const CInt &rhs) { return CInt(lhs) >= rhs; }
	inline bool operator>=(int lhs, const CSize_t &rhs) { return CInt(lhs) >= as_a_size_t(rhs); }
	inline bool operator>=(long long lhs, const CInt &rhs) { return CInt(lhs) >= rhs; }
	inline bool operator>=(long long lhs, const CSize_t &rhs) { return CInt(lhs) >= as_a_size_t(rhs); }
	inline bool operator>=(const CInt &lhs, const CSize_t &rhs) { return lhs >= as_a_size_t(rhs); }
	inline bool operator==(size_t lhs, const CInt &rhs) { return CSize_t(lhs) == rhs; }
	inline bool operator==(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) == rhs; }
	inline bool operator==(int lhs, const CInt &rhs) { return CInt(lhs) == rhs; }
	inline bool operator==(int lhs, const CSize_t &rhs) { return CInt(lhs) == as_a_size_t(rhs); }
	inline bool operator==(long long lhs, const CInt &rhs) { return CInt(lhs) == rhs; }
	inline bool operator==(long long lhs, const CSize_t &rhs) { return CInt(lhs) == as_a_size_t(rhs); }
	inline bool operator==(const CInt &lhs, const CSize_t &rhs) { return lhs == as_a_size_t(rhs); }
	inline bool operator!=(size_t lhs, const CInt &rhs) { return CSize_t(lhs) != rhs; }
	inline bool operator!=(size_t lhs, const CSize_t &rhs) { return CSize_t(lhs) != rhs; }
	inline bool operator!=(int lhs, const CInt &rhs) { return CInt(lhs) != rhs; }
	inline bool operator!=(int lhs, const CSize_t &rhs) { return CInt(lhs) != as_a_size_t(rhs); }
	inline bool operator!=(long long lhs, const CInt &rhs) { return CInt(lhs) != rhs; }
	inline bool operator!=(long long lhs, const CSize_t &rhs) { return CInt(lhs) != as_a_size_t(rhs); }
	inline bool operator!=(const CInt &lhs, const CSize_t &rhs) { return lhs != as_a_size_t(rhs); }

	static void s_type_test1() {
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
#ifndef MSVC2010_COMPATIBILE
		size_t szt5 = (size_t)szt4;
#endif /*MSVC2010_COMPATIBILE*/
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
	}
#endif /*MSE_PRIMITIVES_DISABLED*/

	class CSaferPtrBase {
	public:
		/* setToNull() needs to be available even when the smart pointer is const, because the object it points to may become
		invalid (deleted). */
		virtual void setToNull() const = 0;
	};

	/* TSaferPtr behaves similar to, and is largely compatible with, native pointers. It's a bit safer in that it initializes to
	nullptr by default and checks for attempted dereference of null pointers. */
	template<typename _Ty>
	class TSaferPtr : public CSaferPtrBase {
	public:
		TSaferPtr() : m_ptr(nullptr) {}
		TSaferPtr(_Ty* ptr) : m_ptr(ptr) {}
		TSaferPtr(const TSaferPtr<_Ty>& src) : m_ptr(src.m_ptr) {}
		virtual ~TSaferPtr() {}

		virtual void setToNull() const { m_ptr = nullptr; }

		void pointer(_Ty* ptr) { m_ptr = ptr; }
		_Ty* pointer() const { return m_ptr; }
		_Ty& operator*() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TSaferPtr")); }
			return (*m_ptr);
		}
		_Ty* operator->() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TSaferPtr")); }
			return m_ptr;
		}
		TSaferPtr<_Ty>& operator=(_Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		TSaferPtr<_Ty>& operator=(const TSaferPtr<_Ty>& _Right_cref) {
			m_ptr = _Right_cref.m_ptr;
			return (*this);
		}
		bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSaferPtr<_Ty> &_Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const TSaferPtr<_Ty> &_Right_cref) const { return (!((*this) == _Right_cref)); }

		bool operator!() const { return (!m_ptr); }
		operator bool() const { return m_ptr; }

		explicit operator _Ty*() const {
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
		been deleted) even in cases when this smart pointer is const. */
		mutable _Ty* m_ptr;
	};

	/* TSaferPtrForLegacy is similar to TSaferPtr, but more readily converts to a native pointer implicitly. So when replacing
	native pointers with safer pointers in legacy code, fewer code changes (explicit casts) may be required when using this
	template. */
	template<typename _Ty>
	class TSaferPtrForLegacy : public CSaferPtrBase {
	public:
		TSaferPtrForLegacy() : m_ptr(nullptr) {}
		TSaferPtrForLegacy(_Ty* ptr) : m_ptr(ptr) {}
		virtual ~TSaferPtrForLegacy() {}

		virtual void setToNull() const { m_ptr = nullptr; }

		void pointer(_Ty* ptr) { m_ptr = ptr; }
		_Ty* pointer() const { return m_ptr; }
		_Ty& operator*() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TSaferPtrForLegacy")); }
			return (*m_ptr);
		}
		_Ty* operator->() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TSaferPtrForLegacy")); }
			return m_ptr;
		}
		TSaferPtrForLegacy<_Ty>& operator=(_Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		//operator bool() const { return m_ptr; }

		operator _Ty*() const {
			if (nullptr == m_ptr) {
				int q = 3; /* just a line of code for putting a debugger break point */
			}
			return m_ptr;
		}

		/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
		been deleted) even in cases when this smart pointer is const. */
		mutable _Ty* m_ptr;
	};

	/* CRPTracker is intended to keep track of all the pointers pointing to an object. CRPTracker objects are intended to be always
	associated with (infact, a member of) the one object that is the target of the pointers it tracks. Though at the moment, it
	doesn't need to actually know which object it is associated with. */
	class CRPTracker {
	public:
		CRPTracker() {}
		CRPTracker(const CRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		CRPTracker(CRPTracker&& src) { /* see above */ }
		~CRPTracker() {
			if (!fast_mode1()) {
				delete m_ptr_to_regptr_set_ptr;
			}
		}
		CRPTracker& operator=(const CRPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		CRPTracker& operator=(CRPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const CRPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true; }
		bool operator!=(const CRPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
				(*m_ptr_to_regptr_set_ptr).insert(item);
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (sc_fm1_max_pointers == m_fm1_num_pointers) {
					/* Too many pointers. Initiate and switch to slow mode. */
					/* Initialize slow storage. */
					m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
					/* First copy the pointers from fast storage to slow storage. */
					for (int i = 0; i < sc_fm1_max_pointers; i += 1) {
						std::unordered_set<const CSaferPtrBase*>::value_type item(m_fm1_ptr_to_regptr_array[i]);
						(*m_ptr_to_regptr_set_ptr).insert(item);
					}
					/* Add the new pointer to slow storage. */
					std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
					(*m_ptr_to_regptr_set_ptr).insert(item);
				}
				else {
					if (1 == sc_fm1_max_pointers) {
						m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
						m_fm1_num_pointers = 1;
					}
					else if (2 == sc_fm1_max_pointers) {
						if (1 == m_fm1_num_pointers) {
							m_fm1_ptr_to_regptr_array[1] = (&sp_ref);
							m_fm1_num_pointers = 2;
						}
						else {
							assert(0 == m_fm1_num_pointers);
							m_fm1_ptr_to_regptr_array[0] = (&sp_ref);
							m_fm1_num_pointers = 1;
						}
					}
					else {
						m_fm1_ptr_to_regptr_array[m_fm1_num_pointers] = (&sp_ref);
						m_fm1_num_pointers += 1;
					}
				}
			}
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!fast_mode1()) {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
				}
				else if (2 == sc_fm1_max_pointers){
					if (1 == m_fm1_num_pointers) {
						m_fm1_num_pointers = 0;
					}
					else if (2 == m_fm1_num_pointers) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[1]) {
							m_fm1_num_pointers = 1;
						}
						else {
							assert((&sp_ref) == m_fm1_ptr_to_regptr_array[0]);
							m_fm1_ptr_to_regptr_array[0] = m_fm1_ptr_to_regptr_array[1];
							m_fm1_num_pointers = 1;
						}
					}
					else { /* There are no registered pointers to be unregistered. */ assert(false); }
				}
				else {
					int found_index = -1;
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						if ((&sp_ref) == m_fm1_ptr_to_regptr_array[i]) {
							found_index = i;
							break;
						}
					}
					if (0 <= found_index) {
						m_fm1_num_pointers -= 1;
						assert(0 <= m_fm1_num_pointers);
						for (int j = found_index; j < m_fm1_num_pointers; j += 1) {
							m_fm1_ptr_to_regptr_array[j] = m_fm1_ptr_to_regptr_array[j + 1];
						}
					}
					else { assert(false); }
				}
			}
		}
		void onObjectDestruction() {
			if (!fast_mode1()) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
			else {
				/* We're gonna use a bunch of (ugly) hard coded cases to try to make fast mode fast. */
				if (1 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					} else {
						assert(1 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else if (2 == sc_fm1_max_pointers) {
					if (0 == m_fm1_num_pointers) {
					}
					else if (1 == m_fm1_num_pointers) {
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						m_fm1_num_pointers = 0;
					}
					else {
						assert(2 == m_fm1_num_pointers);
						(*(m_fm1_ptr_to_regptr_array[0])).setToNull();
						(*(m_fm1_ptr_to_regptr_array[1])).setToNull();
						m_fm1_num_pointers = 0;
					}
				}
				else {
					for (int i = 0; i < m_fm1_num_pointers; i += 1) {
						(*(m_fm1_ptr_to_regptr_array[i])).setToNull();
					}
					m_fm1_num_pointers = 0;
				}
			}
		}

		bool fast_mode1() const { return (nullptr == m_ptr_to_regptr_set_ptr); }
		int m_fm1_num_pointers = 0;
		static const int sc_fm1_max_pointers = 2/*arbitrary*/;
		const CSaferPtrBase* m_fm1_ptr_to_regptr_array[sc_fm1_max_pointers];

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	/* CSORPTracker is a "size optimized" (smaller and slower) version of CSPTracker. Currently not used. */
	class CSORPTracker {
	public:
		CSORPTracker() {}
		CSORPTracker(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object) is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
		}
		CSORPTracker(CSORPTracker&& src) { /* see above */ }
		~CSORPTracker() {
			delete m_ptr_to_regptr_set_ptr;
		}
		CSORPTracker& operator=(const CSORPTracker& src_cref) {
			/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
			valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
			copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
			initialization state). */
			return (*this);
		}
		CSORPTracker& operator=(CSORPTracker&& src) { /* see above */ return (*this); }
		bool operator==(const CSORPTracker& _Right_cref) const {
			/* At the moment the "non-instance-specific" state of all objects of this type is the same (namely the null set). */
			return true;
		}
		bool operator!=(const CSORPTracker& _Right_cref) const { /* see above */ return false; }

		void registerPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				m_ptr_to_regptr_set_ptr = new std::unordered_set<const CSaferPtrBase*>();
			}
			std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
			(*m_ptr_to_regptr_set_ptr).insert(item);
		}
		void unregisterPointer(const CSaferPtrBase& sp_ref) {
			if (!m_ptr_to_regptr_set_ptr) {
				assert(false);
			}
			else {
				auto res = (*m_ptr_to_regptr_set_ptr).erase(&sp_ref);
				assert(0 != res);
			}
		}
		void onObjectDestruction() {
			if (m_ptr_to_regptr_set_ptr) {
				for (auto sp_ref_ptr : (*m_ptr_to_regptr_set_ptr)) {
					(*sp_ref_ptr).setToNull();
				}
			}
		}

		std::unordered_set<const CSaferPtrBase*> *m_ptr_to_regptr_set_ptr = nullptr;
	};

	template<typename _Ty>
	class TRegisteredObj;

	/* TRegisteredPointer behaves similar to (and is largely compatible with) native pointers. It inherits the safety features of
	TSaferPtr (default nullptr initialization and check for null pointer dereference). In addition, when pointed at a
	TRegisteredObj, it will be checked for attempted access after destruction. It's essentially intended to be a safe pointer like
	std::shared_ptr, but that does not take ownership of the target object (i.e. does not take responsibility for deallocation).
	Because it does not take ownership, unlike std::shared_ptr, TRegisteredPointer can be used to point to objects on the stack. */
	template<typename _Ty>
	class TRegisteredPointer : public TSaferPtr<TRegisteredObj<_Ty>> {
	public:
		TRegisteredPointer();
		TRegisteredPointer(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer(const TRegisteredPointer& src_cref);
		virtual ~TRegisteredPointer();
		TRegisteredPointer<_Ty>& operator=(TRegisteredObj<_Ty>* ptr);
		TRegisteredPointer<_Ty>& operator=(const TRegisteredPointer<_Ty>& _Right_cref);
		/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
		explicit operator _Ty*() const;

	private:
		/* If you want a pointer to a TRegisteredPointer<_Ty>, declare the TRegisteredPointer<_Ty> as a
		TRegisteredObj<TRegisteredPointer<_Ty>> instead. So for example:
		auto reg_ptr = TRegisteredObj<TRegisteredPointer<_Ty>>(mse::registered_new<_Ty>());
		auto reg_ptr_to_reg_ptr = &reg_ptr;
		*/
		TRegisteredPointer<_Ty>* operator&() {
			return this;
		}
		(const TRegisteredPointer<_Ty>)* operator&() const {
			return this;
		}
	};

	/* TRegisteredObj is intended as a transparent wrapper for other classes/objects. The purpose is to register the object's
	destruction so that TRegisteredPointers will avoid referencing destroyed objects. Note that TRegisteredObj can be used with
	objects allocated on the stack. */
	template<typename _Ty>
	class TRegisteredObj : public _Ty {
	public:
		//using _Ty::_Ty;
		// the version of the compiler (msvc 2013) being used does not yet support inherited constructors, so we use this macro hack
		// for now
		MSE_USING(TRegisteredObj, _Ty);
		virtual ~TRegisteredObj() {
			mseRPManager().onObjectDestruction();
		}
		TRegisteredObj& operator=(TRegisteredObj&& _X) { _Ty::operator=(std::move(_X)); return (*this); }
		TRegisteredObj& operator=(const TRegisteredObj& _X) { _Ty::operator=(_X); return (*this); }
		TRegisteredPointer<_Ty> operator&() {
			return this;
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return this;
		}
		CRPTracker& mseRPManager() { return m_mseRPManager; }

		mutable CRPTracker m_mseRPManager;
	};

	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer() : TSaferPtr<TRegisteredObj<_Ty>>() {}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(TRegisteredObj<_Ty>* ptr) : TSaferPtr<TRegisteredObj<_Ty>>(ptr) {
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::TRegisteredPointer(const TRegisteredPointer& src_cref) : TSaferPtr<TRegisteredObj<_Ty>>(src_cref.m_ptr) {
		if (nullptr != src_cref.m_ptr) {
			(*(src_cref.m_ptr)).mseRPManager().registerPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>::~TRegisteredPointer() {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(TRegisteredObj<_Ty>* ptr) {
		if (nullptr != (*this).m_ptr) {
			(*((*this).m_ptr)).mseRPManager().unregisterPointer(*this);
		}
		TSaferPtr<TRegisteredObj<_Ty>>::operator=(ptr);
		if (nullptr != ptr) {
			(*ptr).mseRPManager().registerPointer(*this);
		}
		return (*this);
	}
	template<typename _Ty>
	TRegisteredPointer<_Ty>& TRegisteredPointer<_Ty>::operator=(const TRegisteredPointer<_Ty>& _Right_cref) {
		return operator=(_Right_cref.m_ptr);
	}
	/* This native pointer cast operator is just for compatibility with existing/legacy code and ideally should never be used. */
	template<typename _Ty>
	TRegisteredPointer<_Ty>::operator _Ty*() const {
		if (nullptr == (*this).m_ptr) {
			int q = 5; /* just a line of code for putting a debugger break point */
		}
		return (*this).m_ptr;
	}

	/* registered_new is intended to be analogous to std::make_shared */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObj<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}

	static void s_regptr_test1() {

		class A {
		public:
			A() {}
			A(const A& _X) : b(_X.b) {}
			A(A&& _X) : b(std::move(_X.b)) {}
			virtual ~A() {}
			A& operator=(A&& _X) { b = std::move(_X.b); return (*this); }
			A& operator=(const A& _X) { b = _X.b; return (*this); }

			int b = 3;
		};
		class B {
		public:
			static int foo1(A* a_native_ptr) { return a_native_ptr->b; }
			static int foo2(mse::TRegisteredPointer<A> A_registered_ptr) { return A_registered_ptr->b; }
		protected:
			~B() {}
		};

		A* A_native_ptr = nullptr;
		/* mse::TRegisteredPointer<> is basically a "safe" version of the native pointer. */
		mse::TRegisteredPointer<A> A_registered_ptr1;

		{
			A a;
			mse::TRegisteredObj<A> registered_a;
			/* mse::TRegisteredObj<A> is a class that is publicly derived from A, and so should be a compatible substitute for A
			in almost all cases. */

			assert(a.b == registered_a.b);
			A_native_ptr = &a;
			A_registered_ptr1 = &registered_a;
			assert(A_native_ptr->b == A_registered_ptr1->b);

			auto A_registered_ptr2 = &registered_a;
			/* A_registered_ptr2 is actually an mse::TRegisteredPointer<A>, not a native pointer. */
			A_registered_ptr2 = nullptr;
			bool expected_exception = false;
			try {
				int i = A_registered_ptr2->b; /* this is gonna throw an exception */
			}
			catch (...) {
				//std::cerr << "expected exception" << std::endl;
				expected_exception = true;
				/* The exception is triggered by an attempt to dereference a null "registered pointer". */
			}
			assert(expected_exception);

			/* mse::TRegisteredPointers can be coerced into native pointers if you need to interact with legacy code or libraries. */
			B::foo1((A*)A_registered_ptr1);

			if (A_registered_ptr2) {
			}
			else if (A_registered_ptr2 != A_registered_ptr1) {
				A_registered_ptr2 = A_registered_ptr1;
				assert(A_registered_ptr2 == A_registered_ptr1);
			}

			A a2 = a;
			mse::TRegisteredObj<A> registered_a2 = registered_a;

			a2 = std::move(a);
			registered_a2 = std::move(registered_a);

			A a3(std::move(a2));
			mse::TRegisteredObj<A> registered_a3(std::move(registered_a2));
		}

		bool expected_exception = false;
		try {
			/* A_registered_ptr1 "knows" that the (registered) object it was pointing to has now been deallocated. */
			int i = A_registered_ptr1->b; /* So this is gonna throw an exception */
		}
		catch (...) {
			//std::cerr << "expected exception" << std::endl;
			expected_exception = true;
		}
		assert(expected_exception);

		{
			/* For heap allocations mse::registered_new is kind of analagous to std::make_shared, but again,
			mse::TRegisteredPointers don't take ownership so you are responsible for deallocation. */
			auto A_registered_ptr3 = mse::registered_new<A>();
			assert(3 == A_registered_ptr3->b);
			mse::registered_delete<A>(A_registered_ptr3);
			bool expected_exception = false;
			try {
				/* A_registered_ptr3 "knows" that the (registered) object it was pointing to has now been deallocated. */
				int i = A_registered_ptr3->b; /* So this is gonna throw an exception */
			}
			catch (...) {
				//std::cerr << "expected exception" << std::endl;
				expected_exception = true;
			}
			assert(expected_exception);
		}
	}

}

#endif /*ndef MSEPRIMITIVES_H*/
