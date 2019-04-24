// Copyright (C) 2011 - 2012 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// The idea and interface is based on Boost.Optional library
// authored by Fernando Luis Cacciola Carballal

#pragma once
# ifndef MSEOPTIONAL_H_
# define MSEOPTIONAL_H_

#ifndef MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE
#include "msescope.h"
#endif // !MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE
#include "msepointerbasics.h"

#include<utility>
#include<type_traits>
#include<initializer_list>
#include<cassert>
#include<functional>
#include<string>
#include<stdexcept>
#ifdef MSE_HAS_CXX17
#include<optional>
#endif // MSE_HAS_CXX17

#ifdef _MSC_VER
#define MSE_OPTIONAL_IMPLEMENTATION1
#else // _MSC_VER
/* msvc(2017) requires this define, but we'll use it for non-msvc compilers as well for consistency. This may result in some
reduction of functionality/compatibility. */
#define MSE_OPTIONAL_IMPLEMENTATION1
#endif // _MSC_VER


#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4702 4189 )
#endif /*_MSC_VER*/


# define TR2_OPTIONAL_REQUIRES(...) typename std::enable_if<__VA_ARGS__::value, bool>::type = false

# if defined __GNUC__ // NOTE: GNUC is also defined for Clang
#   if (__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)
#     define TR2_OPTIONAL_GCC_4_8_AND_HIGHER___
#   elif (__GNUC__ > 4)
#     define TR2_OPTIONAL_GCC_4_8_AND_HIGHER___
#   endif
#
#   if (__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)
#     define TR2_OPTIONAL_GCC_4_7_AND_HIGHER___
#   elif (__GNUC__ > 4)
#     define TR2_OPTIONAL_GCC_4_7_AND_HIGHER___
#   endif
#
#   if (__GNUC__ == 4) && (__GNUC_MINOR__ == 8) && (__GNUC_PATCHLEVEL__ >= 1)
#     define TR2_OPTIONAL_GCC_4_8_1_AND_HIGHER___
#   elif (__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)
#     define TR2_OPTIONAL_GCC_4_8_1_AND_HIGHER___
#   elif (__GNUC__ > 4)
#     define TR2_OPTIONAL_GCC_4_8_1_AND_HIGHER___
#   endif
# endif
#
# if defined __clang_major__
#   if (__clang_major__ == 3 && __clang_minor__ >= 5)
#     define TR2_OPTIONAL_CLANG_3_5_AND_HIGHTER_
#   elif (__clang_major__ > 3)
#     define TR2_OPTIONAL_CLANG_3_5_AND_HIGHTER_
#   endif
#   if defined TR2_OPTIONAL_CLANG_3_5_AND_HIGHTER_
#     define TR2_OPTIONAL_CLANG_3_4_2_AND_HIGHER_
#   elif (__clang_major__ == 3 && __clang_minor__ == 4 && __clang_patchlevel__ >= 2)
#     define TR2_OPTIONAL_CLANG_3_4_2_AND_HIGHER_
#   endif
# endif
#
# if defined _MSC_VER
#   if (_MSC_VER >= 1900)
#     define TR2_OPTIONAL_MSVC_2015_AND_HIGHER___
#   endif
# endif

# if defined __clang__
#   if (__clang_major__ > 2) || (__clang_major__ == 2) && (__clang_minor__ >= 9)
#     define OPTIONAL_HAS_THIS_RVALUE_REFS 1
#   else
#     define OPTIONAL_HAS_THIS_RVALUE_REFS 0
#   endif
# elif defined TR2_OPTIONAL_GCC_4_8_1_AND_HIGHER___
#   define OPTIONAL_HAS_THIS_RVALUE_REFS 1
# elif defined TR2_OPTIONAL_MSVC_2015_AND_HIGHER___
#   define OPTIONAL_HAS_THIS_RVALUE_REFS 1
# else
#   define OPTIONAL_HAS_THIS_RVALUE_REFS 0
# endif


# if defined TR2_OPTIONAL_GCC_4_8_1_AND_HIGHER___
#   define OPTIONAL_HAS_CONSTEXPR_INIT_LIST 1
#   define OPTIONAL_CONSTEXPR_INIT_LIST constexpr
# else
#   define OPTIONAL_HAS_CONSTEXPR_INIT_LIST 0
#   define OPTIONAL_CONSTEXPR_INIT_LIST
# endif

# if defined TR2_OPTIONAL_CLANG_3_5_AND_HIGHTER_ && (defined __cplusplus) && (__cplusplus != 201103L)
#   define OPTIONAL_HAS_MOVE_ACCESSORS 1
# else
#   define OPTIONAL_HAS_MOVE_ACCESSORS 0
# endif

# // In C++11 constexpr implies const, so we need to make non-const members also non-constexpr
# if (defined __cplusplus) && (__cplusplus == 201103L)
#   define OPTIONAL_MUTABLE_CONSTEXPR
# else
#   define OPTIONAL_MUTABLE_CONSTEXPR constexpr
# endif

namespace mse {

	// workaround: std utility functions aren't constexpr yet
	template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type& t) noexcept
	{
		return static_cast<T&&>(t);
	}

	template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type&& t) noexcept
	{
		static_assert(!std::is_lvalue_reference<T>::value, "!!");
		return static_cast<T&&>(t);
	}

	template <class T> inline constexpr typename std::remove_reference<T>::type&& constexpr_move(T&& t) noexcept
	{
		return static_cast<typename std::remove_reference<T>::type&&>(t);
	}

