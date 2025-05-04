
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSELEGACYHELPERS_H_
#define MSELEGACYHELPERS_H_

#ifndef MSE_LEGACYHELPERS_DISABLED
#include "msepoly.h"
#include "msefunctional.h"
#include <cstring>
#else // !MSE_LEGACYHELPERS_DISABLED
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif // !MSE_LEGACYHELPERS_DISABLED

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4127 4702 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion-null"
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifdef MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) (
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) )[size]
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_UNSAFE_LH_LARGE_FIXED_ARRAY_INITIALIZER_HELPER(element_type, size) 
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) element_type *
#define MSE_LH_ADDRESS_OF_ARRAY_ELEMENT(array1, index) (&(array1[index]))

#define MSE_LH_ALLOC_POINTER1(element_type) (element_type *)malloc(sizeof(element_type))
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) (iterator_type)malloc(num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) (element_type *)realloc(ptr, num_bytes)
#define MSE_LH_FREE(ptr) free(ptr)

/* generally prefer MSE_LH_ALLOC_DYN_ARRAY1() or MSE_LH_ALLOC_POINTER1() over MSE_LH_ALLOC() */
#define MSE_LH_ALLOC(element_type, ptr, num_bytes) ptr = (element_type *)malloc(num_bytes)

#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) element_type *
#define MSE_LH_LOCAL_VAR_ONLY_ARRAY_ITERATOR_TYPE(element_type) element_type *
#define MSE_LH_PARAM_ONLY_ARRAY_ITERATOR_TYPE(element_type) MSE_LH_LOCAL_VAR_ONLY_ARRAY_ITERATOR_TYPE(element_type)

#define MSE_LH_FUNCTION_POINTER_TYPE_PREFIX
#define MSE_LH_FUNCTION_POINTER_TYPE_SUFFIX(params) (*
#define MSE_LH_FUNCTION_POINTER_TYPE_POST_NAME_SUFFIX(params) )params
#define MSE_LH_FUNCTION_POINTER_DECLARATION(return_type, params, name) MSE_LH_FUNCTION_POINTER_TYPE_PREFIX return_type MSE_LH_FUNCTION_POINTER_TYPE_SUFFIX(params) name MSE_LH_FUNCTION_POINTER_TYPE_POST_NAME_SUFFIX(params)

#define MSE_LH_FREAD(ptr, size, count, stream) fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) fwrite(ptr, size, count, stream)
#define MSE_LH_GETLINE(lineptr, nptr, stream) getline(lineptr, nptr, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMCMP(element_type, destination, source, num_bytes) memcmp(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_MEMCMP(destination, source, num_bytes) memcmp(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_STRLEN(str) strlen(str)
#define MSE_LH_STRNLEN_S(str, strsz) strlnen_s(str, strsz)
#define MSE_LH_STRCPY(destination, source) strcpy(destination, source)
#define MSE_LH_STRCMP(destination, source) strcmp(destination, source)
#define MSE_LH_STRNCMP(destination, source, count) strncmp(destination, source, count)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) object_type
#define MSE_LH_POINTER_TYPE(element_type) element_type *
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) element_type *
#define MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type) element_type *
#define MSE_LH_PARAM_ONLY_POINTER_TYPE(element_type) MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type)
#define MSE_LH_NULL_POINTER NULL
#define MSE_LH_VOID_STAR void *

#define MSE_LH_CAST(type, value) ((type)value)
#define MSE_LH_UNSAFE_CAST(type, value) ((type)value)
#define MSE_LH_UNSAFE_MAKE_POINTER_TO(target) &(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_TO(target) &(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_TEMPORARY_ARRAY_OF_RAW_POINTERS_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_POINTER_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_ARRAY_ITERATOR_FROM(iter) (iter)

#ifndef MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE
#endif // !MSE_LH_SUPPRESS_CHECK_IN_XSCOPE

#define MSE_LH_IF_ENABLED(x)
#define MSE_LH_IF_DISABLED(x) x

#else /*MSE_LEGACYHELPERS_DISABLED*/

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX mse::lh::TNativeArrayReplacement< 
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) , size >
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) 
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) mse::lh::TStrongVectorIterator< element_type >
#define MSE_LH_ADDRESS_OF_ARRAY_ELEMENT(array1, index) mse::lh::address_of_array_element_replacement(array1, index)

#define MSE_LH_ALLOC_POINTER1(element_type) mse::lh::allocate<mse::lh::TLHNullableAnyPointer<element_type> >()
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) mse::lh::allocate_dyn_array1<iterator_type>(num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) mse::lh::reallocate(ptr, num_bytes)
#define MSE_LH_FREE(ptr) mse::lh::free(ptr)

/* generally prefer MSE_LH_ALLOC_DYN_ARRAY1() or MSE_LH_ALLOC_POINTER1() over MSE_LH_ALLOC() */
#define MSE_LH_ALLOC(element_type, ptr, num_bytes) mse::lh::allocate(ptr, num_bytes)

/* Note: MSE_LH_ARRAY_ITERATOR_TYPE() / mse::lh::TLHNullableAnyRandomAccessIterator<> is intended to replace legacy raw 
pointers being used as array/buffer iterators, and as such is different from "conventional" iterators in that "one past 
the last element" end markers are not valid values. Valid values are either null, or point to a valid object. */
#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) mse::lh::TLHNullableAnyRandomAccessIterator< element_type >
/* MSE_LH_LOCAL_VAR_ONLY_ARRAY_ITERATOR_TYPE is a significantly restricted version of MSE_LH_ARRAY_ITERATOR_TYPE. You might choose to
use it, despite its restrictions, as a local variable type because it accepts some (high performance) iterators that (the 
otherwise more flexible) MSE_LH_ARRAY_ITERATOR_TYPE doesn't. */
#define MSE_LH_LOCAL_VAR_ONLY_ARRAY_ITERATOR_TYPE(element_type) mse::lh::TXScopeLHNullableAnyRandomAccessIterator< element_type >
#define MSE_LH_PARAM_ONLY_ARRAY_ITERATOR_TYPE(element_type) MSE_LH_LOCAL_VAR_ONLY_ARRAY_ITERATOR_TYPE(element_type)

#define MSE_LH_FUNCTION_POINTER_TYPE_PREFIX mse::lh::TNativeFunctionPointerReplacement<
#define MSE_LH_FUNCTION_POINTER_TYPE_SUFFIX(params) params>
#define MSE_LH_FUNCTION_POINTER_TYPE_POST_NAME_SUFFIX(params)
#define MSE_LH_FUNCTION_POINTER_DECLARATION(return_type, params, name) MSE_LH_FUNCTION_POINTER_TYPE_PREFIX return_type MSE_LH_FUNCTION_POINTER_TYPE_SUFFIX(params) name MSE_LH_FUNCTION_POINTER_TYPE_POST_NAME_SUFFIX(params)

#define MSE_LH_FREAD(ptr, size, count, stream) mse::lh::fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) mse::lh::fwrite(ptr, size, count, stream)
#define MSE_LH_GETLINE(lineptr, nptr, stream) mse::lh::getline(lineptr, nptr, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) mse::lh::memcpy<mse::lh::TLHNullableAnyRandomAccessIterator<element_type>, mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMCMP(element_type, destination, source, num_bytes) mse::lh::memcmp< mse::lh::TLHNullableAnyRandomAccessIterator<element_type>, mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) mse::lh::memset< mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) mse::lh::memcpy(destination, source, num_bytes)
#define MSE_LH_MEMCMP(destination, source, num_bytes) mse::lh::memcmp(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) mse::lh::memset(ptr, value, num_bytes)
#define MSE_LH_STRLEN(str) mse::lh::strlen(str)
#define MSE_LH_STRNLEN_S(str, strsz) mse::lh::strlnen_s(str, strsz)
#define MSE_LH_STRCPY(destination, source) mse::lh::strcpy(destination, source)
#define MSE_LH_STRCMP(destination, source) mse::lh::strcmp(destination, source)
#define MSE_LH_STRNCMP(destination, source, count) mse::lh::strncmp(destination, source, count)

/* MSE_LH_ADDRESSABLE_TYPE() is a type annotation used to indicate that the '&' operator may/will be used to obtain the address of
the associated declared object(s). */
#define MSE_LH_ADDRESSABLE_TYPE(object_type) mse::TRegisteredObj< object_type >
#define MSE_LH_POINTER_TYPE(element_type) mse::lh::TLHNullableAnyPointer< element_type >
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) mse::TRefCountingPointer< element_type >
/* MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE is a significantly restricted version of MSE_LH_POINTER_TYPE. You might choose to use it, despite its
restrictions, as a local variable type because it accepts some (high performance) pointers that (the otherwise more flexible)
MSE_LH_POINTER_TYPE doesn't. (Including raw pointers.) */
#define MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type) mse::lh::TXScopeLHNullableAnyPointer< element_type >
#define MSE_LH_PARAM_ONLY_POINTER_TYPE(element_type) MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type)
#define MSE_LH_NULL_POINTER nullptr
#define MSE_LH_VOID_STAR mse::lh::void_star_replacement

