
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPRIMITIVES_H
#define MSEPRIMITIVES_H

#include <cassert>
#include <climits>       // ULONG_MAX
#include <limits>       // std::numeric_limits
#include <stdexcept>      // primitives_range_error
#include <functional>

#ifndef MSEPOINTERBASICS_H
#define MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION
#endif /*ndef MSEPOINTERBASICS_H*/

#if __cplusplus >= 201703L
#define MSE_PRM_HAS_CXX17
#if __cplusplus > 201703L
#define MSE_PRM_HAS_CXX20
#endif // __cplusplus > 201703L
#endif // __cplusplus >= 201703L
/*compiler specific defines*/
#ifdef _MSC_VER
#if _MSVC_LANG >= 201703L || (defined(_HAS_CXX17) && (_HAS_CXX17 >= 1))
#define MSE_PRM_HAS_CXX17
#if _MSVC_LANG > 201703L || (defined(_HAS_CXX20) && (_HAS_CXX20 >= 1))
#define MSE_PRM_HAS_CXX20
#endif // _MSVC_LANG > 201703L || (defined(_HAS_CXX20) && (_HAS_CXX20 >= 1))
#endif // _MSVC_LANG >= 201703L || (defined(_HAS_CXX17) && (_HAS_CXX17 >= 1))
#if (1700 > _MSC_VER)
#define MSVC2010_COMPATIBLE 1
#endif /*(1700 > _MSC_VER)*/
#if (1900 > _MSC_VER)
#define MSVC2013_COMPATIBLE 1
#endif /*(1900 > _MSC_VER)*/
#if (1910 > _MSC_VER)
#define MSVC2015_COMPATIBLE 1
#endif /*(1910 > _MSC_VER)*/
#else /*_MSC_VER*/
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBLE 1
#if ((5 > __GNUC__) && (!defined(__clang__)))
#define GPP4P8_COMPATIBLE 1
#endif /*((5 > __GNUC__) && (!defined(__clang__)))*/
#endif /*(defined(__GNUC__) || defined(__GNUG__))*/
#endif /*_MSC_VER*/

#if __cpp_exceptions >= 199711
#define MSE_PRM_TRY try
#define MSE_PRM_CATCH(x) catch(x)
#define MSE_PRM_CATCH_ANY catch(...)
#define MSE_PRM_FUNCTION_TRY try
#define MSE_PRM_FUNCTION_CATCH(x) catch(x)
#define MSE_PRM_FUNCTION_CATCH_ANY catch(...)
#else // __cpp_exceptions >= 199711
#define MSE_PRM_TRY if (true)
#define MSE_PRM_CATCH(x) if (false)
#define MSE_PRM_CATCH_ANY if (false)
#define MSE_PRM_FUNCTION_TRY
#define MSE_PRM_FUNCTION_CATCH(x) void mse_placeholder_function_catch(x)
#define MSE_PRM_FUNCTION_CATCH_ANY void mse_placeholder_function_catch_any()
#define MSE_CUSTOM_THROW_DEFINITION(x) exit(-11)
#endif // __cpp_exceptions >= 199711

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_PRIMITIVES_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#if defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_PRM_CONSTEXPR
#else // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_PRM_CONSTEXPR constexpr
#endif // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)

#ifdef MSE_PRM_HAS_CXX17
#define MSE_PRM_IF_CONSTEXPR if constexpr 
#else // MSE_PRM_HAS_CXX17
#define MSE_PRM_IF_CONSTEXPR if 
#endif // MSE_PRM_HAS_CXX17

#ifdef MSVC2015_COMPATIBLE
#ifndef MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
/* msvc2015's incomplete support for "constexpr" means that range checks that should be done at compile time would
be done at run time, at significant cost. So by default we disable range checks upon assignment. */
#define MSE_PRIMITIVE_ASSIGN_RANGE_CHECK_DISABLED 1
#endif // !MSE_FORCE_PRIMITIVE_ASSIGN_RANGE_CHECK_ENABLED
#endif // MSVC2015_COMPATIBLE


#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NOEXCEPT")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

/* This automatic (potential) definition of MSE_CHECK_USE_BEFORE_SET is also done in msepointerbasics.h */
#ifndef MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#ifndef MSE_CHECK_USE_BEFORE_SET
#ifndef NDEBUG
#define MSE_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

#if defined(MSE_NON_THREADSAFE_CHECK_USE_BEFORE_SET) && !defined(MSE_CHECK_USE_BEFORE_SET)
#define MSE_CHECK_USE_BEFORE_SET
#endif // defined(MSE_NON_THREADSAFE_CHECK_USE_BEFORE_SET) && !defined(MSE_CHECK_USE_BEFORE_SET)
#endif // !MSE_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_CHECK_USE_BEFORE_SET

#ifdef MSE_NON_THREADSAFE_CHECK_USE_BEFORE_SET
#define MSE_PRIMITIVES_IMPL_INITIALIZED_FLAG_TYPE bool
#else // MSE_NON_THREADSAFE_CHECK_USE_BEFORE_SET
#ifdef MSE_CHECK_USE_BEFORE_SET
#include <atomic>
#ifdef MSE_PRM_HAS_CXX17
#define MSE_PRIMITIVES_IMPL_INITIALIZED_FLAG_TYPE std::atomic_bool
#else // MSE_PRM_HAS_CXX17
namespace mse {
	namespace impl {
		namespace primitives {
			template<typename _ITYPE>
			class TAtomic : public std::atomic<_ITYPE> {
			public:
				typedef std::atomic<_ITYPE> _ATOMIC_ITYPE;
				using value_type = _ITYPE;
				using difference_type = _ITYPE;

				TAtomic() noexcept = default;

				constexpr TAtomic(_ITYPE _Val) noexcept
					: _ATOMIC_ITYPE{ /*(_ATOMIC_UINT)*/_Val }
				{	// construct from _Val, initialization is not TAtomic
				}
				constexpr TAtomic(const TAtomic& src) noexcept : _ATOMIC_ITYPE{ src.load() } {}

				_ITYPE operator=(_ITYPE _Val) volatile noexcept
				{	// assign from _Val
					return (_ATOMIC_ITYPE::operator=(_Val));
				}

				_ITYPE operator=(_ITYPE _Val) noexcept
				{	// assign from _Val
					return (_ATOMIC_ITYPE::operator=(_Val));
				}
			};
		}
	}
}
#define MSE_PRIMITIVES_IMPL_INITIALIZED_FLAG_TYPE mse::impl::primitives::TAtomic<bool>
#endif // MSE_PRM_HAS_CXX17
#endif // MSE_CHECK_USE_BEFORE_SET
#endif // MSE_NON_THREADSAFE_CHECK_USE_BEFORE_SET


#ifndef MSE_CINT_BASE_INTEGER_TYPE
#if (SIZE_MAX <= UINT_MAX) ||(!defined(MSE_MATCH_CINT_SIZE_TO_CSIZE_T))
#define MSE_CINT_BASE_INTEGER_TYPE int
#else // (SIZE_MAX <= UINT_MAX) ||(!defined(MSE_MATCH_CINT_SIZE_TO_CSIZE_T))
#if SIZE_MAX <= ULONG_MAX
#define MSE_CINT_BASE_INTEGER_TYPE long int
#else // SIZE_MAX <= ULONG_MAX
#define MSE_CINT_BASE_INTEGER_TYPE long long int
#endif // SIZE_MAX <= ULONG_MAX
#endif // (SIZE_MAX <= UINT_MAX) ||(!defined(MSE_MATCH_CINT_SIZE_TO_CSIZE_T))
#endif // !MSE_CINT_BASE_INTEGER_TYPE

#ifndef MSE_DEFAULT_INT_VALUE
#define MSE_DEFAULT_INT_VALUE 0
#endif // !MSE_DEFAULT_INT_VALUE

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4505 4127 4018 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif /*__GNUC__*/
#endif /*__clang__*/


namespace mse {

	class primitives_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};

	/* This class is just meant to act like the "bool" type, except that it has a default intialization value (false). */
	class CNDBool {
	public:
		// Constructs zero.
		CNDBool() : m_val(false) {}

		// Copy constructor
		CNDBool(const CNDBool &x) : m_val(x.m_val) { note_value_assignment(); };

		// Assignment operator
		CNDBool& operator=(const CNDBool &x) { note_value_assignment(); m_val = x.m_val; return (*this); }

		// Constructors from primitive boolean types
		CNDBool(bool   x) { note_value_assignment(); m_val = x; }

		// Casts to primitive boolean types
		operator bool() const { assert_initialized(); return m_val; }
		/* provisional */
		const bool& mse_base_type_ref() const & { (*this).assert_initialized(); return (*this).m_val; }
		const bool& mse_base_type_ref() const&& = delete;
		bool& mse_base_type_ref() & { (*this).assert_initialized(); return (*this).m_val; }
		bool& mse_base_type_ref() && = delete;

		/* For comparisons, we're just going to rely on our operator bool() and the native bool comparison operators. */
		/*
		bool operator ==(const CNDBool &x) const { return (bool(*this) == bool(x)); }
		bool operator ==(bool x) const { return (bool(*this) == x); }
		bool operator !=(const CNDBool& x) const { return (bool(*this) != bool(x)); }
		bool operator !=(bool x) const { return (bool(*this) != x); }
		*/

		CNDBool& operator |=(const CNDBool &x) { assert_initialized(); m_val |= x.m_val; return (*this); }
		CNDBool& operator &=(const CNDBool &x) { assert_initialized(); m_val &= x.m_val; return (*this); }
		CNDBool& operator ^=(const CNDBool &x) { assert_initialized(); m_val ^= x.m_val; return (*this); }

#ifdef MSE_CHECK_USE_BEFORE_SET
		void assert_initialized() const { assert(m_initialized); }
#else // MSE_CHECK_USE_BEFORE_SET
		void assert_initialized() const {}
#endif // MSE_CHECK_USE_BEFORE_SET

		void async_shareable_and_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		bool m_val;

#ifdef MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() { m_initialized = true; }
		MSE_PRIMITIVES_IMPL_INITIALIZED_FLAG_TYPE m_initialized = false;
#else // MSE_CHECK_USE_BEFORE_SET
		void note_value_assignment() {}
#endif // MSE_CHECK_USE_BEFORE_SET
	};
}

namespace std {
	template<>
	struct hash<mse::CNDBool> {	// hash functor
		typedef mse::CNDBool argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::CNDBool& _Keyval) const _NOEXCEPT {
			return (hash<bool>()(_Keyval));
		}
	};
}

namespace mse {

	class disable_AR_range_extension_t {};
	class enable_AR_range_extension_t {};
	class disable_AR_overflow_checking_t {};
	class enable_AR_overflow_checking_t {};
	template<class... Bases>
	struct int_options_t : Bases... { };

#ifndef MSE_DISABLE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC
//#define MSE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC
#endif // !MSE_DISABLE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC

#ifdef MSE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC
#define RETURN_RANGE_EXTENSION_DEFAULT mse::int_options_t<mse::enable_AR_range_extension_t>
#else // MSE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC
#define RETURN_RANGE_EXTENSION_DEFAULT mse::int_options_t<>
#endif // MSE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC


	template<typename TBase = MSE_CINT_BASE_INTEGER_TYPE, typename TOption1 = RETURN_RANGE_EXTENSION_DEFAULT> class TInt;
	typedef TInt<MSE_CINT_BASE_INTEGER_TYPE> CNDInt;

	template<typename TOption1 = RETURN_RANGE_EXTENSION_DEFAULT> class TSize_t;
	typedef TSize_t<RETURN_RANGE_EXTENSION_DEFAULT> CNDSize_t;
	static size_t as_a_size_t(CNDSize_t const& n);