#ifndef MSE_HAS_CXX17

	namespace us {
		namespace impl {

			// BEGIN workaround for missing std::is_trivially_destructible
# if defined TR2_OPTIONAL_GCC_4_8_AND_HIGHER___
	// leave it: it is already there
# elif defined TR2_OPTIONAL_CLANG_3_4_2_AND_HIGHER_
	// leave it: it is already there
# elif defined TR2_OPTIONAL_MSVC_2015_AND_HIGHER___
	// leave it: it is already there
# elif defined TR2_OPTIONAL_DISABLE_EMULATION_OF_TYPE_TRAITS
	// leave it: the user doesn't want it
# else
			template <typename T>
			using std::is_trivially_destructible = std::has_trivial_destructor<T>;
# endif
			// END workaround for missing std::is_trivially_destructible

# if (defined TR2_OPTIONAL_GCC_4_7_AND_HIGHER___)
	// leave it; our metafunctions are already defined.
# elif defined TR2_OPTIONAL_CLANG_3_4_2_AND_HIGHER_
	// leave it; our metafunctions are already defined.
# elif defined TR2_OPTIONAL_MSVC_2015_AND_HIGHER___
	// leave it: it is already there
# elif defined TR2_OPTIONAL_DISABLE_EMULATION_OF_TYPE_TRAITS
	// leave it: the user doesn't want it
# else


	// workaround for missing traits in GCC and CLANG
			template <class T>
			struct std::is_nothrow_move_constructible
			{
				constexpr static bool value = std::is_nothrow_constructible<T, T&&>::value;
			};


			template <class T, class U>
			struct is_assignable
			{
				template <class X, class Y>
				constexpr static bool has_assign(...) { return false; }

				template <class X, class Y, size_t S = sizeof((std::declval<X>() = std::declval<Y>(), true)) >
				// the comma operator is necessary for the cases where operator= returns void
				constexpr static bool has_assign(bool) { return true; }

				constexpr static bool value = has_assign<T, U>(true);
			};


			template <class T>
			struct std::is_nothrow_move_assignable
			{
				template <class X, bool has_any_move_assign>
				struct has_nothrow_move_assign {
					constexpr static bool value = false;
				};

				template <class X>
				struct has_nothrow_move_assign<X, true> {
					constexpr static bool value = noexcept(std::declval<X&>() = std::declval<X&&>());
				};

				constexpr static bool value = has_nothrow_move_assign<T, is_assignable<T&, T&&>::value>::value;
			};
			// end workaround


# endif



	// 20.5.4, optional for object types
			template <class T> class optional;

			// 20.5.5, optional for lvalue reference types
			template <class T> class optional<T&>;


			/*
			// workaround: std utility functions aren't constexpr yet
			template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type& t) noexcept
			{
				return static_cast<T&&>(t);
			}

			template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type&& t) noexcept
			{
				static_assert(!std::is_lvalue_reference<T>::value, "!!");
				return static_cast<T&&>(t);
			}

			template <class T> inline constexpr typename std::remove_reference<T>::type&& constexpr_move(T&& t) noexcept
			{
				return static_cast<typename std::remove_reference<T>::type&&>(t);
			}
			*/


#if defined NDEBUG
# define TR2_OPTIONAL_ASSERTED_EXPRESSION(CHECK, EXPR) (EXPR)
#else
# define TR2_OPTIONAL_ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : ([]{assert(!#CHECK);}(), (EXPR)))
#endif


			namespace detail_
			{

				// static_addressof: a constexpr version of addressof
				template <typename T>
				struct has_overloaded_addressof
				{
					template <class X>
					constexpr static bool has_overload(...) { return false; }

					template <class X, size_t S = sizeof(std::declval<X&>().operator&()) >
					constexpr static bool has_overload(bool) { return true; }

					constexpr static bool value = has_overload<T>(true);
				};

				template <typename T, TR2_OPTIONAL_REQUIRES(!has_overloaded_addressof<T>)>
				constexpr T* static_addressof(T& ref)
				{
					return &ref;
				}

				template <typename T, TR2_OPTIONAL_REQUIRES(has_overloaded_addressof<T>)>
				T* static_addressof(T& ref)
				{
					return std::addressof(ref);
				}


				// the call to convert<A>(b) has return type A and converts b to type A iff b decltype(b) is implicitly convertible to A  
				template <class U>
				constexpr U convert(U v) { return v; }

			} // namespace detail


			constexpr struct trivial_init_t {} trivial_init{};


			// 20.5.6, In-place construction
			constexpr struct in_place_t {} in_place{};


			// 20.5.7, Disengaged state indicator
			struct nullopt_t
			{
				struct init {};
				constexpr explicit nullopt_t(init) {}
			};
			constexpr nullopt_t nullopt{ nullopt_t::init() };


			// 20.5.8, class bad_optional_access
			class bad_optional_access : public std::logic_error {
			public:
				explicit bad_optional_access(const std::string& what_arg) : std::logic_error{ what_arg } {}
				explicit bad_optional_access(const char* what_arg) : std::logic_error{ what_arg } {}
			};


			template <class T>
			union storage_t
			{
				unsigned char dummy_;
				T value_;

				constexpr storage_t(trivial_init_t) noexcept : dummy_() {};

				template <class... Args>
				constexpr storage_t(Args&&... args) : value_(constexpr_forward<Args>(args)...) {}

				~storage_t() {}
			};


			template <class T>
			union constexpr_storage_t
			{
				unsigned char dummy_;
				T value_;

				constexpr constexpr_storage_t(trivial_init_t) noexcept : dummy_() {};

				template <class... Args>
				constexpr constexpr_storage_t(Args&&... args) : value_(constexpr_forward<Args>(args)...) {}

				~constexpr_storage_t() = default;
			};


			template <class T>
			struct optional_base
			{
				bool init_;
				storage_t<T> storage_;

				constexpr optional_base() noexcept : init_(false), storage_(trivial_init) {};

				explicit constexpr optional_base(const T& v) : init_(true), storage_(v) {}

				explicit constexpr optional_base(T&& v) : init_(true), storage_(constexpr_move(v)) {}

				template <class... Args> explicit optional_base(in_place_t, Args&&... args)
					: init_(true), storage_(constexpr_forward<Args>(args)...) {}

				template <class U, class... Args, TR2_OPTIONAL_REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
				explicit optional_base(in_place_t, std::initializer_list<U> il, Args&&... args)
					: init_(true), storage_(il, std::forward<Args>(args)...) {}

				~optional_base() { if (init_) storage_.value_.T::~T(); }
			};


			template <class T>
			struct constexpr_optional_base
			{
				bool init_;
				constexpr_storage_t<T> storage_;

				constexpr constexpr_optional_base() noexcept : init_(false), storage_(trivial_init) {};

				explicit constexpr constexpr_optional_base(const T& v) : init_(true), storage_(v) {}

				explicit constexpr constexpr_optional_base(T&& v) : init_(true), storage_(constexpr_move(v)) {}

				template <class... Args> explicit constexpr constexpr_optional_base(in_place_t, Args&&... args)
					: init_(true), storage_(constexpr_forward<Args>(args)...) {}

				template <class U, class... Args, TR2_OPTIONAL_REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
				OPTIONAL_CONSTEXPR_INIT_LIST explicit constexpr_optional_base(in_place_t, std::initializer_list<U> il, Args&&... args)
					: init_(true), storage_(il, std::forward<Args>(args)...) {}

				~constexpr_optional_base() = default;
			};

			template <class T>
			using OptionalBase = typename std::conditional<
				std::is_trivially_destructible<T>::value,                          // if possible
				constexpr_optional_base<typename std::remove_const<T>::type>, // use base with trivial destructor
				optional_base<typename std::remove_const<T>::type>
			>::type;



			template <class T>
			class optional : private OptionalBase<T>
			{
				static_assert(!std::is_same<typename std::decay<T>::type, nullopt_t>::value, "bad T");
				static_assert(!std::is_same<typename std::decay<T>::type, in_place_t>::value, "bad T");


				constexpr bool initialized() const noexcept { return OptionalBase<T>::init_; }
				typename std::remove_const<T>::type* dataptr() { return std::addressof(OptionalBase<T>::storage_.value_); }
				constexpr const T* dataptr() const { return detail_::static_addressof(OptionalBase<T>::storage_.value_); }

# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1
				constexpr const T& contained_val() const& { return OptionalBase<T>::storage_.value_; }
#   if OPTIONAL_HAS_MOVE_ACCESSORS == 1
				OPTIONAL_MUTABLE_CONSTEXPR T&& contained_val() && { return std::move(OptionalBase<T>::storage_.value_); }
				OPTIONAL_MUTABLE_CONSTEXPR T& contained_val() & { return OptionalBase<T>::storage_.value_; }
#   else
				T& contained_val() & { return OptionalBase<T>::storage_.value_; }
				T&& contained_val() && { return std::move(OptionalBase<T>::storage_.value_); }
#   endif
# else
				constexpr const T& contained_val() const { return OptionalBase<T>::storage_.value_; }
				T& contained_val() { return OptionalBase<T>::storage_.value_; }
# endif

				void clear() noexcept {
					if (initialized()) dataptr()->T::~T();
					OptionalBase<T>::init_ = false;
				}

				template <class... Args>
				void initialize(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
				{
					assert(!OptionalBase<T>::init_);
					::new (static_cast<void*>(dataptr())) T(std::forward<Args>(args)...);
					OptionalBase<T>::init_ = true;
				}

				template <class U, class... Args>
				void initialize(std::initializer_list<U> il, Args&&... args) noexcept(noexcept(T(il, std::forward<Args>(args)...)))
				{
					assert(!OptionalBase<T>::init_);
					::new (static_cast<void*>(dataptr())) T(il, std::forward<Args>(args)...);
					OptionalBase<T>::init_ = true;
				}

			public:
				typedef T value_type;

				// 20.5.5.1, constructors
				constexpr optional() noexcept : OptionalBase<T>() {};
				constexpr optional(nullopt_t) noexcept : OptionalBase<T>() {};

				optional(const optional& rhs)
					: OptionalBase<T>()
				{
					if (rhs.initialized()) {
						::new (static_cast<void*>(dataptr())) T(*rhs);
						OptionalBase<T>::init_ = true;
					}
				}

				optional(optional&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
					: OptionalBase<T>()
				{
					if (rhs.initialized()) {
						::new (static_cast<void*>(dataptr())) T(std::move(*rhs));
						OptionalBase<T>::init_ = true;
					}
				}

				constexpr optional(const T& v) : OptionalBase<T>(v) {}

				constexpr optional(T&& v) : OptionalBase<T>(constexpr_move(v)) {}

				template <class... Args>
				explicit constexpr optional(in_place_t, Args&&... args)
					: OptionalBase<T>(in_place_t{}, constexpr_forward<Args>(args)...) {}

				template <class U, class... Args, TR2_OPTIONAL_REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
				OPTIONAL_CONSTEXPR_INIT_LIST explicit optional(in_place_t, std::initializer_list<U> il, Args&&... args)
					: OptionalBase<T>(in_place_t{}, il, constexpr_forward<Args>(args)...) {}

				// 20.5.4.2, Destructor
				~optional() = default;

				// 20.5.4.3, assignment
				optional& operator=(nullopt_t) noexcept
				{
					clear();
					return *this;
				}

				optional& operator=(const optional& rhs)
				{
					if (initialized() == true && rhs.initialized() == false) clear();
					else if (initialized() == false && rhs.initialized() == true)  initialize(*rhs);
					else if (initialized() == true && rhs.initialized() == true)  contained_val() = *rhs;
					return *this;
				}

				optional& operator=(optional&& rhs)
					noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)
				{
					if (initialized() == true && rhs.initialized() == false) clear();
					else if (initialized() == false && rhs.initialized() == true)  initialize(std::move(*rhs));
					else if (initialized() == true && rhs.initialized() == true)  contained_val() = std::move(*rhs);
					return *this;
				}

				template <class U>
				auto operator=(U&& v)
					-> typename std::enable_if
					<
					std::is_same<typename std::decay<U>::type, T>::value,
					optional&
					>::type
				{
					if (initialized()) { contained_val() = std::forward<U>(v); }
					else { initialize(std::forward<U>(v)); }
					return *this;
				}


				template <class... Args>
				void emplace(Args&&... args)
				{
					clear();
					initialize(std::forward<Args>(args)...);
				}

				template <class U, class... Args>
				void emplace(std::initializer_list<U> il, Args&&... args)
				{
					clear();
					initialize<U, Args...>(il, std::forward<Args>(args)...);
				}

				// 20.5.4.4, Swap
				void swap(optional<T>& rhs) noexcept(std::is_nothrow_move_constructible<T>::value && noexcept(std::swap(std::declval<T&>(), std::declval<T&>())))
				{
					if (initialized() == true && rhs.initialized() == false) { rhs.initialize(std::move(**this)); clear(); }
					else if (initialized() == false && rhs.initialized() == true) { initialize(std::move(*rhs)); rhs.clear(); }
					else if (initialized() == true && rhs.initialized() == true) { using std::swap; swap(**this, *rhs); }
				}

				// 20.5.4.5, Observers

				explicit constexpr operator bool() const noexcept { return initialized(); }
				constexpr bool has_value() const noexcept { return initialized(); }

				constexpr T const* operator ->() const {
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), dataptr());
				}

# if OPTIONAL_HAS_MOVE_ACCESSORS == 1

				OPTIONAL_MUTABLE_CONSTEXPR T* operator ->() {
					assert(initialized());
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return dataptr();
				}

				constexpr T const& operator *() const& {
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), contained_val());
				}

				OPTIONAL_MUTABLE_CONSTEXPR T& operator *() & {
					assert(initialized());
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return contained_val();
				}

				OPTIONAL_MUTABLE_CONSTEXPR T&& operator *() && {
					assert(initialized());
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return constexpr_move(contained_val());
				}

				constexpr T const& value() const& {
					return initialized() ? contained_val() : (MSE_THROW(bad_optional_access("bad optional access")), contained_val());
				}

				OPTIONAL_MUTABLE_CONSTEXPR T& value() & {
					return initialized() ? contained_val() : (MSE_THROW(bad_optional_access("bad optional access")), contained_val());
				}

				OPTIONAL_MUTABLE_CONSTEXPR T&& value() && {
					if (!initialized()) MSE_THROW(bad_optional_access("bad optional access"));
					return std::move(contained_val());
				}