#define MSE_LH_CAST(type, value) ((type const &)(value))
#define MSE_LH_UNSAFE_CAST(type, value) mse::us::lh::unsafe_cast<type>(value)
#define MSE_LH_UNSAFE_MAKE_POINTER_TO(target) MSE_LH_POINTER_TYPE(mse::us::unsafe_make_any_pointer_to(target))
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_TO(target) std::addressof(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_FROM(ptr) mse::us::lh::make_raw_pointer_from(ptr)
#define MSE_LH_UNSAFE_MAKE_TEMPORARY_ARRAY_OF_RAW_POINTERS_FROM(ptr) mse::us::lh::make_temporary_array_of_raw_pointers_from(ptr)
#define MSE_LH_UNSAFE_MAKE_POINTER_FROM(ptr) mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(ptr)
#define MSE_LH_UNSAFE_MAKE_ARRAY_ITERATOR_FROM(iter) mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(iter)

#ifndef MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE MSE_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE MSE_SUPPRESS_CHECK_IN_DECLSCOPE
#endif // !MSE_LH_SUPPRESS_CHECK_IN_XSCOPE


#define MSE_LH_IF_ENABLED(x) x
#define MSE_LH_IF_DISABLED(x)

namespace mse {
	namespace lh {
		namespace impl {
			template<class _TIter>
			size_t strnlen_s_helper1(std::false_type, _TIter const& str, size_t strsz) {
				size_t count = 0;
				while ((strsz > count) && ('\0' != str[count])) { count += 1; }
				return count;
			}
			template<class _TIter>
			size_t strnlen_s_helper1(std::true_type, _TIter const& str, size_t strsz) {
				if (!str) {
					return 0;
				}
				return strnlen_s_helper1(std::false_type(), str, strsz);
			}
		}
		template<class _TIter>
		size_t strnlen_s(_TIter const& str, size_t strsz) {
			return impl::strnlen_s_helper1(typename mse::impl::IsNullable_msemsearray<_TIter>::type(), str, strsz);
		}
	}
}
#ifdef MSE_SAFER_SUBSTITUTES_DISABLED

namespace mse {
	namespace lh {
		typedef decltype(NULL) NULL_t;

		template <typename _udTy, size_t _Size>
		using TNativeArrayReplacement = _udTy[_Size];

#define MSE_UNSAFE_LH_LARGE_FIXED_ARRAY_INITIALIZER_HELPER(element_type, size) 

		template <typename _Ty>
		using TLHNullableAnyPointer = _Ty*;

		template <typename _Ty>
		using TXScopeLHNullableAnyPointer = _Ty*;

		template <typename _Ty>
		using TLHNullableAnyRandomAccessIterator = _Ty*;

		template <typename _Ty>
		using TXScopeLHNullableAnyRandomAccessIterator = _Ty*;

		template <typename _Ty>
		using TStrongVectorIterator = _Ty*;

		template <class X, class... Args>
		TStrongVectorIterator<X> make_strong_vector_iterator(Args&&... args) {
			return TStrongVectorIterator<X>(std::forward<Args>(args)...);
		}

		template <typename _Ty>
		using TXScopeStrongVectorIterator = _Ty*;

		template <class X, class... Args>
		TXScopeStrongVectorIterator<X> make_xscope_strong_vector_iterator(Args&&... args) {
			return TXScopeStrongVectorIterator<X>(std::forward<Args>(args)...);
		}

		/* TXScopeStrongVectorIterator<> does not directly convert to mse::rsv::TFParam<mse::TXScopeCSSSXSTERandomAccessIterator<> >.
		But the following function can be used to obtain a "locking" scope iterator that does. */
		template <class X>
		auto make_xscope_locking_vector_iterator(const TXScopeStrongVectorIterator<X>& xs_strong_iter) {
			return xs_strong_iter;
		}
		template <class X>
		auto make_xscope_locking_vector_iterator(TXScopeStrongVectorIterator<X>&& xs_strong_iter) = delete;

		template <class TArray, class TIndex>
		auto address_of_array_element_replacement(TArray& array1, TIndex index) {
			return &(array1[index]);
		}

		template<class _Fty>
		using TNativeFunctionPointerReplacement = _Fty*;

		namespace impl {
			template<class _TPtr>
			class CAllocF {
			public:
				static void free(_TPtr& ptr) {
					::free(ptr);
				}
				static void allocate(_TPtr& ptr, size_t num_bytes) {
					ptr = _TPtr(::malloc(num_bytes));
				}
				static void reallocate(_TPtr& ptr, size_t num_bytes) {
					ptr = _TPtr(::realloc(ptr, num_bytes));
				}
			};
		}

		template<class _TPointer>
		_TPointer allocate() {
			_TPointer ptr;
			auto num_bytes = sizeof(decltype(*ptr));
			ptr = _TPointer(::malloc(num_bytes));
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate_dyn_array1(size_t num_bytes) {
			_TDynArrayIter ptr;
			ptr = _TDynArrayIter(::malloc(num_bytes));
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter reallocate(const _TDynArrayIter& ptr2, size_t num_bytes) {
			_TDynArrayIter ptr = ptr2;
			ptr = _TDynArrayIter(::realloc(ptr2, num_bytes));
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate(_TDynArrayIter& ptr, size_t num_bytes) {
			ptr = _TDynArrayIter(::malloc(num_bytes));
			return ptr;
		}
		template<class _TDynArrayIter>
		void free(_TDynArrayIter& ptr) {
			::free(ptr);
		}

		template<class _TIter>
		size_t fread(_TIter const& ptr, size_t size, size_t count, FILE* stream) {
			return ::fread(ptr, size, count, stream);
		}
		template<class _TIter>
		size_t fwrite(_TIter const& ptr, size_t size, size_t count, FILE* stream) {
			return ::fwrite(ptr, size, count, stream);
		}

		template<class _TIter, class _TIter2>
		_TIter memcpy(_TIter const& destination, _TIter2 const& source, size_t num_bytes) {
			::memcpy(destination, source, num_bytes);
			return destination;
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement, class _TIter2>
		_TElement* memcpy(_TElement* destination, _TIter2 const& source, size_t num_bytes) {
			::memcpy(destination, source, num_bytes);
			return destination;
		}
		template<class _TIter, class _TIter2>
		int memcmp(_TIter const& destination, _TIter2 const& source, size_t num_bytes) {
			return ::memcmp(destination, source, num_bytes);
		}
		template<class _TIter>
		_TIter memset(_TIter const& iter, int value, size_t num_bytes) {
			::memset(iter, value, num_bytes);
			return iter;
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement>
		_TElement* memset(_TElement* iter, int value, size_t num_bytes) {
			::memset(iter, value, num_bytes);
			return iter;
		}
		template<class _TIter>
		size_t strlen(_TIter const& str) {
			return ::strlen(str);
		}
		template<class _TIter, class _TIter2>
		_TIter strcpy(_TIter const& destination, _TIter2 const& source) {
			::strcpy(destination, source);
			return destination;
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::strcpy(). */
		template<class _TElement, class _TIter2>
		_TElement* strcpy(_TElement* destination, _TIter2 const& source) {
			::strcpy(destination, source);
			return destination;
		}
		template<class _TIter, class _TIter2>
		int strcmp(_TIter const& str1, _TIter2 const& str2) {
			return ::strcmp(std::addressof(str1[0]), std::addressof(str2[0]));
		}
		template<class _TIter, class _TIter2>
		int strncmp(_TIter const& str1, _TIter2 const& str2, size_t count) {
			return ::strncmp(std::addressof(str1[0]), std::addressof(str2[0]), count);
		}
	}
	namespace us {
		namespace lh {
			template<typename _Ty, typename _Ty2>
			_Ty unsafe_cast(const _Ty2& x) {
				return (_Ty const &)(x);
			}

			template<typename _Ty>
			auto make_raw_pointer_from(_Ty&& ptr) {
				return ptr;
			}
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty& ptr) {
				return ptr;
			}
		}
	}

	namespace lh {

		/* todo: make distinct xscope and non-xscope versions */
		typedef void* void_star_replacement;
	}
}

#else // MSE_SAFER_SUBSTITUTES_DISABLED

namespace mse {
	namespace lh {
		typedef decltype(NULL) NULL_t;
		typedef decltype(0) ZERO_LITERAL_t;
		static const auto NULL_AND_ZERO_ARE_THE_SAME_TYPE = std::is_same<NULL_t, ZERO_LITERAL_t>::value;

		class void_star_replacement;
	}
	namespace us {
		namespace lh {
			template<typename _Ty, typename _Ty2>
			_Ty unsafe_cast(const _Ty2& x);
			template<typename _Ty>
			_Ty unsafe_cast(const mse::lh::void_star_replacement& x);

			template<typename _Ty>
			auto make_raw_pointer_from(_Ty const& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)));
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)));
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty&& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)));

			auto make_raw_pointer_from(mse::lh::void_star_replacement const& vsr) -> void*;
		}
	}
	namespace lh {
		template <typename _udTy, size_t _Size>
		class TNativeArrayReplacement;

		template <typename _Ty>
		class TLHNullableAnyPointer : public mse::TNullableAnyPointer<_Ty> {
		public:
			typedef mse::TNullableAnyPointer<_Ty> base_class;
			typedef TLHNullableAnyPointer _Myt;
			TLHNullableAnyPointer(const TLHNullableAnyPointer& src) = default;
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TLHNullableAnyPointer(_Ty2 val) : base_class(std::nullptr_t()) {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
			non-const reference. */
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value) || (std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			TLHNullableAnyPointer(TNativeArrayReplacement<_Ty, _Size>& val) : base_class(val.begin()) {}
			template <size_t _Size>
			TLHNullableAnyPointer(const TNativeArrayReplacement<_Ty, _Size>& val) : base_class(val.cbegin()) {}

			//MSE_USING(TLHNullableAnyPointer, base_class);
			TLHNullableAnyPointer(const base_class& src) : base_class(src) {}
			TLHNullableAnyPointer() = default;
			TLHNullableAnyPointer(std::nullptr_t) : base_class(std::nullptr_t()) {}
			template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
				(!std::is_convertible<_TPointer1, TLHNullableAnyPointer>::value)
				&& (!std::is_base_of<base_class, _TPointer1>::value)
				&& (!std::is_same<_TPointer1, std::nullptr_t>::value)
				&& (!std::is_same<_TPointer1, NULL_t>::value)
				&& (!std::is_same<_TPointer1, ZERO_LITERAL_t>::value)
				&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
				&& mse::impl::is_potentially_not_xscope<_TPointer1>::value
			> MSE_IMPL_EIS >
			TLHNullableAnyPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)") : base_class(pointer) {}

			friend void swap(TLHNullableAnyPointer& first, TLHNullableAnyPointer& second) {
				swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			}

			/* Some of the comparision operators are currently inherited from base class. */

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
			friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				if (!bool(_Left_cref)) {
					if (!bool(_Right_cref)) {
						return true;
					}
					else {
						return false;
					}
				}
				else if (!bool(_Right_cref)) {
					return false;
				}
				return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
			}
			friend bool operator!=(const _Myt& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)

			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }

			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
#ifndef MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return bool(_Right_cref);
			}
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return bool(_Left_cref);
			}

			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return !bool(_Right_cref);
			}
#endif // !MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return !bool(_Left_cref);
			}
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

			TLHNullableAnyPointer& operator=(const TLHNullableAnyPointer& _Right_cref) {
				base_class::operator=(_Right_cref);
				return (*this);
			}

			operator bool() const {
				return base_class::operator bool();
			}

			void async_not_shareable_and_not_passable_tag() const {}

		private:
			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
		};

#ifndef MSE_HAS_CXX20
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator==(const _Ty2 lhs, const TLHNullableAnyPointer<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator!=(const _Ty2 lhs, const TLHNullableAnyPointer<_Ty>& rhs) { return rhs != lhs; }
#endif // !MSE_HAS_CXX20
	}
	namespace us {
		namespace lh {
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(_Ty* ptr) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*ptr)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(mse::us::TSaferPtrForLegacy<_Tx2>(ptr));
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(const _Ty& ptr) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*ptr)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(ptr);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(_Ty& ptr) {
				/* Note that, for example, in the case of mse::lh::TNativeArrayReplacement<>, its "operator*()" and "operator*() const"
				return different types. (Specifically, they return types that differ by a const qualifier.) */
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*ptr)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(ptr);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(_Ty&& ptr) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*ptr)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(MSE_FWD(ptr));
			}
		}
	}
	namespace lh {
		template <typename _Ty>
		class TXScopeLHNullableAnyPointer : public mse::TXScopeNullableAnyPointer<_Ty> {
		public:
			typedef mse::TXScopeNullableAnyPointer<_Ty> base_class;
			typedef TXScopeLHNullableAnyPointer _Myt;
			TXScopeLHNullableAnyPointer(const TXScopeLHNullableAnyPointer& src) = default;
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TXScopeLHNullableAnyPointer(_Ty2 val) : base_class(std::nullptr_t()) {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			template <size_t _Size>
			TXScopeLHNullableAnyPointer(TNativeArrayReplacement<_Ty, _Size>& val) : base_class(val.begin()) {}
			template <size_t _Size>
			TXScopeLHNullableAnyPointer(const TNativeArrayReplacement<_Ty, _Size>& val) : base_class(val.cbegin()) {}

			//MSE_USING(TXScopeLHNullableAnyPointer, base_class);
			TXScopeLHNullableAnyPointer() = default;
			TXScopeLHNullableAnyPointer(std::nullptr_t) : base_class(std::nullptr_t()) {}
			template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<
				(!std::is_convertible<_TPointer1, TXScopeLHNullableAnyPointer>::value)
				&& (!std::is_base_of<base_class, _TPointer1>::value)
				&& (!std::is_same<_TPointer1, std::nullptr_t>::value)
				&& (!std::is_same<_TPointer1, NULL_t>::value)
				&& (!std::is_same<_TPointer1, ZERO_LITERAL_t>::value)
				&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
			> MSE_IMPL_EIS >
			TXScopeLHNullableAnyPointer(const _TPointer1& pointer) : base_class(pointer) {}

			friend void swap(TXScopeLHNullableAnyPointer& first, TXScopeLHNullableAnyPointer& second) {
				swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			}

			/* Some of the comparision operators are currently inherited from base class. */

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
			friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				if (!bool(_Left_cref)) {
					if (!bool(_Right_cref)) {
						return true;
					}
					else {
						return false;
					}
				}
				else if (!bool(_Right_cref)) {
					return false;
				}
				return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
			}
			friend bool operator!=(const _Myt& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)

			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }

			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
#ifndef MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return bool(_Right_cref);
			}
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return bool(_Left_cref);
			}

			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return !bool(_Right_cref);
			}
#endif // !MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return !bool(_Left_cref);
			}
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

			operator bool() const {
				return base_class::operator bool();
			}

			void async_not_shareable_and_not_passable_tag() const {}

		private:
			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
		};

