
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEPOINTERBASICS_H
#define MSEPOINTERBASICS_H

#include <assert.h>
#include <climits>       // ULONG_MAX
#include <limits>       // std::numeric_limits
#include <stdexcept>      // primitives_range_error
#include <memory>
#include <unordered_set>

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
#define MSE_POINTERBASICS_DISABLED
#define MSE_SAFERPTR_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/


/* start of scope pointer defines */

/* This is done here rather than in msescope.h because some elements in this file have to know whether or not
MSE_SCOPEPOINTER_DISABLED will ultimately be defined. */

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_SCOPEPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

/* MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED is deprecated */
#ifdef MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED
#define MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // MSE_SCOPEPOINTER_USE_RELAXED_REGISTERED

/* By default, norad pointers are used to catch unsafe misuse of scope pointers in debug mode. Defining
MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED will cause them to be used in non-debug modes as well. */
#if (!defined(NDEBUG)) && (!defined(MSE_SCOPEPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))
#define MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // (!defined(NDEBUG)) && (!defined(MSE_SCOPEPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))

#ifdef MSE_SCOPEPOINTER_DISABLED
#undef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // MSE_SCOPEPOINTER_DISABLED

#ifdef MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#undef MSE_SAFERPTR_DISABLED
#else // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED
#ifdef NDEBUG
/* By default we make scope pointers simply an alias for native pointers in non-debug builds. */
#define MSE_SCOPEPOINTER_DISABLED
#endif // NDEBUG
#endif // MSE_SCOPEPOINTER_RUNTIME_CHECKS_ENABLED

/* end of scope pointer defines */

/* start of thread_local pointer defines */

/* This is done here rather than in msethreadlocal.h because some elements in this file have to know whether or not
MSE_THREADLOCALPOINTER_DISABLED will ultimately be defined. */

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_THREADLOCALPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

/* By default, norad pointers are used to catch unsafe misuse of thread_local pointers in debug mode. Defining
MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED will cause them to be used in non-debug modes as well. */
#if (!defined(NDEBUG)) && (!defined(MSE_THREADLOCALPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))
#define MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED
#endif // (!defined(NDEBUG)) && (!defined(MSE_THREADLOCALPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))

#ifdef MSE_THREADLOCALPOINTER_DISABLED
#undef MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED
#endif // MSE_THREADLOCALPOINTER_DISABLED

#ifdef MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED
#undef MSE_SAFERPTR_DISABLED
#else // MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED
#ifdef NDEBUG
/* By default we make thread_local pointers simply an alias for native pointers in non-debug builds. */
#define MSE_THREADLOCALPOINTER_DISABLED
#endif // NDEBUG
#endif // MSE_THREADLOCALPOINTER_RUNTIME_CHECKS_ENABLED

/* end of thread_local pointer defines */

/* start of static pointer defines */

/* This is done here rather than in msethreadlocal.h because some elements in this file have to know whether or not
MSE_STATICPOINTER_DISABLED will ultimately be defined. */

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)
#define MSE_STATICPOINTER_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED)*/

#if (!defined(NDEBUG)) && (!defined(MSE_STATICPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))
#define MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // (!defined(NDEBUG)) && (!defined(MSE_STATICPOINTER_DEBUG_RUNTIME_CHECKS_DISABLED))

#ifdef MSE_STATICPOINTER_DISABLED
#undef MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED
#endif // MSE_STATICPOINTER_DISABLED

#ifdef MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED
#undef MSE_SAFERPTR_DISABLED
#else // MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED
#ifdef NDEBUG
/* By default we make static pointers simply an alias for native pointers in non-debug builds. */
#define MSE_STATICPOINTER_DISABLED
#endif // NDEBUG
#endif // MSE_STATICIMMUTABLEPOINTER_RUNTIME_CHECKS_ENABLED

/* end of static pointer defines */


#if defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR
#else // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)
#define MSE_CONSTEXPR constexpr
#endif // defined(MSVC2013_COMPATIBLE) || defined(MSVC2010_COMPATIBLE)

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4505 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#endif /*__GNUC__*/
#endif /*__clang__*/

namespace mse {

	class primitives_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#define MSE_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

	/* This macro roughly simulates constructor inheritance. */
#define MSE_USING_V1(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	namespace impl {
		template<bool _Val>
		struct Cat_base_msepointerbasics : std::integral_constant<bool, _Val> {	// base class for type predicates
		};
	}
	template<class _Ty, class... _Args>
	struct is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics : impl::Cat_base_msepointerbasics<false> {};
	template<class _Ty, class _Tz>
	struct is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<_Ty, _Tz> : impl::Cat_base_msepointerbasics<std::is_base_of<typename std::remove_reference<_Ty>::type, typename std::remove_reference<_Tz>::type>::value> {};
	template<class _Ty>
	struct is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<_Ty> : impl::Cat_base_msepointerbasics<false> {};

	/* This macro roughly simulates constructor inheritance. */
#define MSE_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if< \
	std::is_constructible<Base, Args...>::value \
	&& !is_a_pair_with_the_first_a_base_of_the_second_msepointerbasics<Derived, Args...>::value \
	>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

	namespace impl {
		template<class T, class EqualTo>
		struct HasOrInheritsAssignmentOperator_msepointerbasics_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype(std::declval<U>() = std::declval<V>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct HasOrInheritsAssignmentOperator_msepointerbasics : HasOrInheritsAssignmentOperator_msepointerbasics_impl<
			typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};
	}