# else

				T* operator ->() {
					assert(initialized());
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return dataptr();
				}

				constexpr T const& operator *() const {
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return TR2_OPTIONAL_ASSERTED_EXPRESSION(initialized(), contained_val());
				}

				T& operator *() {
					assert(initialized());
					if (!has_value()) { MSE_THROW(bad_optional_access("bad optional access")); }
					return contained_val();
				}

				constexpr T const& value() const {
					return initialized() ? contained_val() : (MSE_THROW(bad_optional_access("bad optional access")), contained_val());
				}

				T& value() {
					return initialized() ? contained_val() : (MSE_THROW(bad_optional_access("bad optional access")), contained_val());
				}

# endif

# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1

				template <class V>
				constexpr T value_or(V&& v) const&
				{
					return *this ? **this : detail_::convert<T>(constexpr_forward<V>(v));
				}

#   if OPTIONAL_HAS_MOVE_ACCESSORS == 1

				template <class V>
				OPTIONAL_MUTABLE_CONSTEXPR T value_or(V&& v) &&
				{
					return *this ? constexpr_move(const_cast<optional<T>&>(*this).contained_val()) : detail_::convert<T>(constexpr_forward<V>(v));
				}

#   else

				template <class V>
				T value_or(V&& v) &&
				{
					return *this ? constexpr_move(const_cast<optional<T>&>(*this).contained_val()) : detail_::convert<T>(constexpr_forward<V>(v));
				}

#   endif

# else

				template <class V>
				constexpr T value_or(V&& v) const
				{
					return *this ? **this : detail_::convert<T>(constexpr_forward<V>(v));
				}