	template<typename TBase> class TFloatingPoint;
	typedef TFloatingPoint<float> CNDFloat;
	typedef TFloatingPoint<double> CNDDouble;
	typedef TFloatingPoint<long double> CNDLongDouble;

	namespace impl {
		template<typename _TDestination, typename _TSource>
		MSE_PRM_CONSTEXPR static bool sg_can_exceed_upper_bound() {

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
		MSE_PRM_CONSTEXPR static bool sg_can_exceed_lower_bound() {
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
			MSE_PRM_CONSTEXPR const bool rhs_can_exceed_upper_bound = impl::sg_can_exceed_upper_bound<_TDestination, _TSource>();
			MSE_PRM_CONSTEXPR const bool rhs_can_exceed_lower_bound = impl::sg_can_exceed_lower_bound<_TDestination, _TSource>();
			MSE_PRM_CONSTEXPR const bool can_exceed_bounds = rhs_can_exceed_upper_bound || rhs_can_exceed_lower_bound;
			MSE_PRM_IF_CONSTEXPR (can_exceed_bounds) {
				MSE_PRM_IF_CONSTEXPR (rhs_can_exceed_upper_bound) {

#ifdef MSE_SUPPRESS_CSIZE_T_TO_CINT_CONVERSION_RANGE_CHECK
					MSE_PRM_CONSTEXPR const bool this_is_a_CSize_t_to_CInt_conversion = ((std::is_same<_TDestination, MSE_CINT_BASE_INTEGER_TYPE>::value || std::is_same<_TDestination, CNDInt>::value)
						&& (std::is_same<_TSource, CNDSize_t>::value || std::is_same<_TSource, size_t>::value));
					MSE_PRM_IF_CONSTEXPR (!this_is_a_CSize_t_to_CInt_conversion) {
#endif // MSE_SUPPRESS_CSIZE_T_TO_CINT_CONVERSION_RANGE_CHECK

					if (x > _TSource(std::numeric_limits<_TDestination>::max())) {
						MSE_THROW(primitives_range_error("range error - value to be assigned is out of range of the target (integer) type"));
					}

#ifdef MSE_SUPPRESS_CSIZE_T_TO_CINT_CONVERSION_RANGE_CHECK
					}
#endif // MSE_SUPPRESS_CSIZE_T_TO_CINT_CONVERSION_RANGE_CHECK
				}
				MSE_PRM_IF_CONSTEXPR (rhs_can_exceed_lower_bound) {
					/* We're assuming that std::numeric_limits<>::lowest() will never be greater than zero. */
					if (_TSource(0) > x) {
						MSE_PRM_IF_CONSTEXPR (0 == std::numeric_limits<_TDestination>::lowest()) {
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
	}

#ifdef MSE_IMPL_USE_INT128_COMPILER_EXTENSION
#ifdef __SIZEOF_INT128__
#if ((18446744073709551615U == ULLONG_MAX) && (9223372036854775807 == LLONG_MAX))
#define MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT __uint128_t
#define MSE_IMPL_LONG_LONG_LONG_INT __int128_t
#endif //((18446744073709551615 == ULLONG_MAX) && (9223372036854775807 == LLONG_MAX))
#endif
#endif // MSE_IMPL_USE_INT128_COMPILER_EXTENSION

//define MSE_TINT_TYPE_WITH_THE_LOWER_FLOOR(_Ty, _Tz) typename std::conditional<impl::sg_can_exceed_lower_bound<_Tz, _Ty>(), _Ty, _Tz>::type

	namespace impl {

		template <class _Ty>
		struct is_arithmetic : public std::is_arithmetic<_Ty> {};
		template <> struct is_arithmetic<CNDBool> : std::integral_constant<bool, true> {};
		template <> struct is_arithmetic<const CNDBool> : std::integral_constant<bool, true> {};
		template <typename TOption1> struct is_arithmetic<TSize_t<TOption1> > : std::integral_constant<bool, true> {};
		template <typename TOption1> struct is_arithmetic<const TSize_t<TOption1> > : std::integral_constant<bool, true> {};
		template <typename TBase, typename TOption1> struct is_arithmetic<TInt<TBase, TOption1> > : std::integral_constant<bool, true> {};
		template <typename TBase, typename TOption1> struct is_arithmetic<const TInt<TBase, TOption1> > : std::integral_constant<bool, true> {};
		template <typename TBase> struct is_arithmetic<TFloatingPoint<TBase> > : std::integral_constant<bool, true> {};
		template <typename TBase> struct is_arithmetic<const TFloatingPoint<TBase> > : std::integral_constant<bool, true> {};

		template <class _Ty>
		struct make_signed : public std::make_signed<_Ty> {};
		template <class _Ty>
		struct make_unsigned : public std::make_unsigned<_Ty> {};
#ifdef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
		template <> struct make_signed<MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> { using type = MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_signed<MSE_IMPL_LONG_LONG_LONG_INT> { using type = MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_signed<const MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> { using type = const MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_signed<const MSE_IMPL_LONG_LONG_LONG_INT> { using type = const MSE_IMPL_LONG_LONG_LONG_INT; };

		template <> struct make_unsigned<MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> { using type = MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_unsigned<MSE_IMPL_LONG_LONG_LONG_INT> { using type = MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_unsigned<const MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> { using type = const MSE_IMPL_LONG_LONG_LONG_INT; };
		template <> struct make_unsigned<const MSE_IMPL_LONG_LONG_LONG_INT> { using type = const MSE_IMPL_LONG_LONG_LONG_INT; };
#endif // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT

#ifdef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
		template<typename _Ty>
		struct is_biggest_available_type {
			static const bool value = std::is_same<MSE_IMPL_LONG_LONG_LONG_INT, _Ty>::value || std::is_same<MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT, _Ty>::value;
		};
#else // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
		template<typename _Ty>
		struct is_biggest_available_type {
			static const bool value = std::is_same<long long int, _Ty>::value || std::is_same<unsigned long long int, _Ty>::value;
		};
#endif // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT

		/* Efficient (run-time) arithmetic overflow detection for types other than the largest ones are enabled by 
		default. (It can be disabled by defining MSE_DISABLE_RETURN_RANGE_EXTENDED_TYPE_FOR_INTEGER_ARITHMETIC). */
		template<typename _Ty> struct next_bigger_native_int_type { typedef _Ty type; };

#ifdef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
		template<> struct next_bigger_native_int_type<long long int> { typedef MSE_IMPL_LONG_LONG_LONG_INT type; };
		template<> struct next_bigger_native_int_type<unsigned long long int> { typedef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT type; };
#endif // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT

#define NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(x, candidate_next_bigger) \
		template<> struct next_bigger_native_int_type<x> { \
			typedef typename std::conditional<(std::numeric_limits<x>::digits) < std::numeric_limits<candidate_next_bigger>::digits, candidate_next_bigger, typename next_bigger_native_int_type<candidate_next_bigger>::type>::type type; \
		};

		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(unsigned long int, unsigned long long int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(unsigned int, unsigned long int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(unsigned short int, unsigned int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(unsigned char, unsigned short int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(long int, long long int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(int, long int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(short int, int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(char, short int);
		NEXT_BIGGER_NATIVE_INT_TYPE_SPECIALIZATION(signed char, short int);

		namespace range_encompassing {
			template<typename _Ty, typename _Tz>
			struct next_bigger_candidate {
				typedef typename std::conditional<std::is_signed<_Ty>::value || std::is_signed<_Tz>::value
					, typename mse::impl::make_signed<_Ty>::type, _Ty>::type candidate_type;

				typedef typename next_bigger_native_int_type<candidate_type>::type type;
			};
			template<typename _Ty, typename _Tz, typename candidate_type>
			struct last_encompasses_first_two {
				static const bool value = !((sg_can_exceed_upper_bound<candidate_type, _Ty>()) || (sg_can_exceed_upper_bound<candidate_type, _Tz>())
					|| (sg_can_exceed_lower_bound<candidate_type, _Ty>()) || (sg_can_exceed_lower_bound<candidate_type, _Tz>()));
			};

			/* range_encompassing_native_int_type1<> determines the smallest (integer) type whose range encompasses the
			ranges of the two given (integer) types. Ideally it would use a recursive algorithm to accomplish this. But
			it was not readily apparent how to do the recursion in a supported way. So knowing that the maximum
			recursion depth needed is limited, we just manually emulate the recursion by creating a distinct struct for
			each level of recursion. */
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper7 {
				typedef void type; /* Should induce a compile error if this type is actually ever used. */
			};
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper6 {
				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<candidate_type, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper7<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper5 {
				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<candidate_type, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper6<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper4 {
				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<candidate_type, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper5<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper3 {
				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<candidate_type, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper4<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};
			template<typename candidate_type, typename _Tz> struct range_encompassing_native_int_type_helper2 {
				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<candidate_type, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper3<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};

			/* This template struct should deduce the smallest native integer type that can encompass the combined range
			of both template parameters (or the one that comes closest if there isn't one). */
			template<typename _Ty, typename _Tz> struct range_encompassing_native_int_type {
				typedef typename std::conditional<std::is_signed<_Ty>::value || std::is_signed<_Tz>::value, typename mse::impl::make_signed<_Ty>::type, _Ty>::type candidate_type;

				typedef typename std::conditional<!(is_biggest_available_type<candidate_type>::value || last_encompasses_first_two<_Ty, _Tz, candidate_type>::value)
					, typename range_encompassing_native_int_type_helper2<typename next_bigger_candidate<candidate_type, _Tz>::type, _Tz>::type
					, candidate_type>::type type;
			};
		}

		template<typename _Ty>
		struct corresponding_TInt { typedef typename std::conditional<std::is_arithmetic<_Ty>::value, TInt<_Ty>, _Ty>::type type; };
#ifdef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
		template<>
		struct corresponding_TInt<MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> { typedef TInt<MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT> type; };
		template<>
		struct corresponding_TInt<MSE_IMPL_LONG_LONG_LONG_INT> { typedef TInt<MSE_IMPL_LONG_LONG_LONG_INT> type; };
#endif // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT

#define MSE_TINT_TYPE(_Ty) typename mse::impl::corresponding_TInt<_Ty>::type
#define MSE_NATIVE_INT_TYPE(_Ty) MSE_TINT_TYPE(_Ty)::base_type

#define MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz) typename mse::impl::range_encompassing::range_encompassing_native_int_type<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz)>::type

		template<typename _Ty, typename _Tz, typename TOption1>
		using native_int_result_type1 = typename std::conditional<(!std::is_base_of<enable_AR_range_extension_t, TOption1>::value)
			, MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz), typename mse::impl::next_bigger_native_int_type<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type>::type;

		template<typename _Ty, typename _Tz, typename TOption1>
		using native_int_add_result_type1 = typename std::conditional<(!std::is_base_of<enable_AR_range_extension_t, TOption1>::value)
			, MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz), typename mse::impl::next_bigger_native_int_type<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type>::type;

		template<typename _Ty, typename _Tz, typename TOption1>
		using native_int_subtract_result_type1 = typename std::conditional<(!std::is_base_of<enable_AR_range_extension_t, TOption1>::value)
			, typename mse::impl::make_signed<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type
			, typename std::conditional<std::is_signed<_Ty>::value || std::is_signed<_Tz>::value, typename mse::impl::next_bigger_native_int_type<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type, typename mse::impl::make_signed<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type>::type>::type;

		template<typename _Ty, typename _Tz, typename TOption1>
		using native_int_multiply_result_type1 = typename std::conditional<(!std::is_base_of<enable_AR_range_extension_t, TOption1>::value)
			, MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz), typename mse::impl::next_bigger_native_int_type<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz)>::type>::type;

		template<typename _Ty, typename _Tz, typename TOption1>
		using native_int_divide_result_type1 = MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(_Ty, _Tz);

//#define MSE_TINT_RESULT_TYPE1(_Ty, _Tz, TOption1) TInt<native_int_result_type1<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz), TOption1>, TOption1>

#ifndef MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW
#define MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW 63/*arbitrary*/
#endif // !MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW

/* It's useful for our overflow checking implementation if the results of integer arithmetic operations are restrained 
from promoting those results to the largest available (signed and unsigned) integer types. But this is presumably only 
workable when the largest available integer types are generally larger than necessary for any value you might want to 
store. Whether or not that is the case is perhaps kind of a judgement call. */
#ifndef MSE_IMPL_BITSIZE_REQUIRED_TO_RESERVE_THE_LARGEST_INTEGERS_FOR_OVERFLOW_CHECKING
#define MSE_IMPL_BITSIZE_REQUIRED_TO_RESERVE_THE_LARGEST_INTEGERS_FOR_OVERFLOW_CHECKING 255/*arbitrary*/
#endif // !MSE_IMPL_BITSIZE_REQUIRED_TO_RESERVE_THE_LARGEST_INTEGERS_FOR_OVERFLOW_CHECKING

		/* MSE_TINT_RESULT_TYPE1(_Ty, _Tz) should evaluate to the mse::TInt<> specialization that can encompass the range of 
		results of a multiply operation on the two parameter types, if such a specialization is available, otherwise the 
		specialization (of appropriated 'signed'ness) with the largest available range. If, however, that largest available
		range is greater or equal to that of a 128-bit integer (arbitrary), then the resulting type should instead be the 
		smallest specialization whose range encompasses the ranges of both parameters. Choosing a smaller specialization in 
		this case may result in a narrowing conversion, and thus a run-time check, that would not have occurred otherwise. We 
		choose to incur this extra run-time overhead to ensure that there will be an available (larger) integer type that can 
		accommodate the result of any subsequent arithmetic operation. This allows us to ensure against unchecked overflow in
		all arithmetic operations, at the cost of never using the largest available type as a return type of any arithmetic 
		operation. Again, this policy is only used (and thus, full overflow checking achieved,) when the largest available 
		integer type is 128-bits (arbitrary) or greater.
		*/
#define MSE_TINT_RESULT_TYPE1(_Ty, _Tz, TOption1) TInt<typename std::conditional<mse::impl::is_biggest_available_type<mse::impl::native_int_result_type1<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz), TOption1> >::value \
			&& (MSE_IMPL_BITSIZE_REQUIRED_TO_RESERVE_THE_LARGEST_INTEGERS_FOR_OVERFLOW_CHECKING <= std::numeric_limits<mse::impl::native_int_result_type1<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz), TOption1> >::digits) \
		, MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz)) \
		, mse::impl::native_int_result_type1<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz), TOption1> \
	>::type, TOption1>

#define MSE_TINT_ADD_RESULT_TYPE1(_Ty, _Tz, TOption1) MSE_TINT_RESULT_TYPE1(_Ty, _Tz, TOption1)
#define MSE_TINT_SUBTRACT_RESULT_TYPE1(_Ty, _Tz, TOption1) typename mse::impl::make_signed<MSE_TINT_RESULT_TYPE1(_Ty, _Tz, TOption1)>::type
#define MSE_TINT_MULTIPLY_RESULT_TYPE1(_Ty, _Tz, TOption1) MSE_TINT_RESULT_TYPE1(_Ty, _Tz, TOption1)
#define MSE_TINT_DIVIDE_RESULT_TYPE1(_Ty, _Tz, TOption1) TInt<mse::impl::native_int_divide_result_type1<MSE_NATIVE_INT_TYPE(_Ty), MSE_NATIVE_INT_TYPE(_Tz), TOption1>, TOption1>

		/* The CNDInt and TSize_t classes are meant to substitute for standard "int" and "size_t" types. The differences between
		the standard types and these classes are that the classes have a default intialization value (zero), and the
		classes, as much as possible, try to prevent the problematic behaviour of (possibly negative) signed integers
		being cast (inadvertently) to the unsigned size_t type. For example, the expression (0 > (int)5 - (size_t)7) evaluates
		(unintuitively) to false, whereas the expression (0 > (CNDInt)5 - (TSize_t)7) evaluates to true. Also, the classes do
		some range checking. For example, the code "TSize_t s = -2;" will throw an exception. */
		template<typename _Ty>
		class TArithmeticBase1 {
		public:
			// Constructs zero.
			TArithmeticBase1() : m_val(MSE_DEFAULT_INT_VALUE) {}

			// Copy constructor
			TArithmeticBase1(const TArithmeticBase1 &x) : m_val(x.m_val) { note_value_assignment(); };

			// Constructors from primitive integer types
			explicit TArithmeticBase1(_Ty x) : m_val(x) { note_value_assignment(); }

			template<typename _Tz>
			void assign_check_range(const _Tz &x) {
				note_value_assignment();
				impl::g_assign_check_range<_Ty, _Tz>(x);
			}

			_Ty m_val;

#ifdef MSE_CHECK_USE_BEFORE_SET
			void note_value_assignment() { m_initialized = true; }
			void assert_initialized() const { assert(m_initialized); }
			MSE_PRIMITIVES_IMPL_INITIALIZED_FLAG_TYPE m_initialized = false;
#else // MSE_CHECK_USE_BEFORE_SET
			void note_value_assignment() {}
			void assert_initialized() const {}
#endif // MSE_CHECK_USE_BEFORE_SET
		};
	}
	template <class _Ty>
	struct is_arithmetic : public mse::impl::is_arithmetic<_Ty> {};

	template <class _Ty>
	struct make_signed : public mse::impl::make_signed<_Ty> {};
	template <class _Ty>
	struct make_unsigned : public mse::impl::make_unsigned<_Ty> {};

	template<typename TBase/* = MSE_CINT_BASE_INTEGER_TYPE*/, typename TOption1/* = RETURN_RANGE_EXTENSION_DEFAULT*/>
	class TInt : private impl::TArithmeticBase1<TBase> {
	public:
		typedef impl::TArithmeticBase1<TBase> base_class;
		typedef TBase base_type;

		TInt() : base_class() {}
		TInt(const TInt &x) : base_class(x) {};
		//explicit TInt(const base_class &x) : base_class(x) {};
		//explicit TInt(const TSize_t &x) : base_class(CNDInt(x)) { (*this).template assign_check_range<CNDInt>(CNDInt(x)); };

		template<typename _Ty, class = typename std::enable_if<std::is_convertible<_Ty, base_type>::value, void>::type>
		TInt(const _Ty& x) : base_class(checked_and_adjusted_x(x)) {}

		TInt& operator=(const TInt &x) { (*this).note_value_assignment(); (*this).m_val = x.m_val; return (*this); }
		template<typename _Ty>
		TInt& operator=(const _Ty& x) { (*this).template assign_check_range<_Ty>(x); (*this).m_val = (base_type)x; return (*this); }

		operator base_type() const { (*this).assert_initialized(); return (*this).m_val; }
		/* provisional */
		const base_type& mse_base_type_ref() const & { (*this).assert_initialized(); return (*this).m_val; }
		const base_type& mse_base_type_ref() const&& = delete;
		base_type& mse_base_type_ref() & { (*this).assert_initialized(); return (*this).m_val; }
		base_type& mse_base_type_ref() && = delete;

		TInt operator ~() const { (*this).assert_initialized(); return TInt(~(*this).m_val); }
		TInt& operator |=(const TInt &x) { (*this).assert_initialized(); (*this).m_val |= x.m_val; return (*this); }
		TInt& operator &=(const TInt &x) { (*this).assert_initialized(); (*this).m_val &= x.m_val; return (*this); }
		TInt& operator ^=(const TInt &x) { (*this).assert_initialized(); (*this).m_val ^= x.m_val; return (*this); }

		auto operator -() const->MSE_TINT_SUBTRACT_RESULT_TYPE1(TInt, TInt, TOption1) { (*this).assert_initialized(); return TInt{ 0 } - (*this); }
		TInt& operator +=(const TInt &x) { (*this) = (*this) + x; return (*this); }
		TInt& operator -=(const TInt &x) { (*this) = (*this) - x; return (*this); }
		TInt& operator *=(const TInt &x) { (*this) = (*this) * x; return (*this); }
		TInt& operator /=(const TInt &x) { (*this) = (*this) / x; return (*this); }
		TInt& operator %=(const TInt &x) { (*this) = (*this) % x; return (*this); }
		TInt& operator >>=(const TInt &x) { (*this) = (*this) >> x; return (*this); }
		TInt& operator <<=(const TInt &x) { (*this) = (*this) << x; return (*this); }

		template<typename TReturn, typename TArg1, typename TArg2>
		void check_for_add_overflow(TArg1 x, TArg2 y) const {
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				/* We're assuming standard ranges for signed integers [-(2^(2^n-1)), 2^(2^n-1)-1], and that the range of TReturn  encompasses the range of TArg1 and TArg2. */
				MSE_PRM_IF_CONSTEXPR((std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg1>::max()) && (std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg2>::max())
					&& (((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))
						|| ((std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg1>::min()) && (std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg2>::min())))) {
					// no chance of overflow, nothing to do
				}
				else {
					if ((0 < y) && (std::numeric_limits<TReturn>::max() - y < x)) {
						MSE_THROW(primitives_range_error("range error - result of the addition operation is out of range of the target (integer) type"));
					}
					MSE_PRM_IF_CONSTEXPR(!((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))) {
						if ((0 > y) && (std::numeric_limits<TReturn>::min() - y > x)) {
							MSE_THROW(primitives_range_error("range error - result of the addition operation is out of range of the target (integer) type"));
						}
					}
				}
			}
		}
		auto operator +(const TInt& x) const->MSE_TINT_ADD_RESULT_TYPE1(TInt, TInt, TOption1) {
			(*this).assert_initialized();

#if defined __has_builtin
#  if __has_builtin (__builtin_add_overflow)
#    define MSE_IMPL_HAS__builtin_add_overflow
#  endif
#endif
#ifdef MSE_IMPL_HAS__builtin_add_overflow
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				mse::impl::native_int_add_result_type1<base_type, base_type, TOption1> retval;
				auto overflow_flag = __builtin_add_overflow(mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>((*this).m_val)
					, mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>(x.m_val), &retval);
				if (overflow_flag) {
					MSE_THROW(primitives_range_error("range error - result of the addition operation is out of range of the target (integer) type"));
				}
				return retval;
			} else {
				return (mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>((*this).m_val) + mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>(x.m_val));
			}
#else /*MSE_IMPL_HAS__builtin_add_overflow*/
			check_for_add_overflow<mse::impl::native_int_add_result_type1<base_type, base_type, TOption1> >((*this).m_val, x.m_val);
			return (mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>((*this).m_val) + mse::impl::native_int_add_result_type1<base_type, base_type, TOption1>(x.m_val));
#endif /*MSE_IMPL_HAS__builtin_add_overflow*/
		}
		template<typename _Ty2, typename TOption2>
		auto operator +(const TInt<_Ty2, TOption2> &x) const ->MSE_TINT_ADD_RESULT_TYPE1(TInt, TInt<_Ty2>, TOption1) {
			(*this).assert_initialized();
			return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>((*this).m_val) + TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>(x.m_val));
		}
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator +(_Ty2 x) const { (*this).assert_initialized(); return ((*this) + TInt<_Ty2>(x)); }

		template<typename TReturn, typename TArg1, typename TArg2>
		void check_for_subtract_overflow(TArg1 x, TArg2 y) const {
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				/* We're assuming standard ranges for signed integers [-(2^(2^n-1)), 2^(2^n-1)-1], and that the range of TReturn  encompasses the range of TArg1 and TArg2. */
				MSE_PRM_IF_CONSTEXPR((std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg1>::max()) && (std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg2>::max())
					&& (((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))
						|| ((std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg1>::min()) && (std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg2>::min())))) {
					// no chance of overflow, nothing to do
				}
				else {
					if ((0 < y) && (std::numeric_limits<TReturn>::min() + y > x)) {
						MSE_THROW(primitives_range_error("range error - result of the subtraction operation is out of range of the target (integer) type"));
					}
					MSE_PRM_IF_CONSTEXPR(!((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))) {
						if ((0 > y) && (std::numeric_limits<TReturn>::max() + y < x)) {
							MSE_THROW(primitives_range_error("range error - result of the subtraction operation is out of range of the target (integer) type"));
						}
					}
				}
			}
		}
		auto operator -(const TInt &x) const ->MSE_TINT_SUBTRACT_RESULT_TYPE1(TInt, TInt, TOption1) {
			(*this).assert_initialized();

#if defined __has_builtin
#  if __has_builtin (__builtin_sub_overflow )
#    define MSE_IMPL_HAS__builtin_sub_overflow 
#  endif
#endif
#ifdef MSE_IMPL_HAS__builtin_sub_overflow 
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1> retval;
				auto overflow_flag = __builtin_sub_overflow(mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>((*this).m_val)
					, mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>(x.m_val), &retval);
				if (overflow_flag) {
					MSE_THROW(primitives_range_error("range error - result of the subtraction operation is out of range of the target (integer) type"));
				}
				return retval;
			} else {
				return (mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>((*this).m_val) - mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>(x.m_val));
			}
#else /*MSE_IMPL_HAS__builtin_sub_overflow*/
			check_for_subtract_overflow<mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1> >((*this).m_val, x.m_val);
			return (mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>((*this).m_val) - mse::impl::native_int_subtract_result_type1<base_type, base_type, TOption1>(x.m_val));
#endif /*MSE_IMPL_HAS__builtin_sub_overflow*/
		}
		template<typename _Ty2, typename TOption2>
		auto operator -(const TInt<_Ty2, TOption2> &x) const ->MSE_TINT_SUBTRACT_RESULT_TYPE1(TInt, TInt<_Ty2>, TOption1) {
			(*this).assert_initialized();
			return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>((*this).m_val) - TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator -(_Ty2 x) const { (*this).assert_initialized(); return ((*this) - TInt<_Ty2>(x)); }

		template<typename TReturn, typename TArg1, typename TArg2>
		void check_for_multiply_overflow(TArg1 x, TArg2 y) const {
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				/* We're assuming standard ranges for signed integers [-(2^(2^n-1)), 2^(2^n-1)-1], and that the range of TReturn  encompasses the range of TArg1 and TArg2. */
				MSE_PRM_IF_CONSTEXPR((std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg1>::max()) && (std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg2>::max())
					&& (((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))
						|| ((std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg1>::min()) && (std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg2>::min())))) {
					// no chance of overflow, nothing to do
				}
				else {
					/* needs to be checked */
					if (0 < x) {
						if (0 < y) {
							if (std::numeric_limits<TReturn>::max() / y < x) {
								MSE_THROW(primitives_range_error("range error - result of the multiplication operation is out of range of the target (integer) type"));
							}
						} else {
							MSE_PRM_IF_CONSTEXPR(!((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))) {
								if (0 > y) {
									if (std::numeric_limits<TReturn>::min() / x > y) {
										MSE_THROW(primitives_range_error("range error - result of the multiplication operation is out of range of the target (integer) type"));
									}
								}
							}
						}
					} else {
						MSE_PRM_IF_CONSTEXPR ((0 == std::numeric_limits<TArg1>::min())) {
							// x must be 0
						} else if (0 > x) {
							if (0 < y) {
								if (std::numeric_limits<TReturn>::min() / y > x) {
									MSE_THROW(primitives_range_error("range error - result of the multiplication operation is out of range of the target (integer) type"));
								}
							} else {
								MSE_PRM_IF_CONSTEXPR((0 == std::numeric_limits<TArg2>::min())) {
									// y must be 0
								} else {
									if (((-1 == y) && (std::numeric_limits<TReturn>::min() == x))
										|| ((-1 == x) && (std::numeric_limits<TReturn>::min() == y))) {
										MSE_THROW(primitives_range_error("range error - result of the multiplication operation is out of range of the target (integer) type"));
									} else {
										if (std::numeric_limits<TReturn>::max() / y > x) {
											MSE_THROW(primitives_range_error("range error - result of the multiplication operation is out of range of the target (integer) type"));
										}
									}
								}
							}
						}
					}
				}
			}
		}
		auto operator *(const TInt& x) const->MSE_TINT_MULTIPLY_RESULT_TYPE1(TInt, TInt, TOption1) {
			(*this).assert_initialized();

#if defined __has_builtin
#  if __has_builtin (__builtin_mul_overflow )
#    define MSE_IMPL_HAS__builtin_mul_overflow 
#  endif
#endif
#ifdef MSE_IMPL_HAS__builtin_mul_overflow 
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1> retval;
				auto overflow_flag = __builtin_mul_overflow(mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>((*this).m_val)
					, mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>(x.m_val), &retval);
				if (overflow_flag) {
					MSE_THROW(primitives_range_error("range error - result of the multiply operation is out of range of the target (integer) type"));
				}
				return retval;
			} else {
				return (mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>((*this).m_val) * mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>(x.m_val));
			}
#else /*MSE_IMPL_HAS__builtin_mul_overflow*/
			check_for_multiply_overflow<mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1> >((*this).m_val, x.m_val);
			return (mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>((*this).m_val) * mse::impl::native_int_multiply_result_type1<base_type, base_type, TOption1>(x.m_val));
#endif /*MSE_IMPL_HAS__builtin_mul_overflow*/
		}
		template<typename _Ty2, typename TOption2>
		auto operator *(const TInt<_Ty2, TOption2> &x) const ->MSE_TINT_MULTIPLY_RESULT_TYPE1(TInt, TInt<_Ty2>, TOption1) {
			(*this).assert_initialized();
			return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>((*this).m_val) * TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>(x.m_val));
		}
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator *(_Ty2 x) const { (*this).assert_initialized(); return ((*this) * TInt<_Ty2>(x)); }

		template<typename TReturn, typename TArg1, typename TArg2>
		void check_for_divide_overflow(TArg1 x, TArg2 y) const {
			MSE_PRM_IF_CONSTEXPR(std::is_base_of<mse::enable_AR_overflow_checking_t, TOption1>::value) {
				/* We're assuming standard ranges for signed integers [-(2^(2^n-1)), 2^(2^n-1)-1], and that the range of TReturn  encompasses the range of TArg1 and TArg2. */
				/* Division by zero is presumed to be already checked elsewhere. */
				MSE_PRM_IF_CONSTEXPR((std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg1>::max()) && (std::numeric_limits<TReturn>::max() > std::numeric_limits<TArg2>::max())
					&& (((0 == std::numeric_limits<TArg1>::min()) && (0 == std::numeric_limits<TArg2>::min()))
						|| ((std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg1>::min()) && (std::numeric_limits<TReturn>::min() < std::numeric_limits<TArg2>::min())))) {
					// no chance of overflow, nothing to do
				}
				else {
					if ((-1 == y) && (std::numeric_limits<TReturn>::min() == x)) {
						MSE_THROW(primitives_range_error("range error - result of the division operation is out of range of the target (integer) type"));
					}
				}
			}
		}
		auto operator /(const TInt &x) const->MSE_TINT_DIVIDE_RESULT_TYPE1(TInt, TInt, TOption1) {
			(*this).assert_initialized();
			if (x.m_val == 0) { MSE_THROW(std::domain_error("attempted division by zero - TInt")); }
			check_for_divide_overflow<mse::impl::native_int_divide_result_type1<base_type, base_type, TOption1> >((*this).m_val, x.m_val);
			return (mse::impl::native_int_divide_result_type1<base_type, base_type, TOption1>((*this).m_val) / mse::impl::native_int_divide_result_type1<base_type, base_type, TOption1>(x.m_val));
		}
		template<typename _Ty2, typename TOption2>
		auto operator /(const TInt<_Ty2, TOption2> &x) const->MSE_TINT_DIVIDE_RESULT_TYPE1(TInt, TInt<_Ty2>, TOption1) {
			(*this).assert_initialized();
			return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>((*this).m_val) / TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>), TOption1>(x.m_val));
		}
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator /(_Ty2 x) const { (*this).assert_initialized(); return ((*this) / TInt<_Ty2>(x)); }

		/* We use templated equality comparison operators only in an attempt to avoid compile errors due to ambiguity. */
		//bool operator ==(const TInt& x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) == (x.m_val)); }
		template<typename _Ty2, typename TOption2>
		bool operator ==(const TInt<_Ty2, TOption2>& x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)((*this).m_val) == MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator ==(_Ty2 x) const { (*this).assert_initialized(); return ((*this) == TInt<_Ty2>(x)); }

		//bool operator !=(const TInt& x) const { return !((*this) == x); }
		template<typename _Ty2, typename TOption2>
		bool operator !=(const TInt<_Ty2, TOption2>& x) const { return !((*this) == x); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator !=(_Ty2 x) const { return !((*this) == x); }

		//bool operator <(const TInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) < (x.m_val)); }
		template<typename _Ty2>
		bool operator <(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)((*this).m_val) < MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator <(_Ty2 x) const { (*this).assert_initialized(); return ((*this) < TInt<_Ty2>(x)); }

		//bool operator >(const TInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) > (x.m_val)); }
		template<typename _Ty2>
		bool operator >(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)((*this).m_val) > MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator >(_Ty2 x) const { (*this).assert_initialized(); return ((*this) > TInt<_Ty2>(x)); }

		//bool operator <=(const TInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) <= (x.m_val)); }
		template<typename _Ty2>
		bool operator <=(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)((*this).m_val) <= MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator <=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) <= TInt<_Ty2>(x)); }

		//bool operator >=(const TInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) >= (x.m_val)); }
		template<typename _Ty2>
		bool operator >=(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)((*this).m_val) >= MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TInt, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator >=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) >= TInt<_Ty2>(x)); }