#define MSE_USING_ASSIGNMENT_OPERATOR(Base) \
	template<class _Ty2mse_uao, class _TBase2 = Base, typename = typename std::enable_if<mse::impl::HasOrInheritsAssignmentOperator_msepointerbasics<_TBase2>::value>::type> \
	auto& operator=(_Ty2mse_uao&& _X) { Base::operator=(std::forward<decltype(_X)>(_X)); return (*this); } \
	template<class _Ty2mse_uao, class _TBase2 = Base, typename = typename std::enable_if<mse::impl::HasOrInheritsAssignmentOperator_msepointerbasics<_TBase2>::value>::type> \
	auto& operator=(const _Ty2mse_uao& _X) { Base::operator=(_X); return (*this); }

#define MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(ClassName) \
	ClassName(const ClassName&) = default; \
	ClassName(ClassName&&) = default;

#define MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Derived, Base) MSE_USING(Derived, Base) MSE_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Derived)
#define MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS_AND_USING_ASSIGNMENT_OPERATOR(Derived, Base) \
	MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Derived, Base) MSE_USING_ASSIGNMENT_OPERATOR(Base)

#if defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_NORADPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED) || defined(MSE_THREADLOCALPOINTER_DISABLED) || defined(MSE_STATICPOINTER_DISABLED)
#define MSE_SOME_POINTER_TYPE_IS_DISABLED
#endif /*defined(MSE_REGISTEREDPOINTER_DISABLED) || defined(MSE_NORADPOINTER_DISABLED) || defined(MSE_SCOPEPOINTER_DISABLED) || defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_SAFERPTR_DISABLED) || defined(MSE_THREADLOCALPOINTER_DISABLED) || defined(MSE_STATICPOINTER_DISABLED)*/

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
#define MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION
#else /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/
#define MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION	auto operator&() { return this; } auto operator&() const { return this; }
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

#define MSE_DEFAULT_OPERATOR_NEW_DECLARATION	void* operator new(size_t size) { return ::operator new(size); }
#define MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION	MSE_DEFAULT_OPERATOR_NEW_DECLARATION MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION

#define MSE_USING_ASSIGNMENT_OPERATOR_AND_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION(Base)	\
	MSE_USING_ASSIGNMENT_OPERATOR(Base) MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION


	namespace us {
		namespace impl {
	class XScopeTagBase { public: void xscope_tag() const {} };
		}
	}

	namespace impl {
		/* The purpose of these template functions are just to produce a compile error on attempts to instantiate
		when certain conditions are not met. */
		template<class _Ty, class = typename std::enable_if<(!std::is_base_of<mse::us::impl::XScopeTagBase, _Ty>::value), void>::type>
		void T_valid_if_not_an_xscope_type() {}

		template<class _Ty>
		void T_valid_if_not_an_xscope_type(const _Ty&) {
			T_valid_if_not_an_xscope_type<_Ty>();
		}
	}

	namespace us {
		namespace impl {
			class NotAsyncShareableTagBase {};
			class NotAsyncPassableTagBase {};
		}
	}

	namespace impl {
		template<typename _Ty>
		class TPlaceHolder_msepointerbasics {};
		template<typename _Ty>
		class TPlaceHolder2_msepointerbasics {};
	}

	namespace us {
		namespace impl {
			template<typename _Ty>
			class TPointerID {};

			/* TPointer is just a wrapper for native pointers that can act as a base class. */
			template<typename _Ty, typename _TID = TPointerID<_Ty>>
			class TPointer : public mse::us::impl::NotAsyncShareableTagBase {
			public:
				TPointer() : m_ptr(nullptr) {}
				TPointer(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
				TPointer(const TPointer<_Ty, _TID>& src) : m_ptr(src.m_ptr) { note_value_assignment(); }
				template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
				TPointer(const TPointer<_Ty2, TPointerID<_Ty2> >& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
				virtual ~TPointer() {}

				void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
				_Ty* raw_pointer() const { return m_ptr; }
				_Ty* get() const { return m_ptr; }
				_Ty& operator*() const {
					assert_initialized();
#ifndef NDEBUG
					if (nullptr == m_ptr) {
						MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointer"));
					}
#endif // !NDEBUG
					return (*m_ptr);
				}
				_Ty* operator->() const {
					assert_initialized();
#ifndef NDEBUG
					if (nullptr == m_ptr) {
						MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointer"));
					}
#endif // !NDEBUG
					return m_ptr;
				}
				TPointer<_Ty, _TID>& operator=(_Ty* ptr) {
					note_value_assignment();
					m_ptr = ptr;
					return (*this);
				}
				TPointer<_Ty, _TID>& operator=(const TPointer<_Ty, _TID>& _Right_cref) {
					note_value_assignment();
					m_ptr = _Right_cref.m_ptr;
					return (*this);
				}
				template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
				TPointer<_Ty, _TID>& operator=(const TPointer<_Ty2, TPointerID<_Ty2> >& _Right_cref) {
					note_value_assignment();
					m_ptr = _Right_cref.m_ptr;
					return (*this);
				}
				bool operator==(const _Ty* _Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
				bool operator!=(const _Ty* _Right_cref) const { /*assert_initialized();*/ return (!((*this) == _Right_cref)); }
				bool operator==(const TPointer<_Ty, _TID> &_Right_cref) const { /*assert_initialized();*/ return (_Right_cref == m_ptr); }
				bool operator!=(const TPointer<_Ty, _TID> &_Right_cref) const { /*assert_initialized();*/ return (!((*this) == _Right_cref)); }

				bool operator!() const { assert_initialized(); return (!m_ptr); }
				operator bool() const {
					assert_initialized();
					return (m_ptr != nullptr);
				}

				explicit operator _Ty*() const {
					assert_initialized();
#ifdef NATIVE_PTR_DEBUG_HELPER1
					if (nullptr == m_ptr) {
						int q = 3; /* just a line of code for putting a debugger break point */
				}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
					return m_ptr;
			}

				_Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
				void note_value_assignment() { m_initialized = true; }
				void assert_initialized() const { assert(m_initialized); }
				bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
				void note_value_assignment() {}
				void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			};

			template<typename _Ty, typename _TID = TPointerID<_Ty>>
			class TPointerForLegacy : public mse::us::impl::NotAsyncShareableTagBase {
			public:
				TPointerForLegacy() : m_ptr(nullptr) {}
				TPointerForLegacy(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
				template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value || std::is_same<const _Ty2, _Ty>::value, void>::type>
				TPointerForLegacy(const TPointerForLegacy<_Ty2, _TID>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
				virtual ~TPointerForLegacy() {}

				void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
				_Ty* raw_pointer() const { return m_ptr; }
				_Ty* get() const { return m_ptr; }
				_Ty& operator*() const {
					assert_initialized();
#ifndef NDEBUG
					if (nullptr == m_ptr) {
						MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointerForLegacy"));
					}
#endif // !NDEBUG
					return (*m_ptr);
				}
				_Ty* operator->() const {
					assert_initialized();
#ifndef NDEBUG
					if (nullptr == m_ptr) {
						MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TPointerForLegacy"));
					}
#endif // !NDEBUG
					return m_ptr;
				}
				TPointerForLegacy<_Ty, _TID>& operator=(_Ty* ptr) {
					note_value_assignment();
					m_ptr = ptr;
					return (*this);
				}
				bool operator==(const _Ty* _Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
				bool operator!=(const _Ty* _Right_cref) const { /*assert_initialized();*/ return (!((*this) == _Right_cref)); }
				bool operator==(const TPointerForLegacy<_Ty, _TID> &_Right_cref) const { /*assert_initialized();*/ return (m_ptr == _Right_cref); }
				bool operator!=(const TPointerForLegacy<_Ty, _TID> &_Right_cref) const { /*assert_initialized();*/ return (!((*this) == _Right_cref)); }

				bool operator!() const { assert_initialized(); return (!m_ptr); }
				operator bool() const {
					assert_initialized();
					return (m_ptr != nullptr);
				}

				operator _Ty*() const {
					assert_initialized();
#ifdef NATIVE_PTR_DEBUG_HELPER1
					if (nullptr == m_ptr) {
						int q = 3; /* just a line of code for putting a debugger break point */
					}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
					return m_ptr;
				}

				_Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
				void note_value_assignment() { m_initialized = true; }
				void assert_initialized() const { assert(m_initialized); }
				bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
				void note_value_assignment() {}
				void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			};
		}
	}


	namespace us {
		namespace impl {
			class CSaferPtrBase : public mse::us::impl::NotAsyncShareableTagBase {
			public:
				/* spb_set_to_null() needs to be available even when the smart pointer is const, because the object it points to may become
				invalid (deleted). */
				virtual void spb_set_to_null() const = 0;
			};
		}

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_TSAFERPTR_CHECK_USE_BEFORE_SET
#define MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
#endif // !MSE_SUPPRESS_TSAFERPTR_CHECK_USE_BEFORE_SET
#endif // !NDEBUG

		/* TSaferPtr behaves similar to, and is largely compatible with, native pointers. It's a bit safer in that it initializes to
		nullptr by default and checks for attempted dereference of null pointers. */
		template<typename _Ty>
		class TSaferPtr : public impl::CSaferPtrBase {
		public:
			TSaferPtr() : m_ptr(nullptr) {}
			TSaferPtr(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
			TSaferPtr(const TSaferPtr<_Ty>& src) : m_ptr(src.m_ptr) { note_value_assignment(); }
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TSaferPtr(const TSaferPtr<_Ty2>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
			virtual ~TSaferPtr() {}

			virtual void spb_set_to_null() const { m_ptr = nullptr; }

			void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
			_Ty* raw_pointer() const { return m_ptr; }
			_Ty* get() const { return m_ptr; }
			_Ty& operator*() const {
				assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTR_CHECKS
				if (nullptr == m_ptr) {
					MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtr"));
				}
#endif /*MSE_DISABLE_TSAFERPTR_CHECKS*/
				return (*m_ptr);
			}
			_Ty* operator->() const {
				assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTR_CHECKS
				if (nullptr == m_ptr) {
					MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtr"));
				}
#endif /*MSE_DISABLE_TSAFERPTR_CHECKS*/
				return m_ptr;
			}
			TSaferPtr<_Ty>& operator=(_Ty* ptr) {
				note_value_assignment();
				m_ptr = ptr;
				return (*this);
			}
			TSaferPtr<_Ty>& operator=(const TSaferPtr<_Ty>& _Right_cref) {
				note_value_assignment();
				m_ptr = _Right_cref.m_ptr;
				return (*this);
			}
			bool operator==(const _Ty* _Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
			bool operator!=(const _Ty* _Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }
			bool operator==(const TSaferPtr<_Ty> &_Right_cref) const { assert_initialized(); return (_Right_cref == m_ptr); }
			bool operator!=(const TSaferPtr<_Ty> &_Right_cref) const { assert_initialized(); return (!((*this) == _Right_cref)); }

			bool operator!() const { assert_initialized(); return (!m_ptr); }
			operator bool() const {
				assert_initialized();
				return (m_ptr != nullptr);
			}

			explicit operator _Ty*() const {
				assert_initialized();
#ifdef NATIVE_PTR_DEBUG_HELPER1
				if (nullptr == m_ptr) {
					int q = 3; /* just a line of code for putting a debugger break point */
				}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
				return m_ptr;
			}

			/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
			been deleted) even in cases when this smart pointer is const. */
			mutable _Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			void note_value_assignment() { m_initialized = true; }
			void assert_initialized() const { assert(m_initialized); }
			bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			void note_value_assignment() {}
			void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		};

		/* TSaferPtrForLegacy is similar to TSaferPtr, but more readily converts to a native pointer implicitly. So when replacing
		native pointers with safer pointers in legacy code, fewer code changes (explicit casts) may be required when using this
		template. */
		template<typename _Ty>
		class TSaferPtrForLegacy : public impl::CSaferPtrBase {
		public:
			TSaferPtrForLegacy() : m_ptr(nullptr) {}
			TSaferPtrForLegacy(_Ty* ptr) : m_ptr(ptr) { note_value_assignment(); }
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value, void>::type>
			TSaferPtrForLegacy(const TSaferPtrForLegacy<_Ty2>& src_cref) : m_ptr(src_cref.m_ptr) { note_value_assignment(); }
			virtual ~TSaferPtrForLegacy() {}

			virtual void spb_set_to_null() const { m_ptr = nullptr; }

			void raw_pointer(_Ty* ptr) { note_value_assignment(); m_ptr = ptr; }
			_Ty* raw_pointer() const { return m_ptr; }
			_Ty* get() const { return m_ptr; }
			_Ty& operator*() const {
				assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS
				if (nullptr == m_ptr) {
					MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtrForLegacy"));
				}
#endif /*MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS*/
				return (*m_ptr);
			}
			_Ty* operator->() const {
				assert_initialized();
#ifndef MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS
				if (nullptr == m_ptr) {
					MSE_THROW(primitives_null_dereference_error("attempt to dereference null pointer - mse::TSaferPtrForLegacy"));
				}
#endif /*MSE_DISABLE_TSAFERPTRFORLEGACY_CHECKS*/
				return m_ptr;
			}
			TSaferPtrForLegacy<_Ty>& operator=(_Ty* ptr) {
				note_value_assignment();
				m_ptr = ptr;
				return (*this);
			}
			//operator bool() const { return m_ptr; }

			operator _Ty*() const {
				assert_initialized();
#ifdef NATIVE_PTR_DEBUG_HELPER1
				if (nullptr == m_ptr) {
					int q = 3; /* just a line of code for putting a debugger break point */
				}
#endif /*NATIVE_PTR_DEBUG_HELPER1*/
				return m_ptr;
			}

			/* m_ptr needs to be mutable so that it can be set to nullptr when the object it points to is no longer valid (i.e. has
			been deleted) even in cases when this smart pointer is const. */
			mutable _Ty* m_ptr;

#ifdef MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			void note_value_assignment() { m_initialized = true; }
			void assert_initialized() const { assert(m_initialized); }
			bool m_initialized = false;
#else // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
			void note_value_assignment() {}
			void assert_initialized() const {}
#endif // MSE_TSAFERPTR_CHECK_USE_BEFORE_SET
		};
	}
	/* Deprecated in this namespace. Use mse::us::TSaferPtr<> instead. */
	template<typename _Ty> using TSaferPtr = mse::us::TSaferPtr<_Ty>;
	/* Deprecated in this namespace. Use mse::us::TSaferPtrForLegacy<> instead. */
	template<typename _Ty> using TSaferPtrForLegacy = mse::us::TSaferPtrForLegacy<_Ty>;

#if defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)
	template<typename _Ty> auto pointer_to(_Ty& _X) { return &_X; }

	template<typename _Ty>
	auto pointer_to(_Ty&& _X) {
		/* Some compilers (prudently) don't allow you to obtain a pointer to an r-value. But since it's safe and supported
		for the library's safe elements, for compatibility reasons, here we enable you to do it when the those elements are
		"disabled" (i.e. replaced with their native counterparts). */
		const _Ty& X2 = _X;
		return &X2;
	}
#else /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

	namespace impl {
		template<typename _Ty, class = typename std::enable_if<(!std::is_pointer<_Ty>::value), void>::type>
		void T_valid_if_not_raw_pointer_msepointerbasics() {}
	}

	template<typename _Ty>
	auto pointer_to(const _Ty& _X) {
		impl::T_valid_if_not_raw_pointer_msepointerbasics<decltype(&_X)>();
		return &_X;
	}

	template<typename _Ty>
	auto pointer_to(_Ty&& _X) -> decltype(&std::forward<_Ty>(_X)) {
		const _Ty& X2 = _X;
		impl::T_valid_if_not_raw_pointer_msepointerbasics<decltype(&X2)>();
		return &std::forward<_Ty>(_X);
	}
#endif /*defined(MSE_SOME_POINTER_TYPE_IS_DISABLED)*/

	template<typename _Ty>
	_Ty* not_null_from_nullable(const _Ty* src) {
		return src;
	}


	template <class _TTargetType, class _TLeasePointerType> class TSyncWeakFixedConstPointer;

	/* If, for example, you want a safe pointer to a member of a registered pointer target, you can use a
	TSyncWeakFixedPointer to store a copy of the registered pointer along with the pointer targeting the
	member. */
	template <class _TTargetType, class _TLeasePointerType>
	class TSyncWeakFixedPointer : public mse::us::impl::NotAsyncShareableTagBase {
	public:
		TSyncWeakFixedPointer(const TSyncWeakFixedPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TSyncWeakFixedPointer(const TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		_TTargetType& operator*() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		_TTargetType* operator->() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TSyncWeakFixedPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;
		bool operator!=(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const;

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
		static TSyncWeakFixedPointer make(_TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TSyncWeakFixedPointer(target, lease_pointer);
		}

	private:
		TSyncWeakFixedPointer(_TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TSyncWeakFixedPointer& operator=(const TSyncWeakFixedPointer& _Right_cref) = delete;

		_TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
		friend class TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType>;
	};

	template <class _TTargetType, class _TLeasePointerType>
	TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType> make_syncweak(_TTargetType& target, const _TLeasePointerType& lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::make(target, lease_pointer);
	}

	template <class _TTargetType, class _TLeasePointerType>
	class TSyncWeakFixedConstPointer : public mse::us::impl::NotAsyncShareableTagBase {
	public:
		TSyncWeakFixedConstPointer(const TSyncWeakFixedConstPointer&) = default;
		template<class _TLeasePointerType2, class = typename std::enable_if<std::is_convertible<_TLeasePointerType2, _TLeasePointerType>::value, void>::type>
		TSyncWeakFixedConstPointer(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType2>&src) : m_target_pointer(std::addressof(*src)), m_lease_pointer(src.lease_pointer()) {}
		TSyncWeakFixedConstPointer(const TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>&src) : m_target_pointer(src.m_target_pointer), m_lease_pointer(src.m_lease_pointer) {}
		const _TTargetType& operator*() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return (*m_target_pointer);
		}
		const _TTargetType* operator->() const {
			/*const auto &test_cref =*/ *m_lease_pointer; // this should throw if m_lease_pointer is no longer valid
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TSyncWeakFixedConstPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TSyncWeakFixedConstPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }

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
		static TSyncWeakFixedConstPointer make(const _TTargetType2& target, const _TLeasePointerType2& lease_pointer) {
			return TSyncWeakFixedConstPointer(target, lease_pointer);
		}

	private:
		TSyncWeakFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeasePointerType lease_pointer/* usually a registered pointer */)
			: m_target_pointer(&target), m_lease_pointer(lease_pointer) {}
		TSyncWeakFixedConstPointer& operator=(const TSyncWeakFixedConstPointer& _Right_cref) = delete;

		const _TTargetType* m_target_pointer;
		_TLeasePointerType m_lease_pointer;
	};

	template <class _TTargetType, class _TLeasePointerType>
	bool TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator==(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (_Right_cref == m_target_pointer); }
	template <class _TTargetType, class _TLeasePointerType>
	bool TSyncWeakFixedPointer<_TTargetType, _TLeasePointerType>::operator!=(const TSyncWeakFixedConstPointer<_TTargetType, _TLeasePointerType> &_Right_cref) const { return (!((*this) == _Right_cref)); }

	template<class _TTargetType, class _Ty>
	TSyncWeakFixedPointer<_TTargetType, _Ty> make_pointer_to_member(_TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedConstPointer<_TTargetType, _Ty> make_pointer_to_member(const _TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedConstPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}
	template<class _TTargetType, class _Ty>
	TSyncWeakFixedConstPointer<_TTargetType, _Ty> make_const_pointer_to_member(const _TTargetType& target, const _Ty &lease_pointer) {
		return TSyncWeakFixedConstPointer<_TTargetType, _Ty>::make(target, lease_pointer);
	}

	namespace impl {
		template<class _Ty, class _Ty2, class = typename std::enable_if<std::is_same<_Ty, _Ty2>::value>::type>
		static void T_valid_if_same_msepointerbasics() {}
		template<class _TLeasePointer, class _TMemberObjectPointer>
		static void make_pointer_to_member_v2_checks_msepointerbasics(const _TLeasePointer &/*lease_pointer*/, const _TMemberObjectPointer& member_object_ptr) {
			/* Check for possible problematic parameters. */
			if (!member_object_ptr) { MSE_THROW("null member_object_ptr - make_pointer_to_member_v2_checks_msepointerbasics()"); }
			/*
			typedef typename std::remove_reference<decltype(*lease_pointer)>::type _TLeaseTarget;
			typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
			_TTarget _TLeaseTarget::* l_member_object_ptr = member_object_ptr;
			typedef typename std::remove_reference<decltype(l_member_object_ptr)>::type _TMemberObjectPointer2;
			T_valid_if_same_msepointerbasics<const _TMemberObjectPointer2, const _TMemberObjectPointer>();
			*/
		}
	}
	template<class _TLeasePointer, class _TMemberObjectPointer>
	static auto make_pointer_to_member_v2(const _TLeasePointer &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		impl::T_valid_if_not_an_xscope_type(lease_pointer);
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::TSyncWeakFixedPointer<_TTarget, _TLeasePointer>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
	template<class _TLeasePointer, class _TMemberObjectPointer>
	static auto make_const_pointer_to_member_v2(const _TLeasePointer &lease_pointer, const _TMemberObjectPointer& member_object_ptr) {
		impl::T_valid_if_not_an_xscope_type(lease_pointer);
		typedef typename std::remove_reference<decltype((*lease_pointer).*member_object_ptr)>::type _TTarget;
		impl::make_pointer_to_member_v2_checks_msepointerbasics(lease_pointer, member_object_ptr);
		return mse::TSyncWeakFixedConstPointer<_TTarget, _TLeasePointer>::make((*lease_pointer).*member_object_ptr, lease_pointer);
	}
}

namespace std {
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TSyncWeakFixedPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TSyncWeakFixedPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TSyncWeakFixedPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TSyncWeakFixedConstPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
}

namespace mse {

	namespace us {
		namespace impl {
			class StrongPointerTagBase {
			public:
				void strong_pointer_tag() const {}
			};
			class StrongPointerNotAsyncShareableTagBase : public StrongPointerTagBase, public mse::us::impl::NotAsyncShareableTagBase {};

			class NeverNullTagBase {
			public:
				void never_null_tag() const {}
			};
			class StrongPointerNeverNullTagBase : public StrongPointerTagBase, public NeverNullTagBase {};
			class StrongPointerNeverNullNotAsyncShareableTagBase : public StrongPointerTagBase, public NeverNullTagBase, public mse::us::impl::NotAsyncShareableTagBase {};

			class ExclusivePointerTagBase {
			public:
				void exclusive_pointer_tag() const {}
			};
			class StrongExclusivePointerTagBase : public StrongPointerTagBase, public ExclusivePointerTagBase {};
		}
	}

	namespace impl {
		template <typename T> struct is_shared_ptr : std::false_type {};
		template <typename T> struct is_shared_ptr<std::shared_ptr<T> > : std::true_type {};

		template<typename _TStrongPointer, class = typename std::enable_if<
			(std::is_base_of<mse::us::impl::StrongPointerTagBase, _TStrongPointer>::value)
			|| (std::is_pointer<_TStrongPointer>::value)/* for when scope pointers are "disabled" */
			|| (is_shared_ptr<_TStrongPointer>::value)/* for when refcounting pointers are "disabled" */
			, void>::type>
			class is_valid_if_strong_pointer {
			public:
				static void no_op() {}
		};

		template<typename _TNeverNullPointer, class = typename std::enable_if<
			(std::is_base_of<mse::us::impl::NeverNullTagBase, _TNeverNullPointer>::value)
			|| (std::is_pointer<_TNeverNullPointer>::value)/* for when scope pointers are "disabled" */
			|| (is_shared_ptr<_TNeverNullPointer>::value)/* for when refcounting pointers are "disabled" */
			, void>::type>
			class is_valid_if_never_null_pointer {
			public:
				static void no_op() {}
		};

		template<typename _TStrongAndNeverNullPointer>
		class is_valid_if_strong_and_never_null_pointer : public is_valid_if_strong_pointer<_TStrongAndNeverNullPointer>, public is_valid_if_never_null_pointer<_TStrongAndNeverNullPointer> {
		public:
			static void no_op() {}
		};
	}

	template <class _TTargetType, class _TLeaseType> class TStrongFixedConstPointer;

	/* If, for example, you want an "owning" pointer to a member of a refcounting pointer target, you can use a
	TStrongFixedPointer to store a copy of the owning (refcounting) pointer along with the pointer targeting the
	member. */
	template <class _TTargetType, class _TLeaseType>
	class TStrongFixedPointer : public mse::us::impl::StrongPointerNeverNullNotAsyncShareableTagBase {
	public:
		TStrongFixedPointer(const TStrongFixedPointer&) = default;
		template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
		TStrongFixedPointer(const TStrongFixedPointer<_TTargetType, _TLeaseType2>&src) : m_target_pointer(std::addressof(*src)), m_lease(src.lease()) {}
		virtual ~TStrongFixedPointer() {
			/* This is just a no-op function that will cause a compile error when _TLeaseType is not an eligible type. */
			mse::impl::is_valid_if_strong_pointer<_TLeaseType>::no_op();
		}
		_TTargetType& operator*() const {
			return (*m_target_pointer);
		}
		_TTargetType* operator->() const {
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TStrongFixedPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TStrongFixedPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TStrongFixedConstPointer<_TTargetType, _TLeaseType> &_Right_cref) const;
		bool operator!=(const TStrongFixedConstPointer<_TTargetType, _TLeaseType> &_Right_cref) const;

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
		_TLeaseType lease() const { return (*this).m_lease; }

		template <class _TTargetType2, class _TLeaseType2>
		static TStrongFixedPointer make(_TTargetType2& target, const _TLeaseType2& lease) {
			return TStrongFixedPointer(target, lease);
		}
		template <class _TTargetType2, class _TLeaseType2>
		static TStrongFixedPointer make(_TTargetType2& target, _TLeaseType2&& lease) {
			return TStrongFixedPointer(target, std::forward<decltype(lease)>(lease));
		}

	protected:
		TStrongFixedPointer(_TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
			: m_target_pointer(std::addressof(target)), m_lease(lease) {}
		TStrongFixedPointer(_TTargetType& target/* often a struct member */, _TLeaseType&& lease)
			: m_target_pointer(std::addressof(target)), m_lease(std::forward<decltype(lease)>(lease)) {}
	private:
		TStrongFixedPointer& operator=(const TStrongFixedPointer& _Right_cref) = delete;
		MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		_TTargetType* m_target_pointer;
		_TLeaseType m_lease;

		friend class TStrongFixedConstPointer<_TTargetType, _TLeaseType>;
	};

	template <class _TTargetType, class _TLeaseType>
	TStrongFixedPointer<_TTargetType, _TLeaseType> make_strong(_TTargetType& target, const _TLeaseType& lease) {
		return TStrongFixedPointer<_TTargetType, _TLeaseType>::make(target, lease);
	}
	template <class _TTargetType, class _TLeaseType>
	auto make_strong(_TTargetType& target, _TLeaseType&& lease) -> TStrongFixedPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type> {
		return TStrongFixedPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type>::make(target, std::forward<decltype(lease)>(lease));
	}

	template <class _TTargetType, class _TLeaseType>
	class TStrongFixedConstPointer : public mse::us::impl::StrongPointerNeverNullNotAsyncShareableTagBase {
	public:
		TStrongFixedConstPointer(const TStrongFixedConstPointer&) = default;
		template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
		TStrongFixedConstPointer(const TStrongFixedConstPointer<_TTargetType, _TLeaseType2>&src) : m_target_pointer(std::addressof(*src)), m_lease(src.lease()) {}
		TStrongFixedConstPointer(const TStrongFixedPointer<_TTargetType, _TLeaseType>&src) : m_target_pointer(src.m_target_pointer), m_lease(src.m_lease) {}
		virtual ~TStrongFixedConstPointer() {
			/* This is just a no-op function that will cause a compile error when _TLeaseType is not an eligible type. */
			mse::impl::is_valid_if_strong_pointer<_TLeaseType>::no_op();
		}
		const _TTargetType& operator*() const {
			return (*m_target_pointer);
		}
		const _TTargetType* operator->() const {
			return m_target_pointer;
		}

		bool operator==(const _TTargetType* _Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const _TTargetType* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TStrongFixedConstPointer &_Right_cref) const { return (_Right_cref == m_target_pointer); }
		bool operator!=(const TStrongFixedConstPointer &_Right_cref) const { return (!((*this) == _Right_cref)); }

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
		_TLeaseType lease() const { return (*this).m_lease; }

		template <class _TTargetType2, class _TLeaseType2>
		static TStrongFixedConstPointer make(const _TTargetType2& target, const _TLeaseType2& lease) {
			return TStrongFixedConstPointer(target, lease);
		}
		template <class _TTargetType2, class _TLeaseType2>
		static TStrongFixedConstPointer make(const _TTargetType2& target, _TLeaseType2&& lease) {
			return TStrongFixedConstPointer(target, std::forward<decltype(lease)>(lease));
		}

	protected:
		TStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, const _TLeaseType& lease/* usually a reference counting pointer */)
			: m_target_pointer(std::addressof(target)), m_lease(lease) {}
		TStrongFixedConstPointer(const _TTargetType& target/* often a struct member */, _TLeaseType&& lease)
			: m_target_pointer(std::addressof(target)), m_lease(std::forward<decltype(lease)>(lease)) {}
	private:
		TStrongFixedConstPointer& operator=(const TStrongFixedConstPointer& _Right_cref) = delete;

		const _TTargetType* m_target_pointer;
		_TLeaseType m_lease;
	};

	template <class _TTargetType, class _TLeaseType>
	TStrongFixedConstPointer<_TTargetType, _TLeaseType> make_const_strong(const _TTargetType& target, const _TLeaseType& lease) {
		return TStrongFixedConstPointer<_TTargetType, _TLeaseType>::make(target, lease);
	}
	template <class _TTargetType, class _TLeaseType>
	auto make_const_strong(const _TTargetType& target, _TLeaseType&& lease) -> TStrongFixedConstPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type> {
		return TStrongFixedConstPointer<_TTargetType, typename std::remove_reference<_TLeaseType>::type>::make(target, std::forward<decltype(lease)>(lease));
	}

	template <class _TTargetType, class _TLeaseType>
	bool TStrongFixedPointer<_TTargetType, _TLeaseType>::operator==(const TStrongFixedConstPointer<_TTargetType, _TLeaseType> &_Right_cref) const { return (_Right_cref == m_target_pointer); }
	template <class _TTargetType, class _TLeaseType>
	bool TStrongFixedPointer<_TTargetType, _TLeaseType>::operator!=(const TStrongFixedConstPointer<_TTargetType, _TLeaseType> &_Right_cref) const { return (!((*this) == _Right_cref)); }
}

namespace std {
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TStrongFixedPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TStrongFixedPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TStrongFixedPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
	template <class _TTargetType, class _TLeaseType>
	struct hash<mse::TStrongFixedConstPointer<_TTargetType, _TLeaseType> > {	// hash functor
		typedef mse::TStrongFixedConstPointer<_TTargetType, _TLeaseType> argument_type;
		typedef size_t result_type;
		size_t operator()(const mse::TStrongFixedConstPointer<_TTargetType, _TLeaseType>& _Keyval) const {
			const _TTargetType* ptr1 = nullptr;
			if (_Keyval) {
				ptr1 = std::addressof(*_Keyval);
			}
			return (hash<const _TTargetType *>()(ptr1));
		}
	};
}

namespace mse {
	namespace us {
		namespace impl {

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4127 )
#endif /*_MSC_VER*/

			/* CSAllocRegistry essentially just maintains a list of all objects allocated by a registered "new" call and not (yet)
			subsequently deallocated with a corresponding registered delete. */
			class CSAllocRegistry {
			public:
				CSAllocRegistry() {}
				~CSAllocRegistry() {}
				bool registerPointer(void *alloc_ptr) {
					if (nullptr == alloc_ptr) { return true; }
					{
						if (1 <= sc_fs1_max_objects) {
							/* We'll add this object to fast storage. */
							if (sc_fs1_max_objects == m_num_fs1_objects) {
								/* Too many objects. We're gonna move the oldest object to slow storage. */
								moveObjectFromFastStorage1ToSlowStorage(0);
							}
							auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
							fs1_object_ref = alloc_ptr;
							m_num_fs1_objects += 1;
							return true;
						}
						else {
							/* Add the mapping to slow storage. */
							std::unordered_set<CFS1Object>::value_type item(alloc_ptr);
							m_pointer_set.insert(item);
						}
					}
					return true;
				}
				bool unregisterPointer(void *alloc_ptr) {
					if (nullptr == alloc_ptr) { return true; }
					bool retval = false;
					{
						/* check if the object is in "fast storage 1" first */
						for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
							if (alloc_ptr == m_fs1_objects[i]) {
								removeObjectFromFastStorage1(i);
								return true;
							}
						}

						/* The object was not in "fast storage 1". It's proably in "slow storage". */
						auto num_erased = m_pointer_set.erase(alloc_ptr);
						if (1 <= num_erased) {
							retval = true;
						}
					}
					return retval;
				}
				bool registerPointer(const void *alloc_ptr) { return (*this).registerPointer(const_cast<void *>(alloc_ptr)); }
				bool unregisterPointer(const void *alloc_ptr) { return (*this).unregisterPointer(const_cast<void *>(alloc_ptr)); }
				void reserve_space_for_one_more() {
					/* The purpose of this function is to ensure that the next call to registerPointer() won't
					need to allocate more memory, and thus won't have any chance of throwing an exception due to
					memory allocation failure. */
					m_pointer_set.reserve(m_pointer_set.size() + 1);
				}

				bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_pointer_set.size())); }

			private:
				/* So this tracker stores the allocation pointers in either "fast storage1" or "slow storage". The code for
				"fast storage1" is ugly. The code for "slow storage" is more readable. */
				void removeObjectFromFastStorage1(int fs1_obj_index) {
					for (int j = fs1_obj_index; j < (m_num_fs1_objects - 1); j += 1) {
						m_fs1_objects[j] = m_fs1_objects[j + 1];
					}
					m_num_fs1_objects -= 1;
				}
				void moveObjectFromFastStorage1ToSlowStorage(int fs1_obj_index) {
					auto& fs1_object_ref = m_fs1_objects[fs1_obj_index];
					/* First we're gonna copy this object to slow storage. */
					//std::unordered_set<CFS1Object>::value_type item(fs1_object_ref);
					m_pointer_set.insert(fs1_object_ref);
					/* Then we're gonna remove the object from fast storage */
					removeObjectFromFastStorage1(fs1_obj_index);
				}

				typedef void* CFS1Object;

#ifndef MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
#define MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */
#endif // !MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
				MSE_CONSTEXPR static const int sc_fs1_max_objects = MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS;
				CFS1Object m_fs1_objects[sc_fs1_max_objects];
				int m_num_fs1_objects = 0;

				/* "slow storage" */
				std::unordered_set<CFS1Object> m_pointer_set;
			};

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

			template<typename _Ty>
			inline CSAllocRegistry& tlSAllocRegistry_ref() {
				thread_local static CSAllocRegistry tlSAllocRegistry;
				return tlSAllocRegistry;
			}
		}
	}
}

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

#undef MSE_THROW

#endif /*ndef MSEPOINTERBASICS_H*/