# endif

				// 20.6.3.6, modifiers
				void reset() noexcept { clear(); }
			};


			template <class T>
			class optional<T&>
			{
				static_assert(!std::is_same<T, nullopt_t>::value, "bad T");
				static_assert(!std::is_same<T, in_place_t>::value, "bad T");
				T* ref;

			public:

				// 20.5.5.1, construction/destruction
				constexpr optional() noexcept : ref(nullptr) {}

				constexpr optional(nullopt_t) noexcept : ref(nullptr) {}

				constexpr optional(T& v) noexcept : ref(detail_::static_addressof(v)) {}

				optional(T&&) = delete;

				constexpr optional(const optional& rhs) noexcept : ref(rhs.ref) {}

				explicit constexpr optional(in_place_t, T& v) noexcept : ref(detail_::static_addressof(v)) {}

				explicit optional(in_place_t, T&&) = delete;

				~optional() = default;

				// 20.5.5.2, mutation
				optional& operator=(nullopt_t) noexcept {
					ref = nullptr;
					return *this;
				}

				// optional& operator=(const optional& rhs) noexcept {
				// ref = rhs.ref;
				// return *this;
				// }

				// optional& operator=(optional&& rhs) noexcept {
				// ref = rhs.ref;
				// return *this;
				// }

				template <typename U>
				auto operator=(U&& rhs) noexcept
					-> typename std::enable_if
					<
					std::is_same<typename std::decay<U>::type, optional<T&>>::value,
					optional&
					>::type
				{
					ref = rhs.ref;
					return *this;
				}

				template <typename U>
				auto operator=(U&& rhs) noexcept
					-> typename std::enable_if
					<
					!std::is_same<typename std::decay<U>::type, optional<T&>>::value,
					optional&
					>::type
					= delete;

				void emplace(T& v) noexcept {
					ref = detail_::static_addressof(v);
				}

				void emplace(T&&) = delete;


				void swap(optional<T&>& rhs) noexcept
				{
					std::swap(ref, rhs.ref);
				}

				// 20.5.5.3, observers
				constexpr T* operator->() const {
					return TR2_OPTIONAL_ASSERTED_EXPRESSION(ref, ref);
				}

				constexpr T& operator*() const {
					return TR2_OPTIONAL_ASSERTED_EXPRESSION(ref, *ref);
				}

				constexpr T& value() const {
					return ref ? *ref : (MSE_THROW(bad_optional_access("bad optional access")), *ref);
				}

				explicit constexpr operator bool() const noexcept {
					return ref != nullptr;
				}

				constexpr bool has_value() const noexcept {
					return ref != nullptr;
				}

				template <class V>
				constexpr typename std::decay<T>::type value_or(V&& v) const
				{
					return *this ? **this : detail_::convert<typename std::decay<T>::type>(constexpr_forward<V>(v));
				}

				// x.x.x.x, modifiers
				void reset() noexcept { ref = nullptr; }
			};


			template <class T>
			class optional<T&&>
			{
				static_assert(sizeof(T) == 0, "optional rvalue references disallowed");
			};

#ifdef MSE_HAS_CXX17
			template<class _Ty>
			optional(_Ty)->optional<_Ty>;
#endif /* MSE_HAS_CXX17 */


			// 20.5.8, Relational operators
			template <class T> constexpr bool operator==(const optional<T>& x, const optional<T>& y)
			{
				return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
			}

			template <class T> constexpr bool operator!=(const optional<T>& x, const optional<T>& y)
			{
				return !(x == y);
			}

			template <class T> constexpr bool operator<(const optional<T>& x, const optional<T>& y)
			{
				return (!y) ? false : (!x) ? true : *x < *y;
			}

			template <class T> constexpr bool operator>(const optional<T>& x, const optional<T>& y)
			{
				return (y < x);
			}

			template <class T> constexpr bool operator<=(const optional<T>& x, const optional<T>& y)
			{
				return !(y < x);
			}

			template <class T> constexpr bool operator>=(const optional<T>& x, const optional<T>& y)
			{
				return !(x < y);
			}


			// 20.5.9, Comparison with nullopt
			template <class T> constexpr bool operator==(const optional<T>& x, nullopt_t) noexcept
			{
				return (!x);
			}

			template <class T> constexpr bool operator==(nullopt_t, const optional<T>& x) noexcept
			{
				return (!x);
			}

			template <class T> constexpr bool operator!=(const optional<T>& x, nullopt_t) noexcept
			{
				return bool(x);
			}

			template <class T> constexpr bool operator!=(nullopt_t, const optional<T>& x) noexcept
			{
				return bool(x);
			}

			template <class T> constexpr bool operator<(const optional<T>&, nullopt_t) noexcept
			{
				return false;
			}

			template <class T> constexpr bool operator<(nullopt_t, const optional<T>& x) noexcept
			{
				return bool(x);
			}

			template <class T> constexpr bool operator<=(const optional<T>& x, nullopt_t) noexcept
			{
				return (!x);
			}

			template <class T> constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept
			{
				return true;
			}

			template <class T> constexpr bool operator>(const optional<T>& x, nullopt_t) noexcept
			{
				return bool(x);
			}

			template <class T> constexpr bool operator>(nullopt_t, const optional<T>&) noexcept
			{
				return false;
			}

			template <class T> constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept
			{
				return true;
			}

			template <class T> constexpr bool operator>=(nullopt_t, const optional<T>& x) noexcept
			{
				return (!x);
			}



			// 20.5.10, Comparison with T
			template <class T> constexpr bool operator==(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x == v : false;
			}

			template <class T> constexpr bool operator==(const T& v, const optional<T>& x)
			{
				return bool(x) ? v == *x : false;
			}

			template <class T> constexpr bool operator!=(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x != v : true;
			}

			template <class T> constexpr bool operator!=(const T& v, const optional<T>& x)
			{
				return bool(x) ? v != *x : true;
			}

			template <class T> constexpr bool operator<(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x < v : true;
			}

			template <class T> constexpr bool operator>(const T& v, const optional<T>& x)
			{
				return bool(x) ? v > *x : true;
			}

			template <class T> constexpr bool operator>(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x > v : false;
			}

			template <class T> constexpr bool operator<(const T& v, const optional<T>& x)
			{
				return bool(x) ? v < *x : false;
			}

			template <class T> constexpr bool operator>=(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x >= v : false;
			}

			template <class T> constexpr bool operator<=(const T& v, const optional<T>& x)
			{
				return bool(x) ? v <= *x : false;
			}

			template <class T> constexpr bool operator<=(const optional<T>& x, const T& v)
			{
				return bool(x) ? *x <= v : true;
			}

			template <class T> constexpr bool operator>=(const T& v, const optional<T>& x)
			{
				return bool(x) ? v >= *x : true;
			}


			// Comparison of optional<T&> with T
			template <class T> constexpr bool operator==(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x == v : false;
			}

			template <class T> constexpr bool operator==(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v == *x : false;
			}

			template <class T> constexpr bool operator!=(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x != v : true;
			}

			template <class T> constexpr bool operator!=(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v != *x : true;
			}

			template <class T> constexpr bool operator<(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x < v : true;
			}

			template <class T> constexpr bool operator>(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v > *x : true;
			}

			template <class T> constexpr bool operator>(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x > v : false;
			}

			template <class T> constexpr bool operator<(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v < *x : false;
			}

			template <class T> constexpr bool operator>=(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x >= v : false;
			}

			template <class T> constexpr bool operator<=(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v <= *x : false;
			}

			template <class T> constexpr bool operator<=(const optional<T&>& x, const T& v)
			{
				return bool(x) ? *x <= v : true;
			}

			template <class T> constexpr bool operator>=(const T& v, const optional<T&>& x)
			{
				return bool(x) ? v >= *x : true;
			}

			// Comparison of optional<T const&> with T
			template <class T> constexpr bool operator==(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x == v : false;
			}

			template <class T> constexpr bool operator==(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v == *x : false;
			}

			template <class T> constexpr bool operator!=(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x != v : true;
			}

			template <class T> constexpr bool operator!=(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v != *x : true;
			}

			template <class T> constexpr bool operator<(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x < v : true;
			}

			template <class T> constexpr bool operator>(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v > *x : true;
			}

			template <class T> constexpr bool operator>(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x > v : false;
			}

			template <class T> constexpr bool operator<(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v < *x : false;
			}

			template <class T> constexpr bool operator>=(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x >= v : false;
			}

			template <class T> constexpr bool operator<=(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v <= *x : false;
			}

			template <class T> constexpr bool operator<=(const optional<const T&>& x, const T& v)
			{
				return bool(x) ? *x <= v : true;
			}

			template <class T> constexpr bool operator>=(const T& v, const optional<const T&>& x)
			{
				return bool(x) ? v >= *x : true;
			}


			// 20.5.12, Specialized algorithms
			template <class T>
			void swap(optional<T>& x, optional<T>& y) noexcept(noexcept(x.swap(y)))
			{
				x.swap(y);
			}


			template <class T>
			constexpr optional<typename std::decay<T>::type> make_optional(T&& v)
			{
				return optional<typename std::decay<T>::type>(constexpr_forward<T>(v));
			}

			template <class X>
			constexpr optional<X&> make_optional(std::reference_wrapper<X> v)
			{
				return optional<X&>(v.get());
			}

		}
	}