#ifndef MSE_HAS_CXX20
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator==(const _Ty2 lhs, const TXScopeLHNullableAnyPointer<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator!=(const _Ty2 lhs, const TXScopeLHNullableAnyPointer<_Ty>& rhs) { return rhs != lhs; }
#endif // !MSE_HAS_CXX20

		namespace us {
			namespace impl {
				template <typename _Ty>
				class TLHNullableAnyRandomAccessIteratorBase {
				public:
					typedef mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty> base_class;
					MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);
					typedef TLHNullableAnyRandomAccessIteratorBase _Myt;

					//MSE_USING(TLHNullableAnyRandomAccessIteratorBase, base_class);
					TLHNullableAnyRandomAccessIteratorBase() : m_iter() {}
					TLHNullableAnyRandomAccessIteratorBase(const std::nullptr_t& src) : m_iter() {}
					TLHNullableAnyRandomAccessIteratorBase(const base_class& src) : m_iter(src) {}
					TLHNullableAnyRandomAccessIteratorBase(_Ty arr[]) : m_iter(arr) {}

					TLHNullableAnyRandomAccessIteratorBase(const TLHNullableAnyRandomAccessIteratorBase& src) = default;

					template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
					TLHNullableAnyRandomAccessIteratorBase(_Ty2 val) : m_iter(std::nullptr_t()) {
						/* This constructor is just to support zero being used as a null pointer value. */
						assert(0 == val);
					}
					/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
					non-const reference. */
					template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value) || (std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
					TLHNullableAnyRandomAccessIteratorBase(TNativeArrayReplacement<_Ty2, _Size>& val) : m_iter(val.begin()) {}
					template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value)/* || (std::is_same<_Ty2, _Ty>::value)*/> MSE_IMPL_EIS >
					TLHNullableAnyRandomAccessIteratorBase(const TNativeArrayReplacement<_Ty2, _Size>& val) : m_iter(val.cbegin()) {}

					template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
						(!std::is_convertible<_TRandomAccessIterator1, TLHNullableAnyRandomAccessIteratorBase>::value)
						&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
						&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
						&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
						&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
						&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
					> MSE_IMPL_EIS >
					TLHNullableAnyRandomAccessIteratorBase(const _TRandomAccessIterator1& random_access_iterator) : m_iter(constructor_helper1(
						typename HasOrInheritsPlusEqualsOperator<_TRandomAccessIterator1>::type(), random_access_iterator)) {
					}

					friend void swap(TLHNullableAnyRandomAccessIteratorBase& first, TLHNullableAnyRandomAccessIteratorBase& second) {
						swap(first.contained_iter(), second.contained_iter());
					}

					_Ty& operator*() const {
						return *((*this).contained_iter());
					}
					_Ty* operator->() const {
						return (*this).contained_iter().operator->();
					}
					reference operator[](difference_type _Off) const {
						return (*this).contained_iter().operator[](_Off);
					}
					void operator +=(difference_type x) {
						(*this).contained_iter().operator+=(x);
					}
					void operator -=(difference_type x) { operator +=(-x); }
					TLHNullableAnyRandomAccessIteratorBase& operator ++() { operator +=(1); return (*this); }
					TLHNullableAnyRandomAccessIteratorBase operator ++(int) { auto _Tmp = (*this); operator +=(1); return _Tmp; }
					TLHNullableAnyRandomAccessIteratorBase& operator --() { operator -=(1); return (*this); }
					TLHNullableAnyRandomAccessIteratorBase operator --(int) { auto _Tmp = (*this); operator -=(1); return _Tmp; }

					TLHNullableAnyRandomAccessIteratorBase operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
					TLHNullableAnyRandomAccessIteratorBase operator-(difference_type n) const { return ((*this) + (-n)); }
					difference_type operator-(const TLHNullableAnyRandomAccessIteratorBase<_Ty>& _Right_cref) const {
						return ((*this).contained_iter() - _Right_cref.contained_iter());
					}

#if !defined(MSE_HAS_CXX17)
					MSE_IMPL_LESS_THAN_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_GREATER_THAN_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_GREATER_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_LESS_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);

					friend bool operator==(const TLHNullableAnyRandomAccessIteratorBase& lhs, const TLHNullableAnyRandomAccessIteratorBase& rhs) {
						return mse::lh::TXScopeLHNullableAnyPointer<_Ty>(lhs) == mse::lh::TXScopeLHNullableAnyPointer<_Ty>(rhs);
					}
					friend bool operator!=(const TLHNullableAnyRandomAccessIteratorBase& lhs, const TLHNullableAnyRandomAccessIteratorBase& rhs) { return !(lhs == rhs); }
#else // !defined(MSE_HAS_CXX17)
					/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
					MSE_IMPL_LESS_THAN_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_GREATER_THAN_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_GREATER_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);
					MSE_IMPL_LESS_THAN_OR_EQUAL_TO_OPERATOR_DECLARATION(TLHNullableAnyRandomAccessIteratorBase);

					template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
						(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					> MSE_IMPL_EIS >
					friend bool operator!=(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
						return !(_Left_cref == _Right_cref);
					}

					template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
						(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value || std::is_base_of<_Myt, TRHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TLHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TLHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TLHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
					template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
						(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty> >::value))
					> MSE_IMPL_EIS >
					friend std::strong_ordering operator<=>(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
						return (mse::us::impl::as_ref<_Myt>(_Left_cref).contained_iter() <=> _Right_cref);
					}

					template<typename TLHSIterator_ecwapt, typename TRHSIterator_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
						(std::is_base_of<_Myt, TLHSIterator_ecwapt>::value)
						&& MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSIterator_ecwapt) && (mse::impl::SupportsSubtraction_poly<TRHSIterator_ecwapt>::value)
						&& ((!mse::impl::SeemsToSupportEqualityComparisonWithArbitraryIteratorTypes_poly<mse::impl::first_or_placeholder_if_subclass_of_second_mseany<TRHSIterator_ecwapt, _Myt>, _Myt>::value)
							|| (mse::impl::first_is_or_is_subclass_of_any<TRHSIterator_ecwapt, _Myt, mse::us::impl::TNullableAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessIteratorBase<_Ty>, mse::us::impl::TAnyRandomAccessConstIteratorBase<_Ty>, TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, mse::us::impl::TAnyRandomAccessConstIteratorBase<mse::impl::remove_const_t<_Ty> > >::value))
					> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
					/* Note that, unlike with "conventional" iterators, this equality comparison operator may dereference the iterator if its
					value is not null. So, for example, "one past the last element" end markers are not valid values for this iterator type. */
					friend bool operator==(const TLHSIterator_ecwapt& _Left_cref, const TRHSIterator_ecwapt& _Right_cref) {
						bool null_and_not_null_flag = false;
						MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TLHSIterator_ecwapt>::value) {
							MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
								if (bool(_Right_cref) != bool(_Left_cref)) {
									null_and_not_null_flag = true;
								}
								else if (!bool(_Left_cref)) {
									/* If both sides are null, we'll treat the values as equivalent? */
									return true;
								}
							}
 else if (!bool(_Left_cref)) {
	 null_and_not_null_flag = true;
							}
						}
 else MSE_IF_CONSTEXPR(mse::impl::IsExplicitlyCastableToBool_pb<TRHSIterator_ecwapt>::value) {
	 if (!bool(_Right_cref)) {
		 null_and_not_null_flag = true;
	 }
						}
						if (null_and_not_null_flag) {
							return false;
						}
						return (std::addressof(*_Right_cref) == std::addressof(*_Left_cref));
					}
#endif // !defined(MSE_HAS_CXX17)

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSC_HAS_CXX17))
					friend bool operator!=(const std::nullptr_t& _Left_cref, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return !(_Left_cref == _Right_cref);
					}
					friend bool operator!=(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, const std::nullptr_t& _Right_cref) {
						return !(_Left_cref == _Right_cref);
					}

					friend bool operator==(const std::nullptr_t&, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return !bool(_Right_cref);
					}
					friend bool operator==(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, const std::nullptr_t&) {
						return !bool(_Left_cref);
					}
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSC_HAS_CXX17))
#ifndef MSE_HAS_CXX20
					template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TRHS>::value)> MSE_IMPL_EIS >
					friend bool operator!=(const std::nullptr_t& _Left_cref, const TRHS& _Right_cref) {
						return bool(_Right_cref);
					}
					template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TLHS>::value)> MSE_IMPL_EIS >
					friend bool operator!=(const TLHS& _Left_cref, const std::nullptr_t& _Right_cref) {
						return bool(_Left_cref);
					}

					template<typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TRHS>::value)> MSE_IMPL_EIS >
					friend bool operator==(const std::nullptr_t&, const TRHS& _Right_cref) {
						return !bool(_Right_cref);
					}
#endif // !MSE_HAS_CXX20
					template<typename TLHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TLHS>::value)> MSE_IMPL_EIS >
					friend bool operator==(const TLHS& _Left_cref, const std::nullptr_t&) {
						return !bool(_Left_cref);
					}
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (defined(_MSC_VER) && !defined(MSC_HAS_CXX17))

#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
					friend bool operator!=(const NULL_t& _Left_cref, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return !(_Left_cref == _Right_cref);
					}
					friend bool operator!=(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, const NULL_t& _Right_cref) {
						return !(_Left_cref == _Right_cref);
					}

					friend bool operator==(const NULL_t&, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return !bool(_Right_cref);
					}
					friend bool operator==(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, const NULL_t&) {
						return !bool(_Left_cref);
					}
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY)
					/* We use a templated equality comparison operator to avoid potentially competing with the base class equality comparison with nullptr operator. */
#ifndef MSE_HAS_CXX20
					template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value || std::is_same<ZERO_LITERAL_t, TLHS>::value) && std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TRHS>::value> MSE_IMPL_EIS >
					friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
						assert(0 == _Left_cref);
						return bool(_Right_cref);
					}
					template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TLHS>::value && (std::is_same<NULL_t, TRHS>::value || std::is_same<ZERO_LITERAL_t, TRHS>::value)> MSE_IMPL_EIS >
					friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
						assert(0 == _Right_cref);
						return bool(_Left_cref);
					}

					template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value || std::is_same<ZERO_LITERAL_t, TLHS>::value) && std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TRHS>::value> MSE_IMPL_EIS >
					friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
						assert(0 == _Left_cref);
						return !bool(_Right_cref);
					}
#endif // !MSE_HAS_CXX20
					template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<TLHNullableAnyRandomAccessIteratorBase, TLHS>::value && (std::is_same<NULL_t, TRHS>::value || std::is_same<ZERO_LITERAL_t, TRHS>::value)> MSE_IMPL_EIS >
					friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
						assert(0 == _Right_cref);
						return !bool(_Left_cref);
					}
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY)

#if 0
#ifndef MSE_HAS_CXX20
					template <size_t _Size>
					friend bool operator!=(const TNativeArrayReplacement<_Ty, _Size>& _Left_cref, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return !(TLHNullableAnyRandomAccessIteratorBase(_Left_cref) == _Right_cref);
					}
					template <size_t _Size>
					friend bool operator!=(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, const TNativeArrayReplacement<_Ty, _Size>& _Right_cref) {
						return !(_Left_cref == TLHNullableAnyRandomAccessIteratorBase(_Right_cref));
					}

					template <size_t _Size>
					friend bool operator==(const TNativeArrayReplacement<_Ty, _Size>& _Left_cref, const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						return (TLHNullableAnyRandomAccessIteratorBase(_Left_cref) == _Right_cref);
					}
#endif // !MSE_HAS_CXX20
					template <size_t _Size>
					friend bool operator==(const TLHNullableAnyRandomAccessIteratorBase& _Left_cref, TNativeArrayReplacement<_Ty, _Size>& _Right_cref) {
						return (_Left_cref == TLHNullableAnyRandomAccessIteratorBase(_Right_cref));
					}