		// INCREMENT/DECREMENT OPERATORS
		TInt& operator ++() {
			(*this).assert_initialized();
			MSE_PRM_IF_CONSTEXPR (std::numeric_limits<base_type>::digits >= MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW) {
				++((*this).m_val);
			}
			else {
				(*this) += 1;
			}
			return (*this);
		}
		TInt operator ++(int) {
			(*this).assert_initialized();
			TInt tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		TInt& operator --() {
			MSE_PRM_IF_CONSTEXPR (std::numeric_limits<base_type>::digits >= MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW) {
				--((*this).m_val);
			}
			else {
				(*this) -= 1;
			}
			return (*this);
		}
		TInt operator --(int) {
			(*this).assert_initialized();
			TInt tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		void async_shareable_and_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template<typename _Ty2>
		auto checked_and_adjusted_x(const _Ty2& x) {
			(*this).template assign_check_range<_Ty2>(x);
			return static_cast<base_type>(x);
		}

		template<typename _Ty2, typename TOption1_2> friend class TInt;
		template<typename TOption1_2> friend class TSize_t;
	};
}

namespace std {
#ifndef _THROW0
#define _THROW0()
#endif // !_THROW0
#ifndef _STCONS
#define _STCONS(ty, name, val)	static constexpr ty name = static_cast<ty>(val)
#endif // !_STCONS