#endif // !MSE_HAS_CXX17


	namespace impl {
		namespace optional {
#ifdef MSE_HAS_CXX17
			template <class T> using optional_base = std::optional<T>;
			using in_place_t_base = std::in_place_t;
			using nullopt_t_base = std::nullopt_t;
			using bad_optional_access_base = std::bad_optional_access;
#else // MSE_HAS_CXX17
			template <class T> using optional_base = mse::us::impl::optional<T>;
			using in_place_t_base = mse::us::impl::in_place_t;
			using nullopt_t_base = mse::us::impl::nullopt_t;
			using bad_optional_access_base = mse::us::impl::bad_optional_access;
#endif // MSE_HAS_CXX17
		}
	}

	namespace mstd {
		// 20.5.6, In-place construction
		typedef mse::impl::optional::in_place_t_base in_place_t;
		constexpr in_place_t in_place{};
		// 20.5.7, Disengaged state indicator
		typedef typename mse::impl::optional::nullopt_t_base nullopt_t;
#ifdef MSE_HAS_CXX17
		MSE_INLINE_VAR constexpr const auto& nullopt = std::nullopt;
#else // MSE_HAS_CXX17
		constexpr nullopt_t nullopt{ nullopt_t::init() };
#endif // MSE_HAS_CXX17
		// 20.5.8, class bad_optional_access
		typedef typename mse::impl::optional::bad_optional_access_base bad_optional_access;

		template <class T>
		class optional : public mse::impl::optional::optional_base<T> {
		public:
			typedef mse::impl::optional::optional_base<T> base_class;
			typedef typename base_class::value_type value_type;

#ifdef MSE_HAS_CXX17
#ifdef MSE_OPTIONAL_IMPLEMENTATION1

			MSE_USING(optional, base_class);

			template<class... _Types, class = std::enable_if_t<std::is_constructible_v<T, _Types...> > >
			constexpr explicit optional(mse::mstd::in_place_t, _Types&&... _Args) : base_class(mse::mstd::in_place, std::forward<_Types>(_Args)...) {}
			template<class _Elem, class... _Types, class = std::enable_if_t<std::is_constructible_v<T, std::initializer_list<_Elem>&, _Types...> > >
			constexpr explicit optional(mse::mstd::in_place_t, std::initializer_list<_Elem> _Ilist, _Types&&... _Args)
				: base_class(mse::mstd::in_place, _Ilist, std::forward<_Types>(_Args)...) {}

			template<class T2>
			using _AllowDirectConversion = std::integral_constant<bool, std::conjunction_v<
				//std::negation<std::is_same<std::remove_cv_t<std::remove_reference_t<T2>>, optional>>,
				std::negation<std::is_base_of<base_class, std::remove_cv_t<std::remove_reference_t<T2>>>>,
				std::negation<std::is_same<std::remove_cv_t<std::remove_reference_t<T2>>, mse::mstd::in_place_t>>,
				std::is_constructible<T, T2> > >;
			template<class T2 = T, std::enable_if_t<std::conjunction_v<_AllowDirectConversion<T2>, std::is_convertible<T2, T>>, int> = 0>
			constexpr optional(T2&& _Right) : base_class(mse::mstd::in_place, std::forward<T2>(_Right)) {}
			template<class T2 = T, std::enable_if_t<std::conjunction_v<_AllowDirectConversion<T2>, std::negation<std::is_convertible<T2, T> > >, int> = 0>
			constexpr explicit optional(T2&& _Right) : base_class(mse::mstd::in_place, std::forward<T2>(_Right)) {}

#else // MSE_OPTIONAL_IMPLEMENTATION1
			using base_class::base_class;
			template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<optional, T2> >
			explicit optional(T2&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}
#endif // MSE_OPTIONAL_IMPLEMENTATION1

			optional(const base_class& src) : base_class(src) {}
			optional(base_class&& src) : base_class(std::forward<decltype(src)>(src)) {}

#else // MSE_HAS_CXX17
			using base_class::base_class;
			MSE_USING(optional, base_class);
			template<class T2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<optional, T2> >
			explicit optional(T2&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}
#endif // MSE_HAS_CXX17

			optional(const optional& src_ref) : base_class(static_cast<const base_class&>(src_ref)) {}

			~optional() {
#ifndef MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE
				mse::impl::T_valid_if_not_an_xscope_type<T>();
#endif // !MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE
			}

			//using base_class::operator=;

			/* inherit the shareability and passability of the contained type */
			template<class T2 = T, class = typename std::enable_if<(std::is_same<T2, T>::value)
				&& (mse::impl::is_marked_as_shareable_msemsearray<T2>::value), void>::type>
			void async_shareable_tag() const {}
			template<class T2 = T, class = typename std::enable_if<(std::is_same<T2, T>::value)
				&& (mse::impl::is_marked_as_passable_msemsearray<T2>::value), void>::type>
			void async_passable_tag() const {} 
		};

#ifdef MSE_HAS_CXX17
		template<class _Ty>
		optional(_Ty)->optional<_Ty>;
#endif /* MSE_HAS_CXX17 */

		// 20.5.8, Relational operators
		template <class T> constexpr bool operator==(const optional<T>& x, const optional<T>& y)
		{
			return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
		}

		template <class T> constexpr bool operator!=(const optional<T>& x, const optional<T>& y)
		{
			return !(x == y);
		}

		template <class T> constexpr bool operator<(const optional<T>& x, const optional<T>& y)
		{
			return (!y) ? false : (!x) ? true : *x < *y;
		}

		template <class T> constexpr bool operator>(const optional<T>& x, const optional<T>& y)
		{
			return (y < x);
		}

		template <class T> constexpr bool operator<=(const optional<T>& x, const optional<T>& y)
		{
			return !(y < x);
		}

		template <class T> constexpr bool operator>=(const optional<T>& x, const optional<T>& y)
		{
			return !(x < y);
		}


		// 20.5.9, Comparison with nullopt
		template <class T> constexpr bool operator==(const optional<T>& x, nullopt_t) noexcept
		{
			return (!x);
		}

		template <class T> constexpr bool operator==(nullopt_t, const optional<T>& x) noexcept
		{
			return (!x);
		}

		template <class T> constexpr bool operator!=(const optional<T>& x, nullopt_t) noexcept
		{
			return bool(x);
		}

		template <class T> constexpr bool operator!=(nullopt_t, const optional<T>& x) noexcept
		{
			return bool(x);
		}

		template <class T> constexpr bool operator<(const optional<T>&, nullopt_t) noexcept
		{
			return false;
		}

		template <class T> constexpr bool operator<(nullopt_t, const optional<T>& x) noexcept
		{
			return bool(x);
		}

		template <class T> constexpr bool operator<=(const optional<T>& x, nullopt_t) noexcept
		{
			return (!x);
		}

		template <class T> constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept
		{
			return true;
		}

		template <class T> constexpr bool operator>(const optional<T>& x, nullopt_t) noexcept
		{
			return bool(x);
		}

		template <class T> constexpr bool operator>(nullopt_t, const optional<T>&) noexcept
		{
			return false;
		}

		template <class T> constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept
		{
			return true;
		}

		template <class T> constexpr bool operator>=(nullopt_t, const optional<T>& x) noexcept
		{
			return (!x);
		}


		// 20.5.10, Comparison with T
		template <class T> constexpr bool operator==(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x == v : false;
		}

		template <class T> constexpr bool operator==(const T& v, const optional<T>& x)
		{
			return bool(x) ? v == *x : false;
		}

		template <class T> constexpr bool operator!=(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x != v : true;
		}

		template <class T> constexpr bool operator!=(const T& v, const optional<T>& x)
		{
			return bool(x) ? v != *x : true;
		}

		template <class T> constexpr bool operator<(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x < v : true;
		}

		template <class T> constexpr bool operator>(const T& v, const optional<T>& x)
		{
			return bool(x) ? v > *x : true;
		}

		template <class T> constexpr bool operator>(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x > v : false;
		}

		template <class T> constexpr bool operator<(const T& v, const optional<T>& x)
		{
			return bool(x) ? v < *x : false;
		}

		template <class T> constexpr bool operator>=(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x >= v : false;
		}

		template <class T> constexpr bool operator<=(const T& v, const optional<T>& x)
		{
			return bool(x) ? v <= *x : false;
		}

		template <class T> constexpr bool operator<=(const optional<T>& x, const T& v)
		{
			return bool(x) ? *x <= v : true;
		}

		template <class T> constexpr bool operator>=(const T& v, const optional<T>& x)
		{
			return bool(x) ? v >= *x : true;
		}


		// Comparison of optional<T&> with T
		template <class T> constexpr bool operator==(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x == v : false;
		}

		template <class T> constexpr bool operator==(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v == *x : false;
		}

		template <class T> constexpr bool operator!=(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x != v : true;
		}

		template <class T> constexpr bool operator!=(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v != *x : true;
		}

		template <class T> constexpr bool operator<(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x < v : true;
		}

		template <class T> constexpr bool operator>(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v > *x : true;
		}

		template <class T> constexpr bool operator>(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x > v : false;
		}

		template <class T> constexpr bool operator<(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v < *x : false;
		}

		template <class T> constexpr bool operator>=(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x >= v : false;
		}

		template <class T> constexpr bool operator<=(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v <= *x : false;
		}

		template <class T> constexpr bool operator<=(const optional<T&>& x, const T& v)
		{
			return bool(x) ? *x <= v : true;
		}

		template <class T> constexpr bool operator>=(const T& v, const optional<T&>& x)
		{
			return bool(x) ? v >= *x : true;
		}

		// Comparison of optional<T const&> with T
		template <class T> constexpr bool operator==(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x == v : false;
		}

		template <class T> constexpr bool operator==(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v == *x : false;
		}

		template <class T> constexpr bool operator!=(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x != v : true;
		}

		template <class T> constexpr bool operator!=(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v != *x : true;
		}

		template <class T> constexpr bool operator<(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x < v : true;
		}

		template <class T> constexpr bool operator>(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v > *x : true;
		}

		template <class T> constexpr bool operator>(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x > v : false;
		}

		template <class T> constexpr bool operator<(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v < *x : false;
		}

		template <class T> constexpr bool operator>=(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x >= v : false;
		}

		template <class T> constexpr bool operator<=(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v <= *x : false;
		}

		template <class T> constexpr bool operator<=(const optional<const T&>& x, const T& v)
		{
			return bool(x) ? *x <= v : true;
		}

		template <class T> constexpr bool operator>=(const T& v, const optional<const T&>& x)
		{
			return bool(x) ? v >= *x : true;
		}


		// 20.5.12, Specialized algorithms
		template <class T>
		void swap(optional<T>& x, optional<T>& y) noexcept(noexcept(x.swap(y)))
		{
			x.swap(y);
		}


		template <class T>
		constexpr optional<typename std::decay<T>::type> make_optional(T&& v)
		{
			return optional<typename std::decay<T>::type>(constexpr_forward<T>(v));
		}

		template <class X>
		constexpr optional<X&> make_optional(std::reference_wrapper<X> v)
		{
			return optional<X&>(v.get());
		}
	}

	// 20.5.6, In-place construction
	typedef mstd::in_place_t in_place_t;
	constexpr in_place_t in_place{};
	// 20.5.7, Disengaged state indicator
	typedef mstd::nullopt_t nullopt_t;