#endif // 0


					TLHNullableAnyRandomAccessIteratorBase& operator=(const TLHNullableAnyRandomAccessIteratorBase& _Right_cref) {
						(*this).contained_iter() = _Right_cref.contained_iter();
						return (*this);
					}

					explicit operator bool() const {
						return bool((*this).contained_iter());
					}

					//MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TLHNullableAnyRandomAccessIteratorBase);

					void async_not_shareable_and_not_passable_tag() const {}

					template<class T, class EqualTo>
					struct HasOrInheritsPlusEqualsOperator_impl
					{
						template<class U, class V>
						static auto test(U*) -> decltype(std::declval<U>() += 1, std::declval<V>() += 1, bool(true));
						template<typename, typename>
						static auto test(...) -> std::false_type;

						using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
					};
					template<class T, class EqualTo = T>
					struct HasOrInheritsPlusEqualsOperator : HasOrInheritsPlusEqualsOperator_impl<
						mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

				private:

					template <typename _TRandomAccessIterator1>
					static auto constructor_helper2(std::true_type, const _TRandomAccessIterator1& random_access_iterator) {
						/* The element type of this iterator and the element type of the random_access_iterator parameter are both
						intergral types of the same size, but not the same type. (Generally, one is signed and the other is
						not/un-signed.) Here we allow the "reinterpretation" of random_access_iterator as being an iterator that
						points to elements of type _Ty. This is technically not "type safe", but it doesn't really compromise
						"memory safety". At some point we may want to make it so that this functionality is only accessible by the
						unsafe_cast<>() function. */
						typedef mse::impl::remove_reference_t<decltype(*random_access_iterator)> _Ty2;
						auto temp = TLHNullableAnyRandomAccessIteratorBase<_Ty2>(random_access_iterator);
						return reinterpret_cast<TLHNullableAnyRandomAccessIteratorBase&>(temp);
					}
					template <typename _TRandomAccessIterator1>
					static auto constructor_helper2(std::false_type, const _TRandomAccessIterator1& random_access_iterator) {
						return random_access_iterator;
					}

					template <typename _TRandomAccessIterator1>
					static auto constructor_helper1(std::true_type, const _TRandomAccessIterator1& random_access_iterator) {
						typedef mse::impl::remove_reference_t<decltype(*random_access_iterator)> _Ty2;
						typedef mse::impl::remove_const_t<_Ty> _ncTy;
						typedef mse::impl::remove_const_t<_Ty2> _ncTy2;
						typedef typename mse::impl::conjunction<std::is_integral<_Ty>, std::is_integral<_Ty2>
							, mse::impl::negation< std::is_same<_ncTy, _ncTy2 > >
							, std::integral_constant<bool, sizeof(_Ty) == sizeof(_Ty2)> >::type res1_t;
						/* res1_t is std::true_type if the element type of this iterator and the element type of the random_access_iterator
						parameter are both intergral types of the same size, but not the same type. (Generally, one is signed and the
						other is not/un-signed.) */
						return constructor_helper2(res1_t(), random_access_iterator);
					}
					template <typename _TPointer1>
					static auto constructor_helper1(std::false_type, const _TPointer1& pointer1) {
						/* For legacy compatibility we're going to support interpreting pointers as iterators to an array of size 1. */
						auto lh_any_pointer1 = TLHNullableAnyPointer<_Ty>(pointer1);
						/* mse::nii_array<_Ty, 1> has been designed to be "bit identical" to std::array<_Ty, 1> which should be
						"bit identical" to _Ty[1] which should be "bit identical" to _Ty, so it should be safe reinterpret a
						pointer to a _Ty as a pointer to a mse::nii_array<_Ty, 1>.*/
						auto lh_any_pointer2 = *reinterpret_cast<const TLHNullableAnyPointer<mse::nii_array<_Ty, 1> >*>(std::addressof(lh_any_pointer1));
						return mse::make_begin_iterator(lh_any_pointer2);
					}

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					const base_class& contained_iter() const& { return (*this).m_iter; }
					//const base_class& contained_iter() const&& { return (*this).m_iter; }
					base_class& contained_iter()& { return (*this).m_iter; }
					base_class&& contained_iter()&& { return std::move(*this).m_iter; }

					base_class m_iter;

					template <typename _Ty2>
					friend class TXScopeLHNullableAnyRandomAccessIterator;
				};
			}
		}

		template <typename _Ty>
		class TXScopeLHNullableAnyRandomAccessIterator;

		/* Note: MSE_LH_ARRAY_ITERATOR_TYPE() / mse::lh::TLHNullableAnyRandomAccessIterator<> is intended to replace legacy raw
		pointers being used as array/buffer iterators, and as such is different from "conventional" iterators in that "one past
		the last element" end markers are not valid values. Valid values are either null, or point to a valid object. */
		template <typename _Ty>
		class TLHNullableAnyRandomAccessIterator : public mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Ty> {
		public:
			typedef mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Ty> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);
			typedef TLHNullableAnyRandomAccessIterator _Myt;

			//MSE_USING(TLHNullableAnyRandomAccessIterator, base_class);
			TLHNullableAnyRandomAccessIterator() : base_class() {}
			TLHNullableAnyRandomAccessIterator(const std::nullptr_t& src) : base_class() {}
			TLHNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
			TLHNullableAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}

			TLHNullableAnyRandomAccessIterator(const TLHNullableAnyRandomAccessIterator& src) = default;

			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(_Ty2 val) : base_class(std::nullptr_t()) {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
			non-const reference. */
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value) || (std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(val.begin()) {}
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value)/* || (std::is_same<_Ty2, _Ty>::value)*/> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(val.cbegin()) {}

			template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
				(!std::is_convertible<_TRandomAccessIterator1, TLHNullableAnyRandomAccessIterator>::value)
				&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
				&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
				&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
				&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
				&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
				&& (mse::impl::is_potentially_not_xscope<_TRandomAccessIterator1>::value)
			> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator)  MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRandomAccessIterator1)")
				: base_class(random_access_iterator)
			{
				mse::impl::T_valid_if_not_an_xscope_type<_TRandomAccessIterator1>();
			}

			friend void swap(TLHNullableAnyRandomAccessIterator& first, TLHNullableAnyRandomAccessIterator& second) {
				swap(first.contained_iter(), second.contained_iter());
			}

			TLHNullableAnyRandomAccessIterator& operator=(const TLHNullableAnyRandomAccessIterator& _Right_cref) {
				(*this).contained_iter() = _Right_cref.contained_iter();
				return (*this);
			}

			MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TLHNullableAnyRandomAccessIterator);

			void async_not_shareable_and_not_passable_tag() const {}

		private:

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			const base_class& contained_iter() const& { return (*this); }
			//const base_class& contained_iter() const&& { return (*this); }
			base_class& contained_iter()& { return (*this); }
			base_class&& contained_iter()&& { return std::move(*this); }
		};
	}
	namespace us {
		namespace lh {
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty* iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*iter)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Tx2>(iter));
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(const _Ty& iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*iter)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(iter);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty& iter) {
				/* Note that, for example, in the case of mse::lh::TNativeArrayReplacement<>, its "operator*()" and "operator*() const"
				return different types. (Specifically, they return types that differ by a const qualifier.) */
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*iter)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(iter);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty&& iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::remove_reference_t<decltype(*iter)>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(MSE_FWD(iter));
			}
		}
	}
	namespace lh {

		/* Note: mse::lh::TXScopeLHNullableAnyRandomAccessIterator<> is intended to replace legacy raw pointers being used as 
		array/buffer iterators, and as such is different from "conventional" iterators in that "one past the last element" end 
		markers are not valid values. Valid values are either null, or point to a valid object. */
		template <typename _Ty>
		class TXScopeLHNullableAnyRandomAccessIterator : public mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Ty> {
		public:
			typedef mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Ty> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			//MSE_USING(TXScopeLHNullableAnyRandomAccessIterator, base_class);
			TXScopeLHNullableAnyRandomAccessIterator() : base_class() {}
			TXScopeLHNullableAnyRandomAccessIterator(const std::nullptr_t& src) : base_class() {}
			TXScopeLHNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
			TXScopeLHNullableAnyRandomAccessIterator(_Ty arr[]) : base_class(arr) {}

			TXScopeLHNullableAnyRandomAccessIterator(const TXScopeLHNullableAnyRandomAccessIterator& src) = default;

			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(_Ty2 val) : base_class(std::nullptr_t()) {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
			non-const reference. */
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value) || (std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(val.begin()) {}
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<const _Ty2, _Ty>::value)/* || (std::is_same<_Ty2, _Ty>::value)*/> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(const TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(val.cbegin()) {}

			template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<
				(!std::is_convertible<_TRandomAccessIterator1, TXScopeLHNullableAnyRandomAccessIterator>::value)
				&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
				&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
				&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
				&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
				&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
			> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) : base_class(random_access_iterator) {}

			friend void swap(TXScopeLHNullableAnyRandomAccessIterator& first, TXScopeLHNullableAnyRandomAccessIterator& second) {
				swap(first.contained_iter(), second.contained_iter());
			}

			MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeLHNullableAnyRandomAccessIterator);

			void async_not_shareable_and_not_passable_tag() const {}

		private:

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			const base_class& contained_iter() const& { return (*this); }
			//const base_class& contained_iter() const&& { return (*this); }
			base_class& contained_iter()& { return (*this); }
			base_class&& contained_iter()&& { return std::move(*this); }
		};

		template <typename _Ty>
		using TStrongTargetVector = 
#ifndef MSE_LEGACYHELPERS_DISABLED
			mse::stnii_vector<mse::impl::remove_const_t<_Ty> >
#else //MSE_LEGACYHELPERS_DISABLED
			std::vector<mse::impl::remove_const_t<_Ty> >