	template<typename TBase, typename TOption1>
	class numeric_limits<mse::TInt<TBase, TOption1> > {	// limits for type int
	public:
		typedef TBase base_type;

		static constexpr base_type(min)() _THROW0()
		{	// return minimum value
			return numeric_limits<TBase>::min();
		}
		static constexpr base_type(max)() _THROW0()
		{	// return maximum value
			return numeric_limits<TBase>::max();
		}
		static constexpr base_type lowest() _THROW0()
		{	// return most negative value
			return numeric_limits<TBase>::lowest();
		}
		static constexpr base_type epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return numeric_limits<TBase>::epsilon();
		}
		static constexpr base_type round_error() _THROW0()
		{	// return largest rounding error
			return numeric_limits<TBase>::round_error();
		}
		static constexpr base_type denorm_min() _THROW0()
		{	// return minimum denormalized value
			return numeric_limits<TBase>::denorm_min();
		}
		static constexpr base_type infinity() _THROW0()
		{	// return positive infinity
			return numeric_limits<TBase>::infinity();
		}
		static constexpr base_type quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return numeric_limits<TBase>::quiet_NaN();
		}
		static constexpr base_type signaling_NaN() _THROW0()
		{	// return signaling NaN
			return numeric_limits<TBase>::signaling_NaN();
		}
		//_STCONS(float_denorm_style, has_denorm, numeric_limits<TBase>::has_denorm);
		//_STCONS(bool, has_denorm_loss, numeric_limits<TBase>::has_denorm_loss);
		_STCONS(bool, has_infinity, numeric_limits<TBase>::has_infinity);
		_STCONS(bool, has_quiet_NaN, numeric_limits<TBase>::has_quiet_NaN);
		_STCONS(bool, has_signaling_NaN, numeric_limits<TBase>::has_signaling_NaN);
		_STCONS(bool, is_bounded, numeric_limits<TBase>::is_bounded);
		_STCONS(bool, is_exact, numeric_limits<TBase>::is_exact);
		_STCONS(bool, is_iec559, numeric_limits<TBase>::is_iec559);
		_STCONS(bool, is_integer, numeric_limits<TBase>::is_integer);
		_STCONS(bool, is_modulo, numeric_limits<TBase>::is_modulo);
		_STCONS(bool, is_signed, numeric_limits<TBase>::is_signed);
		_STCONS(bool, is_specialized, numeric_limits<TBase>::is_specialized);
		_STCONS(bool, tinyness_before, numeric_limits<TBase>::tinyness_before);
		_STCONS(bool, traps, numeric_limits<TBase>::traps);
		_STCONS(float_round_style, round_style, numeric_limits<TBase>::round_style);
		_STCONS(int, digits, numeric_limits<TBase>::digits);
		_STCONS(int, digits10, numeric_limits<TBase>::digits10);
		_STCONS(int, max_digits10, numeric_limits<TBase>::max_digits10);
		_STCONS(int, max_exponent, numeric_limits<TBase>::max_exponent);
		_STCONS(int, max_exponent10, numeric_limits<TBase>::max_exponent10);
		_STCONS(int, min_exponent, numeric_limits<TBase>::min_exponent);
		_STCONS(int, min_exponent10, numeric_limits<TBase>::min_exponent10);
		_STCONS(int, radix, numeric_limits<TBase>::radix);
	};

	template<typename TBase, typename TOption1>
	struct hash<mse::TInt<TBase, TOption1> > {	// hash functor
		typedef mse::TInt<TBase, TOption1> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TInt<TBase, TOption1>& _Keyval) const _NOEXCEPT {
			return (hash<typename mse::TInt<TBase, TOption1>::base_type>()(_Keyval));
		}
	};