#ifdef MSE_HAS_CXX17
	MSE_INLINE_VAR constexpr const auto& nullopt = std::nullopt;
#else // MSE_HAS_CXX17
	constexpr nullopt_t nullopt{ nullopt_t::init() };
#endif // MSE_HAS_CXX17
	// 20.5.8, class bad_optional_access
	typedef mstd::bad_optional_access bad_optional_access;

	template <class T>
	using optional = mstd::optional<T>;


#ifndef MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE

	template <class _Ty>
	class xscope_optional : public mse::impl::optional::optional_base<_Ty>, public mse::us::impl::XScopeTagBase
		, MSE_INHERIT_XSCOPE_TAG_BASE_SET_FROM(_Ty, xscope_optional<_Ty>)
	{
	public:
		typedef mse::impl::optional::optional_base<_Ty> base_class;
		typedef typename base_class::value_type value_type;

#ifdef MSE_HAS_CXX17
#ifdef MSE_OPTIONAL_IMPLEMENTATION1

		MSE_USING(xscope_optional, base_class);

		template<class... _Types, class = std::enable_if_t<std::is_constructible_v<_Ty, _Types...> > >
		constexpr explicit xscope_optional(mse::in_place_t, _Types&&... _Args) : base_class(mse::in_place, std::forward<_Types>(_Args)...) {}
		template<class _Elem, class... _Types, class = std::enable_if_t<std::is_constructible_v<_Ty, std::initializer_list<_Elem>&, _Types...> > >
		constexpr explicit xscope_optional(mse::in_place_t, std::initializer_list<_Elem> _Ilist, _Types&&... _Args)
			: base_class(mse::in_place, _Ilist, std::forward<_Types>(_Args)...) {}

		template<class _Ty2>
		using _AllowDirectConversion = std::integral_constant<bool, std::conjunction_v<
			//std::negation<std::is_same<std::remove_cv_t<std::remove_reference_t<_Ty2>>, xscope_optional>>,
			std::negation<std::is_base_of<base_class, std::remove_cv_t<std::remove_reference_t<_Ty2>>>>,
			std::negation<std::is_same<std::remove_cv_t<std::remove_reference_t<_Ty2>>, mse::in_place_t>>,
			std::is_constructible<_Ty, _Ty2> > >;
		template<class _Ty2 = _Ty, std::enable_if_t<std::conjunction_v<_AllowDirectConversion<_Ty2>, std::is_convertible<_Ty2, _Ty>>, int> = 0>
		constexpr xscope_optional(_Ty2&& _Right) : base_class(mse::in_place, std::forward<_Ty2>(_Right)) {}
		template<class _Ty2 = _Ty, std::enable_if_t<std::conjunction_v<_AllowDirectConversion<_Ty2>, std::negation<std::is_convertible<_Ty2, _Ty> > >, int> = 0>
		constexpr explicit xscope_optional(_Ty2&& _Right) : base_class(mse::in_place, std::forward<_Ty2>(_Right)) {}

#else // MSE_OPTIONAL_IMPLEMENTATION1
		using base_class::base_class;
		template<class _Ty2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<xscope_optional, _Ty2> >
		explicit xscope_optional(_Ty2&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}
#endif // MSE_OPTIONAL_IMPLEMENTATION1

		xscope_optional(const base_class& src) : base_class(src) {}
		xscope_optional(base_class&& src) : base_class(std::forward<decltype(src)>(src)) {}

#else // MSE_HAS_CXX17
		using base_class::base_class;
		MSE_USING(xscope_optional, base_class);
		template<class _Ty2, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<xscope_optional, _Ty2> >
		explicit xscope_optional(_Ty2&& _X) : base_class(std::forward<decltype(_X)>(_X)) {}
#endif // MSE_HAS_CXX17

		xscope_optional(const xscope_optional& src_ref) : base_class(static_cast<const base_class&>(src_ref)) {}
		xscope_optional(const mstd::optional<_Ty>& src_ref) : base_class(static_cast<const base_class&>(src_ref)) {}

		xscope_optional& operator=(nullopt_t) noexcept {
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::clear();
			return *this;
		}
		xscope_optional& operator=(const xscope_optional& rhs) {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::operator=(rhs);
			return *this;
		}
		xscope_optional& operator=(xscope_optional&& rhs) noexcept(std::is_nothrow_move_assignable<_Ty>::value && std::is_nothrow_move_constructible<_Ty>::value) {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::operator=(std::forward<base_class>(rhs));
			return *this;
		}
		template <class U, class = mse::impl::disable_if_is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<xscope_optional, U> >
		auto operator=(U&& v) -> typename std::enable_if<std::is_same<typename std::decay<U>::type, _Ty>::value, xscope_optional&>::type {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::operator=(std::forward<U>(v));
			return *this;
		}
		template <class... Args>
		void emplace(Args&&... args) {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::emplace(std::forward<Args>(args)...);
		}
		template <class U, class... Args>
		void emplace(std::initializer_list<U> il, Args&&... args) {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::emplace(il, std::forward<Args>(args)...);
		}
		void reset() noexcept {
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::reset();
		}
		void swap(xscope_optional<_Ty>& rhs) noexcept(std::is_nothrow_move_constructible<_Ty>::value && noexcept(std::swap(std::declval<_Ty&>(), std::declval<_Ty&>()))) {
			valid_if_Ty_is_not_marked_as_unreturnable<_Ty>();
			valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator<_Ty>();
			base_class::swap(rhs);
		}

		/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (
			(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<_Ty2>::Has>()) || (!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty2>::value)
			), void>::type>
		void xscope_returnable_tag() const {} /* Indication that this type is can be used as a function return value. */

		/* inherit the shareability and passability of the contained type */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
			&& (mse::impl::is_marked_as_xscope_shareable_msemsearray<_Ty2>::value), void>::type>
		void xscope_async_shareable_tag() const {}
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
			&& (mse::impl::is_marked_as_xscope_passable_msemsearray<_Ty2>::value), void>::type>
		void xscope_async_passable_tag() const {}

	private:
		/* If _Ty is "marked" as not safe to use as a function return value, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class _Ty2, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
			&& (!std::is_base_of<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_marked_as_unreturnable() const {}

		/* If _Ty is "marked" as containing an accessible "scope address of" operator, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class _Ty2, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
			&& (!std::is_base_of<mse::us::impl::ReferenceableByScopePointerTagBase, _Ty2>::value)
			, void>::type>
		void valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_address_of_operator() const {}

		MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
	};

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _Ty>
	xscope_optional(_Ty)->xscope_optional<_Ty>;
#endif /* MSE_HAS_CXX17 */

	// 20.5.8, Relational operators
	template <class T> constexpr bool operator==(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
	}

	template <class T> constexpr bool operator!=(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return !(x == y);
	}

	template <class T> constexpr bool operator<(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return (!y) ? false : (!x) ? true : *x < *y;
	}

	template <class T> constexpr bool operator>(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return (y < x);
	}

	template <class T> constexpr bool operator<=(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return !(y < x);
	}

	template <class T> constexpr bool operator>=(const xscope_optional<T>& x, const xscope_optional<T>& y)
	{
		return !(x < y);
	}


	// 20.5.9, Comparison with nullopt
	template <class T> constexpr bool operator==(const xscope_optional<T>& x, nullopt_t) noexcept
	{
		return (!x);
	}

	template <class T> constexpr bool operator==(nullopt_t, const xscope_optional<T>& x) noexcept
	{
		return (!x);
	}

	template <class T> constexpr bool operator!=(const xscope_optional<T>& x, nullopt_t) noexcept
	{
		return bool(x);
	}

	template <class T> constexpr bool operator!=(nullopt_t, const xscope_optional<T>& x) noexcept
	{
		return bool(x);
	}

	template <class T> constexpr bool operator<(const xscope_optional<T>&, nullopt_t) noexcept
	{
		return false;
	}

	template <class T> constexpr bool operator<(nullopt_t, const xscope_optional<T>& x) noexcept
	{
		return bool(x);
	}

	template <class T> constexpr bool operator<=(const xscope_optional<T>& x, nullopt_t) noexcept
	{
		return (!x);
	}

	template <class T> constexpr bool operator<=(nullopt_t, const xscope_optional<T>&) noexcept
	{
		return true;
	}

	template <class T> constexpr bool operator>(const xscope_optional<T>& x, nullopt_t) noexcept
	{
		return bool(x);
	}

	template <class T> constexpr bool operator>(nullopt_t, const xscope_optional<T>&) noexcept
	{
		return false;
	}

	template <class T> constexpr bool operator>=(const xscope_optional<T>&, nullopt_t) noexcept
	{
		return true;
	}

	template <class T> constexpr bool operator>=(nullopt_t, const xscope_optional<T>& x) noexcept
	{
		return (!x);
	}



	// 20.5.10, Comparison with T
	template <class T> constexpr bool operator==(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x == v : false;
	}

	template <class T> constexpr bool operator==(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v == *x : false;
	}

	template <class T> constexpr bool operator!=(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x != v : true;
	}

	template <class T> constexpr bool operator!=(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v != *x : true;
	}

	template <class T> constexpr bool operator<(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x < v : true;
	}

	template <class T> constexpr bool operator>(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v > *x : true;
	}

	template <class T> constexpr bool operator>(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x > v : false;
	}

	template <class T> constexpr bool operator<(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v < *x : false;
	}

	template <class T> constexpr bool operator>=(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x >= v : false;
	}

	template <class T> constexpr bool operator<=(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v <= *x : false;
	}

	template <class T> constexpr bool operator<=(const xscope_optional<T>& x, const T& v)
	{
		return bool(x) ? *x <= v : true;
	}

	template <class T> constexpr bool operator>=(const T& v, const xscope_optional<T>& x)
	{
		return bool(x) ? v >= *x : true;
	}


	// Comparison of xscope_optional<T&> with T
	template <class T> constexpr bool operator==(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x == v : false;
	}

	template <class T> constexpr bool operator==(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v == *x : false;
	}

	template <class T> constexpr bool operator!=(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x != v : true;
	}

	template <class T> constexpr bool operator!=(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v != *x : true;
	}

	template <class T> constexpr bool operator<(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x < v : true;
	}

	template <class T> constexpr bool operator>(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v > *x : true;
	}

	template <class T> constexpr bool operator>(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x > v : false;
	}

	template <class T> constexpr bool operator<(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v < *x : false;
	}

	template <class T> constexpr bool operator>=(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x >= v : false;
	}

	template <class T> constexpr bool operator<=(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v <= *x : false;
	}

	template <class T> constexpr bool operator<=(const xscope_optional<T&>& x, const T& v)
	{
		return bool(x) ? *x <= v : true;
	}

	template <class T> constexpr bool operator>=(const T& v, const xscope_optional<T&>& x)
	{
		return bool(x) ? v >= *x : true;
	}

	// Comparison of xscope_optional<T const&> with T
	template <class T> constexpr bool operator==(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x == v : false;
	}

	template <class T> constexpr bool operator==(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v == *x : false;
	}

	template <class T> constexpr bool operator!=(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x != v : true;
	}

	template <class T> constexpr bool operator!=(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v != *x : true;
	}

	template <class T> constexpr bool operator<(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x < v : true;
	}

	template <class T> constexpr bool operator>(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v > *x : true;
	}

	template <class T> constexpr bool operator>(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x > v : false;
	}

	template <class T> constexpr bool operator<(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v < *x : false;
	}

	template <class T> constexpr bool operator>=(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x >= v : false;
	}

	template <class T> constexpr bool operator<=(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v <= *x : false;
	}

	template <class T> constexpr bool operator<=(const xscope_optional<const T&>& x, const T& v)
	{
		return bool(x) ? *x <= v : true;
	}

	template <class T> constexpr bool operator>=(const T& v, const xscope_optional<const T&>& x)
	{
		return bool(x) ? v >= *x : true;
	}


	// 20.5.12, Specialized algorithms
	template <class T>
	void swap(xscope_optional<T>& x, xscope_optional<T>& y) noexcept(noexcept(x.swap(y)))
	{
		x.swap(y);
	}


	template <class T>
	constexpr xscope_optional<typename std::decay<T>::type> make_xscope_optional(T&& v)
	{
		return xscope_optional<typename std::decay<T>::type>(constexpr_forward<T>(v));
	}

	template <class X>
	constexpr xscope_optional<X&> make_xscope_optional(std::reference_wrapper<X> v)
	{
		return xscope_optional<X&>(v.get());
	}
#endif // !MSE_OPTIONAL_NO_XSCOPE_DEPENDENCE

}

