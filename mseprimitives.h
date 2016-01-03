
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

	class CRPManager {
	public:
		CRPManager() {}
		~CRPManager() {}
		bool registerPointer(const CSaferPtrBase& sp_ref) {
			std::unordered_set<const CSaferPtrBase*>::value_type item(&sp_ref);
			auto res = m_ptr_to_regptr_set.insert(item);
			return res.second;
		}
		bool unregisterPointer(const CSaferPtrBase& sp_ref) {
			auto res = m_ptr_to_regptr_set.erase(&sp_ref);
			return (0 != res);
		}
		void onObjectDestruction() {
			for (auto sp_ref_ptr : m_ptr_to_regptr_set) {
				(*sp_ref_ptr).setToNull();
			}
		}
		std::unordered_set<const CSaferPtrBase*> m_ptr_to_regptr_set;
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
		CRPManager& mseRPManager() { return m_mseRPManager; }

		mutable CRPManager m_mseRPManager;
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
		TRegisteredPointer<_Ty> operator&() {
			return TRegisteredPointer<_Ty>(this);
		}
		TRegisteredPointer<const _Ty> operator&() const {
			return TRegisteredPointer<const _Ty>(this);
		}
		CRPManager& mseRPManager() { return m_mseRPManager; }

		mutable CRPManager m_mseRPManager;
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
		mseRPManager().onObjectDestruction(); /* Just in case there are pointers to this pointer out there. */
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

	/* registered_new is intended to be analogous to std:make_shared */
	template <class _Ty, class... Args>
	TRegisteredPointer<_Ty> registered_new(Args&&... args) {
		return new TRegisteredObj<_Ty>(args...);
	}
	template <class _Ty>
	void registered_delete(const TRegisteredPointer<_Ty>& regPtrRef) {
		auto a = dynamic_cast<TRegisteredObj<_Ty> *>((_Ty *)regPtrRef);
		delete a;
	}

}
#endif /*ndef MSEPRIMITIVES_H*/