#define MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(x) \
	template <typename TOption1> \
	struct make_signed<mse::TInt<x, TOption1> > { \
		using type = mse::TInt<typename mse::impl::make_signed<x>::type, TOption1>; \
	}; \
	template <typename TOption1> \
	struct make_signed<mse::TInt<const x, TOption1> > { \
		using type = mse::TInt<typename mse::impl::make_signed<const x>::type, TOption1>; \
	}; \
	template <typename TOption1> \
	struct make_signed<const mse::TInt<x, TOption1> > { \
		using type = const mse::TInt<typename mse::impl::make_signed<x>::type, TOption1>; \
	}; \
	template <typename TOption1> \
	struct make_signed<const mse::TInt<const x, TOption1> > { \
		using type = const mse::TInt<typename mse::impl::make_signed<const x>::type, TOption1>; \
	};
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(unsigned char);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(unsigned short int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(unsigned long int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(unsigned long long int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(unsigned int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(char);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(short int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(long int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(long long int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(int);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(signed char);

#define MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(x) \
	template <> \
	struct make_unsigned<mse::TInt<x> > { \
		using type = mse::TInt<typename mse::impl::make_unsigned<x>::type>; \
	}; \
	template <typename TOption1> \
	struct make_unsigned<mse::TInt<const x, TOption1> > { \
		using type = mse::TInt<typename mse::impl::make_unsigned<const x>::type, TOption1>; \
	}; \
	template <typename TOption1> \
	struct make_unsigned<const mse::TInt<x, TOption1> > { \
		using type = const mse::TInt<typename mse::impl::make_unsigned<x>::type, TOption1>; \
	}; \
	template <typename TOption1> \
	struct make_unsigned<const mse::TInt<const x, TOption1> > { \
		using type = const mse::TInt<typename mse::impl::make_unsigned<const x>::type, TOption1>; \
	};
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(unsigned char);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(unsigned short int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(unsigned long int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(unsigned long long int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(unsigned int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(char);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(short int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(long int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(long long int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(int);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(signed char);

#ifdef MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT);
	MSE_IMPL_MAKE_SIGNED_TINT_SPECIALIZATION(MSE_IMPL_LONG_LONG_LONG_INT);
	MSE_IMPL_MAKE_UNSIGNED_TINT_SPECIALIZATION(MSE_IMPL_LONG_LONG_LONG_INT);
#endif // MSE_IMPL_UNSIGNED_LONG_LONG_LONG_INT

}

namespace mse {
	/* Note that TSize_t does not have a default conversion to size_t. This is by design. Use the as_a_size_t() member
	function to get a size_t when necessary. */
	template<typename TOption1/* = RETURN_RANGE_EXTENSION_DEFAULT*/>
	class TSize_t : private impl::TArithmeticBase1<size_t> {
	public:
		typedef impl::TArithmeticBase1<size_t> base_class;
		typedef size_t base_type;
		typedef typename std::conditional<(!std::is_base_of<enable_AR_range_extension_t, TOption1>::value), typename CNDInt::base_type, MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, signed char/*just to make sure the resulting type is signed*/)>::type signed_size_t;
		typedef TInt<signed_size_t, TOption1> TSignedSize_t;

		TSize_t() : base_class() {}
		TSize_t(const TSize_t &x) : base_class(x) {};
		//TSize_t(const base_class &x) : base_class(x) {};
		/*explicit */TSize_t(CNDInt x) : base_class(static_cast<base_type>(x)) { (*this).template assign_check_range<CNDInt>(x); }

		template<typename _Ty, class = typename std::enable_if<std::is_convertible<_Ty, base_type>::value, void>::type>
		TSize_t(const _Ty& x) : base_class(static_cast<base_type>(x)) { (*this).template assign_check_range<_Ty>(x); }

		TSize_t& operator=(const TSize_t &x) { (*this).note_value_assignment(); (*this).m_val = x.m_val; return (*this); }
		template<typename _Ty>
		TSize_t& operator=(const _Ty& x) { (*this).template assign_check_range<_Ty>(x); (*this).m_val = static_cast<base_type>(x); return (*this); }

		operator signed_size_t() const { (*this).assert_initialized(); impl::g_assign_check_range<signed_size_t, decltype(m_val)>(m_val); return signed_size_t(m_val); }
#ifndef MSVC2010_COMPATIBLE
		explicit operator TSignedSize_t() const { (*this).assert_initialized(); return TSignedSize_t(m_val); }
		//explicit operator CNDInt() const { (*this).assert_initialized(); return CNDInt(m_val); }
		explicit operator size_t() const { (*this).assert_initialized(); return (m_val); }
		/* provisional */
		const size_t& mse_base_type_ref() const & { (*this).assert_initialized(); return (*this).m_val; }
		const size_t& mse_base_type_ref() const&& = delete;
		size_t& mse_base_type_ref() & { (*this).assert_initialized(); return (*this).m_val; }
		size_t& mse_base_type_ref() && = delete;

		//explicit operator typename CNDInt::base_type() const { (*this).assert_initialized(); return CNDInt(m_val); }
#endif /*MSVC2010_COMPATIBLE*/
		//size_t as_a_size_t() const { (*this).assert_initialized(); return m_val; }

		TSize_t operator ~() const { (*this).assert_initialized(); return (~m_val); }
		TSize_t& operator |=(const TSize_t &x) { (*this).assert_initialized(); m_val |= x.m_val; return (*this); }
		TSize_t& operator &=(const TSize_t &x) { (*this).assert_initialized(); m_val &= x.m_val; return (*this); }
		TSize_t& operator ^=(const TSize_t &x) { (*this).assert_initialized(); m_val ^= x.m_val; return (*this); }

		TSignedSize_t operator -() const { (*this).assert_initialized(); /* Should unsigned types even support this operator? */
			return (-(TSignedSize_t(m_val)));
		}
		TSize_t& operator +=(const TSize_t &x) { (*this) = (*this) + x; return (*this); }
		TSize_t& operator -=(const TSize_t &x) { (*this) = (*this) - x; return (*this);  }
		TSize_t& operator *=(const TSize_t &x) { (*this) = (*this) * x; return (*this); }
		TSize_t& operator /=(const TSize_t &x) { (*this) = (*this) / x; return (*this); }
		TSize_t& operator %=(const TSize_t &x) { (*this) = (*this) % x; return (*this); }
		TSize_t& operator >>=(const TSize_t &x) { (*this) = (*this) >> x; return (*this); }
		TSize_t& operator <<=(const TSize_t &x) { (*this) = (*this) << x; return (*this); }

		auto operator +(const TSize_t& x) const->MSE_TINT_ADD_RESULT_TYPE1(TSize_t, TSize_t, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type), TOption1>((*this).m_val) + MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type)(x.m_val)); }
		template<typename _Ty2, typename TOption2>
		auto operator +(const TInt<_Ty2, TOption2>& x) const->MSE_TINT_ADD_RESULT_TYPE1(TSize_t, TInt<_Ty2>, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>((*this).m_val) + TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator +(_Ty2 x) const { (*this).assert_initialized(); return ((*this) + TInt<_Ty2>(x)); }

		auto operator -(const TSize_t& x) const->MSE_TINT_SUBTRACT_RESULT_TYPE1(TSize_t, TSize_t, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type), TOption1>((*this).m_val) - MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type)(x.m_val)); }
		template<typename _Ty2, typename TOption2>
		auto operator -(const TInt<_Ty2, TOption2>& x) const->MSE_TINT_SUBTRACT_RESULT_TYPE1(TSize_t, TInt<_Ty2>, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>((*this).m_val) - TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator -(_Ty2 x) const { (*this).assert_initialized(); return ((*this) - TInt<_Ty2>(x)); }

		auto operator *(const TSize_t& x) const->MSE_TINT_MULTIPLY_RESULT_TYPE1(TSize_t, TSize_t, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type), TOption1>((*this).m_val) * MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(base_type, base_type)(x.m_val)); }
		template<typename _Ty2, typename TOption2>
		auto operator *(const TInt<_Ty2, TOption2>& x) const->MSE_TINT_MULTIPLY_RESULT_TYPE1(TSize_t, TInt<_Ty2>, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>((*this).m_val) * TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator *(_Ty2 x) const { (*this).assert_initialized(); return ((*this) * TInt<_Ty2>(x)); }

		TSize_t operator /(const TSize_t &x) const {
			if (x.m_val == 0) { MSE_THROW(std::domain_error("attempted division by zero - TSize_t")); }
			(*this).assert_initialized(); return (m_val / x.m_val);
		}
		TSignedSize_t operator /(const CNDInt &x) const { (*this).assert_initialized(); return (TSignedSize_t(m_val) / x); }
		TSize_t operator /(size_t x) const { (*this).assert_initialized(); return ((*this) / TSize_t(x)); }
		template<typename _Ty2, typename TOption2>
		auto operator /(const TInt<_Ty2, TOption2> &x) const->MSE_TINT_DIVIDE_RESULT_TYPE1(TSize_t, TInt<_Ty2>, TOption1) { (*this).assert_initialized(); return (TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>((*this).m_val) / TInt<MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>), TOption1>(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		auto operator /(_Ty2 x) const { (*this).assert_initialized(); return ((*this) / TInt<_Ty2>(x)); }

		/* We use templated equality comparison operators only in an attempt to avoid compile errors due to ambiguity. */
		template<typename TOption2>
		bool operator ==(const TSize_t<TOption2>& x) const { (*this).assert_initialized(); x.assert_initialized(); return (m_val == x.m_val); }
		//bool operator ==(const CNDInt& x) const { (*this).assert_initialized(); x.assert_initialized(); return (TSignedSize_t(m_val) == x); }
		//bool operator ==(size_t x) const { (*this).assert_initialized(); return ((*this) == TSize_t(x)); }
		template<typename _Ty2, typename TOption2>
		bool operator ==(const TInt<_Ty2, TOption2>& x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)((*this).m_val) == MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator ==(_Ty2 x) const { (*this).assert_initialized(); return ((*this) == TInt<_Ty2>(x)); }

		template<typename TOption2>
		bool operator !=(const TSize_t<TOption2>& x) const { return !((*this) == x); }
		//bool operator !=(const CNDInt& x) const { return !((*this) == x); }
		//bool operator !=(size_t x) const { return !((*this) == x); }
		template<typename _Ty2>
		bool operator !=(const TInt<_Ty2>& x) const { return !((*this) == x); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator !=(_Ty2 x) const { return !((*this) == x); }

		template<typename TOption2>
		bool operator <(const TSize_t<TOption2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (m_val < x.m_val); }
		//bool operator <(const CNDInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (TSignedSize_t(m_val) < x); }
		//bool operator <(size_t x) const { (*this).assert_initialized(); return ((*this) < TSize_t(x)); }
		template<typename _Ty2>
		bool operator <(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)((*this).m_val) < MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator <(_Ty2 x) const { (*this).assert_initialized(); return ((*this) < TInt<_Ty2>(x)); }

		template<typename TOption2>
		bool operator >(const TSize_t<TOption2>&x) const { (*this).assert_initialized(); x.assert_initialized(); return (m_val > x.m_val); }
		//bool operator >(const CNDInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (TSignedSize_t(m_val) > x); }
		//bool operator >(size_t x) const { (*this).assert_initialized(); return ((*this) > TSize_t(x)); }
		template<typename _Ty2>
		bool operator >(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)((*this).m_val) > MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator >(_Ty2 x) const { (*this).assert_initialized(); return ((*this) > TInt<_Ty2>(x)); }

		template<typename TOption2>
		bool operator <=(const TSize_t<TOption2>&x) const { (*this).assert_initialized(); x.assert_initialized(); return (m_val <= x.m_val); }
		//bool operator <=(const CNDInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (TSignedSize_t(m_val) <= x); }
		//bool operator <=(size_t x) const { (*this).assert_initialized(); return ((*this) <= TSize_t(x)); }
		template<typename _Ty2>
		bool operator <=(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)((*this).m_val) <= MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator <=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) <= TInt<_Ty2>(x)); }

		template<typename TOption2>
		bool operator >=(const TSize_t<TOption2>&x) const { (*this).assert_initialized(); x.assert_initialized(); return (m_val >= x.m_val); }
		//bool operator >=(const CNDInt &x) const { (*this).assert_initialized(); x.assert_initialized(); return (TSignedSize_t(m_val) >= x); }
		//bool operator >=(size_t x) const { (*this).assert_initialized(); return ((*this) >= TSize_t(x)); }
		template<typename _Ty2>
		bool operator >=(const TInt<_Ty2> &x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)((*this).m_val) >= MSE_RANGE_ENCOMPASSING_NATIVE_INT_TYPE(TSize_t, TInt<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_integral<_Ty2>::value, void>::type>
		bool operator >=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) >= TInt<_Ty2>(x)); }

		// INCREMENT/DECREMENT OPERATORS
		TSize_t& operator ++() {
			MSE_PRM_IF_CONSTEXPR (std::numeric_limits<base_type>::digits >= MSE_IMPL_BITSIZE_AT_OR_ABOVE_WHICH_INCREMENT_OPERATORS_MAY_NOT_CHECK_FOR_OVERFLOW) {
				++((*this).m_val);
			}
			else {
				(*this) += 1;
			}
			return (*this);
		}
		TSize_t operator ++(int) { (*this).assert_initialized();
			TSize_t tmp(*this); // copy
			operator++(); // pre-increment
			return tmp;   // return old value
		}
		TSize_t& operator --() { (*this).assert_initialized(); (*this) -= 1; return (*this); }
		TSize_t operator --(int) { (*this).assert_initialized();
			TSize_t tmp(*this); // copy
			operator--(); // pre-decrement
			return tmp;   // return old value
		}

		void async_shareable_and_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		//base_type m_val;

		friend size_t as_a_size_t(CNDSize_t const& n);
	};
	size_t as_a_size_t(CNDSize_t const& n) { n.assert_initialized(); return n.m_val; }
}

namespace std {
#ifndef _THROW0
#define _THROW0()
#endif // !_THROW0
#ifndef _STCONS
#define _STCONS(ty, name, val)	static constexpr ty name = (ty)(val)
#endif // !_STCONS

	template<typename TOption1> class numeric_limits<mse::TSize_t<TOption1> > {	// limits for type int
	public:
		typedef size_t base_type;

		static constexpr base_type(min)() _THROW0()
		{	// return minimum value
			return numeric_limits<size_t>::min();
		}
		static constexpr base_type(max)() _THROW0()
		{	// return maximum value
			return numeric_limits<size_t>::max();
		}
		static constexpr base_type lowest() _THROW0()
		{	// return most negative value
			return numeric_limits<size_t>::lowest();
		}
		static constexpr base_type epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return numeric_limits<size_t>::epsilon();
		}
		static constexpr base_type round_error() _THROW0()
		{	// return largest rounding error
			return numeric_limits<size_t>::round_error();
		}
		static constexpr base_type denorm_min() _THROW0()
		{	// return minimum denormalized value
			return numeric_limits<size_t>::denorm_min();
		}
		static constexpr base_type infinity() _THROW0()
		{	// return positive infinity
			return numeric_limits<size_t>::infinity();
		}
		static constexpr base_type quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return numeric_limits<size_t>::quiet_NaN();
		}
		static constexpr base_type signaling_NaN() _THROW0()
		{	// return signaling NaN
			return numeric_limits<size_t>::signaling_NaN();
		}
		//_STCONS(float_denorm_style, has_denorm, numeric_limits<size_t>::has_denorm);
		//_STCONS(bool, has_denorm_loss, numeric_limits<size_t>::has_denorm_loss);
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

	template<typename TOption1>
	struct hash<mse::TSize_t<TOption1> > {	// hash functor
		typedef mse::TSize_t<TOption1> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TSize_t<TOption1>& _Keyval) const _NOEXCEPT {
			return (hash<typename mse::TSize_t<TOption1>::base_type>()(mse::as_a_size_t(_Keyval)));
		}
	};

	template <typename TOption1>
	struct make_signed<mse::TSize_t<TOption1> > {
		using type = mse::TInt<typename mse::impl::make_signed<size_t>::type, TOption1>;
	};
	template <typename TOption1>
	struct make_signed<const mse::TSize_t<TOption1> > {
		using type = const mse::TInt<typename mse::impl::make_signed<size_t>::type, TOption1>;
	};

	template <typename TOption1>
	struct make_unsigned<mse::TSize_t<TOption1> > {
		using type = mse::TSize_t<TOption1>;
	};
	template <typename TOption1>
	struct make_unsigned<const mse::TSize_t<TOption1> > {
		using type = const mse::TSize_t<TOption1>;
	};
}

namespace mse {
	namespace impl {
		template<typename _Ty>
		struct corresponding_TFloatingPoint { typedef typename std::conditional<std::is_arithmetic<_Ty>::value, TFloatingPoint<_Ty>, _Ty>::type type; };
#define MSE_TFLOATINGPOINT_TYPE(_Ty) typename mse::impl::corresponding_TFloatingPoint<_Ty>::type
#define MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty) MSE_TFLOATINGPOINT_TYPE(_Ty)::base_type
#define MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz) typename std::conditional<sizeof(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty)) < sizeof(MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz)), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz), MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty)>::type

#define MSE_NATIVE_FLOATINGPOINT_RESULT_TYPE1(_Ty, _Tz) MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz)
#define MSE_NATIVE_FLOATINGPOINT_ADD_RESULT_TYPE1(_Ty, _Tz) MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz)
#define MSE_NATIVE_FLOATINGPOINT_SUBTRACT_RESULT_TYPE1(_Ty, _Tz) MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz)
#define MSE_NATIVE_FLOATINGPOINT_MULTIPLY_RESULT_TYPE1(_Ty, _Tz) MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz)
#define MSE_NATIVE_FLOATINGPOINT_DIVIDE_RESULT_TYPE1(_Ty, _Tz) MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(_Ty, _Tz)

#define MSE_TFLOATINGPOINT_RESULT_TYPE1(_Ty, _Tz) TFloatingPoint<MSE_NATIVE_FLOATINGPOINT_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz))>
#define MSE_TFLOATINGPOINT_ADD_RESULT_TYPE1(_Ty, _Tz) TFloatingPoint<MSE_NATIVE_FLOATINGPOINT_ADD_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz))>
#define MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(_Ty, _Tz) TFloatingPoint<MSE_NATIVE_FLOATINGPOINT_SUBTRACT_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz))>
#define MSE_TFLOATINGPOINT_MULTIPLY_RESULT_TYPE1(_Ty, _Tz) TFloatingPoint<MSE_NATIVE_FLOATINGPOINT_MULTIPLY_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz))>
#define MSE_TFLOATINGPOINT_DIVIDE_RESULT_TYPE1(_Ty, _Tz) TFloatingPoint<MSE_NATIVE_FLOATINGPOINT_DIVIDE_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(_Ty), MSE_NATIVE_FLOATINGPOINT_TYPE(_Tz))>
	}

	/* Native floating point types don't pose quite the same safety and correctness risks that native integer types do, but substitutes
	that can act as base classes are still needed. */
	template<typename TBase>
	class TFloatingPoint : private impl::TArithmeticBase1<TBase> {
	public:
		typedef impl::TArithmeticBase1<TBase> base_class;
		typedef TBase base_type;

		TFloatingPoint() : base_class() {}
		TFloatingPoint(const TFloatingPoint& x) : base_class(x) {};
		//explicit TFloatingPoint(const base_class &x) : base_class(x) {};
		//explicit TFloatingPoint(const TSize_t &x) : base_class(CNDInt(x)) { (*this).template assign_check_range<CNDInt>(CNDInt(x)); };

		template<typename _Ty, class = typename std::enable_if<std::is_convertible<_Ty, base_type>::value, void>::type>
			TFloatingPoint(const _Ty& x) : base_class(checked_and_adjusted_x(x)) {}

		TFloatingPoint& operator=(const TFloatingPoint& x) { (*this).note_value_assignment(); (*this).m_val = x.m_val; return (*this); }
		template<typename _Ty>
		TFloatingPoint& operator=(const _Ty& x) { (*this).template assign_check_range<_Ty>(x); (*this).m_val = x/*static_cast<base_type>(x)*/; return (*this); }

		operator base_type() const { (*this).assert_initialized(); return (*this).m_val; }
		/* provisional */
		const base_type& mse_base_type_ref() const& { (*this).assert_initialized(); return (*this).m_val; }
		const base_type& mse_base_type_ref() const&& = delete;
		base_type& mse_base_type_ref()& { (*this).assert_initialized(); return (*this).m_val; }
		base_type& mse_base_type_ref() && = delete;

		auto operator -() const->MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(TFloatingPoint, TFloatingPoint) { (*this).assert_initialized(); return -((*this).m_val); }
		TFloatingPoint& operator +=(const TFloatingPoint& x) { (*this) = (*this) + x; return (*this); }
		TFloatingPoint& operator -=(const TFloatingPoint& x) { (*this) = (*this) - x; return (*this); }
		TFloatingPoint& operator *=(const TFloatingPoint& x) { (*this) = (*this) * x; return (*this); }
		TFloatingPoint& operator /=(const TFloatingPoint& x) { (*this) = (*this) / x; return (*this); }

		//auto operator +(const TFloatingPoint& x) const->MSE_TFLOATINGPOINT_ADD_RESULT_TYPE1(TFloatingPoint, TFloatingPoint) { (*this).assert_initialized(); x.assert_initialized(); return (MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(base_type, base_type)((*this).m_val) + MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(base_type, base_type)(x.m_val)); }
		template<typename _Ty2>
		auto operator +(const TFloatingPoint<_Ty2>& x) const->MSE_TFLOATINGPOINT_ADD_RESULT_TYPE1(TFloatingPoint, TFloatingPoint<_Ty2>) {
			(*this).assert_initialized(); x.assert_initialized();
			typedef MSE_NATIVE_FLOATINGPOINT_ADD_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint), MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint<_Ty2>)) native_add_result_type;
			return (native_add_result_type((*this).m_val) + native_add_result_type(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		auto operator +(_Ty2 x) const { (*this).assert_initialized(); return ((*this) + TFloatingPoint<_Ty2>(x)); }

		//auto operator -(const TFloatingPoint& x) const->MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(TFloatingPoint, TFloatingPoint) { (*this).assert_initialized(); x.assert_initialized(); return (MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(base_type, base_type)((*this).m_val) - MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(base_type, base_type)(x.m_val)); }
		template<typename _Ty2>
		auto operator -(const TFloatingPoint<_Ty2>& x) const->MSE_TFLOATINGPOINT_SUBTRACT_RESULT_TYPE1(TFloatingPoint, TFloatingPoint<_Ty2>) {
			(*this).assert_initialized(); x.assert_initialized();
			typedef MSE_NATIVE_FLOATINGPOINT_SUBTRACT_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint), MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint<_Ty2>)) native_subtract_result_type;
			return (native_subtract_result_type((*this).m_val) - native_subtract_result_type(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		auto operator -(_Ty2 x) const { (*this).assert_initialized(); return ((*this) - TFloatingPoint<_Ty2>(x)); }

		//auto operator *(const TFloatingPoint& x) const->MSE_TFLOATINGPOINT_MULTIPLY_RESULT_TYPE1(TFloatingPoint, TFloatingPoint) { (*this).assert_initialized(); x.assert_initialized(); return (MSE_TFLOATINGPOINT_MULTIPLY_RESULT_TYPE1(base_type, base_type)((*this).m_val) * MSE_TFLOATINGPOINT_MULTIPLY_RESULT_TYPE1(base_type, base_type)(x.m_val)); }
		template<typename _Ty2>
		auto operator *(const TFloatingPoint<_Ty2>& x) const->MSE_TFLOATINGPOINT_MULTIPLY_RESULT_TYPE1(TFloatingPoint, TFloatingPoint<_Ty2>) {
			(*this).assert_initialized(); x.assert_initialized();
			typedef MSE_NATIVE_FLOATINGPOINT_MULTIPLY_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint), MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint<_Ty2>)) native_multiply_result_type;
			return (native_multiply_result_type((*this).m_val) * native_multiply_result_type(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		auto operator *(_Ty2 x) const { (*this).assert_initialized(); return ((*this) * TFloatingPoint<_Ty2>(x)); }

		//auto operator /(const TFloatingPoint& x) const->MSE_TFLOATINGPOINT_DIVIDE_RESULT_TYPE1(TFloatingPoint, TFloatingPoint) { (*this).assert_initialized(); x.assert_initialized(); MSE_PRM_IF_CONSTEXPR(false) { if (x.m_val == 0) { MSE_THROW(std::domain_error("attempted division by zero - TFloatingPoint")); } } return (MSE_TFLOATINGPOINT_DIVIDE_RESULT_TYPE1(base_type, base_type)((*this).m_val) / MSE_TFLOATINGPOINT_DIVIDE_RESULT_TYPE1(base_type, base_type)(x.m_val)); }
		template<typename _Ty2>
		auto operator /(const TFloatingPoint<_Ty2>& x) const->MSE_TFLOATINGPOINT_DIVIDE_RESULT_TYPE1(TFloatingPoint, TFloatingPoint<_Ty2>) {
			(*this).assert_initialized(); x.assert_initialized();
			MSE_PRM_IF_CONSTEXPR(false) { if (x.m_val == 0) { MSE_THROW(std::domain_error("attempted division by zero - TFloatingPoint")); } }
			typedef MSE_NATIVE_FLOATINGPOINT_DIVIDE_RESULT_TYPE1(MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint), MSE_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint<_Ty2>)) native_divide_result_type;
			return (native_divide_result_type((*this).m_val) / native_divide_result_type(x.m_val));
		}
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		auto operator /(_Ty2 x) const { (*this).assert_initialized(); return ((*this) / TFloatingPoint<_Ty2>(x)); }

		/* For comparisons, we're just going to rely on our operator base_type() and the native floating-point comparison operators. */
		/*
		//bool operator ==(const TFloatingPoint& x) const { (*this).assert_initialized(); x.assert_initialized(); return (((*this).m_val) == (x.m_val)); }
		template<typename _Ty2>
		bool operator ==(const TFloatingPoint<_Ty2>& x) const { (*this).assert_initialized(); x.assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)((*this).m_val) == MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator ==(_Ty2 x) const { (*this).assert_initialized(); return ((*this) == TFloatingPoint<_Ty2>(x)); }

		//bool operator !=(const TFloatingPoint& x) const { return !((*this) == x); }
		template<typename _Ty2>
		bool operator !=(const TFloatingPoint<_Ty2>& x) const { return !((*this) == x); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator !=(_Ty2 x) const { return !((*this) == x); }

		bool operator <(const TFloatingPoint& x) const { (*this).assert_initialized(); return (((*this).m_val) < (x.m_val)); }
		template<typename _Ty2>
		bool operator <(const TFloatingPoint<_Ty2>& x) const { (*this).assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)((*this).m_val) < MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator <(_Ty2 x) const { (*this).assert_initialized(); return ((*this) < TFloatingPoint<_Ty2>(x)); }

		bool operator >(const TFloatingPoint& x) const { (*this).assert_initialized(); return (((*this).m_val) > (x.m_val)); }
		template<typename _Ty2>
		bool operator >(const TFloatingPoint<_Ty2>& x) const { (*this).assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)((*this).m_val) > MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator >(_Ty2 x) const { (*this).assert_initialized(); return ((*this) > TFloatingPoint<_Ty2>(x)); }

		bool operator <=(const TFloatingPoint& x) const { (*this).assert_initialized(); return (((*this).m_val) <= (x.m_val)); }
		template<typename _Ty2>
		bool operator <=(const TFloatingPoint<_Ty2>& x) const { (*this).assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)((*this).m_val) <= MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator <=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) <= TFloatingPoint<_Ty2>(x)); }

		bool operator >=(const TFloatingPoint& x) const { (*this).assert_initialized(); return (((*this).m_val) >= (x.m_val)); }
		template<typename _Ty2>
		bool operator >=(const TFloatingPoint<_Ty2>& x) const { (*this).assert_initialized(); return (MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)((*this).m_val) >= MSE_RANGE_ENCOMPASSING_NATIVE_FLOATINGPOINT_TYPE(TFloatingPoint, TFloatingPoint<_Ty2>)(x.m_val)); }
		template<typename _Ty2, class = typename std::enable_if<std::is_floating_point<_Ty2>::value, void>::type>
		bool operator >=(_Ty2 x) const { (*this).assert_initialized(); return ((*this) >= TFloatingPoint<_Ty2>(x)); }
		*/

		void async_shareable_and_passable_tag() const {}

	private:
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		template<typename _Ty2>
		auto checked_and_adjusted_x(const _Ty2& x) {
			(*this).template assign_check_range<_Ty2>(x);
			return static_cast<base_type>(x);
		}

		template<typename _Ty2> friend class TFloatingPoint;
	};
}

namespace std {
#ifndef _THROW0
#define _THROW0()
#endif // !_THROW0
#ifndef _STCONS
#define _STCONS(ty, name, val)	static constexpr ty name = static_cast<ty>(val)
#endif // !_STCONS

	template<typename TBase>
	class numeric_limits<mse::TFloatingPoint<TBase>> {	// limits for type int
	public:
		typedef TBase base_type;

		static constexpr base_type(min)() _THROW0()
		{	// return minimum value
			return numeric_limits<TBase>::min();
		}
		static constexpr base_type(max)() _THROW0()
		{	// return maximum value
			return numeric_limits<TBase>::max();
		}
		static constexpr base_type lowest() _THROW0()
		{	// return most negative value
			return numeric_limits<TBase>::lowest();
		}
		static constexpr base_type epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return numeric_limits<TBase>::epsilon();
		}
		static constexpr base_type round_error() _THROW0()
		{	// return largest rounding error
			return numeric_limits<TBase>::round_error();
		}
		static constexpr base_type denorm_min() _THROW0()
		{	// return minimum denormalized value
			return numeric_limits<TBase>::denorm_min();
		}
		static constexpr base_type infinity() _THROW0()
		{	// return positive infinity
			return numeric_limits<TBase>::infinity();
		}
		static constexpr base_type quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return numeric_limits<TBase>::quiet_NaN();
		}
		static constexpr base_type signaling_NaN() _THROW0()
		{	// return signaling NaN
			return numeric_limits<TBase>::signaling_NaN();
		}
		//_STCONS(float_denorm_style, has_denorm, numeric_limits<TBase>::has_denorm);
		//_STCONS(bool, has_denorm_loss, numeric_limits<TBase>::has_denorm_loss);
		_STCONS(bool, has_infinity, numeric_limits<TBase>::has_infinity);
		_STCONS(bool, has_quiet_NaN, numeric_limits<TBase>::has_quiet_NaN);
		_STCONS(bool, has_signaling_NaN, numeric_limits<TBase>::has_signaling_NaN);
		_STCONS(bool, is_bounded, numeric_limits<TBase>::is_bounded);
		_STCONS(bool, is_exact, numeric_limits<TBase>::is_exact);
		_STCONS(bool, is_iec559, numeric_limits<TBase>::is_iec559);
		_STCONS(bool, is_integer, numeric_limits<TBase>::is_integer);
		_STCONS(bool, is_modulo, numeric_limits<TBase>::is_modulo);
		_STCONS(bool, is_signed, numeric_limits<TBase>::is_signed);
		_STCONS(bool, is_specialized, numeric_limits<TBase>::is_specialized);
		_STCONS(bool, tinyness_before, numeric_limits<TBase>::tinyness_before);
		_STCONS(bool, traps, numeric_limits<TBase>::traps);
		_STCONS(float_round_style, round_style, numeric_limits<TBase>::round_style);
		_STCONS(int, digits, numeric_limits<TBase>::digits);
		_STCONS(int, digits10, numeric_limits<TBase>::digits10);
		_STCONS(int, max_digits10, numeric_limits<TBase>::max_digits10);
		_STCONS(int, max_exponent, numeric_limits<TBase>::max_exponent);
		_STCONS(int, max_exponent10, numeric_limits<TBase>::max_exponent10);
		_STCONS(int, min_exponent, numeric_limits<TBase>::min_exponent);
		_STCONS(int, min_exponent10, numeric_limits<TBase>::min_exponent10);
		_STCONS(int, radix, numeric_limits<TBase>::radix);
	};

	template<typename TBase>
	struct hash<mse::TFloatingPoint<TBase>> {	// hash functor
		typedef mse::TFloatingPoint<TBase> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TFloatingPoint<TBase>& _Keyval) const _NOEXCEPT {
			return (hash<typename mse::TFloatingPoint<TBase>::base_type>()(_Keyval));
		}
	};
}