namespace std
{
#ifndef MSE_HAS_CXX17
	template <typename T>
	struct hash<mse::impl::optional::optional_base<T>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::impl::optional::optional_base<T> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};

	template <typename T>
	struct hash<mse::impl::optional::optional_base<T&>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::impl::optional::optional_base<T&> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};
#endif // !MSE_HAS_CXX17

	template <typename T>
	struct hash<mse::mstd::optional<T>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::mstd::optional<T> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};

	template <typename T>
	struct hash<mse::mstd::optional<T&>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::mstd::optional<T&> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};

	template <typename T>
	struct hash<mse::xscope_optional<T>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::xscope_optional<T> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};

	template <typename T>
	struct hash<mse::xscope_optional<T&>>
	{
		typedef typename hash<T>::result_type result_type;
		typedef mse::xscope_optional<T&> argument_type;

		constexpr result_type operator()(argument_type const& arg) const {
			return arg ? std::hash<T>{}(*arg) : result_type{};
		}
	};
}

# undef TR2_OPTIONAL_REQUIRES
# undef TR2_OPTIONAL_ASSERTED_EXPRESSION

namespace mse {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	namespace self_test {
		class COptionalTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
				{
					// from http://en.cppreference.com/w/cpp/utility/optional
					class CB {
					public:
						static mse::mstd::optional<std::string> create(bool b) {
							if (b)
								return std::string("Godzilla");
							return {};
						}