#endif //MSE_LEGACYHELPERS_DISABLED
			;

		/* This data type was motivated by the need for a direct substitute for native pointers targeting dynamically
		allocated (native) arrays, which can kind of play a dual role as a reference to the array object and/or as an
		iterator. */
		template <typename _Ty>
		class TStrongVectorIterator : public mse::TRAIterator<mse::TRefCountingPointer<TStrongTargetVector<_Ty> > > {
		public:
			typedef mse::TRAIterator<mse::TRefCountingPointer<TStrongTargetVector<_Ty> > > base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TStrongVectorIterator() = default;
			TStrongVectorIterator(const std::nullptr_t& src) : TStrongVectorIterator() {}
			TStrongVectorIterator(const TStrongVectorIterator& src) = default;
			TStrongVectorIterator(TStrongVectorIterator&& src) = default;
			TStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(_Ilist), 0) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, base_class>::value)
				|| (std::is_same<_Ty2, mse::TXScopeRAIterator<mse::TRefCountingPointer<TStrongTargetVector<_Ty>>> >::value)
				> MSE_IMPL_EIS >
			TStrongVectorIterator(const _Ty2& src) : base_class(src) {}
			/* turns out that size_type and NULL_t could be the same type on some platforms */
			//explicit TStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(_N), 0) {}
			explicit TStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(_N, _V), 0) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TStrongVectorIterator(_Ty2 val) : TStrongVectorIterator() {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			/*
			template <class... Args>
			TStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			friend bool operator==(const TStrongVectorIterator& _Left_cref, const TStrongVectorIterator& _Right_cref) {
				if (!(_Left_cref.target_container_ptr() == _Right_cref.target_container_ptr())) {
					return false;
				}
				return (mse::us::impl::as_ref<base_class>(_Left_cref) == mse::us::impl::as_ref<base_class>(_Right_cref));
			}
#ifndef MSE_HAS_CXX20
			friend bool operator!=(const TStrongVectorIterator& _Left_cref, const TStrongVectorIterator& _Right_cref) { return !(_Left_cref == _Right_cref); }
#endif // !MSE_HAS_CXX20

			TStrongVectorIterator& operator=(const TStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TStrongVectorIterator);

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TStrongVectorIterator make(Args&&... args) {
				return TStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <typename _Ty>
		bool operator==(const NULL_t lhs, const TStrongVectorIterator<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty>
		bool operator!=(const NULL_t lhs, const TStrongVectorIterator<_Ty>& rhs) { return rhs != lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator==(const _Ty2 lhs, const TStrongVectorIterator<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator!=(const _Ty2 lhs, const TStrongVectorIterator<_Ty>& rhs) { return rhs != lhs; }

		template <class X, class... Args>
		TStrongVectorIterator<X> make_strong_vector_iterator(Args&&... args) {
			return TStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		template <typename _Ty>
		class TXScopeStrongVectorIterator : public mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> {
		public:
			typedef mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TXScopeStrongVectorIterator() = default;
			TXScopeStrongVectorIterator(const std::nullptr_t& src) : TXScopeStrongVectorIterator() {}
			TXScopeStrongVectorIterator(const TXScopeStrongVectorIterator& src) = default;
			TXScopeStrongVectorIterator(TXScopeStrongVectorIterator&& src) = default;
			TXScopeStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_Ilist), 0) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, base_class>::value)
				|| (std::is_same<_Ty2, mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> >::value)
				> MSE_IMPL_EIS >
			TXScopeStrongVectorIterator(const _Ty2& src) : base_class(src) {}
			/* turns out that size_type and NULL_t could be the same type on some platforms */
			//explicit TXScopeStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N), 0) {}
			explicit TXScopeStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N, _V), 0) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TXScopeStrongVectorIterator(_Ty2 val) : TXScopeStrongVectorIterator() {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			/*
			template <class... Args>
			TXScopeStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			friend bool operator==(const TXScopeStrongVectorIterator& _Left_cref, const TXScopeStrongVectorIterator& _Right_cref) {
				if (!(_Left_cref.target_container_ptr() == _Right_cref.target_container_ptr())) {
					return false;
				}
				return (mse::us::impl::as_ref<base_class>(_Left_cref) == mse::us::impl::as_ref<base_class>(_Right_cref));
			}
#ifndef MSE_HAS_CXX20
			friend bool operator!=(const TXScopeStrongVectorIterator& _Left_cref, const TXScopeStrongVectorIterator& _Right_cref) { return !(_Left_cref == _Right_cref); }
#endif // !MSE_HAS_CXX20

			TXScopeStrongVectorIterator& operator=(const TXScopeStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeStrongVectorIterator);

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TXScopeStrongVectorIterator make(Args&&... args) {
				return TXScopeStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <typename _Ty>
		bool operator==(const NULL_t lhs, const TXScopeStrongVectorIterator<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty>
		bool operator!=(const NULL_t lhs, const TXScopeStrongVectorIterator<_Ty>& rhs) { return rhs != lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator==(const _Ty2 lhs, const TXScopeStrongVectorIterator<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator!=(const _Ty2 lhs, const TXScopeStrongVectorIterator<_Ty>& rhs) { return rhs != lhs; }

		template <class X, class... Args>
		TXScopeStrongVectorIterator<X> make_xscope_strong_vector_iterator(Args&&... args) {
			return TXScopeStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		/* TXScopeStrongVectorIterator<> does not directly convert to mse::rsv::TFParam<mse::TXScopeCSSSXSTERandomAccessIterator<> >.
		But the following function can be used to obtain a "locking" scope iterator that does. */
		template <class X>
		auto make_xscope_locking_vector_iterator(const TXScopeStrongVectorIterator<X>& xs_strong_iter) {
			auto retval = mse::make_xscope_begin_iterator(mse::us::unsafe_make_xscope_pointer_to(*xs_strong_iter.target_container_ptr()));
			retval += xs_strong_iter.position();
			return retval;
		}
		template <class X>
		auto make_xscope_locking_vector_iterator(TXScopeStrongVectorIterator<X>&& xs_strong_iter) = delete;

		template <class TArray, class TIndex>
		auto address_of_array_element_replacement(TArray& array1, TIndex index) {
			return TLHNullableAnyRandomAccessIterator<mse::impl::remove_reference_t<decltype(*(std::begin(array1) + index))> >(std::begin(array1) + index);
		}

		/* deprecated aliases */
		template <typename _Ty>
		using TIPointerWithBundledVector = TStrongVectorIterator<_Ty>;
		template <class X, class... Args>
		TIPointerWithBundledVector<X> make_ipointer_with_bundled_vector(Args&&... args) {
			return make_string_vector_iterator(std::forward<Args>(args)...);
		}

		namespace impl {
			template <class _Ty1, class _Ty2>
			struct add_const_if_second_is_const : std::conditional<std::is_const<_Ty2>::value, std::add_const_t<_Ty1>, _Ty1> {};

			template <class _Ty>
			struct const_preserving_decay : add_const_if_second_is_const<mse::impl::decay_t<_Ty>, _Ty> {};
			template <class _Ty> using const_preserving_decay_t = typename const_preserving_decay<_Ty>::type;
		}

		template <typename _udTy, size_t _Size>
		class TNativeArrayReplacement : public mse::mstd::array<impl::const_preserving_decay_t<_udTy>, _Size> {
		public:
			typedef mse::mstd::array<impl::const_preserving_decay_t<_udTy>, _Size> base_class;
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			using base_class::base_class;

			TNativeArrayReplacement() {}
			/* Technically, this constructor should only be enabled for 'char' types to support initialization from string literals. */
			template <size_t _Size2, MSE_IMPL_EIP mse::impl::enable_if_t<(_Size2 <= _Size)> MSE_IMPL_EIS >
			TNativeArrayReplacement(_Ty const (&arr1)[_Size2]) {
				typedef mse::impl::remove_const_t<_Ty> _ncTy;
				for (size_t i = 0; i < _Size2; i += 1) {
					auto nc_ptr = const_cast<_ncTy*>(std::addressof((*this)[i]));
					*nc_ptr = arr1[i];
				}
				for (size_t i = _Size2; i < _Size; i += 1) {
					auto nc_ptr = const_cast<_ncTy*>(std::addressof((*this)[i]));
					*nc_ptr = _Ty();
				}
			}

			operator mse::TNullableAnyRandomAccessIterator<_Ty>() { return base_class::begin(); }
			explicit operator mse::TXScopeNullableAnyRandomAccessIterator<_Ty>() { return base_class::begin(); }
			explicit operator mse::TAnyRandomAccessIterator<_Ty>() { return base_class::begin(); }
			explicit operator mse::TXScopeAnyRandomAccessIterator<_Ty>() { return base_class::begin(); }
			operator mse::TAnyRandomAccessConstIterator<_Ty>() const { return base_class::cbegin(); }
			explicit operator mse::TXScopeAnyRandomAccessConstIterator<_Ty>() const { return base_class::cbegin(); }
			operator typename mse::mstd::array<_Ty, _Size>::iterator() {
				return base_class::begin();
			}
			template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value)> MSE_IMPL_EIS >
			operator typename mse::mstd::array<_Ty, _Size>::const_iterator() {
				return base_class::cbegin();
			}
			template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			operator mse::TNullableAnyRandomAccessIterator<const _Ty>() {
				return base_class::begin();
			}
			typename base_class::iterator operator+(typename base_class::difference_type n) { return base_class::begin() + n; }
			typename base_class::iterator operator-(typename base_class::difference_type n) { return base_class::begin() - n; }
			typename base_class::difference_type operator-(const typename base_class::iterator& _Right_cref) const { return base_class::begin() - _Right_cref; }
			typename base_class::const_iterator operator+(typename base_class::difference_type n) const { return base_class::cbegin() + n; }
			typename base_class::const_iterator operator-(typename base_class::difference_type n) const { return base_class::cbegin() - n; }
			template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value)> MSE_IMPL_EIS >
			typename base_class::difference_type operator-(const typename base_class::const_iterator& _Right_cref) const { return base_class::cbegin() - _Right_cref; }

			typename base_class::const_reference operator*() const { return (*this).at(0); }
			typename base_class::reference operator*() { return (*this).at(0); }
			typename base_class::const_pointer operator->() const { return std::addressof(operator*()); }
			typename base_class::pointer operator->() { return std::addressof(operator*()); }

#ifdef MSE_LEGACYHELPERS_DISABLED
			TNativeArrayReplacement(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::nii_array<_Ty, _Size>(_Ilist)) {}
			TNativeArrayReplacement(const base_class& x) : base_class(x) {}
#endif // MSE_LEGACYHELPERS_DISABLED

		};