namespace mse {

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator+(_Tz lhs, const TInt<_Ty>& rhs) { return rhs + lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator+(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs + lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator+(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs + lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator-(_Tz lhs, const TInt<_Ty>& rhs) { return -(rhs - lhs); }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator-(_Tz lhs, const TSize_t<TOption1>& rhs) { return lhs - CNDInt(rhs); }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator-(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return -(rhs - lhs); }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator*(_Tz lhs, const TInt<_Ty>& rhs) { return rhs * lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator*(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs * lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator*(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs * lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator/(_Tz lhs, const TInt<_Ty>& rhs) { return TInt<_Tz>(lhs) / rhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator/(_Tz lhs, const TSize_t<TOption1>& rhs) { return CNDInt(lhs) / rhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator/(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return TFloatingPoint<_Tz>(lhs) / rhs; }

#ifndef MSE_PRM_HAS_CXX20
	/* For comparisons, we're just going to rely on our operator bool() and the native bool comparison operators. */
	/*
	inline bool operator==(bool lhs, CNDBool rhs) { rhs.assert_initialized(); return CNDBool(lhs) == rhs; }
	inline bool operator!=(bool lhs, CNDBool rhs) { rhs.assert_initialized(); return CNDBool(lhs) != rhs; }
	*/

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<(_Tz lhs, const TInt<_Ty>& rhs) { return rhs > lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs > lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>(_Tz lhs, const TInt<_Ty>& rhs) { return rhs < lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs < lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<=(_Tz lhs, const TInt<_Ty>& rhs) { return rhs >= lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<=(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs >= lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>=(_Tz lhs, const TInt<_Ty>& rhs) { return rhs <= lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>=(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs <= lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator==(_Tz lhs, const TInt<_Ty>& rhs) { return rhs == lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator==(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs == lhs; }

	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator!=(_Tz lhs, const TInt<_Ty>& rhs) { return rhs != lhs; }
	template<typename _Tz, typename TOption1, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator!=(_Tz lhs, const TSize_t<TOption1>& rhs) { return rhs != lhs; }

	/* For comparisons, we're just going to rely on our operator base_type() and the native floating-point comparison operators. */
	/*
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs > lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs < lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator<=(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs >= lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator>=(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs <= lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator==(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs == lhs; }
	template<typename _Ty, typename _Tz, class = typename std::enable_if<(std::is_arithmetic<_Tz>::value), void>::type>
	inline auto operator!=(_Tz lhs, const TFloatingPoint<_Ty>& rhs) { return rhs != lhs; }
	*/
#endif // !MSE_PRM_HAS_CXX20


#ifdef MSE_PRIMITIVES_DISABLED
	typedef bool CBool;
	typedef MSE_CINT_BASE_INTEGER_TYPE CInt;
	typedef size_t CSize_t;
	static size_t as_a_size_t(CSize_t n) { return (n); }
	typedef float CFloat;
	typedef double CDouble;
	typedef long double CLongDouble;
#else /*MSE_PRIMITIVES_DISABLED*/

	typedef CNDBool CBool;
	typedef CNDInt CInt;
	typedef CNDSize_t CSize_t;
	typedef CNDFloat CFloat;
	typedef CNDDouble CDouble;
	typedef CNDLongDouble CLongDouble;

#endif /*MSE_PRIMITIVES_DISABLED*/


	namespace us {
		namespace impl {
			namespace rrt {

				template<typename _Ty>
				_Ty& force_lvalue_ref(_Ty& x) { return x; }
				template<typename _Ty>
				_Ty& force_lvalue_ref(_Ty&& x) { return force_lvalue_ref(x); }

				template<class T, class EqualTo>
				struct HasOrInheritsMseBaseTypeRefMethod_impl
				{
					template<class U, class V>
					static auto test(U*) -> decltype(force_lvalue_ref(std::declval<U>()).mse_base_type_ref(), force_lvalue_ref(std::declval<V>()).mse_base_type_ref(), bool(true));
					template<typename, typename>
					static auto test(...)->std::false_type;

					static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
					using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				};
				template<class T, class EqualTo = T>
				struct HasOrInheritsMseBaseTypeRefMethod : HasOrInheritsMseBaseTypeRefMethod_impl<
					typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

				template <class _Ty, class _Tz>
				struct base_type_helper1 {
					using type = _Ty;
				};
				template <class _Ty>
				struct base_type_helper1<_Ty, std::true_type> {
					using type = typename std::remove_reference<decltype(force_lvalue_ref(std::declval<_Ty>()).mse_base_type_ref())>::type;
				};
			}

			template <class _Ty>
			struct base_type : rrt::base_type_helper1<_Ty, typename rrt::HasOrInheritsMseBaseTypeRefMethod<_Ty>::type> {};
			template <class _Ty>
			using base_type_t = typename base_type<_Ty>::type;

			namespace rrt {
				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper3(std::true_type, _Tz& x) {
					return x.mse_base_type_ref();
				}
				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper3(std::false_type, _Tz& x) {
					_Tz* ptr1 = std::addressof(x);
					/* A compile error here can occur if, for example, if you try to assign a pointer to a 'const int' to a
					'TXScopeAnyConstPointer<unsigned int>'. 'int' and 'unsigned int' are "incompatible" types in this situation. */
					_Ty* ptr2 = ptr1;
					return *ptr2;
				}

				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper2(std::true_type, _Tz& x) {
					return raw_reference_to_helper3<_Ty>(typename std::is_convertible<base_type_t<_Tz>*, _Ty*>::type(), x);
				}
				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper2(std::false_type, _Tz& x) { return raw_reference_to_helper3<_Ty>(std::false_type(), x); }

				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper1(std::true_type, _Tz& x) { return raw_reference_to_helper3<_Ty>(std::false_type(), x); }
				template<typename _Ty, typename _Tz>
				_Ty& raw_reference_to_helper1(std::false_type, _Tz& x) {
					return raw_reference_to_helper2<_Ty>(typename HasOrInheritsMseBaseTypeRefMethod<_Tz>::type(), x);
				}
			}

			/* This function returns a raw reference of the specified type to the given object (which may be of a different type).
			In particular, it can be used to obtain a 'const int&' to an mse::TInt<int>. */
			template<typename _Ty, typename _Tz>
			_Ty& raw_reference_to(_Tz& x) {
				return rrt::raw_reference_to_helper1<_Ty>(typename std::is_convertible<_Tz*, _Ty*>::type(), x);
			}
			/* for now, we don't need this one */
			template<typename _Ty, typename _Tz>
			_Ty& raw_reference_to(_Tz&& x) = delete;

			/* This function returns a raw reference to the given object. The type of the reference is the "base_type" (not to
			be confused with "base class" type) of the object (which may or may not be different than the type of the object
			itself). In particular, given an object of type mse::TInt<int>, it would return a reference of type 'const int&'. */
			template<typename _Ty>
			base_type_t<_Ty>& base_type_raw_reference_to(_Ty& x) {
				return raw_reference_to<base_type_t<_Ty> >(x);
			}
		}
	}


#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_LONG_LONG_INTEGER_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(long long int, mse::TInt) \
	MACRO_FUNCTION(unsigned long long int, mse::TInt)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_LONG_INTEGER_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(long int, mse::TInt) \
	MACRO_FUNCTION(unsigned long int, mse::TInt)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_INTEGER_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(int, mse::TInt) \
	MACRO_FUNCTION(unsigned int, mse::TInt)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_SHORT_INTEGER_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(short int, mse::TInt) \
	MACRO_FUNCTION(unsigned short int, mse::TInt)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_CHAR_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(char, mse::TInt) \
	MACRO_FUNCTION(unsigned char, mse::TInt) \
	MACRO_FUNCTION(signed char, mse::TInt)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_LONG_LONG_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_LONG_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_SHORT_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_CHAR_TYPES(MACRO_FUNCTION)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_FLOATINGPOINT_TYPES(MACRO_FUNCTION) \
	MACRO_FUNCTION(float, mse::TFloatingPoint) \
	MACRO_FUNCTION(double, mse::TFloatingPoint) \
	MACRO_FUNCTION(long double, mse::TFloatingPoint)

	namespace impl {
		template<typename TBase>
		using TBoolWrapper = mse::CNDBool;
	}
#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_BOOL_TYPE(MACRO_FUNCTION) \
	MACRO_FUNCTION(bool, mse::impl::TBoolWrapper)

#define MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_INTEGER_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_FLOATINGPOINT_TYPES(MACRO_FUNCTION) \
	MSE_IMPL_APPLY_MACRO_FUNCTION_TO_BOOL_TYPE(MACRO_FUNCTION)

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4267 )
#endif /*_MSC_VER*/

	namespace self_test {
		class CPrimitivesTest1 {
		public:
			static void s_test1() {
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
				i4 = i3 - i1;
				i4 -= i1;
				i4 *= i1;
				i4 = i1 * i2;
				i4 /= i1;
				i4 = i1 * i2 / i3;
				CBool b1 = (i1 < i2);
				b1 = (i1 < 17);
				b1 = (19 < i1);
				b1 = (i1 == i2);
				b1 = (i1 == 17);
				b1 = (19 == i1);

				CSize_t szt1(3);
				CSize_t szt2 = 5U;
				CSize_t szt3;
				szt3 = 7;
				CSize_t szt4 = szt1 + szt2;
				szt4 = szt1 + 17;
				szt4 = 19 + szt1;
				szt4 += szt2;
				szt4 -= 23;
				szt4++;
#ifndef MSVC2010_COMPATIBLE
				size_t szt5 = size_t(szt4);
#endif /*MSVC2010_COMPATIBLE*/
				szt4 = szt3 - szt1;
				szt4 *= szt1;
				szt4 = szt1 * szt2;
				szt4 /= szt1;
				szt4 = szt1 * szt2 / szt3;

				CInt i11 = 19 + szt1;
				CInt i12 = szt1 * i11;
				i12 = szt1;
				i12 = szt3 - szt1;
				i12 = szt3 - i11;

				bool b3 = (szt1 < szt2);
				b3 = (szt1 < 17);
				b3 = (19 < szt1);
				CBool b2 = (19 < szt1);
				b3 = (szt1 == szt2);
				b3 = (szt1 == 17);
				b3 = (19 == szt1);
				CBool b4 = (b1 == b2);
				b4 = (b3 == b1);
				b4 = (b1 && b2);
				b4 = (b1 || b3);
				b4 = (b3 && b1);
				b4 |= b1;
				b4 &= b3;
#endif // MSE_SELF_TESTS
			}
		};

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

	}

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

}

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#pragma pop_macro("_NOEXCEPT")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#ifndef MSEPOINTERBASICS_H
#undef MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION
#endif /*ndef MSEPOINTERBASICS_H*/

#endif /*ndef MSEPRIMITIVES_H*/