						// std::nullopt can be used to create any (empty) std::optional
						static auto create2(bool b) {
							return b ? mse::mstd::optional<std::string>{"Godzilla"} : mse::nullopt;
						}
					};

					std::cout << "create(false) returned "
						<< CB::create(false).value_or("empty") << '\n';

					// optional-returning factory functions are usable as conditions of while and if
					if (auto str = CB::create2(true)) {
						std::cout << "create2(true) returned " << *str << '\n';
					}

					auto o1 = CB::create2(true);
					auto o2 = o1;
					o1 = o2;
					o1.swap(o2);
					std::swap(o1, o2);

					auto ui_o1 = mse::impl::optional::optional_base<std::string>{ "Mothra" };
					auto ui_o2 = ui_o1;
					ui_o1 = ui_o2;
					ui_o1.swap(ui_o2);
					std::swap(ui_o1, ui_o2);
				}
				{
					mse::mstd::optional<int> o1, // empty
						o2 = 1, // init from rvalue
						o3 = o2; // copy-constructor

								 // calls std::string( initializer_list<CharT> ) constructor
					mse::mstd::optional<std::string> o4(mse::mstd::in_place, { 'a', 'b', 'c' });

					// calls std::string( size_type count, CharT ch ) constructor
					mse::mstd::optional<std::string> o5(mse::mstd::in_place, 3, 'A');

					// Move-constructed from std::string using deduction guide to pick the type

					mse::mstd::optional<std::string> o6(std::string{ "deduction" });

					std::cout << *o2 << ' ' << *o3 << ' ' << *o4 << ' ' << *o5 << ' ' << *o6 << '\n';
				}
				{
					mse::mstd::optional<const char*> s1 = "abc", s2; // constructor
					s2 = s1; // assignment
					s1 = "def"; // decaying assignment (U = char[4], T = const char*)
					std::cout << *s2 << ' ' << *s1 << '\n';
				}
				{
					using namespace std::string_literals;

					mse::mstd::optional<int> opt1 = 1;
					std::cout << "opt1: " << *opt1 << '\n';

					*opt1 = 2;
					std::cout << "opt1: " << *opt1 << '\n';

					mse::mstd::optional<std::string> opt2 = "abc"s;
					std::cout << "opt2: " << *opt2 << " size: " << opt2->size() << '\n';

					// You can "take" the contained value by calling operator* on a rvalue to optional

					auto taken = *std::move(opt2);
					std::cout << "taken: " << taken << " opt2: " << *opt2 << "size: " << opt2->size() << '\n';
				}
				{
					mse::mstd::optional<int> opt = {};

					try {
						int n = opt.value();
					}
					catch (const std::exception& e) {
						std::cout << e.what() << '\n';
					}
				}
				{
					class CB {
					public:
						static mse::mstd::optional<const char*> maybe_getenv(const char* n)
						{
#ifdef _MSC_VER
							char *x;
							size_t len;
							errno_t err = _dupenv_s(&x, &len, n);
							if ((!err) && (1 <= len))
#else /*_MSC_VER*/
							if (const char* x = std::getenv(n))
#endif /*_MSC_VER*/
								return x;
							else
								return {};
						}
					};
					std::cout << CB::maybe_getenv("MYPWD").value_or("(none)") << '\n';
				}

				{
					mse::xscope_optional<int> o1, // empty
						o2(1), // init from rvalue
						o3 = o2; // copy-constructor

								 // calls std::string( initializer_list<CharT> ) constructor
					mse::xscope_optional<std::string> o4(mse::in_place, { 'a', 'b', 'c' });

					// calls std::string( size_type count, CharT ch ) constructor
					mse::xscope_optional<std::string> o5(mse::in_place, 3, 'A');

					// Move-constructed from std::string using deduction guide to pick the type

					mse::xscope_optional<std::string> o6(std::string{ "deduction" });

					std::cout << *o2 << ' ' << *o3 << ' ' << *o4 << ' ' << *o5 << ' ' << *o6 << '\n';
				}
				{
					mse::xscope_optional<const char*> s1("abc"), s2; // constructor
					s2 = s1; // assignment
					s1 = "def"; // decaying assignment (U = char[4], T = const char*)
					std::cout << *s2 << ' ' << *s1 << '\n';
				}
				{
					using namespace std::string_literals;

					mse::xscope_optional<int> opt1(1);
					std::cout << "opt1: " << *opt1 << '\n';

					*opt1 = 2;
					std::cout << "opt1: " << *opt1 << '\n';

					mse::xscope_optional<std::string> opt2("abc"s);
					std::cout << "opt2: " << *opt2 << " size: " << opt2->size() << '\n';

					// You can "take" the contained value by calling operator* on a rvalue to optional

					auto taken = *std::move(opt2);
					std::cout << "taken: " << taken << " opt2: " << *opt2 << "size: " << opt2->size() << '\n';
				}
				{
					mse::xscope_optional<int> opt = {};

					try {
						int n = opt.value();
					}
					catch (const std::exception& e) {
						std::cout << e.what() << '\n';
					}
				}

#endif // MSE_SELF_TESTS
			}

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/
		};
	}
} // namespace mse

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

# endif //MSEOPTIONAL_H_