#define MSE_UNSAFE_LH_LARGE_FIXED_ARRAY_INITIALIZER_HELPER(element_type, size)  std::array<element_type, size>

		template<class _Fty>
		class TNativeFunctionPointerReplacement : public mse::mstd::function<_Fty> {
		public:
			typedef mse::mstd::function<_Fty> base_class;
			TNativeFunctionPointerReplacement() noexcept : base_class() {}
			TNativeFunctionPointerReplacement(std::nullptr_t) noexcept : base_class(nullptr) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TNativeFunctionPointerReplacement(_Ty2 val) : base_class(std::nullptr_t()) {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}

			template <typename _Fty2, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_convertible<const _Fty2*, const TNativeFunctionPointerReplacement*>::value)
				&& (!std::is_convertible<_Fty2, std::nullptr_t>::value) && (!std::is_same<_Fty2, int>::value) 
				&& (!std::is_same<_Fty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Fty2, NULL_t>::value) && (mse::impl::is_potentially_not_xscope<_Fty2>::value)
				&& (std::is_constructible<base_class, _Fty2>::value)
			> MSE_IMPL_EIS >
			TNativeFunctionPointerReplacement(const _Fty2& func) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Fty2)") : base_class(func) {
				mse::impl::T_valid_if_not_an_xscope_type<_Fty2>();
			}
			template <typename _Fty2, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_convertible<const _Fty2*, const TNativeFunctionPointerReplacement*>::value)
				&& (!std::is_convertible<_Fty2, std::nullptr_t>::value) && (!std::is_same<_Fty2, int>::value) 
				&& (!std::is_same<_Fty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Fty2, NULL_t>::value) && (mse::impl::is_potentially_not_xscope<_Fty2>::value)
				&& (std::is_constructible<base_class, _Fty2>::value)
			> MSE_IMPL_EIS >
			TNativeFunctionPointerReplacement(_Fty2&& func) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Fty2)") : base_class(MSE_FWD(func)) {
				mse::impl::T_valid_if_not_an_xscope_type<_Fty2>();
			}

			base_class const& operator*() const { return (*this); }
			auto operator->() const { return std::addressof(operator*()); }
		};


		namespace impl {
			template<class _TPtr>
			class CAllocF {
			public:
				static void free(_TPtr& ptr) {
					ptr = nullptr;
				}
				static void allocate(_TPtr& ptr, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype(*ptr)> target_t;
					if (0 == num_bytes) {
						ptr = nullptr;
					}
					else if (sizeof(target_t) == num_bytes) {
						ptr = mse::make_refcounting<target_t>();
					}
					else {
						assert(false);
						ptr = mse::make_refcounting<target_t>();
						//MSE_THROW(std::bad_alloc("the given allocation size is not supported for this pointer type - CAllocF<_TPtr>::allocate()"));
					}
				}
				//static void reallocate(_TPtr& ptr, size_t num_bytes);
			};
			template<class _Ty>
			class CAllocF<_Ty*> {
			public:
				static void free(_Ty* ptr) {
					::free(ptr);
				}
				static void allocate(_Ty*& ptr, size_t num_bytes) {
					ptr = ::malloc(num_bytes);
				}
				static void reallocate(_Ty*& ptr, size_t num_bytes) {
					ptr = ::realloc(ptr, num_bytes);
				}
			};
			template<class _Ty>
			void free(_Ty* ptr) { CAllocF<_Ty*>::free(ptr); }
			template<class _Ty>
			void allocate(_Ty*& ptr, size_t num_bytes) { CAllocF<_Ty*>::allocate(ptr, num_bytes); }
			template<class _Ty>
			void reallocate(_Ty*& ptr, size_t num_bytes) { CAllocF<_Ty*>::reallocate(ptr, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TStrongVectorIterator<_Ty>> {
			public:
				static void free(mse::lh::TStrongVectorIterator<_Ty>& ptr) {
					ptr = mse::lh::TStrongVectorIterator<_Ty>();
				}
				static void allocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					tmp.resize(num_bytes / sizeof(_Ty));
					ptr = tmp;
				}
				static void reallocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
					ptr.resize(num_bytes / sizeof(_Ty));
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TStrongVectorIterator<_Ty>& ptr) { CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::free(ptr); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::allocate(ptr, num_bytes); }
			template<class _Ty>
			void reallocate_overloaded(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::reallocate(ptr, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
			public:
				static void free(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr) {
					ptr = mse::lh::TStrongVectorIterator<_Ty>();
				}
				static void allocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					tmp.resize(num_bytes / sizeof(_Ty));
					ptr = tmp;
				}
				//static void reallocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes);
			};
			template<class _Ty>
			void free_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr) { CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::free(ptr); }
			template<class _Ty>
			void allocate_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::allocate(ptr, num_bytes); }
			template<class _Ty>
			void reallocate_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::reallocate(ptr, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>> {
			public:
				static void free(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr) {
					ptr = mse::lh::TStrongVectorIterator<_Ty>();
				}
				static void allocate(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					tmp.resize(num_bytes / sizeof(_Ty));
					ptr = tmp;
				}
				//static void reallocate(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes);
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr) { CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::free(ptr); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::allocate(ptr, num_bytes); }
			//template<class _Ty>
			//void reallocate_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::reallocate(ptr, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>> {
			public:
				static void free(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr) {
					ptr = mse::lh::TXScopeStrongVectorIterator<_Ty>();
				}
				static void allocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
					mse::lh::TXScopeStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
					ptr = tmp;
				}
				static void reallocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
					ptr.resize(num_bytes / sizeof(_Ty));
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr) { CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::free(ptr); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::allocate(ptr, num_bytes); }
			template<class _Ty>
			void reallocate_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) { CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::reallocate(ptr, num_bytes); }

			template<class T, class EqualTo>
			struct IsSupportedByAllocateOverloaded_impl
			{
				template<class U, class V>
				static auto test(U* u) -> decltype(allocate_overloaded(*u, 5), std::declval<V>(), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct IsSupportedByAllocateOverloaded : IsSupportedByAllocateOverloaded_impl<
				mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

			template<class _Ty>
			auto free_helper1(std::true_type, _Ty& ptr) {
				return free_overloaded(ptr);
			}
			template<class _Ty>
			auto free_helper1(std::false_type, _Ty& ptr) {
				return CAllocF<_Ty>::free(ptr);
			}
			template<class _Ty>
			auto allocate_helper1(std::true_type, _Ty& ptr, size_t num_bytes) {
				return allocate_overloaded(ptr, num_bytes);
			}
			template<class _Ty>
			auto allocate_helper1(std::false_type, _Ty& ptr, size_t num_bytes) {
				return CAllocF<_Ty>::allocate(ptr, num_bytes);
			}
			template<class _Ty>
			auto reallocate_helper1(std::true_type, _Ty& ptr, size_t num_bytes) {
				return reallocate_overloaded(ptr, num_bytes);
			}
			template<class _Ty>
			auto reallocate_helper1(std::false_type, _Ty& ptr, size_t num_bytes) {
				return CAllocF<_Ty>::reallocate(ptr, num_bytes);
			}
		}

		template<class _TPointer>
		_TPointer allocate() {
			_TPointer ptr;
			auto num_bytes = sizeof(decltype(*ptr));
			MSE_TRY{
				impl::allocate_helper1(typename impl::IsSupportedByAllocateOverloaded<_TPointer>::type(), ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TPointer();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate_dyn_array1(size_t num_bytes) {
			_TDynArrayIter ptr;
			MSE_TRY{
				impl::allocate_helper1(typename impl::IsSupportedByAllocateOverloaded<_TDynArrayIter>::type(), ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter reallocate(const _TDynArrayIter& ptr2, size_t num_bytes) {
			_TDynArrayIter ptr = ptr2;
			MSE_TRY{
				impl::reallocate_helper1(typename impl::IsSupportedByAllocateOverloaded<_TDynArrayIter>::type(), ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate(_TDynArrayIter& ptr, size_t num_bytes) {
			MSE_TRY{
				impl::allocate_helper1(typename impl::IsSupportedByAllocateOverloaded<_TDynArrayIter>::type(), ptr, num_bytes);
			}
			MSE_CATCH_ANY{
				return _TDynArrayIter();
			}
			return ptr;
		}
		template<class _TDynArrayIter>
		void free(_TDynArrayIter& ptr) {
			impl::free_helper1(typename impl::IsSupportedByAllocateOverloaded<_TDynArrayIter>::type(), ptr);
		}

		namespace impl {
			/* Memory safe approximation of fread(). */
			template<class _TIter>
			size_t fread(_TIter const& ptr, size_t size, size_t count, FILE* stream) {
				typedef mse::impl::remove_reference_t<decltype((ptr)[0])> element_t;
				thread_local std::vector<unsigned char> v;
				v.resize(size * count);
				auto num_bytes_read = ::fread(v.data(), size, count, stream);
				auto num_items_read = num_bytes_read / sizeof(element_t);
				size_t uc_index = 0;
				size_t element_index = 0;
				for (; element_index < num_items_read; uc_index += sizeof(element_t), element_index += 1) {
					unsigned char* uc_ptr = &(v[uc_index]);
					if (false) {
						element_t* element_ptr = reinterpret_cast<element_t*>(uc_ptr);
						ptr[element_index] = (*element_ptr);
					}
					else {
						long long int adjusted_value = 0;
						static_assert(sizeof(adjusted_value) >= sizeof(element_t), "The (memory safe) lh::fread() function does not support element types larger than `long long int`. ");
						for (size_t i = 0; i < sizeof(element_t); i += 1) {
							adjusted_value |= ((uc_ptr[i]) << (8 * ((sizeof(element_t) - 1) - i)));
						}
						/* Only element types that are (or can be constructed from) integral types are supported. */
						ptr[element_index] = element_t(adjusted_value);
					}
				}
				v.resize(0);
				v.shrink_to_fit();
				return num_bytes_read;
			}
		}
		/* Memory safe approximation of fread(). */
		template<class _TIter>
		size_t fread(_TIter const& ptr, size_t size, size_t count, FILE* stream) {
			return impl::fread(ptr, size, count, stream);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement>
		size_t fread(_TElement* ptr, size_t size, size_t count, FILE* stream) {
			return impl::fread(ptr, size, count, stream);
		}
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size>
		size_t fread(TNativeArrayReplacement<_udTy, _Size>& nar, size_t size, size_t count, FILE* stream) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			typename mse::mstd::array<_Ty, _Size>::iterator ptr = nar;
			return fread(ptr, size, count, stream);
		}

		/* Memory safe approximation of fwrite(). */
		template<class _TIter>
		size_t fwrite(_TIter const& ptr, size_t size, size_t count, FILE* stream) {
			typedef mse::impl::remove_reference_t<decltype((ptr)[0])> element_t;
			auto num_items_to_write = size * count / sizeof(element_t);
			thread_local std::vector<unsigned char> v;
			v.resize(size * count);
			//assert(num_items_to_write * sizeof(element_t) == size * count);
			size_t uc_index = 0;
			size_t element_index = 0;
			for (; element_index < num_items_to_write; uc_index += sizeof(element_t), element_index += 1) {
				unsigned char* uc_ptr = &(v[uc_index]);
				if (false) {
					typedef mse::impl::remove_const_t<element_t> non_const_element_t;
					non_const_element_t* element_ptr = reinterpret_cast<non_const_element_t*>(uc_ptr);
					(*element_ptr) = ptr[element_index];
				}
				else {
					/* Only element types that are (or convert to) integral types are supported. */
					long long int value(ptr[element_index]);
					static_assert(sizeof(value) >= sizeof(element_t), "The (memory safe) lh::write() function does not support element types larger than `long long int`. ");
					for (size_t i = 0; i < sizeof(element_t); i += 1) {
						uc_ptr[i] = ((value >> (8 * ((sizeof(element_t) - 1) - i))) & 0xff);
					}
				}
			}
			auto res = ::fwrite(v.data(), size, count, stream);
			v.resize(0);
			v.shrink_to_fit();
			return res;
		}

		/* One of the principles of the safe library is that the underlying representation of objects/variable/data (as stored in
		memory) is not directly accessible. The low-level memory functions, like memcpy/memcmp/memset/etc., aren't really compatible
		with the spirit of this principle. But in practice, for many simple cases, these functions have an equivalent "safe"
		counterpart implementation. Though in some cases, the safe implementations will not produce the exact same result. */

		namespace impl {
			template<class T, class EqualTo>
			struct HasOrInheritsSubscriptOperator_impl
			{
				template<class U, class V>
				static auto test(U* u) -> decltype((*u)[0], std::declval<V>(), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct HasOrInheritsSubscriptOperator : HasOrInheritsSubscriptOperator_impl<
				mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

			namespace us {
				template<class _TIter, class _TIter2>
				_TIter memcpy_helper1(std::true_type, _TIter const& destination, _TIter2 const& source, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype((destination)[0])> element_t;
					auto num_items = num_bytes / sizeof(element_t);
					//assert(num_items * sizeof(element_t) == num_bytes);
					for (size_t i = 0; i < num_items; i += 1) {
						destination[i] = source[i];
					}
					return destination;
				}

				template<class _TPointer, class _TPointer2>
				_TPointer memcpy_helper1(std::false_type, _TPointer destination, _TPointer2 source, size_t num_bytes) {
					//typedef mse::impl::remove_reference_t<decltype(*destination)> element_t;
					//auto num_items = num_bytes / sizeof(element_t);
					//assert(1 == num_items);
					//assert(num_items * sizeof(element_t) == num_bytes);
					*destination = *source;
					return destination;
				}
			}
		}
		/* Memory safe approximation of memcpy(). */
		template<class _TIter, class _TIter2>
		_TIter memcpy(_TIter const& destination, _TIter2 const& source, size_t num_bytes) {
			return impl::us::memcpy_helper1(typename std::integral_constant<bool
					, impl::HasOrInheritsSubscriptOperator<_TIter>::value && impl::HasOrInheritsSubscriptOperator<_TIter2>::value >::type()
				, destination, source, num_bytes);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement, class _TIter2>
		_TElement* memcpy(_TElement* destination, _TIter2 const& source, size_t num_bytes) {
			return impl::us::memcpy_helper1(typename impl::HasOrInheritsSubscriptOperator<_TElement*>::type(), destination, source, num_bytes);
		}
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size, class _TIter2>
		auto memcpy(TNativeArrayReplacement<_udTy, _Size>& nar, _TIter2 const& source, size_t num_bytes) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			typename mse::mstd::array<_Ty, _Size>::iterator destination = nar;
			return memcpy(destination, source, num_bytes);
		}

		namespace impl {
			namespace us {
				template<class _TIter, class _TIter2>
				int memcmp_helper1(std::true_type, _TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype((source1)[0])> element_t;
					auto num_items = num_bytes / sizeof(element_t);
					//assert(num_items * sizeof(element_t) == num_bytes);
					for (size_t i = 0; i < num_items; i += 1) {
						auto diff = source1[i] - source2[i];
						if (0 != diff) {
							return diff;
						}
					}
					return 0;
				}

				template<class _TPointer, class _TPointer2>
				int memcmp_helper1(std::false_type, _TPointer source1, _TPointer2 source2, size_t num_bytes) {
					//typedef mse::impl::remove_reference_t<decltype(*source1)> element_t;
					//auto num_items = num_bytes / sizeof(element_t);
					//assert(1 == num_items);
					//assert(num_items * sizeof(element_t) == num_bytes);
					return (*source1) - (*source2);
				}
			}
		}
		/* Memory safe approximation of memcmp(). */
		template<class _TIter, class _TIter2>
		int memcmp(_TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
			return impl::us::memcmp_helper1(typename impl::HasOrInheritsSubscriptOperator<_TIter>::type(), source1, source2, num_bytes);
		}

		namespace impl {
			namespace us {
				template<class element_t>
				auto memset_adjusted_value1(std::true_type, int value) {
					value &= 0xff;
					long long int adjusted_value = value;
					if (sizeof(adjusted_value) >= sizeof(element_t)) {
						for (size_t i = 1; i < sizeof(element_t); i += 1) {
							adjusted_value |= (value << (8 * i));
						}
					}
					else {
						/* In this case, if the value being set is non-zero, then it's likely that this function won't
						faithfully emulate the standard memset() function. */
						assert(0 == value);
					}
					return element_t(adjusted_value);
				}
				template<class element_t>
				auto memset_adjusted_value1(std::false_type, int value) {
					/* The (integer) value is not assignable to the element. If the given value is zero, we'll assume that memset is
					just being used to "reset" the element to "the default" state. */
					assert(0 == value);
					return element_t{};
				}

				template<class _TIter>
				_TIter memset_helper1(std::true_type, _TIter const& iter, int value, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype(iter[0])> element_t;
					const auto element_value = memset_adjusted_value1<element_t>(typename std::is_assignable<element_t&, long long int>::type(), value);
					auto num_items = num_bytes / sizeof(element_t);
					//assert(num_items * sizeof(element_t) == num_bytes);
					for (size_t i = 0; i < num_items; i += 1) {
						iter[i] = element_value;
					}
					return iter;
				}

				template<class _TPointer>
				_TPointer memset_helper1(std::false_type, _TPointer ptr, int value, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype(*ptr)> element_t;
					//auto num_items = num_bytes / sizeof(element_t);
					//assert(1 == num_items);
					//assert(num_items * sizeof(element_t) == num_bytes);
					//*ptr = memset_adjusted_value1<element_t>(typename std::is_assignable<element_t&, long long int>::type(), value);
					*ptr = memset_adjusted_value1<element_t>(typename std::is_constructible<element_t, long long int>::type(), value);
					return ptr;
				}
			}
		}
		/* Memory safe approximation of memset(). */
		template<class _TIter>
		_TIter memset(_TIter const& iter, int value, size_t num_bytes) {
			return impl::us::memset_helper1(typename impl::HasOrInheritsSubscriptOperator<_TIter>::type(), iter, value, num_bytes);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memset(). */
		template<class _TElement>
		_TElement* memset(_TElement* iter, int value, size_t num_bytes) {
			return impl::us::memset_helper1(typename impl::HasOrInheritsSubscriptOperator<_TElement*>::type(), iter, value, num_bytes);
		}
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size>
		auto memset(TNativeArrayReplacement<_udTy, _Size>& nar, int value, size_t num_bytes) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			typename mse::mstd::array<_Ty, _Size>::iterator destination = nar;
			return memset(destination, value, num_bytes);
		}

		/* If at some point for some reason there is an urge to optimize these null-terminated string functions, remember that
		the safe iterator arguments that are expected to be used would generally have access to the (stored) size of the
		container that's holding the characters. */

		template<class _TIter>
		size_t strlen(_TIter const& str) {
			size_t count = 0;
			while ('\0' != str[count]) { count += 1; }
			return count;
		}

		namespace impl {
			template<class _TIter, class _TIter2>
			_TIter strcpy(_TIter const& destination, _TIter2 const& source) {
				size_t count = 0;
				auto* next_char_ptr = std::addressof(source[0]);
				while ('\0' != *next_char_ptr) {
					destination[count] = *next_char_ptr;
					count += 1;
					next_char_ptr = std::addressof(source[count]);
				}
				destination[count] = '\0';
				return destination;
			}
		}
		template<class _TIter, class _TIter2>
		_TIter strcpy(_TIter const& destination, _TIter2 const& source) {
			return impl::strcpy(destination, source);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::strcpy(). */
		template<class _TElement, class _TIter2>
		_TElement* strcpy(_TElement* destination, _TIter2 const& source) {
			return impl::strcpy(destination, source);
		}
		template<class _udTy, size_t _Size, class _TIter2>
		auto strcpy(TNativeArrayReplacement<_udTy, _Size>& nar, _TIter2 const& source) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			typename mse::mstd::array<_Ty, _Size>::iterator destination = nar;
			return strcpy(destination, source);
		}

		template<class _TIter, class _TIter2>
		int strcmp(_TIter const& str1, _TIter2 const& str2) {
			strlen(str1);
			strlen(str2);
			/* todo: This is technically not safe without first verifying that the iterators target contiguous storage. */
			return std::strcmp(std::addressof(str1[0]), std::addressof(str2[0]));
		}

		template<class _TIter, class _TIter2>
		int strncmp(_TIter const& str1, _TIter2 const& str2, size_t count) {
			/* I think this implementation should allow for the unusual case where, for example, str1 is null terminated at 
			index 3, str2 is not null terminated and has a storage capacity of 5, and the count argument is 7. I don't know 
			if such a case is technically "legal", but one could imagine standard strncmp() implementations (incidentally) 
			supporting it. */
			size_t count2 = 0;
			while ((count > count2) && ('\0' != str1[count2]) && ('\0' != str2[count2])) {
				count2 += 1;
			}
			if (count > count2) {
				if (('\0' != str1[count2]) || ('\0' != str2[count2])) {
					count2 += 1;
				}
			}
			/* todo: This is technically not safe without first verifying that the iterators target contiguous storage. */
			return std::strncmp(std::addressof(str1[0]), std::addressof(str2[0]), count2);
		}

		typedef intptr_t lh_ssize_t;

		namespace us {
			namespace impl {
				/* Apr 2025: C getline() implementation from stackoverflow comment: https://stackoverflow.com/a/47229318 */

				/* Modifications, public domain as well, by Antti Haapala, 11/10/2017
				   - Switched to getc on 5/23/2019
				   - Proper error handling on IO error and wraparound
					 check on buffer extension 3/31/2025 - 4/2/2025
				*/

				static const size_t MINIMUM_BUFFER_SIZE = 128;

				lh_ssize_t lh_getline(char** lineptr, size_t* n, FILE* stream) {
					size_t pos;
					int c;

					if (lineptr == NULL || stream == NULL || n == NULL) {
						errno = EINVAL;
						return -1;
					}

					c = getc(stream);
					if (c == EOF) {
						return -1;
					}

					if (*lineptr == NULL) {
						*lineptr = (char*)malloc(MINIMUM_BUFFER_SIZE);
						if (*lineptr == NULL) {
							return -1;
						}
						*n = MINIMUM_BUFFER_SIZE;
					}

					pos = 0;
					while (c != EOF) {
						if (pos + 1 >= *n) {
							size_t new_size = *n + (*n >> 2);

							// have some reasonable minimum
							if (new_size < MINIMUM_BUFFER_SIZE) {
								new_size = MINIMUM_BUFFER_SIZE;
							}

							// size_t wraparound
							if (new_size <= *n) {
								errno = ENOMEM;
								return -1;
							}

							// Note you might also want to check that PTRDIFF_MAX
							// is not exceeded!

							char* new_ptr = (char*)realloc(*lineptr, new_size);
							if (new_ptr == NULL) {
								return -1;
							}
							*n = new_size;
							*lineptr = new_ptr;
						}

						((unsigned char*)(*lineptr))[pos++] = (unsigned char)c;
						if (c == '\n') {
							break;
						}
						c = getc(stream);
					}

					(*lineptr)[pos] = '\0';

					// if an IO error occurred, return -1
					if (c == EOF && !feof(stream)) {
						return -1;
					}

					// otherwise we successfully read until the end-of-file
					// or the delimiter
					return pos;
				}
			}
		}

		template<typename TPointerToCharBufferPointer, typename TPointerToSize_t, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TPointerToCharBufferPointer>::value) || (mse::impl::IsDereferenceable_pb<TPointerToSize_t>::value)> MSE_IMPL_EIS >
		lh_ssize_t getline(TPointerToCharBufferPointer lineptr, TPointerToSize_t nptr, FILE* stream) {
			struct CBufPtr {
				~CBufPtr() {
					free(m_char_buf_ptr);
				}
				char* m_char_buf_ptr = nullptr;
			};
			CBufPtr temp_char_buf_ptr_obj;
			size_t n = *nptr;
			auto res = us::impl::lh_getline(&(temp_char_buf_ptr_obj.m_char_buf_ptr), &n, stream);
			if (0 > res) {
				/* fail */
				return -1;
			}
			const size_t length_including_null_terminator = (size_t)res + 1;
			auto& char_buffer_iter_ref = *lineptr;
			if (length_including_null_terminator > *nptr) {
				char_buffer_iter_ref = reallocate(char_buffer_iter_ref, length_including_null_terminator);
				if (mse::impl::remove_reference_t<decltype(char_buffer_iter_ref)>() == char_buffer_iter_ref) {
					/* reallocation seems to have failed */
					*nptr = 0;
					return -1;
				}
				*nptr = length_including_null_terminator;
			}
			for (size_t i = 0; i < length_including_null_terminator; ++i) {
				char_buffer_iter_ref[i] = (temp_char_buf_ptr_obj.m_char_buf_ptr)[i];
			}
			return res;
		}

		namespace impl {
			template<class T>
			struct NDRegisteredWrapped {
				typedef mse::TNDRegisteredObj<T> type;
			};
			template<>
			struct NDRegisteredWrapped<std::nullptr_t> {
				typedef std::nullptr_t type;
			};
			template<>
			struct NDRegisteredWrapped<void> {
				typedef void type;
			};
			template<class T>
			struct NDNoradWrapped {
				typedef mse::TNDNoradObj<T> type;
			};
			template<>
			struct NDNoradWrapped<std::nullptr_t> {
				typedef std::nullptr_t type;
			};
			template<>
			struct NDNoradWrapped<void> {
				typedef void type;
			};

			/* todo: make distinct xscope and non-xscope versions */
			class explicitly_castable_any : public mse::us::impl::ns_any::any {
			public:
				typedef mse::us::impl::ns_any::any base_class;
				//using base_class::base_class;

				explicitly_castable_any() = default;
				explicitly_castable_any(const explicitly_castable_any&) = default;
				//explicitly_castable_any(explicitly_castable_any&&) = default;
				template<class T>
				explicitly_castable_any(const T& x) : base_class(x) {}

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK1(type) \
					{ \
						auto ptr = mse::us::impl::ns_any::any_cast<type>(this); \
						if (ptr) { \
							return convert<T>(mse::us::impl::ns_any::any_cast<type>(*this)); \
						} \
					}

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK2(type) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK1(type) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK1(mse::TNDNoradObj<type>) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK1(mse::TNDRegisteredObj<type>)

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(type) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK2(type) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK2(mse::impl::remove_const_t<type>)

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_ARITHMETIC_TYPE_CHECK_HELPER1(type, not_used_template_wrapper) MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(type)
#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_WRAPPED_ARITHMETIC_TYPE_CHECK(type, template_wrapper) MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(template_wrapper<type>)

				template<class T>
				explicit operator T() const {
					{
						auto ptr = mse::us::impl::ns_any::any_cast<T>(this);
						if (ptr) {
							return convert<T>(mse::us::impl::ns_any::any_cast<T>(*this));
						}
					}
					{
						auto ptr = mse::us::impl::ns_any::any_cast<typename NDNoradWrapped<T>::type>(this);
						if (ptr) {
							return convert<T>(mse::us::impl::ns_any::any_cast<typename NDNoradWrapped<T>::type>(*this));
						}
					}
					{
						auto ptr = mse::us::impl::ns_any::any_cast<typename NDRegisteredWrapped<T>::type>(this);
						if (ptr) {
							return convert<T>(mse::us::impl::ns_any::any_cast<typename NDRegisteredWrapped<T>::type>(*this));
						}
					}
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK1(std::nullptr_t);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(void*);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(const char*);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(char*);
					MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_ARITHMETIC_TYPE_CHECK_HELPER1);
					MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_WRAPPED_ARITHMETIC_TYPE_CHECK);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK3(mse::CNDSize_t);

					return conversion_operator_helper1<T>(typename mse::impl::IsDereferenceable_pb<T>::type(), this);
					//return mse::us::impl::ns_any::any_cast<T>(*this);
				}

			private:
				template<class T1, class T2>
				static mse::xscope_optional<T1> convert_helper1(std::true_type, const T2& x) {
					return T1(x);
				}
				template<class T1, class T2>
				static mse::xscope_optional<T1> convert_helper1(std::false_type, const T2& x) {
#ifndef NDEBUG
					std::cout << "\nconvert_helper1<>(std::false_type, ): T1: " << typeid(T1).name() << ", T2: " << typeid(T2).name() << " \n";
#endif // !NDEBUG
					MSE_THROW(mse::us::impl::ns_any::bad_any_cast());
					return {};
				}
				template<class T1, class T2>
				static T1 convert(const T2& x) {
					//static const bool b1 = std::is_convertible<T2, T1>::value;
					static const bool b1 = std::is_constructible<T1, T2>::value;
					static const bool b2 = std::is_arithmetic<T1>::value;
					static const bool b3 = std::is_arithmetic<T2>::value;
					static const bool b4 = (sizeof(T1) >= sizeof(T2));
					static const bool b5 = (((!b2) && (!b3)) || (b2 && b3 && b4)); /* This is to exclude implicit support of narrowing casts. */
					static const bool b6 = b1 && b5;
					return convert_helper1<T1>(typename std::integral_constant<bool, b6>::type(), x).value();
				}

				template<class T1, class T2>
				static T1 conversion_operator_helper1(std::true_type, T2* ptr1) {

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK4(type) \
					{ \
						auto ptr = mse::us::impl::ns_any::any_cast<type>(ptr1); \
						if (ptr) { \
							return convert<T1>(mse::us::impl::ns_any::any_cast<type>(*ptr)); \
						} \
					} \
					{ \
						auto ptr = mse::us::impl::ns_any::any_cast<const type>(ptr1); \
						if (ptr) { \
							return convert<T1>(mse::us::impl::ns_any::any_cast<const type>(*ptr)); \
						} \
					}

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MACRO_FUNCTION, pointee_type) \
					MACRO_FUNCTION(mse::TRefCountingPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TRefCountingNotNullPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TNoradPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TNoradNotNullPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TRegisteredPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TRegisteredNotNullPointer<pointee_type>); \
					MACRO_FUNCTION(mse::lh::TLHNullableAnyPointer<pointee_type>); \
					MACRO_FUNCTION(mse::lh::TStrongVectorIterator<pointee_type>); \
					MACRO_FUNCTION(mse::lh::TLHNullableAnyRandomAccessIterator<pointee_type>); \
					MACRO_FUNCTION(pointee_type*);

					typedef mse::impl::remove_reference_t<decltype(*std::declval<T1>())> pointee_t;
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK4, pointee_t);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_TYPE_CHECK4, mse::impl::remove_const_t<pointee_t>);

#ifndef NDEBUG
					std::cout << "\nexplicitly_castable_any::convert_helper1<>(std::true_type, ): T1: " << typeid(T1).name() << ", stored type: " << (*ptr1).type().name() << " \n";
#endif // !NDEBUG
					return mse::us::impl::ns_any::any_cast<T1>(*ptr1);
				}
				template<class T1, class T2>
				static T1 conversion_operator_helper1(std::false_type, T2* ptr) {
#ifndef NDEBUG
					std::cout << "\nexplicitly_castable_any::convert_helper1<>(std::false_type, ): T1: " << typeid(T1).name() << ", stored type: " << (*ptr).type().name() << " \n";
#endif // !NDEBUG
					return mse::us::impl::ns_any::any_cast<T1>(*ptr);
				}
			};
		}

		/* todo: make distinct xscope and non-xscope versions */
		/* todo: duplicate impl::explicitly_castable_any's interface and make it private member instead of a public
		base class */
		class void_star_replacement : public impl::explicitly_castable_any {
		public:
			typedef impl::explicitly_castable_any base_class;
			//using base_class::base_class;
			typedef void_star_replacement _Myt;

			void_star_replacement() = default;
			void_star_replacement(const void_star_replacement&) = default;
			//void_star_replacement(void_star_replacement&&) = default;
			void_star_replacement(std::nullptr_t) : base_class((void*)(nullptr)), m_is_nullptr(true) {}
			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void *, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			void_star_replacement(const T& ptr) : base_class(ptr), m_is_nullptr(!bool(ptr))
					, m_shadow_void_const_ptr(make_void_const_ptr_helper1(typename std::integral_constant<bool, (std::is_same<void*, T>::value || std::is_same<void const*, T>::value)>::type(), ptr)) {}

			explicit operator bool() const {
				return !m_is_nullptr;
			}



#if !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
			friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				if (!bool(_Left_cref)) {
					if (!bool(_Right_cref)) {
						return true;
		}
					else {
						return false;
					}
				}
				else if (!bool(_Right_cref)) {
					return false;
				}
				return (_Right_cref.m_shadow_void_const_ptr == _Left_cref.m_shadow_void_const_ptr);
			}
			MSE_IMPL_UNORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(_Myt);
			MSE_IMPL_EQUALITY_COMPARISON_WITH_ANY_POINTER_TYPE_OPERATOR_DECLARATIONS(_Myt);
#else // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)
			/* We use a templated equality comparison operator to avoid potential arguments being implicitly converted. */
#ifndef MSE_HAS_CXX20
			template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
				(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) || (std::is_base_of<_Myt, TLHSPointer_ecwapt>::value))
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
			> MSE_IMPL_EIS >
			friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
				return !(_Left_cref == _Right_cref);
			}

			template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
				(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) || (std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)) 
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) 
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
			> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
			template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
				(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || (std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)) 
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
			> MSE_IMPL_EIS >
#endif // !MSE_HAS_CXX20
			friend bool operator==(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
				if (!bool(_Left_cref)) {
					if (!bool(_Right_cref)) {
						return true;
					}
					else {
						return false;
					}
				}
				else if (!bool(_Right_cref)) {
					return false;
				}
				return (_Myt(_Right_cref).m_shadow_void_const_ptr == _Myt(_Left_cref).m_shadow_void_const_ptr);
			}
#endif // !defined(MSE_HAS_CXX17) && defined(_MSC_VER)



#if defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }

			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
#else // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
#ifndef MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return bool(_Right_cref);
			}
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return bool(_Left_cref);
			}

			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<NULL_t, TLHS>::value) && (std::is_base_of<_Myt, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Left_cref); return !bool(_Right_cref);
			}
#endif // !MSE_HAS_CXX20
			template<typename TLHS, typename TRHS, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<_Myt, TLHS>::value) && (std::is_same<NULL_t, TRHS>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TLHS& _Left_cref, const TRHS& _Right_cref) {
				assert(0 == _Right_cref); return !bool(_Left_cref);
			}
#endif // defined(MSE_IMPL_MSC_CXX17_PERMISSIVE_MODE_COMPATIBILITY) || (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))



			friend void swap(void_star_replacement& first, void_star_replacement& second) {
				std::swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
				std::swap(first.m_is_nullptr, second.m_is_nullptr);
				std::swap(first.m_shadow_void_const_ptr, second.m_shadow_void_const_ptr);
			}

			void_star_replacement& operator=(void_star_replacement _Right) {
				std::swap(static_cast<base_class&>(*this), static_cast<base_class&>(_Right));
				std::swap((*this).m_is_nullptr, _Right.m_is_nullptr);
				std::swap((*this).m_shadow_void_const_ptr, _Right.m_shadow_void_const_ptr);
				return (*this);
			}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void *, T>::value) || (std::is_same<const void*, T>::value)
					|| (mse::impl::is_instantiation_of<mse::impl::remove_const_t<T>, mse::lh::TNativeFunctionPointerReplacement>::value))> MSE_IMPL_EIS >
			operator T() const {
				//return base_class::operator T();
				const base_class& bc_cref = *this;
				return bc_cref.operator T();
			}

		private:
			template<class T>
			void const* make_void_const_ptr_helper1(std::true_type, const T& src_ptr) {
				return src_ptr;
			}
			template<class T>
			void const* make_void_const_ptr_helper1(std::false_type, const T& src_ptr) {
				return mse::us::lh::make_raw_pointer_from(src_ptr);
			}

			bool m_is_nullptr = true;
			void const* m_shadow_void_const_ptr = nullptr;

			template<typename _Ty>
			friend _Ty mse::us::lh::unsafe_cast(const mse::lh::void_star_replacement& x);
		};
	}
	namespace us {
		namespace lh {

			/* "C-style" (unsafe) casts can convert a native pointer to a native pointer to an incompatible type. It cannot
			convert an object that is not a native pointer(/reference) to an object of incompatible type. The "safe"
			pointers in the library are (often) objects, not native pointers, but for compatibility with legacy code we
			provide a function that can (unsafely) convert the library's safe pointers (and iterators) to corresponding
			pointers (or iterators) pointing to incompatible types. */

			namespace impl {
				namespace ns_unsafe_cast {

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper5(std::false_type, const _Ty2& x) {
						if (x == nullptr) {
							return nullptr;
						}
						return (_Ty)(std::addressof(*x));
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper5(std::true_type, const _Ty2& x) {
						/* _Ty2 is `void*` or `void const*` */
						return (_Ty)(x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper4(std::false_type, const _Ty2& x) {
						return (_Ty)(std::addressof(*x));
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper4(std::true_type, const _Ty2& x) {
						typedef mse::impl::remove_reference_t<mse::impl::remove_const_t<_Ty2> > ncTy2;
						return unsafe_cast_helper5<_Ty>(typename std::integral_constant<bool, (std::is_same<void*, ncTy2>::value || std::is_same<void const*, ncTy2>::value)>::type(), x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper3(std::false_type, const _Ty2& x) {
						return (_Ty const&)(x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper3(std::true_type, const _Ty2& x) {
						/* _Ty is a raw pointer type */
						return unsafe_cast_helper4<_Ty>(typename mse::impl::IsNullable_msemsearray<_Ty2>::type(), x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper2(std::false_type, const _Ty2& x) {
						return unsafe_cast_helper3<_Ty>(typename std::is_pointer<_Ty>::type(), x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper2(std::true_type, const _Ty2& x) {
						return reinterpret_cast<_Ty const&>(x);
					}
				}
			}
			template<typename _Ty, typename _Ty2>
			_Ty unsafe_cast(const _Ty2& x) {
				return impl::ns_unsafe_cast::unsafe_cast_helper2<_Ty>(std::false_type(), x);
			}

			/* Here we provide overloads for the unsafe_cast<>() function that support the (unsafe) casting of selected smart
			pointers, akin to how a raw pointer can be cast to a raw pointer to an incompatible type via "C-style" cast. */

#define MSE_LH_IMPL_UNSAFE_CAST_OVERLOAD(smart_pointer) \
			template<typename _Ty, typename _Ty2> \
			_Ty unsafe_cast(const smart_pointer<_Ty2>& x) { \
				return impl::ns_unsafe_cast::unsafe_cast_helper2<_Ty>(typename mse::impl::is_instantiation_of<_Ty, smart_pointer>::type(), x); \
			}

			MSE_LH_IMPL_UNSAFE_CAST_OVERLOAD(mse::lh::TLHNullableAnyRandomAccessIterator)
				MSE_LH_IMPL_UNSAFE_CAST_OVERLOAD(mse::lh::TXScopeLHNullableAnyRandomAccessIterator)
				MSE_LH_IMPL_UNSAFE_CAST_OVERLOAD(mse::lh::TLHNullableAnyPointer)
				MSE_LH_IMPL_UNSAFE_CAST_OVERLOAD(mse::lh::TXScopeLHNullableAnyPointer)

				template<typename _Ty, typename _Ty2, size_t _Size>
			_Ty unsafe_cast(mse::lh::TNativeArrayReplacement<_Ty2, _Size>& x) {
				return unsafe_cast<_Ty>(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty2>(x));
			}
			template<typename _Ty, typename _Ty2, size_t _Size>
			_Ty unsafe_cast(const mse::lh::TNativeArrayReplacement<_Ty2, _Size>& x) {
				return unsafe_cast<_Ty>(mse::lh::TLHNullableAnyRandomAccessIterator<const _Ty2>(x));
			}
			template<typename _Ty>
			_Ty unsafe_cast(const mse::lh::void_star_replacement& x) {
				return unsafe_cast<_Ty>(const_cast<void*>(x.m_shadow_void_const_ptr));
			}

			template<typename _Ty>
			auto make_raw_pointer_from(_Ty const& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr))) {
				return unsafe_cast<decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))>(ptr);
			}
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr))) {
				/* Note that, for example, in the case of mse::lh::TNativeArrayReplacement<>, its "operator*()" and "operator*() const" 
				return different types. (Specifically, they return types that differ by a const qualifier.) */
				return unsafe_cast<decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))>(ptr);
			}
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty&& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr))) {
				return unsafe_cast<decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))>(MSE_FWD(ptr));
			}

			auto make_raw_pointer_from(mse::lh::void_star_replacement const& vsr) -> void* {
				return unsafe_cast<void*>(vsr);
			}

			/* An iterator pointing into a container of (other) iterators can't be directly converted to a raw pointer iterator 
			pointing into an array of (other) raw pointer iterators. In order to produce such a raw pointer iterator, we first 
			need to construct an array of pointer iterators. Once constructed, we can then return (via conversion operator) a 
			pointer iterator that points into that array. */
			template<typename _Ty>
			class TXScopeConstArrayOfRawPointersStore : public mse::us::impl::XScopeTagBase {
			public:
				TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > iter1) {
					while (*iter1) {
						auto& pointee_ref = *iter1;
						m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
						++iter1;
					}
					m_pointer_vec.push_back(nullptr);
				}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) 
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> >(iter1)) {}
				template<size_t N>
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TNativeArrayReplacement<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>, N> const& safe_array1) {
					for (auto& iter2 : safe_array1) {
						m_pointer_vec.push_back(make_raw_pointer_from(iter2));
					}
				}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> >(iter1)) {}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> >(iter1)) {}
				TXScopeConstArrayOfRawPointersStore(_Ty* const* iter1) : m_converted_ptr(iter1) {}

				_Ty* const* raw_pointer_to_stored_pointers() const {
					if (m_converted_ptr) {
						return m_converted_ptr;
					}
					if (1 <= m_pointer_vec.size()) {
						return std::addressof(m_pointer_vec.front());
					}
					return nullptr;
				}
				operator _Ty* const* () const {
					return raw_pointer_to_stored_pointers();
				}
			private:
				std::vector<_Ty*> m_pointer_vec;
				_Ty* const* m_converted_ptr = nullptr;
			};

			template<typename _Ty>
			auto make_temporary_array_of_raw_pointers_from(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_array_of_raw_pointers_from(mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty, size_t N>
			auto make_temporary_array_of_raw_pointers_from(mse::lh::TNativeArrayReplacement<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>, N> const& safe_array1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(safe_array1);
			}
			template<typename _Ty>
			auto make_temporary_array_of_raw_pointers_from(mse::lh::TXScopeStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_array_of_raw_pointers_from(mse::lh::TStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_array_of_raw_pointers_from(_Ty* const* iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
		}
	}
}

#endif // MSE_SAFER_SUBSTITUTES_DISABLED

#endif /*MSE_LEGACYHELPERS_DISABLED*/

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop

/* Here we're suppressing this "NULL to integral conversion" warning in any file that includes this header as we
intentionally use this (implicit) conversion in the interface of lh::TLHNullableAnyRandomAccessIterator<>. */
#pragma GCC diagnostic ignored "-Wconversion-null"
#endif /*__GNUC__*/
#endif /*__clang__*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSELEGACYHELPERS_H_
