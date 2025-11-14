
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
#include <cinttypes> /* for strtoimax() */

#ifdef MSE_SELF_TESTS
#include <iomanip> /* for std::quoted */
#include <iostream>
#endif // MSE_SELF_TESTS

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4127 4702 )
#endif /*_MSC_VER*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wnull-conversion"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion-null"
#pragma GCC diagnostic ignored "-Wunused-variable"
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
#define MSE_LH_ICONV(cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr) iconv(cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMCMP(element_type, destination, source, num_bytes) memcmp(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_MEMCMP(destination, source, num_bytes) memcmp(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_MEMCHR(iter, ch, count) memchr(iter, ch, count)
#define MSE_LH_STRLEN(str) strlen(str)
#define MSE_LH_STRNLEN_S(str, strsz) strlnen_s(str, strsz)
#define MSE_LH_STRCPY(destination, source) strcpy(destination, source)
#define MSE_LH_STRCMP(destination, source) strcmp(destination, source)
#define MSE_LH_STRNCMP(destination, source, count) strncmp(destination, source, count)
#define MSE_LH_STRCHR(iter, ch) strchr(iter, ch)
#define MSE_LH_STRTOL(str, pointer_to_end_iterator, base) strtol(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOUL(str, pointer_to_end_iterator, base) strtoul(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOLL(str, pointer_to_end_iterator, base) strtoll(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOULL(str, pointer_to_end_iterator, base) strtoull(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOIMAX(str, pointer_to_end_iterator, base) imax(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOUMAX(str, pointer_to_end_iterator, base) umax(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOUMAX(str, pointer_to_end_iterator, base) umax(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOUMAX(str, pointer_to_end_iterator, base) umax(str, (char **)pointer_to_end_iterator, base)
#define MSE_LH_STRTOF(str, pointer_to_end_iterator) strtof(str, (char **)pointer_to_end_iterator)
#define MSE_LH_STRTOD(str, pointer_to_end_iterator) strtod(str, (char **)pointer_to_end_iterator)
#define MSE_LH_STRTOLD(str, pointer_to_end_iterator) strtold(str, (char **)pointer_to_end_iterator)
#define MSE_LH_CHAR_STAR_STAR_CAST_FOR_STRTOX(pointer_to_iterator_arg) (char **)(pointer_to_iterator_arg)
#define MSE_LH_STRTOK(str, delim) strtok(str, delim)
#define MSE_LH_STRTOK_R(str, delim, pointer_to_buffer_iterator) strtok_r(str, delim, pointer_to_buffer_iterator)
#define MSE_LH_STRNDUP(str, size) strndup(str, size)
#define MSE_LH_STRDUP(str) strdup(str)
#define MSE_LH_MEMDUP(iter, size) mse::lh::memdup(iter, size)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) object_type
#define MSE_LH_POINTER_TYPE(element_type) element_type *
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) element_type *
#define MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type) element_type *
#define MSE_LH_PARAM_ONLY_POINTER_TYPE(element_type) MSE_LH_LOCAL_VAR_ONLY_POINTER_TYPE(element_type)
#define MSE_LH_NULL_POINTER NULL
#define MSE_LH_VOID_STAR void *
#define MSE_LH_CONST_VOID_STAR const void *

#define MSE_LH_CAST(type, value) ((type)value)
#define MSE_LH_UNSAFE_CAST(type, value) ((type)value)
#define MSE_LH_UNSAFE_MAKE_POINTER_TO(target) &(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_TO(target) &(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_TEMPORARY_RAW_POINTER_TO_POINTER_PROXY_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_POINTER_FROM(ptr) (ptr)
#define MSE_LH_UNSAFE_MAKE_ARRAY_ITERATOR_FROM(iter) (iter)
#define MSE_LH_UNSAFE_MAKE_FN_WRAPPER(wrappee, function_pointer_with_desired_wrapper_signature) (wrappee)
#define MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER(wrappee_fnptr, function_pointer_with_desired_wrapper_signature) (wrappee_fnptr)

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
#define MSE_LH_IMPL_ICONV_LAMBDA [](iconv_t const& cd, char** inbufptr, size_t* inbytesleftptr, char** outbufptr, size_t* outbytesleftptr){ return iconv(cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr); }
#define MSE_LH_ICONV(cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr) mse::lh::iconv_wrapper(MSE_LH_IMPL_ICONV_LAMBDA, cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) mse::lh::memcpy<mse::lh::TLHNullableAnyRandomAccessIterator<element_type>, mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMCMP(element_type, destination, source, num_bytes) mse::lh::memcmp< mse::lh::TLHNullableAnyRandomAccessIterator<element_type>, mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) mse::lh::memset< mse::lh::TLHNullableAnyRandomAccessIterator<element_type> >(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) mse::lh::memcpy(destination, source, num_bytes)
#define MSE_LH_MEMCMP(destination, source, num_bytes) mse::lh::memcmp(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) mse::lh::memset(ptr, value, num_bytes)
#define MSE_LH_MEMCHR(iter, ch, count) mse::lh::memchr(iter, ch, count)
#define MSE_LH_STRLEN(str) mse::lh::strlen(str)
#define MSE_LH_STRNLEN_S(str, strsz) mse::lh::strlnen_s(str, strsz)
#define MSE_LH_STRCPY(destination, source) mse::lh::strcpy(destination, source)
#define MSE_LH_STRCMP(destination, source) mse::lh::strcmp(destination, source)
#define MSE_LH_STRNCMP(destination, source, count) mse::lh::strncmp(destination, source, count)
#define MSE_LH_STRCHR(iter, ch) mse::lh::strchr(iter, ch)
#define MSE_LH_STRTOL(str, pointer_to_end_iterator, base) mse::lh::strtol(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOUL(str, pointer_to_end_iterator, base) mse::lh::strtoul(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOLL(str, pointer_to_end_iterator, base) mse::lh::strtoll(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOULL(str, pointer_to_end_iterator, base) mse::lh::strtoull(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOIMAX(str, pointer_to_end_iterator, base) mse::lh::strtoimax(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOUMAX(str, pointer_to_end_iterator, base) mse::lh::strtoumax(str, pointer_to_end_iterator, base)
#define MSE_LH_STRTOF(str, pointer_to_end_iterator) mse::lh::strtof(str, pointer_to_end_iterator)
#define MSE_LH_STRTOD(str, pointer_to_end_iterator) mse::lh::strtod(str, pointer_to_end_iterator)
#define MSE_LH_STRTOLD(str, pointer_to_end_iterator) mse::lh::strtold(str, pointer_to_end_iterator)
#define MSE_LH_CHAR_STAR_STAR_CAST_FOR_STRTOX(pointer_to_iterator_arg)
#define MSE_LH_STRTOK(str, delim) mse::lh::strtok(str, delim)
#define MSE_LH_STRTOK_R(str, delim, pointer_to_buffer_iterator) mse::lh::strtok_r(str, delim, pointer_to_buffer_iterator)
#define MSE_LH_STRNDUP(str, size) mse::lh::strndup(str, size)
#define MSE_LH_STRDUP(str) mse::lh::strdup(str)
#define MSE_LH_MEMDUP(iter, size) mse::lh::memdup(iter, size)

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
#define MSE_LH_CONST_VOID_STAR mse::lh::const_void_star_replacement

#define MSE_LH_CAST(type, value) ((type const &)(value))
#define MSE_LH_UNSAFE_CAST(type, value) mse::us::lh::unsafe_cast<type>(value)
#define MSE_LH_UNSAFE_MAKE_POINTER_TO(target) MSE_LH_POINTER_TYPE(mse::us::unsafe_make_any_pointer_to(target))
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_TO(target) std::addressof(target)
#define MSE_LH_UNSAFE_MAKE_RAW_POINTER_FROM(ptr) mse::us::lh::make_raw_pointer_from(ptr)
#define MSE_LH_UNSAFE_MAKE_TEMPORARY_RAW_POINTER_TO_POINTER_PROXY_FROM(ptr) mse::us::lh::make_temporary_raw_pointer_to_pointer_proxy_from(ptr)
#define MSE_LH_UNSAFE_MAKE_POINTER_FROM(ptr) mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(ptr)
#define MSE_LH_UNSAFE_MAKE_ARRAY_ITERATOR_FROM(iter) mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(iter)
#define MSE_LH_UNSAFE_MAKE_FN_WRAPPER(wrappee, function_pointer_with_desired_wrapper_signature) mse::us::lh::unsafe_make_fn_wrapper(wrappee, function_pointer_with_desired_wrapper_signature)
// MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER(wrappee_fnptr, example_fnptr_with_desired_signature) is defined later in this file

#ifndef MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE MSE_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE MSE_SUPPRESS_CHECK_IN_DECLSCOPE
#endif // !MSE_LH_SUPPRESS_CHECK_IN_XSCOPE


#define MSE_LH_IF_ENABLED(x) x
#define MSE_LH_IF_DISABLED(x)

namespace mse {
	typedef intptr_t lh_ssize_t;

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

		namespace us {
			namespace impl {
				/* Apr 2025: C getline() implementation from stackoverflow comment: https://stackoverflow.com/a/47229318 */

				/* Modifications, public domain as well, by Antti Haapala, 11/10/2017
				   - Switched to getc on 5/23/2019
				   - Proper error handling on IO error and wraparound
					 check on buffer extension 3/31/2025 - 4/2/2025
				*/

				static const size_t MINIMUM_BUFFER_SIZE = 128;

				inline lh_ssize_t lh_getline(char** lineptr, size_t* n, FILE* stream) {
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

		/*
		template <class X, class... Args>
		TStrongVectorIterator<X> make_strong_vector_iterator(Args&&... args) {
			return TStrongVectorIterator<X>(std::forward<Args>(args)...);
		}
		*/

		template <typename _Ty>
		using TXScopeStrongVectorIterator = _Ty*;

		/*
		template <class X, class... Args>
		TXScopeStrongVectorIterator<X> make_xscope_strong_vector_iterator(Args&&... args) {
			return TXScopeStrongVectorIterator<X>(std::forward<Args>(args)...);
		}
		*/

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
				static bool reallocate(_TPtr& ptr, size_t num_bytes) {
					auto new_ptr = _TPtr(::realloc(ptr, num_bytes));
					if (new_ptr) {
						ptr = new_ptr;
					}
					return bool(new_ptr);
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

#define MSE_IMPL_LH_STRTOX_DECLARATION(strtox_function_name) \
		inline auto strtox_function_name(const char* str, char** str_end, int base) { return std::strtox_function_name(str, str_end, base); }

		MSE_IMPL_LH_STRTOX_DECLARATION(strtol);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoul);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoll);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoull);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoimax);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoumax);

#define MSE_IMPL_LH_STRTOFX_DECLARATION(strtox_function_name) \
		inline auto strtox_function_name(const char* str, char** str_end) { return std::strtox_function_name(str, str_end); }

		MSE_IMPL_LH_STRTOFX_DECLARATION(strtof);
		MSE_IMPL_LH_STRTOFX_DECLARATION(strtod);
		MSE_IMPL_LH_STRTOFX_DECLARATION(strtold);

		inline auto strtok(char* str, const char* delim) { return std::strtok(str, delim); }
		inline auto strchr(const char* str, int ch) { return std::strchr(str, ch); }
		template<class _TIter>
		inline auto memchr(_TIter str, int ch, size_t count) { return std::memchr(std::addressof(str[0]), ch, count); }

		template<typename TPointerToCharBuffer, typename TPointerToSize_t, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TPointerToCharBuffer>::value) || (mse::impl::IsDereferenceable_pb<TPointerToSize_t>::value)> MSE_IMPL_EIS >
		lh_ssize_t getline(TPointerToCharBuffer lineptr, TPointerToSize_t nptr, FILE* stream) {
			return us::impl::lh_getline(lineptr, nptr, stream);
		}

		inline auto strndup(const char* str_iter, size_t size) {
			char* retval = nullptr;
			auto slen = strlen(str_iter);
			if (size < slen) {
				slen = size;
			}
			retval = allocate_dyn_array1<char*>(1 + slen);
			auto dest_iter = retval;
			for (size_t i = 0; slen > i; i += 1) {
				*dest_iter = str_iter[i];
				++dest_iter;
			}
			*dest_iter = 0;
			return retval;
		}
		inline auto strdup(const char* str_iter) {
			auto slen = strlen(str_iter);
			return strndup(str_iter, slen);
		}

		template<typename TIterator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TIterator>::value)> MSE_IMPL_EIS >
		inline auto memdup(TIterator const& src_iter, size_t num_bytes) {
			typedef mse::impl::remove_const_t<mse::impl::target_type<TIterator> > new_element_t;
			new_element_t* retval = nullptr;
			retval = allocate_dyn_array1<new_element_t*>(num_bytes);
			auto dest_iter = retval;
			auto num_items = num_bytes / sizeof(new_element_t);
			for (size_t i = 0; num_items > i; i += 1) {
				*dest_iter = src_iter[i];
				++dest_iter;
			}
			return retval;
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
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(_Ty&& ptr) {
				return ptr;
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(_Ty& ptr) {
				return ptr;
			}
			template< typename TWrappee, typename TWrapperRet, typename... TArgs>
			auto unsafe_make_fn_wrapper(const TWrappee& wrappee, mse::lh::TNativeFunctionPointerReplacement<TWrapperRet(TArgs...)>) {
				return wrappee;
			}
		}
	}

	namespace lh {
		/* todo: make distinct xscope and non-xscope versions */
		typedef void* void_star_replacement;
		typedef const void* const_void_star_replacement;
	}
}

#else // MSE_SAFER_SUBSTITUTES_DISABLED

namespace mse {
	namespace lh {
		typedef decltype(NULL) NULL_t;
		typedef decltype(0) ZERO_LITERAL_t;
		static const auto NULL_AND_ZERO_ARE_THE_SAME_TYPE = std::is_same<NULL_t, ZERO_LITERAL_t>::value;

		class void_star_replacement;
		class const_void_star_replacement;
	}
	namespace us {
		namespace lh {
			template<typename _Ty, typename _Ty2>
			_Ty unsafe_cast(const _Ty2& x);
			template<typename _Ty>
			_Ty unsafe_cast(const mse::lh::void_star_replacement& x);
			template<typename _Ty>
			_Ty unsafe_cast(const mse::lh::const_void_star_replacement& x);

			template<typename _Ty>
			auto make_raw_pointer_from(_Ty const& ptr)/* -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))*/;
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)));
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty&& ptr) -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)));
			template<typename _Ty>
			auto make_raw_pointer_from(_Ty* ptr) { return ptr; }
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
			/* We hesitate to support construction from raw pointers, as raw pointers are considered scope pointers in the scpptool-enforced
			safe subset, and TLHNullableAnyPointer<> is not supposed to hold a scope pointer. But this type is intended for facilitating (the
			migration of, and) interfacing with legacy code, which may be using raw pointers as non-scope pointers. So we leave it to the
			scpptool static analyzer to flag the likely safety violation committed when invoking this constructor, rather than sacrifice
			the interoperability with legacy that would by lost by not providing this constructor. */
			TLHNullableAnyPointer(_Ty* ptr) : base_class(mse::us::TSaferPtrForLegacy<_Ty>(ptr)) {}
			TLHNullableAnyPointer(void* src) : base_class(mse::us::TSaferPtrForLegacy<_Ty>((_Ty*)src)) {}
			template <typename _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			TLHNullableAnyPointer(const void* src) : base_class(mse::us::TSaferPtrForLegacy<_Ty>((_Ty2*)src)) {}

			template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(
				(!std::is_convertible<_TPointer1, TLHNullableAnyPointer>::value)
				&& (!std::is_base_of<base_class, _TPointer1>::value)
				&& (!std::is_same<_TPointer1, std::nullptr_t>::value)
				&& (!std::is_same<_TPointer1, NULL_t>::value)
				&& (!std::is_same<_TPointer1, ZERO_LITERAL_t>::value)
				&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
				&& mse::impl::is_potentially_not_xscope<_TPointer1>::value
				) || (std::is_base_of<const_void_star_replacement, _TPointer1>::value && std::is_const<_Ty>::value)
			> MSE_IMPL_EIS >
			TLHNullableAnyPointer(const _TPointer1& pointer) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TPointer1)")
				: base_class(construction_helper1(typename std::is_base_of<const_void_star_replacement, _TPointer1>::type(), pointer)) {}

			friend void swap(TLHNullableAnyPointer& first, TLHNullableAnyPointer& second) {
				swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			}

			/* Some of the comparision operators are currently inherited from base class. */

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

			TLHNullableAnyPointer& operator=(const TLHNullableAnyPointer& _Right_cref) {
				base_class::operator=(_Right_cref);
				return (*this);
			}

			explicit operator bool() const {
				return base_class::operator bool();
			}

			void async_not_shareable_and_not_passable_tag() const {}

		private:
			/* Btw things like this akward use of a "construction_helper" is only to support compatibility with older versions of the microsoft compiler whose support for sfinae was limited. */
			static auto construction_helper1(std::true_type, const const_void_star_replacement& src) -> TLHNullableAnyPointer<const _Ty>;
			template<typename _Ty2>
			static auto construction_helper1(std::false_type, const _Ty2& src) { return src; }

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
		};
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::lh::TLHNullableAnyPointer);
	namespace lh {

		template<typename ValueType, typename _Ty, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
		inline retval_t maybe_any_cast(const TLHNullableAnyPointer<_Ty>& operand) {
			return mse::maybe_any_cast<ValueType>(mse::us::impl::as_ref<const typename TLHNullableAnyPointer<_Ty>::base_class>(operand));
		}

		/* specializations defined later in the file */
		template<>
		class TLHNullableAnyPointer<void>;
		template<>
		class TLHNullableAnyPointer<const void>;

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
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, _Ty, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(mse::us::TSaferPtrForLegacy<_Tx2>(ptr));
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(const _Ty& ptr) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(ptr);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(_Ty& ptr) {
				/* Note that, for example, in the case of mse::lh::TNativeArrayReplacement<>, its "operator*()" and "operator*() const"
				return different types. (Specifically, they return types that differ by a const qualifier.) */
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyPointer<_Tx2>(ptr);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_pointer_from(_Ty&& ptr) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
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
			/* We hesitate to support construction from raw pointers, as raw pointers are considered scope pointers in the scpptool-enforced
			safe subset, and TLHNullableAnyPointer<> is not supposed to hold a scope pointer. But this type is intended for facilitating (the
			migration of, and) interfacing with legacy code, which may be using raw pointers as non-scope pointers. So we leave it to the
			scpptool static analyzer to flag the likely safety violation committed when invoking this constructor, rather than sacrifice
			the interoperability with legacy that would by lost by not providing this constructor. */
			TXScopeLHNullableAnyPointer(_Ty* ptr) : base_class(mse::us::TSaferPtrForLegacy<_Ty>(ptr)) {}
			TXScopeLHNullableAnyPointer(void* src) : base_class(mse::us::TSaferPtrForLegacy<_Ty>((_Ty*)src)) {}
			template <typename _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			TXScopeLHNullableAnyPointer(const void* src) : base_class(mse::us::TSaferPtrForLegacy<_Ty>((_Ty2*)src)) {}

			/* Btw things like this akward use of a "construction_helper" is only to support compatibility with older versions of the microsoft compiler whose support for sfinae was limited. */
			static auto construction_helper1(std::true_type, const const_void_star_replacement& src) -> TXScopeLHNullableAnyPointer<const _Ty>;
			template<typename _Ty2>
			static auto construction_helper1(std::false_type, const _Ty2& src) { return src; }

			template <typename _TPointer1, MSE_IMPL_EIP mse::impl::enable_if_t<(
					(!std::is_convertible<_TPointer1, TXScopeLHNullableAnyPointer>::value)
					&& (!std::is_base_of<base_class, _TPointer1>::value)
					&& (!std::is_same<_TPointer1, std::nullptr_t>::value)
					&& (!std::is_same<_TPointer1, NULL_t>::value)
					&& (!std::is_same<_TPointer1, ZERO_LITERAL_t>::value)
					&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TPointer1, _Ty)
				) || (std::is_base_of<const_void_star_replacement, _TPointer1>::value && std::is_const<_Ty>::value)
			> MSE_IMPL_EIS >
			TXScopeLHNullableAnyPointer(const _TPointer1& pointer) : base_class(construction_helper1(typename std::is_base_of<const_void_star_replacement, _TPointer1>::type(), pointer)) {}

			friend void swap(TXScopeLHNullableAnyPointer& first, TXScopeLHNullableAnyPointer& second) {
				swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
			}

			/* Some of the comparision operators are currently inherited from base class. */

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

			explicit operator bool() const {
				return base_class::operator bool();
			}

			void async_not_shareable_and_not_passable_tag() const {}

		private:
			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;
		};
	}
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::lh::TXScopeLHNullableAnyPointer);
	namespace lh {

		template<typename ValueType, typename _Ty, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
		inline retval_t maybe_any_cast(const TXScopeLHNullableAnyPointer<_Ty>& operand) {
			return mse::maybe_any_cast<ValueType>(mse::us::impl::as_ref<const typename TXScopeLHNullableAnyPointer<_Ty>::base_class>(operand));
		}

		/* specializations defined later in the file */
		template<>
		class TXScopeLHNullableAnyPointer<void>;
		template<>
		class TXScopeLHNullableAnyPointer<const void>;

#ifndef MSE_HAS_CXX20
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator==(const _Ty2 lhs, const TXScopeLHNullableAnyPointer<_Ty>& rhs) { return rhs == lhs; }
		template <typename _Ty, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
		bool operator!=(const _Ty2 lhs, const TXScopeLHNullableAnyPointer<_Ty>& rhs) { return rhs != lhs; }
#endif // !MSE_HAS_CXX20

		namespace us {
			namespace impl {
				template <typename _Ty>
				class TLHNullableAnyRandomAccessIteratorBase;
			}
		}
		namespace impl {
			template<class T>
			struct NDRegisteredWrapped {
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T, mse::TNDRegisteredObj<T> > type;
				//typedef mse::TNDRegisteredObj<T> type;
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
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T, mse::TNDNoradObj<T> > type;
				//typedef mse::TNDNoradObj<T> type;
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

				explicitly_castable_any() : base_class(nullptr) {}
				explicitly_castable_any(const explicitly_castable_any&) = default;
				//explicitly_castable_any(explicitly_castable_any&&) = default;
				template<class T>
				explicitly_castable_any(const T& x) : base_class(x) {}

				template<class T>
				explicit operator T() const {
					return conversion_operator_helper1<T>(this);
				}

			//private:
				template<class T1>
				using my_optional = mse::us::impl::ns_optional::optional_base2<T1, mse::non_thread_safe_shared_mutex, mse::us::impl::ns_optional::optional_base2_not_const_lockable_tag>;

				template<class T1, class T2>
				static my_optional<T1> convert_helper1(std::true_type, const T2& x) {
					return T1(x);
				}
				template<class T1, class T2>
				static my_optional<T1> convert_helper1(std::false_type, const T2& x) {
#ifndef NDEBUG
					std::cout << "\nconvert_helper1<>(std::false_type, ): T1: " << typeid(T1).name() << ", T2: " << typeid(T2).name() << " \n";
#endif // !NDEBUG
					MSE_THROW(mse::us::impl::ns_any::bad_any_cast());
					return {};
				}
				template<class T1, class T2>
				static T1 convert(const T2& x) {
					//static const bool b1 = std::is_convertible<T2, T1>::value;
					static const bool b1a = std::is_constructible<T1, T2 const&>::value;
					static const bool b1b = std::is_pointer<T1>::value && (std::is_same<T2, void*>::value || std::is_same<T2, void const*>::value);
					static const bool b1 = (b1a || b1b);
					static const bool b2 = std::is_arithmetic<T1>::value;
					static const bool b3 = std::is_arithmetic<T2>::value;
					static const bool b4 = (sizeof(T1) >= sizeof(T2));
					static const bool b5 = (((!b2) && (!b3)) || (b2 && b3 && b4)); /* This is to exclude implicit support of narrowing casts. */
					static const bool b6 = b1 && b5;
					return convert_helper1<T1>(typename std::integral_constant<bool, b6>::type(), x).value();
				}

				template<class T1, class T2>
				static T1 conversion_operator_helper1(T2* ptr1) {
					auto maybe_retval = conversion_operator_helper2<T1>(ptr1);
					if (maybe_retval.has_value()) {
						return std::move(maybe_retval.value());
					}
#ifndef NDEBUG
					std::cout << "\nexplicitly_castable_any::conversion_operator_helper1<>(): T1: " << typeid(T1).name() << ", stored type: " << (*ptr1).type().name() << " \n";
#endif // !NDEBUG
					return mse::us::impl::ns_any::any_cast<T1>(*ptr1);
				}

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT1(type1) \
					{ \
						auto ptr = mse::us::impl::ns_any::any_cast<type1>(ptr1); \
						if (ptr) { \
							return convert<T1>(*ptr); \
						} \
					}

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT2(type1) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT1(type1) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT1(mse::impl::remove_const_t<type1>) \
					/* While references to `non-const`s aren't constructible from references to `const`s, `non-const` values are constructible from `const` values. */ \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT1(typename std::add_const<type1>::type)

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT3(type1) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT2(type1) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT2(typename NDNoradWrapped<type1>::type) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT2(typename NDRegisteredWrapped<type1>::type)

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(type1) \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT3(mse::impl::remove_const_t<type1>) \
					/* While references to `non-const`s aren't constructible from references to `const`s, `non-const` values are constructible from `const` values. */ \
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT3(typename std::add_const<type1>::type)

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_ARITHMETIC_CAST_ATTEMPT_HELPER1(type1, not_used_template_wrapper) MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(type1)
#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_WRAPPED_ARITHMETIC_CAST_ATTEMPT(type1, template_wrapper) MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(template_wrapper<type1>)

				template<class T1, class T2>
				static my_optional<T1> conversion_operator_helper2(T2* ptr1) {
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(T1);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT2(std::nullptr_t);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(void*);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(const void*);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(const char*);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(char*);

					/* Since explicitly_castable_any is currently only used as a base class for void_ster_replacement, we won't check for arithmetic values.
					But if explicitly_castable_any is ever used in a more general context we might consider doing so. */
					/*
					MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_ARITHMETIC_CAST_ATTEMPT_HELPER1);
					MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_WRAPPED_ARITHMETIC_CAST_ATTEMPT);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4(mse::CNDSize_t);
					*/

					auto maybe_retval = conversion_operator_helper3<T1>(typename mse::impl::IsDereferenceable_pb<T1>::type(), ptr1);
					if (maybe_retval.has_value()) {
						return std::move(maybe_retval.value());
					}
					return {};
				}

				template<class T1, class T2>
				static my_optional<T1> conversion_operator_helper3(std::true_type, T2* ptr1);
				template<class T1, class T2>
				static my_optional<T1> conversion_operator_helper3(std::false_type, T2* ptr1) {
					auto casted_ptr = mse::us::impl::ns_any::any_cast<T1>(&mse::us::impl::as_ref<base_class>(*ptr1));
					if (casted_ptr) {
						return *casted_ptr;
					}
					return {};
				}

				template<class T1, class T2>
				static my_optional<T1> conversion_operator_helper4(std::true_type, T2* ptr1);
				template<class T1, class T2>
				static my_optional<T1> conversion_operator_helper4(std::false_type, T2* ptr1) { return {}; }

				template <typename _Ty2>
				friend class mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase;
			};

			template<typename ValueType, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
			inline retval_t maybe_any_cast_of_explicitly_castable_any(const explicitly_castable_any& operand) {
				auto maybe_casted = explicitly_castable_any::conversion_operator_helper2<ValueType>(&operand);
				if (maybe_casted.has_value()) {
					return std::move(maybe_casted.value());
				}
				return {};
			}
		}
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
						&& (!std::is_base_of<_Myt, _TRandomAccessIterator1>::value)
						&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
						&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
						&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
						&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
						//&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
					> MSE_IMPL_EIS >
					TLHNullableAnyRandomAccessIteratorBase(const _TRandomAccessIterator1& random_access_iterator) : m_iter(constructor_helper1(
						typename mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::type(), random_access_iterator)) {
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

#if (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
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
#else // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
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
#endif // (defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

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

				private:

					template <typename _Ty2>
					static auto construction_helper3(std::false_type, const _Ty2& src) {
						return src;
					}
					template <typename _Ty2>
					static auto construction_helper3(std::true_type, const _Ty2& src) {
						/* argument seems to be a TLHNullableAnyRandomAccessIteratorBase<std::remove_const_t<_Ty> > */
						//return reinterpret_cast<const TLHNullableAnyRandomAccessIteratorBase&>(src).m_iter;
						return src.m_iter;
					}

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
						return construction_helper3(typename std::integral_constant<bool, std::is_const<_Ty>::value && std::is_base_of<TLHNullableAnyRandomAccessIteratorBase<mse::impl::remove_const_t<_Ty> >, _TRandomAccessIterator1>::value>::type(), random_access_iterator);
					}

					template <typename _TRandomAccessIterator1>
					static auto constructor_helper1(std::true_type, const _TRandomAccessIterator1& random_access_iterator) {
						typedef mse::impl::remove_reference_t<mse::impl::target_or_given_default_type<decltype(random_access_iterator), mse::impl::TPlaceHolder<> > > _Ty2;
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

					static void foo1(mse::us::impl::ns_any::any const& any1) {
						auto maybe_casted = mse::lh::impl::explicitly_castable_any::conversion_operator_helper2<int>(&any1);
						if (maybe_casted.has_value()) {
							auto casted_val = maybe_casted.value();
							int q = 5;
						}
					}

					MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

					const base_class& contained_iter() const& { return (*this).m_iter; }
					//const base_class& contained_iter() const&& { return (*this).m_iter; }
					base_class& contained_iter()& { return (*this).m_iter; }
					base_class&& contained_iter()&& { return std::move(*this).m_iter; }

					base_class m_iter;

					MSE_IMPL_MEMBER_GETTER_DECLARATIONS(m_iter, contained_any);

					MSE_IMPL_ANY_CONTAINED_ANY_FRIEND_DECLARATIONS1;

					template <typename _Ty2> friend class TXScopeLHNullableAnyRandomAccessIterator;
					template <typename _Ty2> friend class TLHNullableAnyRandomAccessIteratorBase;

					template<typename ValueType2, typename _Ty2, typename retval_t2>
					friend inline retval_t2 maybe_any_cast(const TLHNullableAnyRandomAccessIteratorBase<_Ty2>& operand);
				};

				template<typename ValueType, typename _Ty, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
				inline retval_t maybe_any_cast(const TLHNullableAnyRandomAccessIteratorBase<_Ty>& operand) {
					/* We need to obtain the "any" object that actually stores the value. In this case, that object is "buried" within layers
					of base classes and member fields, so it's going to take a few operations to get to it. */
					auto nullable_any_ra_iter_base1 = operand.m_iter;
					if (!nullable_any_ra_iter_base1) {
						return retval_t{};
					}
					auto nullable_any_ra_iter1 = mse::TNullableAnyRandomAccessIterator<_Ty>(std::move(nullable_any_ra_iter_base1));
					auto any_ra_iter1 = mse::not_null_from_nullable(nullable_any_ra_iter1);
					auto any1 = mse::us::impl::ns_any::contained_any(any_ra_iter1);

#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT1(target_type, type1) \
					{ \
						auto cast_ptr = mse::us::impl::ns_any::any_cast<mse::us::impl::TCommonizedRandomAccessIterator<target_type, type1>>(std::addressof(any1)); \
						if (cast_ptr) { \
							return retval_t{ mse::lh::impl::explicitly_castable_any::convert<T1>((*cast_ptr).m_random_access_iterator) }; \
						} \
					}

#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT2(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT1(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT1(target_type, mse::impl::remove_const_t<type1>)

#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT3(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT2(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT2(target_type, typename mse::lh::impl::NDNoradWrapped<type1>::type) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT2(target_type, typename mse::lh::impl::NDRegisteredWrapped<type1>::type)

#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT3(target_type, type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT3(target_type, mse::impl::remove_const_t<type1>)

#if !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT5(type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(_Ty, type1); \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(_Ty, typename mse::impl::corresponding_type_with_nonconst_target<type1>::type); \
					MSE_IF_CONSTEXPR(!std::is_const<mse::impl::target_type<typename mse::impl::corresponding_type_with_nonconst_target<type1>::type> >::value) { \
						MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(mse::impl::remove_const_t<_Ty>, typename mse::impl::corresponding_type_with_nonconst_target<type1>::type); \
					}
#else // !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))
#define MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT5(type1) \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(_Ty, type1); \
					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT4(_Ty, typename mse::impl::corresponding_type_with_nonconst_target<type1>::type);
#endif // !(defined(_MSC_VER) && !defined(MSE_HAS_CXX17))

					typedef ValueType T1;

					MSE_IMPL_LH_MAYBE_ANYCAST_CAST_ATTEMPT5(T1);

					return retval_t{};
				}
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
			TLHNullableAnyRandomAccessIterator(void* src) : base_class((_Ty*)src) {}
			template <typename _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const void* src) : base_class((_Ty2*)src) {}

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

			/* Btw things like this akward use of a "construction_helper" is only to support compatibility with older versions of the microsoft compiler whose support for sfinae was limited. */
			static auto construction_helper1(std::true_type, const const_void_star_replacement& src) -> TLHNullableAnyRandomAccessIterator<const _Ty>;
			template<typename _Ty2>
			static auto construction_helper1(std::false_type, const _Ty2& src) { return src; }

			template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<(
					(!std::is_convertible<_TRandomAccessIterator1, TLHNullableAnyRandomAccessIterator>::value)
					&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
					&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
					&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
					&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
					&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
					&& (mse::impl::is_potentially_not_xscope<_TRandomAccessIterator1>::value)
					//&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
				) || (std::is_base_of<const_void_star_replacement, _TRandomAccessIterator1>::value && std::is_const<_Ty>::value)
			> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator)  MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TRandomAccessIterator1)")
				: base_class(construction_helper1(typename std::is_base_of<const_void_star_replacement, _TRandomAccessIterator1>::type(), random_access_iterator))
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
	/* Since there is no mse::TLHNullableAnyRandomAccessConstIterator<> at the moment, we'll designate mse::TLHNullableAnyRandomAccessIterator<const T>
	to be the "const" counterpart to mse::TLHNullableAnyRandomAccessIterator<T>. */
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::lh::TLHNullableAnyRandomAccessIterator);
	namespace lh {

		template <typename _Ty, typename _Tz, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_constructible<_Ty, TLHNullableAnyRandomAccessIterator<_Tz> >::value)
			&& (std::is_constructible<TLHNullableAnyRandomAccessIterator<_Tz>, _Ty>::value)> MSE_IMPL_EIS >
		auto operator-(_Ty const& lhs_cref, TLHNullableAnyRandomAccessIterator<_Tz> const& rhs_cref) -> typename TLHNullableAnyRandomAccessIterator<_Tz>::difference_type {
			return -(rhs_cref - lhs_cref);
		}

		template<typename ValueType, typename _Ty, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
		inline retval_t maybe_any_cast(const TLHNullableAnyRandomAccessIterator<_Ty>& operand) {
			return mse::lh::us::impl::maybe_any_cast<ValueType>(mse::us::impl::as_ref<const typename TLHNullableAnyRandomAccessIterator<_Ty>::base_class>(operand));
		}
	}
	namespace us {
		namespace lh {
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty* iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, _Ty, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<_Tx2>(iter));
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(const _Ty& iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(iter);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty& iter) {
				/* Note that, for example, in the case of mse::lh::TNativeArrayReplacement<>, its "operator*()" and "operator*() const"
				return different types. (Specifically, they return types that differ by a const qualifier.) */
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
				return mse::lh::TLHNullableAnyRandomAccessIterator<_Tx2>(iter);
			}
			template <typename _Tx = void, typename _Ty = void>
			auto unsafe_make_lh_nullable_any_random_access_iterator_from(_Ty&& iter) {
				typedef mse::impl::conditional_t<std::is_same<_Tx, void>::value, mse::impl::target_type<_Ty>, _Tx> _Tx2;
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
			TXScopeLHNullableAnyRandomAccessIterator(void* src) : base_class((_Ty*)src) {}
			template <typename _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(const void* src) : base_class((_Ty2*)src) {}

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

			template <typename _TRandomAccessIterator1, MSE_IMPL_EIP mse::impl::enable_if_t<(
					(!std::is_convertible<_TRandomAccessIterator1, TXScopeLHNullableAnyRandomAccessIterator>::value)
					&& (!std::is_base_of<base_class, _TRandomAccessIterator1>::value)
					&& (!std::is_same<_TRandomAccessIterator1, std::nullptr_t>::value)
					&& (!std::is_same<_TRandomAccessIterator1, NULL_t>::value)
					&& (!std::is_same<_TRandomAccessIterator1, ZERO_LITERAL_t>::value)
					&& MSE_IMPL_TARGET_CAN_BE_COMMONIZED_REFERENCED_AS_CRITERIA1(_TRandomAccessIterator1, _Ty)
					//&& (mse::impl::HasOrInheritsPlusEqualsOperator_msemsearray<_TRandomAccessIterator1>::value)
				) || (std::is_base_of<const_void_star_replacement, _TRandomAccessIterator1>::value && std::is_const<_Ty>::value)
			> MSE_IMPL_EIS >
			TXScopeLHNullableAnyRandomAccessIterator(const _TRandomAccessIterator1& random_access_iterator) 
				: base_class(construction_helper1(typename std::is_base_of<const_void_star_replacement, _TRandomAccessIterator1>::type(), random_access_iterator)) {}

			friend void swap(TXScopeLHNullableAnyRandomAccessIterator& first, TXScopeLHNullableAnyRandomAccessIterator& second) {
				swap(first.contained_iter(), second.contained_iter());
			}

			MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeLHNullableAnyRandomAccessIterator);

			void async_not_shareable_and_not_passable_tag() const {}

		private:

			/* Btw things like this akward use of a "construction_helper" is only to support compatibility with older versions of the microsoft compiler whose support for sfinae was limited. */
			static auto construction_helper1(std::true_type, const const_void_star_replacement& src) -> TXScopeLHNullableAnyRandomAccessIterator<const _Ty>;
			template<typename _Ty2>
			static auto construction_helper1(std::false_type, const _Ty2& src) { return src; }

			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

			const base_class& contained_iter() const& { return (*this); }
			//const base_class& contained_iter() const&& { return (*this); }
			base_class& contained_iter()& { return (*this); }
			base_class&& contained_iter()&& { return std::move(*this); }
		};
	}
	/* Since there is no mse::TXScopeLHNullableAnyRandomAccessConstIterator<> at the moment, we'll designate mse::TXScopeLHNullableAnyRandomAccessIterator<const T>
	to be the "const" counterpart to mse::TXScopeLHNullableAnyRandomAccessIterator<T>. */
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::lh::TXScopeLHNullableAnyRandomAccessIterator);
	namespace lh {

		template <typename _Ty, typename _Tz, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_constructible<_Ty, TXScopeLHNullableAnyRandomAccessIterator<_Tz> >::value)
			&& (std::is_constructible<TXScopeLHNullableAnyRandomAccessIterator<_Tz>, _Ty>::value)> MSE_IMPL_EIS >
		auto operator-(_Ty const& lhs_cref, TXScopeLHNullableAnyRandomAccessIterator<_Tz> const& rhs_cref) -> typename TXScopeLHNullableAnyRandomAccessIterator<_Tz>::difference_type {
			return -(rhs_cref - lhs_cref);
		}

		template<typename ValueType, typename _Ty, typename retval_t = typename std::conditional<mse::impl::is_xscope<ValueType>::value, mse::xscope_fixed_optional<ValueType>, mse::fixed_optional<ValueType> >::type >
		inline retval_t maybe_any_cast(const TXScopeLHNullableAnyRandomAccessIterator<_Ty>& operand) {
			return mse::lh::us::impl::maybe_any_cast<ValueType>(mse::us::impl::as_ref<const typename TXScopeLHNullableAnyRandomAccessIterator<_Ty>::base_class>(operand));
		}

		namespace impl {
			template <typename _Ty>
			using TStrongTargetVectorBase =
#ifndef MSE_LEGACYHELPERS_DISABLED
				mse::stnii_vector<mse::impl::remove_const_t<_Ty> >
#else //MSE_LEGACYHELPERS_DISABLED
				std::vector<mse::impl::remove_const_t<_Ty> >
#endif //MSE_LEGACYHELPERS_DISABLED
				;
		}
		template <typename _Ty>
		class TStrongTargetVector : public impl::TStrongTargetVectorBase<_Ty> {
		public:
			typedef impl::TStrongTargetVectorBase<_Ty> base_class;
			typedef TStrongTargetVector _Myt;

			typedef typename base_class::allocator_type allocator_type;
			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

			typedef typename base_class::iterator iterator;
			typedef typename base_class::const_iterator const_iterator;
			typedef typename base_class::reverse_iterator reverse_iterator;
			typedef typename base_class::const_reverse_iterator const_reverse_iterator;

			//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
			typedef typename base_class::xscope_const_iterator xscope_const_iterator;
			typedef typename base_class::xscope_iterator xscope_iterator;

			MSE_USING(TStrongTargetVector, base_class);

			TStrongTargetVector(_XSTD initializer_list<value_type> _Ilist) : base_class(_Ilist) {}

			TStrongTargetVector(const TStrongTargetVector&) = default;
			TStrongTargetVector(TStrongTargetVector&&) = default;

			TStrongTargetVector& operator=(TStrongTargetVector&& _X) = default;
			TStrongTargetVector& operator=(const TStrongTargetVector& _X) = default;

			friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				const base_class& lhs = _Left_cref;
				const base_class& rhs = _Right_cref;
				return (lhs == rhs);
			}
			MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(TStrongTargetVector)
#ifndef MSE_HAS_CXX20
				friend bool operator<(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				const base_class& lhs = _Left_cref; const base_class& rhs = _Right_cref;
				return (lhs < rhs);
			}
#else // !MSE_HAS_CXX20
				friend std::strong_ordering operator<=>(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				return spaceship_operator_equivalent<base_class>(_Left_cref, _Right_cref);
			}
#endif // !MSE_HAS_CXX20
		} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)");


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
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_constructible<base_class, _Ty2>::value> MSE_IMPL_EIS >
			TStrongVectorIterator(const _Ty2& src) : base_class(src) {}
			/* turns out that size_type and NULL_t could be the same type on some platforms */
			//explicit TStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(_N), 0) {}
			explicit TStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<TStrongTargetVector<_Ty>>(_N, _V), 0) {}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, ZERO_LITERAL_t>::value) || (std::is_same<_Ty2, NULL_t>::value)> MSE_IMPL_EIS >
			TStrongVectorIterator(_Ty2 val) : TStrongVectorIterator() {
				/* This constructor is just to support zero being used as a null pointer value. */
				assert(0 == val);
			}
			template <typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<((std::is_same<_Ty2, mse::impl::remove_const_t<_Ty> >::value) && (!std::is_same<_Ty2, _Ty>::value))> MSE_IMPL_EIS >
			TStrongVectorIterator(const TStrongVectorIterator<_Ty2>& src) 
				/* TStrongVectorIterator<const T> should be structurally identical to TStrongVectorIterator<T>, right? So the following reinterpret_cast<>() should be fine, right? */
				: base_class(reinterpret_cast<const TStrongVectorIterator&>(src)) {}
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
			template<typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, mse::impl::remove_const_t<_Ty> >::value) && (!std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TStrongVectorIterator& _Left_cref, const TStrongVectorIterator<_Ty2>& _Right_cref) {
				/* TStrongVectorIterator<const T> should be structurally identical to TStrongVectorIterator<T>, right? So the following reinterpret_cast<>() should be fine, right? */
				return (_Left_cref == reinterpret_cast<const TStrongVectorIterator&>(_Right_cref));
			}
			template<typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, mse::impl::remove_const_t<_Ty> >::value) && (!std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			friend bool operator==(const TStrongVectorIterator<_Ty2>& _Left_cref, const TStrongVectorIterator& _Right_cref) {
				/* TStrongVectorIterator<const T> should be structurally identical to TStrongVectorIterator<T>, right? So the following reinterpret_cast<>() should be fine, right? */
				return (reinterpret_cast<const TStrongVectorIterator&>(_Left_cref) == _Right_cref);
			}
#ifndef MSE_HAS_CXX20
			friend bool operator!=(const TStrongVectorIterator& _Left_cref, const TStrongVectorIterator& _Right_cref) { return !(_Left_cref == _Right_cref); }
			template<typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, mse::impl::remove_const_t<_Ty> >::value) && (!std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TStrongVectorIterator& _Left_cref, const TStrongVectorIterator<_Ty2>& _Right_cref) { return !(_Left_cref == _Right_cref); }
			template<typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, mse::impl::remove_const_t<_Ty> >::value) && (!std::is_same<_Ty2, _Ty>::value)> MSE_IMPL_EIS >
			friend bool operator!=(const TStrongVectorIterator<_Ty2>& _Left_cref, const TStrongVectorIterator& _Right_cref) { return !(_Left_cref == _Right_cref); }
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
	}
	/* Since there is no mse::TStrongVectorConstIterator<> at the moment, we'll designate mse::TStrongVectorIterator<const T>
	to be the "const" counterpart to mse::TStrongVectorIterator<T>. */
	MSE_IMPL_CORRESPONDING_TYPE_WITH_CONST_TARGET_SPECIALIZATION_IN_IMPL_NAMESPACE(mse::lh::TStrongVectorIterator);
	namespace lh {

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
		class TXScopeStrongVectorIterator : public mse::TXScopeRAIterator<mse::TRefCountingPointer<TStrongTargetVector<_Ty> > > {
		public:
			typedef mse::TXScopeRAIterator<mse::TRefCountingPointer<TStrongTargetVector<_Ty> > > base_class;
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
			operator typename mse::mstd::array<_Ty, _Size>::const_iterator() const {
				return base_class::cbegin();
			}
			template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			operator mse::TNullableAnyRandomAccessIterator<const _Ty>() const {
				return base_class::begin();
			}

			mse::TNullableAnyRandomAccessIterator<_Ty> operator&() {
				return mse::TNullableAnyRandomAccessIterator<_Ty>(*this);
			}
			mse::TNullableAnyRandomAccessIterator<const _Ty> operator&() const {
				return mse::TNullableAnyRandomAccessIterator<const _Ty>(*this);
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

		namespace impl {
			template <class _Ty>
			struct lh_decay : std::decay<_Ty> {};

			template<class _udTy, size_t _Size>
			struct lh_decay<TNativeArrayReplacement<_udTy, _Size> > : std::decay<TLHNullableAnyRandomAccessIterator<const_preserving_decay_t<_udTy> > > {};
			template<class _udTy, size_t _Size>
			struct lh_decay<const TNativeArrayReplacement<_udTy, _Size> > : std::decay<TLHNullableAnyRandomAccessIterator<const const_preserving_decay_t<_udTy> > > {};

			/* todo: decay from function to TNativeFunctionPointerReplacement<>? */

			template <class _Ty>
			auto as_lh_decayed(_Ty const& src) { return typename lh_decay<const _Ty>::type(src); }
			template <class _Ty>
			auto as_lh_decayed(_Ty& src) { return typename lh_decay<_Ty>::type(src); }
			template <class _Ty>
			auto as_lh_decayed(_Ty&& src) { return typename lh_decay<_Ty>::type(MSE_FWD(src)); }
		}

		template<class _Fty>
		class TNativeFunctionPointerReplacement : public mse::mstd::function<_Fty> {
		public:
			typedef mse::mstd::function<_Fty> base_class;
			typedef TNativeFunctionPointerReplacement _Myt;
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
				constructor_helper1(typename std::is_function<mse::impl::target_or_void_type<_Fty2> >::type(), func);
				mse::impl::T_valid_if_not_an_xscope_type<_Fty2>();
			}
			template <typename _Fty2, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_convertible<const _Fty2*, const TNativeFunctionPointerReplacement*>::value)
				&& (!std::is_convertible<_Fty2, std::nullptr_t>::value) && (!std::is_same<_Fty2, int>::value)
				&& (!std::is_same<_Fty2, ZERO_LITERAL_t>::value) && (!std::is_same<_Fty2, NULL_t>::value) && (mse::impl::is_potentially_not_xscope<_Fty2>::value)
				&& (std::is_constructible<base_class, _Fty2>::value)
			> MSE_IMPL_EIS >
			TNativeFunctionPointerReplacement(_Fty2&& func) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Fty2)") : base_class(MSE_FWD(func)) {
				constructor_helper1(typename std::is_function<mse::impl::target_or_void_type<_Fty2> >::type(), MSE_FWD(func));
				mse::impl::T_valid_if_not_an_xscope_type<_Fty2>();
			}

			base_class const& operator*() const { return (*this); }
			auto operator->() const { return std::addressof(operator*()); }

			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
			friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
				if (!_Left_cref) {
					return !_Right_cref;
				}
				else if (!_Right_cref) {
					return false;
				}
				if (_Left_cref.m_maybe_shadow_fn_ptr.has_value() && _Right_cref.m_maybe_shadow_fn_ptr.has_value()) {
					/* If the stored function objects are function pointers, then this equality comparison operator should compare their values 
					as expected. Otherwise, this operator will indicate whether or not the two arguments were constructed with references to 
					the same (function) object instance, which is not quite the same as comparing whether their values are "equal". */
					auto& lhs_shadow_fn_ptr = _Left_cref.m_maybe_shadow_fn_ptr.value();
					auto& rhs_shadow_fn_ptr = _Right_cref.m_maybe_shadow_fn_ptr.value();
					return (lhs_shadow_fn_ptr == rhs_shadow_fn_ptr);
				}
				return false;
			}

#ifndef MSE_HAS_CXX20
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }

			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }

			friend bool operator!=(const _Myt& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
#endif // !MSE_HAS_CXX20
		private:

			template <typename FnPtr>
			void constructor_helper1(std::true_type, FnPtr const& fn_ptr) {
				m_maybe_shadow_fn_ptr = (void const*)std::addressof(*fn_ptr);
			}
			template <typename _Fty2>
			void constructor_helper1(std::false_type, _Fty2 const& func) {
				/* We're potentially taking the address of a lambda here. Is that copacetic? */
				m_maybe_shadow_fn_ptr = (void const*)std::addressof(func);
			}

			/* We store an auxiliary pointer in attempt to support an equality comparison operator (that is notably not supported by 
			the underlying (std::function) base class). */
			mse::us::impl::optional1<void const*> m_maybe_shadow_fn_ptr = nullptr;
		};
	}
	namespace us {
		namespace lh {
			namespace impl {
#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4127 )
#endif /*_MSC_VER*/

				/* CSAllocRegistry essentially just maintains a list of all objects allocated by a registered "new" call and not (yet)
				subsequently deallocated with a corresponding registered delete. */
				class CSAllocRegistry {
				public:
					typedef std::function<bool(void*)> deleter_t;
					struct CAllocInfo1 {
						size_t num_bytes = 0;
						deleter_t deleter;
					};
					CSAllocRegistry() {}
					~CSAllocRegistry() {}
					bool registerPointer(void* alloc_ptr, size_t num_bytes, deleter_t deleter) {
						if (nullptr == alloc_ptr) { return true; }
						{
							if (1 <= sc_fs1_max_objects) {
								/* We'll add this object to fast storage. */
								if (sc_fs1_max_objects == m_num_fs1_objects) {
									/* Too many objects. We're gonna move the oldest object to slow storage. */
									moveObjectFromFastStorage1ToSlowStorage(0);
								}
								auto& fs1_object_ref = m_fs1_objects[m_num_fs1_objects];
								fs1_object_ref = { alloc_ptr, {num_bytes, std::move(deleter)} };
								m_num_fs1_objects += 1;
								return true;
							}
							else {
								/* Add the mapping to slow storage. */
								pointer_to_alloc_info_map_t::value_type item({ alloc_ptr, {num_bytes, std::move(deleter)} });
								m_pointer_to_alloc_info_map.insert(item);
							}
						}
						return true;
					}
					bool unregisterPointer(void* alloc_ptr) {
						if (nullptr == alloc_ptr) { return true; }
						bool retval = false;
						{
							/* check if the object is in "fast storage 1" first */
							for (int i = (m_num_fs1_objects - 1); i >= 0; i -= 1) {
								if (alloc_ptr == m_fs1_objects[i].first) {
									removeObjectFromFastStorage1(i);
									return true;
								}
							}

							/* The object was not in "fast storage 1". It's proably in "slow storage". */
							auto num_erased = m_pointer_to_alloc_info_map.erase(alloc_ptr);
							if (1 <= num_erased) {
								retval = true;
							}
						}
						return retval;
					}
					bool registerPointer(const void* alloc_ptr, size_t num_bytes, deleter_t const& deleter) { return (*this).registerPointer(const_cast<void*>(alloc_ptr), num_bytes, deleter); }
					bool unregisterPointer(const void* alloc_ptr) { return (*this).unregisterPointer(const_cast<void*>(alloc_ptr)); }
					void reserve_space_for_one_more() {
						/* The purpose of this function is to ensure that the next call to registerPointer() won't
						need to allocate more memory, and thus won't have any chance of throwing an exception due to
						memory allocation failure. */
						m_pointer_to_alloc_info_map.reserve(m_pointer_to_alloc_info_map.size() + 1);
					}
					mse::xscope_fixed_optional<CAllocInfo1*> allocation_info_ptr_if_present(void* alloc_ptr) {
						mse::xscope_fixed_optional<CAllocInfo1*> retval;
						auto found_it = m_pointer_to_alloc_info_map.find(alloc_ptr);
						if (m_pointer_to_alloc_info_map.end() != found_it) {
							return &(found_it->second);
						}
						return retval;
					}
					mse::xscope_fixed_optional<deleter_t*> deleter_ptr_if_present(void* alloc_ptr) {
						mse::xscope_fixed_optional<deleter_t*> retval;
						auto found_it = m_pointer_to_alloc_info_map.find(alloc_ptr);
						if (m_pointer_to_alloc_info_map.end() != found_it) {
							return &(found_it->second.deleter);
						}
						return retval;
					}

					bool isEmpty() const { return ((0 == m_num_fs1_objects) && (0 == m_pointer_to_alloc_info_map.size())); }

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
						m_pointer_to_alloc_info_map.insert(fs1_object_ref);
						/* Then we're gonna remove the object from fast storage */
						removeObjectFromFastStorage1(fs1_obj_index);
					}

					//typedef void* CFS1Object;
					typedef std::unordered_map<void*, CAllocInfo1> pointer_to_alloc_info_map_t;
					typedef std::pair<void*, CAllocInfo1> CFS1Object;

#ifndef MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
#define MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS 8/* Arbitrary. The optimal number depends on how slow "slow storage" is. */
#endif // !MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS
					MSE_CONSTEXPR static const int sc_fs1_max_objects = MSE_SALLOC_REGISTRY_FS1_MAX_OBJECTS;
					CFS1Object m_fs1_objects[sc_fs1_max_objects];
					int m_num_fs1_objects = 0;

					/* "slow storage" */
					pointer_to_alloc_info_map_t m_pointer_to_alloc_info_map;
				};

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

				inline CSAllocRegistry& tlSAllocRegistry_ref() {
					thread_local static CSAllocRegistry tlSAllocRegistry;
					return tlSAllocRegistry;
				}
			}
		}
	}
	namespace lh {
		namespace impl {

			template<class TPtr>
			void free_valid_non_null(TPtr& ptr) {
				if (ptr) {
					auto void_ptr = (void*)mse::us::lh::make_raw_pointer_from(ptr);
					auto maybe_deleter_ptr = mse::us::lh::impl::tlSAllocRegistry_ref().deleter_ptr_if_present(void_ptr);
					if (maybe_deleter_ptr.has_value()) {
						auto& deleter_ptr = maybe_deleter_ptr.value();
						assert(deleter_ptr);
						(*deleter_ptr)(void_ptr);
					}
					/* Unregistering the iterator has the side effect of destroying the deleter lambda. If the deleter lambda captures a (shared
					owning) strong vector iterator by value, then the corresponding reference count will be decremented accordingly. */
					mse::us::lh::impl::tlSAllocRegistry_ref().unregisterPointer(void_ptr);
				}
				ptr = nullptr;
			}

			template<class _TPtr>
			class CAllocF {
			public:
				static void free(_TPtr& ptr) {
					if (!ptr) {
						return;
					}
					free_valid_non_null(ptr);
				}
				static void allocate(_TPtr& ptr, size_t num_bytes) {
					typedef mse::impl::target_type<_TPtr> target_t;
					if (0 == num_bytes) {
						ptr = nullptr;
					}
					else if (sizeof(target_t) == num_bytes) {
						ptr = mse::make_refcounting<target_t>();
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer(mse::us::lh::make_raw_pointer_from(ptr), sizeof(target_t), std::function<bool(void*)>{[ptr](void*) {
							/* The "deleter" (lambda) for a TRefCointingPointer<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the TRefCointingPointer<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
					}
					else {
						assert(false);
						ptr = mse::make_refcounting<target_t>();
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer(std::addressof(*ptr), sizeof(target_t), std::function<bool(void*)>{[ptr](void*) {
							/* The "deleter" (lambda) for a TRefCointingPointer<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the TRefCointingPointer<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
						//MSE_THROW(std::bad_alloc("the given allocation size is not supported for this pointer type - CAllocF<_TPtr>::allocate()"));
					}
				}
				//static bool reallocate(_TPtr& ptr, size_t num_bytes);
			};

			template<class _Ty>
			class CAllocF<_Ty*> {
			public:
				static void free(_Ty*& iter) {
					if (iter) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(_Ty*& iter, size_t num_bytes) {
					const size_t num_items = num_bytes / sizeof(_Ty);
					if (1 <= num_items) {
						/* We use calloc() to in an effort to maximize compatibility with unsafe legacy code. */
						iter = (_Ty*)::calloc(num_bytes, 1);
						if (!iter) {
							return;
						}
						MSE_IF_CONSTEXPR(std::is_default_constructible<_Ty>::value) {
							for (size_t i = 0; num_items > i; i += 1) {
								/* placement new() to initialize the allocation */
								new (iter + i) _Ty{};
							}
						}
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer((void*)mse::us::lh::make_raw_pointer_from(iter), num_bytes, std::function<bool(void*)>{[iter, num_bytes](void* viter) {
							_Ty* iter = (_Ty*)viter;
							if (!iter) {
								return false;
							}
							const size_t num_items = num_bytes / sizeof(_Ty);
							if (1 <= num_items) {
								MSE_IF_CONSTEXPR(std::is_default_constructible<_Ty>::value) {
									for (size_t i = 0; num_items > i; i += 1) {
										/* manually calling the destructor of each element */
										(*(iter + i)).~_Ty();
									}
								}
								::free(iter);
							}
							return true;
							} });
					}
					else {
						iter = nullptr;
					}
				}
				static bool reallocate(_Ty*& iter, size_t num_bytes) {
					const size_t new_num_items = num_bytes / sizeof(_Ty);
					_Ty* new_alloc_iter = nullptr;
					if (1 <= new_num_items) {
						/* Even though we're passed a raw pointer, it could have been (unsafely) converted from one of the library's (safe) smart pointers, 
						so we don't actually know how the raw pointer argument's target was allocated. Given that, this reallocate() function is always
						going to (try to) make a new allocation of the new given size, copy the contents from the original allocation, then (try to) 
						deallocate the original allocation. */
						allocate(new_alloc_iter, num_bytes);
						if (!new_alloc_iter) {
							return false;
						}
					}
					size_t source_num_items = 0;

					void* void_ptr = nullptr;
					typedef typename mse::us::lh::impl::CSAllocRegistry::CAllocInfo1 alloc_info_t;
					mse::us::impl::optional1<alloc_info_t*> maybe_alloc_info_ptr;
					if (iter) {
						/* We'll determine if the allocation is being tracked in the registry. */
						void_ptr = (void*)mse::us::lh::make_raw_pointer_from(iter);
						auto l_maybe_allocation_info_ptr = mse::us::lh::impl::tlSAllocRegistry_ref().allocation_info_ptr_if_present(void_ptr);

						if (!(l_maybe_allocation_info_ptr.has_value())) {
							/* This seems to be an allocation we're not tracking. This reallocate() function really should not be used with untracked allocations. */
							//MSE_THROW(std::logic_error("CAllocF<_Ty*>::reallocate() was passed a raw pointer pointing to an allocation that doesn't seem to have been allocated with lh::allocate(). This is not supported."));
							/* I suppose we could just assume the target allocation was malloc()ed. */
							auto new_alloc_vptr = ::realloc(iter, num_bytes);
							if (new_alloc_vptr) {
								iter = (_Ty*)new_alloc_vptr;
								return true;
							}
							return false;
						} else {
							auto* allocation_info_ptr = l_maybe_allocation_info_ptr.value();
							assert(allocation_info_ptr);
							source_num_items = allocation_info_ptr->num_bytes / sizeof(_Ty);
							maybe_alloc_info_ptr = allocation_info_ptr;
						}
					}
					const size_t min_num_items = std::min(new_num_items, source_num_items);
					for (size_t i = 0; min_num_items > i; i += 1) {
						/* Copying the contents of the original allocation to the new one. */
						*(new_alloc_iter + i) = std::move(*(iter + i));
					}
					if (maybe_alloc_info_ptr.has_value()) {
						/* Deallocate the original allocation. */
						auto& alloc_info_ptr = maybe_alloc_info_ptr.value();
						assert(alloc_info_ptr);
						alloc_info_ptr->deleter(void_ptr);
					}
					iter = new_alloc_iter;
					return true;
				}
			};
			template<class _Ty>
			void free_overloaded(_Ty*& iter) { CAllocF<_Ty*>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(_Ty*& iter, size_t num_bytes) { CAllocF<_Ty*>::allocate(iter, num_bytes); }
			template<class _Ty>
			bool reallocate_overloaded(_Ty*& iter, size_t num_bytes) { return CAllocF<_Ty*>::reallocate(iter, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TStrongVectorIterator<_Ty>> {
			public:
				static void free(mse::lh::TStrongVectorIterator<_Ty>& iter) {
					if (1 <= iter.size()) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(mse::lh::TStrongVectorIterator<_Ty>& iter, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					tmp.resize(num_bytes / sizeof(_Ty));
					iter = tmp;
					if (1 <= iter.size()) {
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer((void*)mse::us::lh::make_raw_pointer_from(iter), iter.size() * sizeof(_Ty), std::function<bool(void*)>{[iter](void*) {
							/* The "deleter" (lambda) for a lh::TStrongVectorIterator<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the lh::TStrongVectorIterator<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
					}
				}
				static bool reallocate(mse::lh::TStrongVectorIterator<_Ty>& iter, size_t num_bytes) {
					void* void_ptr = nullptr;
					typedef typename mse::us::lh::impl::CSAllocRegistry::deleter_t deleter_t;
					mse::us::impl::optional1<deleter_t*> maybe_deleter_ptr;
					if (1 <= iter.size()) {
						/* First we unregister the existing allocation, if any, at its current memory address. */
						void_ptr = (void*)mse::us::lh::make_raw_pointer_from(iter);
						auto l_maybe_deleter_ptr = mse::us::lh::impl::tlSAllocRegistry_ref().deleter_ptr_if_present(void_ptr);
						if (l_maybe_deleter_ptr.has_value()) {
							maybe_deleter_ptr = l_maybe_deleter_ptr.value();
						}
						mse::us::lh::impl::tlSAllocRegistry_ref().unregisterPointer(void_ptr);
					}

					iter.resize(num_bytes / sizeof(_Ty));

					if (1 <= iter.size()) {
						auto new_rawptr = (void*)mse::us::lh::make_raw_pointer_from(iter);
						/* And now we (re)register the new allocation at its (possibly) new memory address. */
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer(new_rawptr, iter.size() * sizeof(_Ty), std::function<bool(void*)>{[iter](void*) {
							/* The "deleter" (lambda) for a lh::TStrongVectorIterator<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the lh::TStrongVectorIterator<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
					}
					return true;
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TStrongVectorIterator<_Ty>& iter) { CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TStrongVectorIterator<_Ty>& iter, size_t num_bytes) { CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::allocate(iter, num_bytes); }
			template<class _Ty>
			bool reallocate_overloaded(mse::lh::TStrongVectorIterator<_Ty>& iter, size_t num_bytes) { return CAllocF<mse::lh::TStrongVectorIterator<_Ty>>::reallocate(iter, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>> {
			public:
				static void free(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter) {
					if (1 <= iter.size()) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter, size_t num_bytes) {
					mse::lh::TXScopeStrongVectorIterator<_Ty> tmp;
					tmp.resize(num_bytes / sizeof(_Ty));
					iter = tmp;
					if (1 <= iter.size()) {
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer((void*)mse::us::lh::make_raw_pointer_from(iter), iter.size() * sizeof(_Ty), std::function<bool(void*)>{[iter](void*) {
							/* The "deleter" (lambda) for a lh::TStrongVectorIterator<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the lh::TStrongVectorIterator<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
					}
				}
				static bool reallocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter, size_t num_bytes) {
					void* void_ptr = nullptr;
					typedef typename mse::us::lh::impl::CSAllocRegistry::deleter_t deleter_t;
					mse::us::impl::optional1<deleter_t*> maybe_deleter_ptr;
					if (1 <= iter.size()) {
						/* First we unregister the existing allocation, if any, at its current memory address. */
						void_ptr = (void*)mse::us::lh::make_raw_pointer_from(iter);
						auto l_maybe_deleter_ptr = mse::us::lh::impl::tlSAllocRegistry_ref().deleter_ptr_if_present(void_ptr);
						if (l_maybe_deleter_ptr.has_value()) {
							maybe_deleter_ptr = l_maybe_deleter_ptr.value();
						}
						mse::us::lh::impl::tlSAllocRegistry_ref().unregisterPointer(void_ptr);
					}

					iter.resize(num_bytes / sizeof(_Ty));

					if (1 <= iter.size()) {
						auto new_rawptr = (void*)mse::us::lh::make_raw_pointer_from(iter);
						/* And now we (re)register the new allocation at its (possibly) new memory address. */
						mse::us::lh::impl::tlSAllocRegistry_ref().registerPointer(new_rawptr, iter.size() * sizeof(_Ty), std::function<bool(void*)>{[iter](void*) {
							/* The "deleter" (lambda) for a lh::TStrongVectorIterator<> doesn't need to explicitly do anything. The fact that this lambda
							captures a copy of the lh::TStrongVectorIterator<> means that target object will not be deallocated before (the last copy of)
							this "deleter" lambda itself is destroyed. */
							return true;
							} });
					}
					return true;
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter) { CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter, size_t num_bytes) { CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::allocate(iter, num_bytes); }
			template<class _Ty>
			bool reallocate_overloaded(mse::lh::TXScopeStrongVectorIterator<_Ty>& iter, size_t num_bytes) { return CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>>::reallocate(iter, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
			public:
				static void free(mse::TNullableAnyRandomAccessIterator<_Ty>& iter) {
					if (1 <= iter.size()) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(mse::TNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					CAllocF<mse::lh::TStrongVectorIterator<_Ty> >::allocate(tmp, num_bytes);
					iter = tmp;
				}
				//static bool reallocate(mse::TNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes);
			};
			template<class _Ty>
			void free_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& iter) { CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::allocate(iter, num_bytes); }
			//template<class _Ty>
			//bool reallocate_overloaded(mse::TNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { return CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>>::reallocate(iter, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>> {
			public:
				static void free(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter) {
					if (iter) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					CAllocF<mse::lh::TStrongVectorIterator<_Ty> >::allocate(tmp, num_bytes);
					iter = tmp;
				}
				static bool reallocate(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) {
					if (!iter) {
						allocate(iter, num_bytes);
						return bool(iter);
					}
					auto maybe_strong_vec_iter = mse::lh::us::impl::maybe_any_cast<mse::lh::TStrongVectorIterator<_Ty>>(iter);
					if (maybe_strong_vec_iter.has_value()) {
						auto& strong_vec_iter = maybe_strong_vec_iter.value();
						bool res = CAllocF<mse::lh::TStrongVectorIterator<_Ty> >::reallocate(strong_vec_iter, num_bytes);
						if (res) {
							iter = strong_vec_iter;
						}
						return res;
					}
					MSE_THROW(std::logic_error("the provided pointer/iterator was not recognized as pointing to a target supported for reallocation - CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::reallocate()"));
					return false;
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter) { CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::allocate(iter, num_bytes); }
			template<class _Ty>
			bool reallocate_overloaded(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { return CAllocF<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>>::reallocate(iter, num_bytes); }

			template<class _Ty>
			class CAllocF<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>> {
			public:
				static void free(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter) {
					if (iter) {
						free_valid_non_null(iter);
					}
					iter = nullptr;
				}
				static void allocate(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) {
					mse::lh::TStrongVectorIterator<_Ty> tmp;
					CAllocF<mse::lh::TStrongVectorIterator<_Ty> >::allocate(tmp, num_bytes);
					iter = tmp;
				}
				static bool reallocate(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) {
					if (!iter) {
						allocate(iter, num_bytes);
						return bool(iter);
					}
					{
						auto maybe_strong_vec_iter = mse::lh::us::impl::maybe_any_cast<mse::lh::TStrongVectorIterator<_Ty>>(iter);
						if (maybe_strong_vec_iter.has_value()) {
							auto& strong_vec_iter = maybe_strong_vec_iter.value();
							bool res = CAllocF<mse::lh::TStrongVectorIterator<_Ty> >::reallocate(strong_vec_iter, num_bytes);
							if (res) {
								iter = strong_vec_iter;
							}
							return res;
						}
					}
					{
						auto maybe_strong_vec_iter = mse::lh::us::impl::maybe_any_cast<mse::lh::TXScopeStrongVectorIterator<_Ty>>(iter);
						if (maybe_strong_vec_iter.has_value()) {
							auto& strong_vec_iter = maybe_strong_vec_iter.value();
							bool res = CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty> >::reallocate(strong_vec_iter, num_bytes);
							if (res) {
								iter = strong_vec_iter;
							}
							return res;
						}
					}
					MSE_THROW(std::logic_error("the provided pointer/iterator was not recognized as pointing to a target supported for reallocation - CAllocF<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>>::reallocate()"));
					return false;
				}
			};
			template<class _Ty>
			void free_overloaded(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter) { CAllocF<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>>::free(iter); }
			template<class _Ty>
			void allocate_overloaded(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { CAllocF<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>>::allocate(iter, num_bytes); }
			template<class _Ty>
			bool reallocate_overloaded(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>& iter, size_t num_bytes) { return CAllocF<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<_Ty>>::reallocate(iter, num_bytes); }

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
				bool res = impl::reallocate_helper1(typename impl::IsSupportedByAllocateOverloaded<_TDynArrayIter>::type(), ptr, num_bytes);
				if (!res) {
					return _TDynArrayIter();
				}
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
				/* element_t is presumably either some kind of char (or int8_t) that is one byte in size, or it is some object type whose 
				size is matches the given size parameter. But technically it doesn't have to be either.  */
				static_assert(!std::is_pointer<element_t>::value, "lh::fread()ing pointers is not supported. (Not that anyone would do such a thing.)) ");
				thread_local std::vector<unsigned char> v;
				v.resize(size * count);
				auto num_items_read = ::fread(v.data(), size, count, stream);
				auto num_bytes_read = num_items_read * size;
				auto num_elements_read = num_bytes_read / sizeof(element_t);
				size_t uc_index = 0;
				size_t element_index = 0;
				for (; element_index < num_elements_read; uc_index += sizeof(element_t), element_index += 1) {
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
				return num_items_read;
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
			/* element_t is presumably either some kind of char (or int8_t) that is one byte in size, or it is some object type whose
			size is matches the given size parameter. But technically it doesn't have to be either.  */
			auto num_elements_to_write = size * count / sizeof(element_t);
			thread_local std::vector<unsigned char> v;
			v.resize(size * count);
			size_t uc_index = 0;
			size_t element_index = 0;
			for (; element_index < num_elements_to_write; uc_index += sizeof(element_t), element_index += 1) {
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

		/* Memory safe approximation of memcpy(). */
		template<class _TIter, class _TIter2>
		_TIter memcpy(_TIter const& destination, _TIter2 const& source, size_t num_bytes);
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement, class _TIter2>
		_TElement* memcpy(_TElement* destination, _TIter2 const& source, size_t num_bytes);
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size, class _TIter2>
		auto memcpy(TNativeArrayReplacement<_udTy, _Size>& nar, _TIter2 const& source, size_t num_bytes) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			//typename mse::mstd::array<_Ty, _Size>::iterator destination = nar;
			mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> destination = nar;
			return memcpy(destination, source, num_bytes);
		}

		namespace impl {
			template <typename T, typename = void>
			struct HasOrInheritsSubscriptOperator : std::false_type {};
			template <typename T>
			struct HasOrInheritsSubscriptOperator<T, mse::impl::void_t<decltype(std::declval<T>()[0])> > : std::true_type {};

			namespace us {

				template<class _TPointer, class _TPointer2>
				_TPointer memcpy_helper3(std::false_type, _TPointer const& destination, _TPointer2 const& source, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype(*destination)> element_t;
					if ((sizeof(element_t) != num_bytes) && (0 != num_bytes)) {
						typedef mse::impl::remove_reference_t<decltype(*source)> source_element_t;
						if (sizeof(source_element_t) != num_bytes) {
							auto num_items = num_bytes / sizeof(element_t);
							if (num_items * sizeof(element_t) != num_bytes) {
								num_items = num_bytes / sizeof(source_element_t);
								if (num_items * sizeof(source_element_t) != num_bytes) {
									MSE_THROW(std::logic_error("lh::memcpy(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
										"Note that the library's safe types may be different in size to the original types they were designed to replace "
										"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
								}
							}
							MSE_THROW(std::logic_error("lh::memcpy(): Either the source or destination arguments (or both) is deemed to be a pointer that is not an iterator, "
								"but the given 'number of bytes' argument does not seem to match the size of the pointer's target element type. "
								"If the source and destination arguments are intended to point (in)to buffers, then the pointer argument(s) need to be replaced with iterators. "));
						}
					}

					*destination = *source;
					return destination;
				}

				template<class _TIter, class _TIter2>
				_TIter memcpy_helper3(std::true_type, _TIter const& destination, _TIter2 const& source, size_t num_bytes) {
					//static_assert(std::is_same<mse::lh::void_star_replacement, _TIter>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TIter>::value, "");

					/* Here the destination parameter is presumed to be an lh::void_star_replacement. In order to complete the operation safely we
					need to guess the type stored in the lh::void_star_replacement and recover it. We're going to use some macros to help
					generate a whole bunch of guesses. */

					typedef mse::impl::target_type<_TIter2> element_t;
					auto& ecany_ref = mse::us::impl::as_ref<const mse::lh::impl::explicitly_castable_any>(destination);

#define MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT1(type1) \
					{ \
						auto maybe_casted = mse::lh::impl::maybe_any_cast_of_explicitly_castable_any<type1>(ecany_ref); \
						if (maybe_casted.has_value()) { \
							return mse::lh::memcpy(mse::lh::impl::explicitly_castable_any::convert<type1>(maybe_casted.value()), source, num_bytes); \
						} \
					}

#define MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT2(type1, MACRO_FUNCTION) \
					{ \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TStrongVectorIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeStrongVectorIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradNotNullPointer<type1>); \
						typedef type1* type1_ptr_t; \
						MACRO_FUNCTION(type1_ptr_t); \
					}

					MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT2(element_t, MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT1);
					MSE_IF_CONSTEXPR(!std::is_const<element_t>::value) {
						//MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT2(typename std::add_const<element_t>::type, MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT1);
					}

					MSE_THROW(std::logic_error("could not determine the type represented by the (presumably lh::void_star_replacement) argument - lh::impl::us::memcpy_helper2()"));
					return destination;
				}

				template<class _TPointer, class _TPointer2>
				_TPointer memcpy_helper2(std::false_type, _TPointer const& destination, _TPointer2 const& source, size_t num_bytes) {
					return memcpy_helper3(typename std::integral_constant<bool, std::is_same<const mse::lh::void_star_replacement, const _TPointer>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TPointer>::value>::type(), destination, source, num_bytes);
				}

				template<class _TIter, class _TIter2>
				_TIter memcpy_helper2(std::true_type, _TIter const& destination, _TIter2 const& source, size_t num_bytes) {
					//static_assert(std::is_same<mse::lh::void_star_replacement, _TIter2>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TIter2>::value, "");

					/* Here the source parameter is presumed to be an lh::void_star_replacement. In order to complete the operation safely we 
					need to guess the type stored in the lh::void_star_replacement and recover it. We're going to use some macros to help 
					generate a whole bunch of guesses. */

					typedef mse::impl::target_type<_TIter> element_t;
					auto& ecany_ref = mse::us::impl::as_ref<const mse::lh::impl::explicitly_castable_any>(source);

#define MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT1(type1) \
					{ \
						auto maybe_casted = mse::lh::impl::maybe_any_cast_of_explicitly_castable_any<type1>(ecany_ref); \
						if (maybe_casted.has_value()) { \
							return mse::lh::memcpy(destination, mse::lh::impl::explicitly_castable_any::convert<type1>(maybe_casted.value()), num_bytes); \
						} \
					}

#define MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT2(type1, MACRO_FUNCTION) MSE_IMPL_LH_MEMCPY_HELPER3_ANY_CAST_ATTEMPT2(type1, MACRO_FUNCTION)

					MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT2(element_t, MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT1);
					MSE_IF_CONSTEXPR(!std::is_const<element_t>::value) {
						MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT2(typename std::add_const<element_t>::type, MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT1);
					}

					MSE_THROW(std::logic_error("could not determine the type represented by the (presumably lh::void_star_replacement) argument - lh::impl::us::memcpy_helper2()"));
					return destination;
				}

				template<class _TIter, class _TIter2>
				_TIter memcpy_helper1(std::true_type, _TIter const& destination, _TIter2 const& source, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype((destination)[0])> element_t;
					auto num_items = num_bytes / sizeof(element_t);
					if (num_items * sizeof(element_t) != num_bytes) {
						typedef mse::impl::remove_reference_t<decltype((source)[0])> source_element_t;
						num_items = num_bytes / sizeof(source_element_t);
						if (num_items * sizeof(source_element_t) != num_bytes) {
							MSE_THROW(std::logic_error("lh::memcpy(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
								"Note that the library's safe types may be different in size to the original types they were designed to replace "
								"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
						}
					}
					for (size_t i = 0; i < num_items; i += 1) {
						destination[i] = source[i];
					}
					return destination;
				}

				template<class _TPointer, class _TPointer2>
				_TPointer memcpy_helper1(std::false_type, _TPointer const& destination, _TPointer2 const& source, size_t num_bytes) {
					return memcpy_helper2(typename std::integral_constant<bool,  std::is_same<const mse::lh::void_star_replacement, const _TPointer2>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TPointer2>::value>::type(), destination, source, num_bytes);
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
			return impl::us::memcpy_helper1(typename std::integral_constant<bool
				, impl::HasOrInheritsSubscriptOperator<_TElement*>::value && impl::HasOrInheritsSubscriptOperator<_TIter2>::value >::type(), destination, source, num_bytes);
		}

		/* Memory safe approximation of memcmp(). */
		template<class _TIter, class _TIter2>
		int memcmp(_TIter const& source1, _TIter2 const& source2, size_t num_bytes);
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement, class _TIter2>
		int memcmp(_TElement* source1, _TIter2 const& source2, size_t num_bytes);
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size, class _TIter2>
		auto memcmp(TNativeArrayReplacement<_udTy, _Size>& nar, _TIter2 const& source2, size_t num_bytes) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			//typename mse::mstd::array<_Ty, _Size>::iterator source1 = nar;
			mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> source1 = nar;
			return memcmp(source1, source2, num_bytes);
		}

		namespace impl {
			namespace us {
				template<class _TPointer, class _TPointer2>
				int memcmp_helper4(std::false_type, _TPointer const& source1, _TPointer2 const& source2, size_t num_bytes) {
					//static_assert(std::is_same<mse::lh::void_star_replacement, _TPointer2>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TPointer2>::value, "");

					/* Here the source2 parameter is presumed to be an lh::void_star_replacement. In order to complete the operation safely we
					need to guess the type stored in the lh::void_star_replacement and recover it. We're going to use some macros to help 
					generate a whole bunch of guesses. */

					typedef mse::impl::target_type<_TPointer> element_t;
					auto& ecany_ref = mse::us::impl::as_ref<const mse::lh::impl::explicitly_castable_any>(source2);

#define MSE_IMPL_LH_MEMCMP_HELPER2_ANY_CAST_ATTEMPT1(type1) \
					{ \
						auto maybe_casted = mse::lh::impl::maybe_any_cast_of_explicitly_castable_any<type1>(ecany_ref); \
						if (maybe_casted.has_value()) { \
							return mse::lh::memcmp(source1, mse::lh::impl::explicitly_castable_any::convert<type1>(maybe_casted.value()), num_bytes); \
						} \
					}

					MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT2(element_t, MSE_IMPL_LH_MEMCMP_HELPER2_ANY_CAST_ATTEMPT1);
					MSE_IF_CONSTEXPR(!std::is_const<element_t>::value) {
						MSE_IMPL_LH_MEMCPY_HELPER2_ANY_CAST_ATTEMPT2(typename std::add_const<element_t>::type, MSE_IMPL_LH_MEMCMP_HELPER2_ANY_CAST_ATTEMPT1);
					}

					MSE_THROW(std::logic_error("could not determine the type represented by the (presumably lh::void_star_replacement) argument - lh::impl::us::memcmp_helper4()"));
					return 0;
				}

#ifdef MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT
				template<class _TPointer, class _TPointer2>
				int memcmp_helper4(std::true_type, _TPointer const& source1, _TPointer2 const& source2, size_t num_bytes) {
					//static_assert((std::is_same<mse::lh::void_star_replacement, _TPointer>::value || std::is_same<mse::lh::const_void_star_replacement, _TPointer>::value) && (std::is_same<mse::lh::void_star_replacement, _TPointer2>::value || std::is_same<mse::lh::const_void_star_replacement, _TPointer2>::value), "");

					/* This helper is for the case when both source1 and source2 are `mse::lh::void_star_replacement`s. Not sure what we can
					do in this case? We can make some guesses about what type of pointers or iterators those `mse::lh::void_star_replacement`s 
					contain. We're going to use some macros to help generate a whole bunch of guesses. */

					auto& ecany1_ref = mse::us::impl::as_ref<const mse::lh::impl::explicitly_castable_any>(source1);
					auto& ecany2_ref = mse::us::impl::as_ref<const mse::lh::impl::explicitly_castable_any>(source2);

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT21(type1) \
					{ \
						auto maybe_casted2 = mse::lh::impl::maybe_any_cast_of_explicitly_castable_any<type1>(ecany2_ref); \
						if (maybe_casted2.has_value()) { \
							return mse::lh::memcmp(casted_source1, mse::lh::impl::explicitly_castable_any::convert<type1>(maybe_casted2.value()), num_bytes); \
						} \
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22A(type1, MACRO_FUNCTION) \
					{ \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TStrongVectorIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeStrongVectorIterator<type1>); \
						typedef type1* type1_ptr_t; \
						MACRO_FUNCTION(type1_ptr_t); \
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22B(type1, MACRO_FUNCTION) \
					{ \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradNotNullPointer<type1>); */ \
					}

#if MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22A(type1, MACRO_FUNCTION); \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22B(type1, MACRO_FUNCTION);
#else // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22A(type1, MACRO_FUNCTION);
#endif // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT23(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22(type1, MACRO_FUNCTION); \
					MSE_IF_CONSTEXPR(!std::is_const<type1>::value) { \
						MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT22(typename std::add_const<type1>::type, MACRO_FUNCTION); \
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11(type1) \
					{ \
						auto maybe_casted1 = mse::lh::impl::maybe_any_cast_of_explicitly_castable_any<type1>(ecany1_ref); \
						if (maybe_casted1.has_value()) { \
							auto casted_source1 = mse::lh::impl::explicitly_castable_any::convert<type1>(maybe_casted1.value()); \
							MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT23(mse::impl::target_type<type1>, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT21); \
						} \
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12A(type1, MACRO_FUNCTION) \
					{ \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TStrongVectorIterator<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeStrongVectorIterator<type1>); \
						typedef type1* type1_ptr_t; \
						MACRO_FUNCTION(type1_ptr_t); \
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12B(type1, MACRO_FUNCTION) \
					{ \
						MACRO_FUNCTION(mse::lh::TLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::lh::TXScopeLHNullableAnyPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingPointer<type1>); \
						MACRO_FUNCTION(mse::TRefCountingNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredPointer<type1>); \
						MACRO_FUNCTION(mse::TRegisteredNotNullPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradPointer<type1>); \
						MACRO_FUNCTION(mse::TNoradNotNullPointer<type1>); */ \
					}

#if MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12A(type1, MACRO_FUNCTION); \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12B(type1, MACRO_FUNCTION);
#else // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12A(type1, MACRO_FUNCTION);
#endif // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT13(type1, MACRO_FUNCTION) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12(type1, MACRO_FUNCTION); \
					MSE_IF_CONSTEXPR(!std::is_const<type1>::value) { \
						MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT12(typename std::add_const<type1>::type, MACRO_FUNCTION);
					}

#define MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(type1, not_used_template_wrapper) \
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT13(type1, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);

#if MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
					MSE_IMPL_APPLY_MACRO_FUNCTION_TO_EACH_OF_THE_ARITHMETIC_TYPES(MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14);
#else // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(char, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(unsigned char, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(long int, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(unsigned long int, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(long long int, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
					MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT14(unsigned long long int, MSE_IMPL_LH_MEMCMP_HELPER4_ANY_CAST_ATTEMPT11);
#endif // MSE_IMPL_VOID_STAR_TYPE_GUESSING_EXTENSIVENESS_LEVEL > 5

					MSE_THROW(std::logic_error("could not determine the type represented by the (presumably lh::void_star_replacement) argument - lh::impl::us::memcmp_helper4()"));
					return 0;
				}
#else // MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT
				template<class _TPointer, class _TPointer2>
				int memcmp_helper4(std::true_type, _TPointer const& source1, _TPointer2 const& source2, size_t num_bytes) {
					/* A compile error is expected to occur here. The source1 and source2 arguments are presumably both `lh::void_star_replacement`s. 
					But in order to generate a safe memcmp(), we'd need to be able to extract the (safe) pointer/iterators they contain. But we
					in this context we have no clue what those (safe) types are. If you want to the library to assume and verify that the contained 
					(safe) pointer/iterators are each one of the more common types we know about, you can define the MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT 
					preprocessor macro. But that option might have a significant compile-time cost. */
					return mse::lh::memcmp(std::addressof(*source1), std::addressof(*source2), num_bytes);
				}
#endif // MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT


				template<class _TIter, class _TIter2>
				int memcmp_helper3(std::true_type, _TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
					//static_assert((std::is_same<mse::lh::void_star_replacement, _TIter>::value || std::is_same<mse::lh::const_void_star_replacement, _TIter>::value), "");

					/* Here the source1 parameter is presumed to be an lh::void_star_replacement. In order to complete the operation safely we
					need to guess the type stored in the lh::void_star_replacement and recover it. */
					return -mse::lh::memcmp(source2, source1, num_bytes);
				}

				template<class _TPointer, class _TPointer2>
				int memcmp_helper3(std::false_type, _TPointer const& source1, _TPointer2 const& source2, size_t num_bytes) {
					//typedef mse::impl::remove_reference_t<decltype(*source1)> element_t;
					//auto num_items = num_bytes / sizeof(element_t);
					//assert(1 == num_items);
					//assert(num_items * sizeof(element_t) == num_bytes);
					typedef mse::impl::remove_reference_t<decltype(*source1)> element_t;
					if ((sizeof(element_t) != num_bytes) && (0 != num_bytes)) {
						typedef mse::impl::remove_reference_t<decltype(*source2)> source_element_t;
						if (sizeof(source_element_t) != num_bytes) {
							auto num_items = num_bytes / sizeof(element_t);
							if (num_items * sizeof(element_t) != num_bytes) {
								num_items = num_bytes / sizeof(source_element_t);
								if (num_items * sizeof(source_element_t) != num_bytes) {
									MSE_THROW(std::logic_error("lh::memcmp(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
										"Note that the library's safe types may be different in size to the original types they were designed to replace "
										"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
								}
							}
							MSE_THROW(std::logic_error("lh::memcmp(): Either the source1 or source2 arguments (or both) is deemed to be a pointer that is not an iterator, "
								"but the given 'number of bytes' argument does not seem to match the size of the pointer's target element type. "
								"If the source1 and source2 arguments are intended to point (in)to buffers, then the pointer argument(s) need to be replaced with iterators. "));
						}
					}

					if (*source1 > *source2) {
						return 1;
					}
					else if (*source1 < *source2) {
						return -1;
					}
					return 0;
				}

				template<class _TIter, class _TIter2>
				int memcmp_helper2(std::true_type, _TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
					return memcmp_helper4(typename std::integral_constant<bool, std::is_same<const mse::lh::void_star_replacement, const _TIter>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TIter>::value>::type()
						, source1, source2, num_bytes);
				}

				template<class _TPointer, class _TPointer2>
				int memcmp_helper2(std::false_type, _TPointer const& source1, _TPointer2 const& source2, size_t num_bytes) {
					return memcmp_helper3(typename std::integral_constant<bool, std::is_same<const mse::lh::void_star_replacement, const _TPointer>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TPointer>::value>::type()
						, source1, source2, num_bytes);
				}

				template<class _TIter, class _TIter2>
				int memcmp_helper1(std::true_type, _TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype((source1)[0])> element_t;
					auto num_items = num_bytes / sizeof(element_t);
					if (num_items * sizeof(element_t) != num_bytes) {
						typedef mse::impl::remove_reference_t<decltype((source2)[0])> source_element_t;
						num_items = num_bytes / sizeof(source_element_t);
						if (num_items * sizeof(source_element_t) != num_bytes) {
							MSE_THROW(std::logic_error("lh::memcmp(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
								"Note that the library's safe types may be different in size to the original types they were designed to replace "
								"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
						}
					}
					for (size_t i = 0; i < num_items; i += 1) {
						if (source1[i] > source2[i]) {
							return 1;
						} else if (source1[i] < source2[i]) {
							return -1;
						}
					}
					return 0;
				}

				template<class _TPointer, class _TPointer2>
				int memcmp_helper1(std::false_type, _TPointer source1, _TPointer2 source2, size_t num_bytes) {
					return memcmp_helper2(typename std::integral_constant<bool, std::is_same<const mse::lh::void_star_replacement, const _TPointer2>::value || std::is_same<const mse::lh::const_void_star_replacement, const _TPointer2>::value>::type()
						, source1, source2, num_bytes);
				}
			}
		}
		/* Memory safe approximation of memcmp(). */
		template<class _TIter, class _TIter2>
		int memcmp(_TIter const& source1, _TIter2 const& source2, size_t num_bytes) {
			return impl::us::memcmp_helper1(typename std::integral_constant<bool
				, impl::HasOrInheritsSubscriptOperator<_TIter>::value && impl::HasOrInheritsSubscriptOperator<_TIter2>::value >::type(), source1, source2, num_bytes);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memcpy(). */
		template<class _TElement, class _TIter2>
		int memcmp(_TElement* source1, _TIter2 const& source2, size_t num_bytes) {
			return impl::us::memcmp_helper1(typename std::integral_constant<bool
				, impl::HasOrInheritsSubscriptOperator<_TElement*>::value && impl::HasOrInheritsSubscriptOperator<_TIter2>::value >::type(), source1, source2, num_bytes);
		}

		namespace impl {
			namespace us {
				template<class element_t>
				auto memset_adjusted_value1(std::true_type, int value) {
					value &= 0xff;
					long long int adjusted_value = value;
					if (sizeof(adjusted_value) >= sizeof(element_t)) {
						for (size_t i = 1; i < sizeof(element_t); i += 1) {
							adjusted_value |= (long long int)(value << (8 * i));
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
					auto num_items = num_bytes / sizeof(element_t);
					if (num_items * sizeof(element_t) != num_bytes) {
						MSE_THROW(std::logic_error("lh::memcpy(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
							"Note that the library's safe types may be different in size to the original types they were designed to replace "
							"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
					}

					const auto element_value = memset_adjusted_value1<element_t>(typename std::is_assignable<element_t&, long long int>::type(), value);
					for (size_t i = 0; i < num_items; i += 1) {
						iter[i] = element_value;
					}
					return iter;
				}

				template<class _TPointer>
				_TPointer memset_helper1(std::false_type, _TPointer ptr, int value, size_t num_bytes) {
					typedef mse::impl::remove_reference_t<decltype(*ptr)> element_t;
					if ((sizeof(element_t) != num_bytes) && (0 != num_bytes)) {
						auto num_items = num_bytes / sizeof(element_t);
						if (num_items * sizeof(element_t) != num_bytes) {
							MSE_THROW(std::logic_error("lh::memcpy(): The given 'number of bytes' argument does not seem to be a multiple of the size of the buffer element type. "
								"Note that the library's safe types may be different in size to the original types they were designed to replace "
								"(and maybe have different size in debug and release mode) and this may be need to be taken into account when calculating the argument value. "));
						}
						MSE_THROW(std::logic_error("lh::memcpy(): The first argument is deemed to be a pointer that is not an iterator, "
							"but the given 'number of bytes' argument does not seem to match the size of the pointer's target element type. "
							"If the first argument is intended to point (in)to a buffer, then the pointer argument needs to be replaced with an iterator. "));
					}

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

		namespace impl {
			namespace ns_strtox {
				template <typename T>
				using iterator_smoke_test = std::integral_constant<bool, mse::impl::is_contiguous_sequence_iterator<T>::value || (mse::impl::SupportsSubtraction_poly<T>::value && mse::impl::IsDereferenceable_pb<T>::value && mse::lh::impl::HasOrInheritsSubscriptOperator<T>::value)>;

				template <class _TIter, class _TPointerToIter>
				using strtox_tparams_smoke_test = std::integral_constant<bool,
					((iterator_smoke_test<_TIter>::value && std::is_same<const char, const mse::impl::target_or_void_type<_TIter> >::value) && (
						std::is_same<std::nullptr_t, _TPointerToIter>::value || std::is_same<NULL_t, _TPointerToIter>::value
						/* The interface of strtol() and friends can be potentially used to unsafely obtain a non-const reference to a const
						element. We'll try to see if we can get away with not supporting that unsafe use case. */
						/*
						|| std::is_base_of<mse::lh::TLHNullableAnyRandomAccessIterator<char>, mse::impl::target_or_void_type<_TPointerToIter> >::value
						|| std::is_base_of<mse::lh::TXScopeLHNullableAnyRandomAccessIterator<char>, mse::impl::target_or_void_type<_TPointerToIter> >::value
						*/
						|| std::is_assignable<mse::impl::decay_t<mse::impl::target_or_given_default_type<_TPointerToIter, mse::impl::TPlaceHolder<> > > &, mse::impl::decay_t<_TIter>>::value
					)) || (std::is_convertible<_TIter, const char*>::value && std::is_same<_TPointerToIter, char**>::value)>;

				template <typename T1, typename T2, typename T3>
				void strtox_set_out_param_helper2(std::false_type, T1 const& str_end, T2 const& str, T3 const& distance) {
					mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<const char> const& base_str_ref = str;
					/* The value we need to set for (*str_end) is derived from the value of the (first) str parameter. But str is a
					pointer/iterator to a const char, while (*str_end) is a pointer/iterator to a (non-const) char. So we need to do a
					reinterpret_cast to obtain the needed pointer/iterator to a (non-const) char from the str parameter. Which should be
					fine assuming that TLHNullableAnyRandomAccessIteratorBase<const char> and TLHNullableAnyRandomAccessIteratorBase<char>
					are structurally identical. */
					auto& ncbase_str_ref = reinterpret_cast<mse::lh::us::impl::TLHNullableAnyRandomAccessIteratorBase<char> const&>(base_str_ref);
					(*str_end) = ncbase_str_ref;
					(*str_end) += distance;
					/* Of course it's not fine in the sense that it can be potentially used to obtain an unsafe non-const reference to a 
					const element. The interface of strtol() and friends seems to be intrisically unsafe in this sense. Preferably our 
					implementation wouldn't allow this unsafe use. (Presumably resulting in this part of the code being unreachable.) */
				}
				template <typename T1, typename T2, typename T3>
				void strtox_set_out_param_helper2(std::true_type, T1 const& str_end, T2 const& str, T3 const& distance) {
					(*str_end) = str + distance;
				}

				template <typename T2, typename T3>
				void strtox_set_out_param_helper1(std::nullptr_t str_end, T2 const& str, T3 const& distance) {}
				template <typename T2, typename T3>
				void strtox_set_out_param_helper1(NULL_t str_end, T2 const& str, T3 const& distance) {}
				template <typename _TPointerToIter, typename _TIter, typename T3>
				void strtox_set_out_param_helper1(_TPointerToIter const& str_end, _TIter const& str, T3 const& distance) {
					strtox_set_out_param_helper2(typename std::is_assignable<mse::impl::decay_t<mse::impl::target_or_void_type<_TPointerToIter> >&, mse::impl::decay_t<_TIter> >::type(), str_end, str, distance);
				}

				template<typename _TStrToXFunction, class _TIter, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<strtox_tparams_smoke_test<_TIter, _TPointerToIter>::value> MSE_IMPL_EIS >
				auto strtox(_TStrToXFunction& fn, _TIter const& str, _TPointerToIter const& str_end, int base) {
					const auto len = strlen(str);
					const char* start = std::addressof(*str);
					char* end = bool(str_end) ? (char*)start : nullptr;
					/* clang in c++14 mode (at least) for some reason doesn't support instantiating the lambda function by type, so we use 
					a reference to the original lambda instantiation. */
					//static const _TStrToXFunction fn;
					auto retval = fn(start, &end, base);
					if (end) {
						auto distance = ((const char*)end - start);
						assert(static_cast<decltype(distance)>(len) + 1 >= distance);
						strtox_set_out_param_helper1(str_end, str, distance);
					}
					return retval;
				}

				template<typename _TStrToXFunction, class _TIter, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<strtox_tparams_smoke_test<_TIter, _TPointerToIter>::value> MSE_IMPL_EIS >
				auto strtofx(_TStrToXFunction& fn, _TIter const& str, _TPointerToIter const& str_end) {
					const auto len = strlen(str);
					const char* start = std::addressof(*str);
					char* end = bool(str_end) ? (char*)start : nullptr;
					//static const _TStrToXFunction fn;
					auto retval = fn(start, &end);
					if (end) {
						auto distance = ((const char*)end - start);
						assert(static_cast<decltype(distance)>(len) + 1 >= distance);
						strtox_set_out_param_helper1(str_end, str, distance);
					}
					return retval;
				}
			}
		}

#define MSE_IMPL_LH_STRTOX_DECLARATION(strtox_function_name) \
		template<class _TIter, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtox::strtox_tparams_smoke_test<_TIter, _TPointerToIter>::value> MSE_IMPL_EIS > \
		auto strtox_function_name(_TIter const& str, _TPointerToIter const& str_end, int base) { \
			static const auto lambda1 = [](const char* str, char** str_end, int base) { return std::strtox_function_name(str, str_end, base); }; \
			return impl::ns_strtox::strtox(lambda1, str, str_end, base); \
		} \
		inline auto strtox_function_name(const char* str, char** str_end, int base) { return std::strtox_function_name(str, str_end, base); }

		MSE_IMPL_LH_STRTOX_DECLARATION(strtol);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoul);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoll);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoull);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoimax);
		MSE_IMPL_LH_STRTOX_DECLARATION(strtoumax);

#define MSE_IMPL_LH_STRTOFX_DECLARATION(strtox_function_name) \
		template<class _TIter, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtox::strtox_tparams_smoke_test<_TIter, _TPointerToIter>::value> MSE_IMPL_EIS > \
		auto strtox_function_name(_TIter const& str, _TPointerToIter const& str_end) { \
			static const auto lambda1 = [](const char* str, char** str_end) { return std::strtox_function_name(str, str_end); }; \
			return impl::ns_strtox::strtofx(lambda1, str, str_end); \
		} \
		inline auto strtox_function_name(const char* str, char** str_end) { return std::strtox_function_name(str, str_end); }

		MSE_IMPL_LH_STRTOFX_DECLARATION(strtof);
		MSE_IMPL_LH_STRTOFX_DECLARATION(strtod);
		MSE_IMPL_LH_STRTOFX_DECLARATION(strtold);

		namespace impl {
			namespace ns_strtok {
				template <typename T>
				using iterator_smoke_test = std::integral_constant<bool, mse::impl::is_contiguous_sequence_iterator<T>::value || (mse::impl::SupportsSubtraction_poly<T>::value && mse::impl::IsDereferenceable_pb<T>::value && mse::lh::impl::HasOrInheritsSubscriptOperator<T>::value)>;

				template <class _TIter, class _TConstCharIter2, class _TPointerToIter>
				using strtok_r_tparams_smoke_test = std::integral_constant<bool,
					(std::is_same<const char, const mse::impl::target_or_void_type<_TConstCharIter2> >::value && (iterator_smoke_test<mse::impl::target_or_void_type<_TPointerToIter> >::value && std::is_same<char, mse::impl::target_or_void_type<mse::impl::target_or_void_type<_TPointerToIter> > >::value) && (
						std::is_same<std::nullptr_t, _TIter>::value || std::is_same<NULL_t, _TIter>::value
						|| std::is_assignable<mse::impl::decay_t<mse::impl::target_or_void_type<_TPointerToIter> >&, mse::impl::decay_t<_TIter> > ::value
						)) || (std::is_convertible<_TIter, char*>::value && std::is_convertible<_TConstCharIter2, const char*>::value && std::is_same<_TPointerToIter, char**>::value)>;

				/* Implementation of strtok_r() based on the "Possible implementation" of strtok() here: https://en.cppreference.com/w/cpp/string/byte/strtok */
				template<class _TIter, class _TConstCharIter2, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<strtok_r_tparams_smoke_test<_TIter, _TConstCharIter2, _TPointerToIter>::value> MSE_IMPL_EIS >
				auto strtok_r_impl(_TIter const& str, _TConstCharIter2 const& delim, _TPointerToIter pointer_to_buffer_iterator) -> mse::impl::target_type<_TPointerToIter> {
					/* Using mse::lh::strlen() to verify the strings are null-terminated. */
					mse::lh::strlen(delim);
					const char* delim2 = mse::us::lh::make_raw_pointer_from(delim);

					mse::impl::target_type<_TPointerToIter> buffer = bool(str)
						? mse::impl::target_type<_TPointerToIter>(str)
						: *pointer_to_buffer_iterator;
					mse::lh::strlen(buffer);
					char* buffer2 = mse::us::lh::make_raw_pointer_from(buffer);

					buffer += std::strspn(buffer2, delim2);

					if (*buffer == '\0')
						return nullptr;

					const auto tokenBegin = buffer;

					buffer2 = mse::us::lh::make_raw_pointer_from(buffer);
					buffer += std::strcspn(buffer2, delim2);

					if (*buffer != '\0')
						*buffer++ = '\0';

					*pointer_to_buffer_iterator = buffer;
					return tokenBegin;
				}

				static auto& strtok_buffer_ref() {
					thread_local mse::lh::TLHNullableAnyRandomAccessIterator<char> buffer;
					return buffer;
				}
				template<class _TIter, class _TConstCharIter2, MSE_IMPL_EIP mse::impl::enable_if_t<strtok_r_tparams_smoke_test<mse::impl::decay_t<_TIter>, _TConstCharIter2, mse::lh::TLHNullableAnyRandomAccessIterator<char>*>::value> MSE_IMPL_EIS >
				auto strtok_impl(_TIter const& str, _TConstCharIter2 const& delim) -> mse::lh::TLHNullableAnyRandomAccessIterator<char> {
					auto& buffer = strtok_buffer_ref();
					if (str) {
						buffer = str;
					}
					return strtok_r_impl(str, delim, std::addressof(buffer));
				}
			}
		}
		template<class _TIter, class _TConstCharIter2, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtok::strtok_r_tparams_smoke_test<_TIter, _TConstCharIter2, _TPointerToIter>::value> MSE_IMPL_EIS >
		auto strtok_r(_TIter const& str, _TConstCharIter2 const& delim, _TPointerToIter pointer_to_buffer_iterator) -> mse::impl::target_type<_TPointerToIter> {
			return impl::ns_strtok::strtok_r_impl(str, delim, pointer_to_buffer_iterator);
		}
		template<class _TConstCharIter2, class _TPointerToIter, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtok::strtok_r_tparams_smoke_test<char*, _TConstCharIter2, _TPointerToIter>::value> MSE_IMPL_EIS >
		auto strtok_r(char* str, _TConstCharIter2 const& delim, _TPointerToIter pointer_to_buffer_iterator) -> mse::impl::target_type<_TPointerToIter> {
			return impl::ns_strtok::strtok_r_impl(str, delim, pointer_to_buffer_iterator);
		}
		template<class _TIter, class _TConstCharIter2, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtok::strtok_r_tparams_smoke_test<mse::impl::decay_t<_TIter>, _TConstCharIter2, mse::lh::TLHNullableAnyRandomAccessIterator<char>*>::value> MSE_IMPL_EIS >
		auto strtok(_TIter const& str, _TConstCharIter2 const& delim) -> mse::lh::TLHNullableAnyRandomAccessIterator<char> {
			return impl::ns_strtok::strtok_impl(str, delim);
		}
		template<size_t _Size, class _TConstCharIter2, MSE_IMPL_EIP mse::impl::enable_if_t < impl::ns_strtok::strtok_r_tparams_smoke_test<typename impl::lh_decay<TNativeArrayReplacement<char, _Size> >::type , _TConstCharIter2, mse::lh::TLHNullableAnyRandomAccessIterator<char>* > ::value > MSE_IMPL_EIS >
		auto strtok(TNativeArrayReplacement<char, _Size>& str, _TConstCharIter2 const& delim) -> mse::lh::TLHNullableAnyRandomAccessIterator<char> {
			auto decayed_str = impl::as_lh_decayed(str);
			return impl::ns_strtok::strtok_impl(decayed_str, delim);
		}
		template<class _TConstCharIter2, MSE_IMPL_EIP mse::impl::enable_if_t<impl::ns_strtok::strtok_r_tparams_smoke_test<char*, _TConstCharIter2, mse::lh::TLHNullableAnyRandomAccessIterator<char>*>::value> MSE_IMPL_EIS >
		auto strtok(char* str, _TConstCharIter2 const& delim) -> mse::lh::TLHNullableAnyRandomAccessIterator<char> {
			return impl::ns_strtok::strtok_impl(str, delim);
		}

		namespace impl {
			namespace ns_memchr {
				template <class _TCharIter, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(mse::impl::target_or_given_default_type<_TCharIter, long int>))> MSE_IMPL_EIS >
				mse::lh::TLHNullableAnyRandomAccessIterator<mse::impl::target_or_void_type<_TCharIter> > memchr_helper3(const _TCharIter& iter, int ch, size_t count) {
					if (!iter) {
						return nullptr;
					}
					const auto in_rawptr = (const char *)std::addressof(*iter);
					auto res_rawptr = (const char*)(::memchr(in_rawptr, ch, count));
					if (!res_rawptr) {
						return nullptr;
					}
					return iter + (res_rawptr - in_rawptr);
				}

				template <class _TCharIter, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(mse::impl::target_or_given_default_type<_TCharIter, long int>))> MSE_IMPL_EIS >
				auto memchr_helper2(std::false_type, const _TCharIter& iter, int ch, size_t count) {
					return memchr_helper3(iter, ch, count);
				}
				template <class _TCharIter, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_base_of<mse::lh::void_star_replacement, _TCharIter>::value || std::is_base_of<mse::lh::const_void_star_replacement, _TCharIter>::value> MSE_IMPL_EIS >
				auto memchr_helper2(std::true_type, const _TCharIter& iter, int ch, size_t count) {
					typedef mse::impl::conditional_t<std::is_base_of<mse::lh::void_star_replacement, _TCharIter>::value, char, const char> target_type1;
					return memchr_helper3((mse::lh::TLHNullableAnyRandomAccessIterator<target_type1>)iter, ch, count);
				}
			}
		}
		/* Memory safe approximation of memchr(). */
		template <class _TCharIter, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(mse::impl::target_or_given_default_type<_TCharIter, long int>))
			|| std::is_base_of<mse::lh::void_star_replacement, _TCharIter>::value || std::is_base_of<mse::lh::const_void_star_replacement, _TCharIter>::value> MSE_IMPL_EIS >
		auto memchr(const _TCharIter& iter, int ch, size_t count) {
			return impl::ns_memchr::memchr_helper2(typename std::integral_constant<bool, std::is_base_of<mse::lh::void_star_replacement, _TCharIter>::value || std::is_base_of<mse::lh::const_void_star_replacement, _TCharIter>::value>::type(), iter, ch, count);
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::memchr(). */
		template <class _TElement, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(_TElement))> MSE_IMPL_EIS >
		auto memchr(_TElement* iter, int ch, size_t count) {
			return impl::ns_memchr::memchr_helper2(std::false_type(), iter, ch, count);
		}
		/* And similarly, his overload is to allow native array replacements to decay to a (safe) iterator. */
		template<class _udTy, size_t _Size, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(_udTy))> MSE_IMPL_EIS >
		auto memchr(TNativeArrayReplacement<_udTy, _Size>& nar, int ch, size_t count) {
			typedef impl::const_preserving_decay_t<_udTy> _Ty;
			mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> iter = nar;
			return memchr(iter, ch, count);
		}
		/* Memory safe approximation of strchr(). */
		template <class _TCharIter, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(mse::impl::target_or_given_default_type<_TCharIter, long int>))
			|| std::is_base_of<mse::lh::void_star_replacement, _TCharIter>::value || std::is_base_of<mse::lh::const_void_star_replacement, _TCharIter>::value> MSE_IMPL_EIS >
		auto strchr(const _TCharIter& iter, int ch) {
			return memchr(iter, ch, strlen(iter));
		}
		/* This overload is to allow native arrays to decay to a pointer iterator like they would with std::strchr(). */
		template <class _TElement, MSE_IMPL_EIP mse::impl::enable_if_t<(sizeof(char) == sizeof(_TElement))> MSE_IMPL_EIS >
		auto strchr(_TElement* iter, int ch) {
			return memchr(iter, ch, strlen(iter));
		}

		template<typename TPointerToCharBuffer, typename TPointerToSize_t, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TPointerToCharBuffer>::value) || (mse::impl::IsDereferenceable_pb<TPointerToSize_t>::value)> MSE_IMPL_EIS >
		lh_ssize_t getline(TPointerToCharBuffer lineptr, TPointerToSize_t nptr, FILE* stream) {
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

		template<typename TCharBufferIterator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TCharBufferIterator>::value)> MSE_IMPL_EIS >
		auto strndup(TCharBufferIterator const& str_iter, size_t size) -> TStrongVectorIterator<mse::impl::target_type<TCharBufferIterator> > {
			TStrongVectorIterator<mse::impl::target_type<TCharBufferIterator> > retval;
			size_t slen = 0;
			while (size > slen) {
				if ('\0' == str_iter[slen]) {
					break;
				}
				slen += 1;
			}
			retval.resize(1 + slen);
			auto dest_iter = retval;
			for (size_t i = 0; slen > i; i += 1) {
				*dest_iter = str_iter[i];
				++dest_iter;
			}
			*dest_iter = 0;
			return retval;
		}
		template<typename TCharBufferIterator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TCharBufferIterator>::value)> MSE_IMPL_EIS >
		auto strdup(TCharBufferIterator const& str_iter) -> TStrongVectorIterator<mse::impl::target_type<TCharBufferIterator> > {
			auto slen = strlen(str_iter);
			return strndup(str_iter, slen);
		}

		template<typename TIterator, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::IsDereferenceable_pb<TIterator>::value)> MSE_IMPL_EIS >
		auto memdup(TIterator const& src_iter, size_t num_bytes) -> TStrongVectorIterator<mse::impl::target_type<TIterator> > {
			TStrongVectorIterator<mse::impl::target_type<TIterator> > retval;
			retval.resize(num_bytes);
			auto dest_iter = retval;
			auto num_items = num_bytes / sizeof(mse::impl::target_type<TIterator>);
			for (size_t i = 0; num_items > i; i += 1) {
				*dest_iter = src_iter[i];
				++dest_iter;
			}
			return retval;
		}

		namespace us {
			namespace impl {
				template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t iconv_wrapper_helper2(_TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					std::vector<char> in_vec;
					char** inbuf_raw_ptr = nullptr;
					char* inbuf_raw_iter = nullptr;
					size_t l_inbytesleft = 0;
					if (inbufptr && (*inbufptr) && inbytesleftptr) {
						l_inbytesleft = *inbytesleftptr;
						auto iter1 = *inbufptr;
						for (size_t i = 0; l_inbytesleft > i; ++i) {
							in_vec.push_back(*iter1);
							++iter1;
						}
						inbuf_raw_iter = std::addressof(in_vec.at(0));
						inbuf_raw_ptr = &inbuf_raw_iter;
					}

					std::vector<char> out_vec;
					char** outbuf_raw_ptr = nullptr;
					char* outbuf_raw_iter = nullptr;
					size_t l_outbytesleft = 0;
					if (outbufptr && (*outbufptr) && outbytesleftptr) {
						l_outbytesleft = *outbytesleftptr;
						out_vec.resize(l_outbytesleft);
						outbuf_raw_iter = std::addressof(out_vec.at(0));
						outbuf_raw_ptr = &outbuf_raw_iter;
					}

					auto retval = fn(cd, inbuf_raw_ptr, &l_inbytesleft, outbuf_raw_ptr, &l_outbytesleft);

					if (inbufptr && (*inbufptr) && inbytesleftptr) {
						auto amount_advanced = (*inbuf_raw_ptr) - std::addressof(in_vec.at(0));
						*inbufptr += amount_advanced;
						*inbytesleftptr = l_inbytesleft;
					}
					if (outbufptr && (*outbufptr) && outbytesleftptr) {
						auto amount_advanced = (*outbuf_raw_ptr) - std::addressof(out_vec.at(0));

						auto iter1 = *outbufptr;
						for (size_t i = 0; amount_advanced > int(i); ++i) {
							*iter1 = out_vec.at(i);
							++iter1;
						}

						*outbufptr += amount_advanced;
						*outbytesleftptr = l_outbytesleft;
					}
					return retval;
				}

				template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t iconv_wrapper_helper1(std::true_type, std::true_type, _TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_inbufptr = nullptr;
					size_t l_inbytesleft = 0;
					char** l_outbufptr = nullptr;
					size_t l_outbytesleft = 0;
					return us::impl::iconv_wrapper_helper2(fn, cd, l_inbufptr, &l_inbytesleft, l_outbufptr, &l_outbytesleft);
				}
				template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t iconv_wrapper_helper1(std::true_type, std::false_type, _TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_inbufptr = nullptr;
					size_t l_inbytesleft = 0;
					return us::impl::iconv_wrapper_helper2(fn, cd, l_inbufptr, &l_inbytesleft, outbufptr, outbytesleftptr);
				}
				template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t iconv_wrapper_helper1(std::false_type, std::true_type, _TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_outbufptr = nullptr;
					size_t l_outbytesleft = 0;
					return us::impl::iconv_wrapper_helper2(fn, cd, inbufptr, inbytesleftptr, l_outbufptr, &l_outbytesleft);
				}
				template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t iconv_wrapper_helper1(std::false_type, std::false_type, _TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					return us::impl::iconv_wrapper_helper2(fn, cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr);
				}
			}
		}

		template<typename _TIConvFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
		size_t iconv_wrapper(_TIConvFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
			/* Some of the parameters could be non-dereferenceable types such as std::nullptr_t or decltype(NULL). We will call specific 
			overloads of the helper function based on which parameters are non-dereferenceable. */
			typedef mse::impl::TPlaceHolder<> placeholder_t;
			typedef mse::impl::target_or_given_default_type<TPointerToInCharBuffer, placeholder_t> in_T1;
			typedef mse::impl::target_or_given_default_type<in_T1, placeholder_t> in_T2;
			static const bool in_b1 = std::is_same<placeholder_t, in_T2>::value;
			typedef mse::impl::target_or_given_default_type<TPointerToInSize_t, placeholder_t> in_T3;
			static const bool in_b2 = std::is_same<placeholder_t, in_T3>::value;

			typedef mse::impl::target_or_given_default_type<TPointerToOutCharBuffer, placeholder_t> out_T1;
			typedef mse::impl::target_or_given_default_type<out_T1, placeholder_t> out_T2;
			static const bool out_b1 = std::is_same<placeholder_t, out_T2>::value;
			typedef mse::impl::target_or_given_default_type<TPointerToOutSize_t, placeholder_t> out_T3;
			static const bool out_b2 = std::is_same<placeholder_t, out_T3>::value;

			return us::impl::iconv_wrapper_helper1(typename std::integral_constant<bool, in_b1 || in_b2>::type(), typename std::integral_constant<bool, out_b1 || out_b2>::type()
				, fn, cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr);
		}


		namespace us {
			namespace impl {
				template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t i2d_X509_PUBKEY_wrapper_helper2(_TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					std::vector<char> in_vec;
					char** inbuf_raw_ptr = nullptr;
					char* inbuf_raw_iter = nullptr;
					size_t l_inbytesleft = 0;
					if (inbufptr && (*inbufptr) && inbytesleftptr) {
						l_inbytesleft = *inbytesleftptr;
						auto iter1 = *inbufptr;
						for (size_t i = 0; l_inbytesleft > i; ++i) {
							in_vec.push_back(*iter1);
							++iter1;
						}
						inbuf_raw_iter = std::addressof(in_vec.at(0));
						inbuf_raw_ptr = &inbuf_raw_iter;
					}

					std::vector<char> out_vec;
					char** outbuf_raw_ptr = nullptr;
					char* outbuf_raw_iter = nullptr;
					size_t l_outbytesleft = 0;
					if (outbufptr && (*outbufptr) && outbytesleftptr) {
						l_outbytesleft = *outbytesleftptr;
						out_vec.resize(l_outbytesleft);
						outbuf_raw_iter = std::addressof(out_vec.at(0));
						outbuf_raw_ptr = &outbuf_raw_iter;
					}

					auto retval = fn(cd, inbuf_raw_ptr, &l_inbytesleft, outbuf_raw_ptr, &l_outbytesleft);

					if (inbufptr && (*inbufptr) && inbytesleftptr) {
						auto amount_advanced = (*inbuf_raw_ptr) - std::addressof(in_vec.at(0));
						*inbufptr += amount_advanced;
						*inbytesleftptr = l_inbytesleft;
					}
					if (outbufptr && (*outbufptr) && outbytesleftptr) {
						auto amount_advanced = (*outbuf_raw_ptr) - std::addressof(out_vec.at(0));

						auto iter1 = *outbufptr;
						for (size_t i = 0; amount_advanced > int(i); ++i) {
							*iter1 = out_vec.at(i);
							++iter1;
						}

						*outbufptr += amount_advanced;
						*outbytesleftptr = l_outbytesleft;
					}
					return retval;
				}

				template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t i2d_X509_PUBKEY_wrapper_helper1(std::true_type, std::true_type, _TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_inbufptr = nullptr;
					size_t l_inbytesleft = 0;
					char** l_outbufptr = nullptr;
					size_t l_outbytesleft = 0;
					return us::impl::i2d_X509_PUBKEY_wrapper_helper2(fn, cd, l_inbufptr, &l_inbytesleft, l_outbufptr, &l_outbytesleft);
				}
				template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t i2d_X509_PUBKEY_wrapper_helper1(std::true_type, std::false_type, _TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_inbufptr = nullptr;
					size_t l_inbytesleft = 0;
					return us::impl::i2d_X509_PUBKEY_wrapper_helper2(fn, cd, l_inbufptr, &l_inbytesleft, outbufptr, outbytesleftptr);
				}
				template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t i2d_X509_PUBKEY_wrapper_helper1(std::false_type, std::true_type, _TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					char** l_outbufptr = nullptr;
					size_t l_outbytesleft = 0;
					return us::impl::i2d_X509_PUBKEY_wrapper_helper2(fn, cd, inbufptr, inbytesleftptr, l_outbufptr, &l_outbytesleft);
				}
				template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
				size_t i2d_X509_PUBKEY_wrapper_helper1(std::false_type, std::false_type, _TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
					return us::impl::i2d_X509_PUBKEY_wrapper_helper2(fn, cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr);
				}
			}
		}

		template<typename _TFunction, typename TDescriptor, typename TPointerToInCharBuffer, typename TPointerToInSize_t, typename TPointerToOutCharBuffer, typename TPointerToOutSize_t>
		size_t i2d_X509_PUBKEY_wrapper(_TFunction const& fn, TDescriptor const& cd, TPointerToInCharBuffer const& inbufptr, TPointerToInSize_t const& inbytesleftptr, TPointerToOutCharBuffer const& outbufptr, TPointerToOutSize_t const& outbytesleftptr) {
			/* Some of the parameters could be non-dereferenceable types such as std::nullptr_t or decltype(NULL). We will call specific
			overloads of the helper function based on which parameters are non-dereferenceable. */
			typedef mse::impl::TPlaceHolder<> placeholder_t;
			typedef mse::impl::target_or_given_default_type<TPointerToInCharBuffer, placeholder_t> in_T1;
			typedef mse::impl::target_or_given_default_type<in_T1, placeholder_t> in_T2;
			static const bool in_b1 = std::is_same<placeholder_t, in_T2>::value;
			typedef mse::impl::target_or_given_default_type<TPointerToInSize_t, placeholder_t> in_T3;
			static const bool in_b2 = std::is_same<placeholder_t, in_T3>::value;

			typedef mse::impl::target_or_given_default_type<TPointerToOutCharBuffer, placeholder_t> out_T1;
			typedef mse::impl::target_or_given_default_type<out_T1, placeholder_t> out_T2;
			static const bool out_b1 = std::is_same<placeholder_t, out_T2>::value;
			typedef mse::impl::target_or_given_default_type<TPointerToOutSize_t, placeholder_t> out_T3;
			static const bool out_b2 = std::is_same<placeholder_t, out_T3>::value;

			return us::impl::i2d_X509_PUBKEY_wrapper_helper1(typename std::integral_constant<bool, in_b1 || in_b2>::type(), typename std::integral_constant<bool, out_b1 || out_b2>::type()
				, fn, cd, inbufptr, inbytesleftptr, outbufptr, outbytesleftptr);
		}

		namespace impl {
			template<class T>
			struct NDRegisteredPointerWrapped {
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T*, mse::TNDRegisteredPointer<T> > type;
				//typedef mse::TNDRegisteredPointer<T> type;
			};
			template<>
			struct NDRegisteredPointerWrapped<void> {
				typedef void* type;
			};
			template<>
			struct NDRegisteredPointerWrapped<const void> {
				typedef const void* type;
			};
			template<class T>
			struct NDRegisteredNotNullPointerWrapped {
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T*, mse::TNDRegisteredNotNullPointer<T> > type;
				//typedef mse::TNDRegisteredNotNullPointer<T> type;
			};
			template<>
			struct NDRegisteredNotNullPointerWrapped<void> {
				typedef void* type;
			};
			template<>
			struct NDRegisteredNotNullPointerWrapped<const void> {
				typedef const void* type;
			};

			template<class T>
			struct NDNoradPointerWrapped {
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T*, mse::TNDNoradPointer<T> > type;
				//typedef mse::TNDNoradPointer<T> type;
			};
			template<>
			struct NDNoradPointerWrapped<void> {
				typedef void* type;
			};
			template<>
			struct NDNoradPointerWrapped<const void> {
				typedef const void* type;
			};
			template<class T>
			struct NDNoradNotNullPointerWrapped {
				/* apparently unions can't be base classes */
				typedef mse::impl::conditional_t<std::is_union<T>::value, T*, mse::TNDNoradNotNullPointer<T> > type;
				//typedef mse::TNDNoradNotNullPointer<T> type;
			};
			template<>
			struct NDNoradNotNullPointerWrapped<void> {
				typedef void* type;
			};
			template<>
			struct NDNoradNotNullPointerWrapped<const void> {
				typedef const void* type;
			};

			template<class T1, class T2>
			explicitly_castable_any::my_optional<T1> explicitly_castable_any::conversion_operator_helper4(std::true_type, T2* ptr1) {
#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES2(MACRO_FUNCTION, pointee_type) \
				MACRO_FUNCTION(typename NDNoradPointerWrapped<pointee_type>::type); \
				MACRO_FUNCTION(typename NDNoradNotNullPointerWrapped<pointee_type>::type); \
				MACRO_FUNCTION(typename NDRegisteredPointerWrapped<pointee_type>::type); \
				MACRO_FUNCTION(typename NDRegisteredNotNullPointerWrapped<pointee_type>::type); \
				MACRO_FUNCTION(mse::lh::TStrongVectorIterator<pointee_type>); \

				typedef mse::impl::target_type<T1> pointee_t;
				MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES2(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, pointee_t);
				MSE_IF_CONSTEXPR(std::is_const<pointee_t>::value) {
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES2(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, mse::impl::remove_const_t<pointee_t>);
				}

				return {};
			}

			template<class T1, class T2>
			/*static*/ explicitly_castable_any::my_optional<T1> explicitly_castable_any::conversion_operator_helper3(std::true_type, T2 * ptr1) {

#define MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MACRO_FUNCTION, pointee_type) \
					MACRO_FUNCTION(mse::TRefCountingPointer<pointee_type>); \
					MACRO_FUNCTION(mse::TRefCountingNotNullPointer<pointee_type>); \
					MACRO_FUNCTION(mse::lh::TLHNullableAnyPointer<pointee_type>); \
					MACRO_FUNCTION(mse::lh::TLHNullableAnyRandomAccessIterator<pointee_type>); \
					MACRO_FUNCTION(pointee_type*);

				typedef mse::impl::target_type<T1> pointee_t;
				MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, pointee_t);
				MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, char);
				MSE_IF_CONSTEXPR(std::is_const<pointee_t>::value) {
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, mse::impl::remove_const_t<pointee_t>);
					MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_APPLY_MACRO_FUNCTION_TO_CANDIDATE_POINTER_TYPES(MSE_IMPL_LH_EXPLICITLY_CASTABLE_ANY_CAST_ATTEMPT4, const char);
				}

				{
					auto maybe_retval = conversion_operator_helper4<T1>(typename mse::impl::is_complete_type<pointee_t>::type(), ptr1);
					if (maybe_retval.has_value()) {
						return maybe_retval;
					}
				}

				auto casted_ptr = mse::us::impl::ns_any::any_cast<T1>(&mse::us::impl::as_ref<base_class>(*ptr1));
				if (casted_ptr) {
					return *casted_ptr;
				}
				return {};
			}
		}

		class const_void_star_replacement;

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

			/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
			non-const reference. */
			template <size_t _Size, typename _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_const<_Ty2>::value)> MSE_IMPL_EIS >
			void_star_replacement(TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(mse::lh::TLHNullableAnyRandomAccessIterator<_Ty2>(val.begin())), m_is_nullptr(false) {}
			//template <size_t _Size, typename _Ty2>
			//void_star_replacement(const TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(mse::lh::TLHNullableAnyRandomAccessIterator<const _Ty2>(val.cbegin())), m_is_nullptr(false) {}

			template <typename _Ty2>
			void_star_replacement(mse::lh::TNativeFunctionPointerReplacement<_Ty2>& val) : base_class(val), m_is_nullptr(false) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value && (!std::is_const<mse::impl::target_or_void_type<T> >::value)) || (std::is_same<void *, T>::value))> MSE_IMPL_EIS >
			void_star_replacement(const T& ptr) : base_class(ptr), m_is_nullptr(mse::impl::evaluates_to_false(ptr))
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

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }

			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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

#ifndef MSE_HAS_CXX20
			friend bool operator!=(const void_star_replacement& _Left_cref, const void* _Right_cref) {
				return !(_Left_cref == _Right_cref);
			}
			friend bool operator!=(const void* _Left_cref, const void_star_replacement& _Right_cref) {
				return !(_Right_cref == _Left_cref);
			}

			friend bool operator==(const void* _Left_cref, const void_star_replacement& _Right_cref) {
				return (_Right_cref == _Left_cref);
			}
#endif // !MSE_HAS_CXX20
			friend bool operator==(const void_star_replacement& _Left_cref, const void* _Right_cref) {
				return (_Left_cref.m_shadow_void_const_ptr == _Right_cref);
			}
#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))

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
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void *, T>::value) || (std::is_same<const void*, T>::value) || (std::is_same<uintptr_t, T>::value)
					|| (mse::impl::is_instantiation_of<mse::impl::remove_const_t<T>, mse::lh::TNativeFunctionPointerReplacement>::value))> MSE_IMPL_EIS >
			explicit operator T() const {
				return operator_T_helper1<T>(typename std::is_same<uintptr_t, T>::type());
			}

		private:
			template<class T>
			static void const* make_void_const_ptr_helper1(std::true_type, const T& src_ptr) {
				return src_ptr;
			}
			template<class T>
			static void const* make_void_const_ptr_helper1(std::false_type, const T& src_ptr) {
				/* The hard cast is required for function pointers (in clang 18, but not msvc it seems). */
				return (void const*)mse::us::lh::make_raw_pointer_from(src_ptr);
			}

			template<class T>
			T operator_T_helper1(std::true_type) const {
				return (uintptr_t)m_shadow_void_const_ptr;
			}
			template<class T>
			T operator_T_helper1(std::false_type) const {
				//return base_class::operator T();
				const base_class& bc_cref = *this;
				return bc_cref.operator T();
			}

			bool m_is_nullptr = true;
			void const* m_shadow_void_const_ptr = nullptr;

			friend class const_void_star_replacement;
			template<typename _Ty>
			friend _Ty mse::us::lh::unsafe_cast(const mse::lh::void_star_replacement& x);
		};
	}
	namespace impl {
		template<>
		struct target_type_impl<mse::lh::void_star_replacement> {
			typedef void type;
		};
	}
	namespace lh {

		class const_void_star_replacement : public impl::explicitly_castable_any {
		public:
			typedef impl::explicitly_castable_any base_class;
			//using base_class::base_class;
			typedef const_void_star_replacement _Myt;

			const_void_star_replacement() = default;
			const_void_star_replacement(const const_void_star_replacement&) = default;
			//const_void_star_replacement(const_void_star_replacement&&) = default;
			const_void_star_replacement(std::nullptr_t) : base_class((void*)(nullptr)), m_is_nullptr(true) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_base_of<void_star_replacement, T>::value)> MSE_IMPL_EIS >
			const_void_star_replacement(T src) : base_class(mse::us::impl::as_ref<const base_class>(src)) {}

			/* Note that construction from a const reference to a TNativeArrayReplacement<> is not the same as construction from a
			non-const reference. */
			template <size_t _Size, typename _Ty2>
			const_void_star_replacement(TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(mse::lh::TLHNullableAnyRandomAccessIterator<const _Ty2>(val.begin())), m_is_nullptr(false) {}
			template <size_t _Size, typename _Ty2>
			const_void_star_replacement(const TNativeArrayReplacement<_Ty2, _Size>& val) : base_class(mse::lh::TLHNullableAnyRandomAccessIterator<const _Ty2>(val.cbegin())), m_is_nullptr(false) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void*, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			const_void_star_replacement(const T& ptr) : base_class(ptr), m_is_nullptr(mse::impl::evaluates_to_false(ptr))
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
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) || (std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<void_star_replacement, TLHSPointer_ecwapt>::value))
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || (std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<void_star_replacement, TRHSPointer_ecwapt>::value))
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
			> MSE_IMPL_EIS >
			friend bool operator!=(const TLHSPointer_ecwapt& _Left_cref, const TRHSPointer_ecwapt& _Right_cref) {
				return !(_Left_cref == _Right_cref);
			}

			template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
				(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value || std::is_base_of<_Myt, TRHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TLHSPointer_ecwapt) || (std::is_base_of<_Myt, TLHSPointer_ecwapt>::value) || (std::is_base_of<void_star_replacement, TLHSPointer_ecwapt>::value))
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || (std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<void_star_replacement, TRHSPointer_ecwapt>::value))
				&& (mse::impl::IsExplicitlyCastableToBool_pb<TRHSPointer_ecwapt>::value)
			> MSE_IMPL_EIS >
#else // !MSE_HAS_CXX20
			template<typename TLHSPointer_ecwapt, typename TRHSPointer_ecwapt, MSE_IMPL_EIP mse::impl::enable_if_t<
				(std::is_base_of<_Myt, TLHSPointer_ecwapt>::value)
				&& (MSE_IMPL_IS_DEREFERENCEABLE_CRITERIA1(TRHSPointer_ecwapt) || (std::is_base_of<_Myt, TRHSPointer_ecwapt>::value) || (std::is_base_of<void_star_replacement, TRHSPointer_ecwapt>::value))
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

#if (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
			friend bool operator!=(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !(_Left_cref == _Right_cref); }
			friend bool operator==(const NULL_t& _Left_cref, const _Myt& _Right_cref) { assert(0 == _Left_cref); return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const NULL_t& _Right_cref) { assert(0 == _Right_cref); return !bool(_Left_cref); }

			friend bool operator!=(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator!=(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !(_Left_cref == _Right_cref); }
			friend bool operator==(const std::nullptr_t& _Left_cref, const _Myt& _Right_cref) { return !bool(_Right_cref); }
			friend bool operator==(const _Myt& _Left_cref, const std::nullptr_t& _Right_cref) { return !bool(_Left_cref); }
#else // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))
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
#endif // (!defined(MSE_HAS_CXX17) && defined(_MSC_VER))



			friend void swap(const_void_star_replacement& first, const_void_star_replacement& second) {
				std::swap(static_cast<base_class&>(first), static_cast<base_class&>(second));
				std::swap(first.m_is_nullptr, second.m_is_nullptr);
				std::swap(first.m_shadow_void_const_ptr, second.m_shadow_void_const_ptr);
			}

			const_void_star_replacement& operator=(const_void_star_replacement _Right) {
				std::swap(static_cast<base_class&>(*this), static_cast<base_class&>(_Right));
				std::swap((*this).m_is_nullptr, _Right.m_is_nullptr);
				std::swap((*this).m_shadow_void_const_ptr, _Right.m_shadow_void_const_ptr);
				return (*this);
			}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value && std::is_const<mse::impl::target_or_void_type<T> >::value) || (std::is_same<const void*, T>::value) || (std::is_same<uintptr_t, T>::value)
					|| (mse::impl::is_instantiation_of<mse::impl::remove_const_t<T>, mse::lh::TNativeFunctionPointerReplacement>::value))> MSE_IMPL_EIS >
			explicit operator T() const {
				return operator_T_helper1<T>(typename std::is_same<uintptr_t, T>::type());
			}

		private:
			template<class T>
			static void const* make_void_const_ptr_helper1(std::true_type, const T& src_ptr) {
				return src_ptr;
			}
			template<class T>
			static void const* make_void_const_ptr_helper1(std::false_type, const T& src_ptr) {
				/* The hard cast is required for function pointers (in clang 18, but not msvc it seems). */
				return (void const*)mse::us::lh::make_raw_pointer_from(src_ptr);
			}

			template<class T>
			T operator_T_helper1(std::true_type) const {
				return (uintptr_t)m_shadow_void_const_ptr;
			}
			template<class T>
			T operator_T_helper1(std::false_type) const {
				//return base_class::operator T();
				const base_class& bc_cref = *this;
				return bc_cref.operator T();
			}

			bool m_is_nullptr = true;
			void const* m_shadow_void_const_ptr = nullptr;

			friend class void_star_replacement;
			template<typename _Ty>
			friend _Ty mse::us::lh::unsafe_cast(const mse::lh::const_void_star_replacement& x);
		};
	}
	namespace impl {
		template<>
		struct target_type_impl<mse::lh::const_void_star_replacement> {
			typedef void const type;
		};
	}
	namespace lh {

		template <typename _Ty>
		auto TLHNullableAnyRandomAccessIterator<_Ty>::construction_helper1(std::true_type, const const_void_star_replacement& src) -> TLHNullableAnyRandomAccessIterator<const _Ty> {
			typedef TLHNullableAnyRandomAccessIterator<const _Ty> ret_t;
			return src.operator ret_t();
		}
		template <typename _Ty>
		auto TXScopeLHNullableAnyRandomAccessIterator<_Ty>::construction_helper1(std::true_type, const const_void_star_replacement& src) -> TXScopeLHNullableAnyRandomAccessIterator<const _Ty> {
			typedef TXScopeLHNullableAnyRandomAccessIterator<const _Ty> ret_t;
			return src.operator ret_t();
		}
		template <typename _Ty>
		auto TLHNullableAnyPointer<_Ty>::construction_helper1(std::true_type, const const_void_star_replacement& src) -> TLHNullableAnyPointer<const _Ty> {
			typedef TLHNullableAnyPointer<const _Ty> ret_t;
			return src.operator ret_t();
		}
		template <typename _Ty>
		auto TXScopeLHNullableAnyPointer<_Ty>::construction_helper1(std::true_type, const const_void_star_replacement& src) -> TXScopeLHNullableAnyPointer<const _Ty> {
			typedef TXScopeLHNullableAnyPointer<const _Ty> ret_t;
			return src.operator ret_t();
		}

		template<>
		class TLHNullableAnyPointer<void> : public void_star_replacement {
		public:
			typedef void_star_replacement base_class;
			using base_class::base_class;
			TLHNullableAnyPointer() = default;
			TLHNullableAnyPointer(const TLHNullableAnyPointer&) = default;
			//TLHNullableAnyPointer(TLHNullableAnyPointer&&) = default;
			TLHNullableAnyPointer(const base_class& src) : base_class(src) {}
			//TLHNullableAnyPointer(base_class&& src) : base_class(MSE_FWD(src)) {}

			TLHNullableAnyPointer(std::nullptr_t) : base_class(nullptr) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void*, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			TLHNullableAnyPointer(const T& ptr) : base_class(ptr) {}

			TLHNullableAnyPointer& operator=(TLHNullableAnyPointer _Right) {
				base_class::operator=(_Right);
				return (*this);
			}
		};
		template<>
		class TLHNullableAnyPointer<const void> : public const_void_star_replacement {
		public:
			typedef const_void_star_replacement base_class;
			using base_class::base_class;
			TLHNullableAnyPointer() = default;
			TLHNullableAnyPointer(const TLHNullableAnyPointer&) = default;
			//TLHNullableAnyPointer(TLHNullableAnyPointer&&) = default;
			TLHNullableAnyPointer(const base_class& src) : base_class(src) {}
			//TLHNullableAnyPointer(base_class&& src) : base_class(MSE_FWD(src)) {}

			TLHNullableAnyPointer(std::nullptr_t) : base_class(nullptr) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void*, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			TLHNullableAnyPointer(const T& ptr) : base_class(ptr) {}

			TLHNullableAnyPointer& operator=(TLHNullableAnyPointer _Right) {
				base_class::operator=(_Right);
				return (*this);
			}
		};

		template<>
		class TLHNullableAnyRandomAccessIterator<void> : public void_star_replacement {
		public:
			typedef void_star_replacement base_class;
			using base_class::base_class;
			TLHNullableAnyRandomAccessIterator() = default;
			TLHNullableAnyRandomAccessIterator(const TLHNullableAnyRandomAccessIterator&) = default;
			//TLHNullableAnyRandomAccessIterator(TLHNullableAnyRandomAccessIterator&&) = default;
			TLHNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
			//TLHNullableAnyRandomAccessIterator(base_class&& src) : base_class(MSE_FWD(src)) {}

			TLHNullableAnyRandomAccessIterator(std::nullptr_t) : base_class(nullptr) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void*, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const T& ptr) : base_class(ptr) {}

			TLHNullableAnyRandomAccessIterator& operator=(TLHNullableAnyRandomAccessIterator _Right) {
				base_class::operator=(_Right);
				return (*this);
			}
		};
		template<>
		class TLHNullableAnyRandomAccessIterator<const void> : public const_void_star_replacement {
		public:
			typedef const_void_star_replacement base_class;
			using base_class::base_class;
			TLHNullableAnyRandomAccessIterator() = default;
			TLHNullableAnyRandomAccessIterator(const TLHNullableAnyRandomAccessIterator&) = default;
			//TLHNullableAnyRandomAccessIterator(TLHNullableAnyRandomAccessIterator&&) = default;
			TLHNullableAnyRandomAccessIterator(const base_class& src) : base_class(src) {}
			//TLHNullableAnyRandomAccessIterator(base_class&& src) : base_class(MSE_FWD(src)) {}

			TLHNullableAnyRandomAccessIterator(std::nullptr_t) : base_class(nullptr) {}

			template<class T, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<T> >::value)
				&& ((mse::impl::IsDereferenceable_pb<T>::value) || (std::is_same<void*, T>::value) || (std::is_same<void const*, T>::value))> MSE_IMPL_EIS >
			TLHNullableAnyRandomAccessIterator(const T& ptr) : base_class(ptr) {}

			TLHNullableAnyRandomAccessIterator& operator=(TLHNullableAnyRandomAccessIterator _Right) {
				base_class::operator=(_Right);
				return (*this);
			}
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
					_Ty unsafe_cast_helper11(std::false_type, const _Ty2& x) {
						return (_Ty const&)(x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper11(std::true_type, const _Ty2& x) {
						/* So both _Ty and _Ty2 are dereferenceable, and _Ty is constructible from a raw pointer. So rather than hard/reinterpret cast 
						from the parameter, of type _Ty2, to (presumably incompatible type) _Ty, it might be less dangerous to construct a raw pointer 
						to the parameter's target object, hard cast that raw pointer to a raw pointer that targets the same type as _Ty does, then 
						construct a _Ty object from that raw pointer. */
						if (x) {
							return _Ty((mse::impl::target_type<_Ty>*)std::addressof(*x));
						}
						return _Ty((mse::impl::target_type<_Ty>*)nullptr);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper10(std::false_type, const _Ty2& x) {
						return unsafe_cast_helper11<_Ty>(typename mse::impl::conjunction<
							mse::impl::IsNullable_msemsearray<_Ty2>, mse::impl::IsDereferenceable_pb<_Ty2>, mse::impl::IsDereferenceable_pb<_Ty>
							, std::is_constructible<_Ty, mse::impl::target_or_void_type<_Ty>*> 
						>::type(), x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper10(std::true_type, const _Ty2& x) {
						return _Ty(x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper9(std::false_type, const _Ty2& x) {
						return unsafe_cast_helper10<_Ty>(typename std::is_constructible<_Ty, _Ty2>::type(), x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper9(std::true_type, const _Ty2& x) {
						/* _Ty is a raw reference type */
						return (_Ty)(x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper8(std::false_type, const _Ty2& x) {
						return (_Ty)(x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper8(std::true_type, const _Ty2& x) {
						return (_Ty)(std::addressof(*x));
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper7(std::false_type, const _Ty2& x) {
						return mse::us::lh::unsafe_cast<_Ty>(mse::us::impl::as_ref<const mse::lh::const_void_star_replacement>(x));
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper7(std::true_type, const _Ty2& x) {
						return mse::us::lh::unsafe_cast<_Ty>(mse::us::impl::as_ref<const mse::lh::void_star_replacement>(x));
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper6(std::false_type, const _Ty2& x) {
						return unsafe_cast_helper8<_Ty>(typename mse::impl::IsDereferenceable_pb<_Ty2>::type(), x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper6(std::true_type, const _Ty2& x) {
						return unsafe_cast_helper7<_Ty>(typename std::is_base_of<mse::lh::void_star_replacement, _Ty2>::type(), x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper5(std::false_type, const _Ty2& x) {
						if (x == nullptr)
						{
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
						return unsafe_cast_helper6<_Ty>(typename std::integral_constant<bool, (std::is_base_of<mse::lh::void_star_replacement, _Ty2>::value || std::is_base_of<mse::lh::const_void_star_replacement, _Ty2>::value)>::type(), x);
					}
					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper4(std::true_type, const _Ty2& x) {
						typedef mse::impl::remove_reference_t<mse::impl::remove_const_t<_Ty2> > ncTy2;
						return unsafe_cast_helper5<_Ty>(typename std::integral_constant<bool, (std::is_same<void*, ncTy2>::value || std::is_same<void const*, ncTy2>::value)>::type(), x);
					}

					template<typename _Ty, typename _Ty2>
					_Ty unsafe_cast_helper3(std::false_type, const _Ty2& x) {
						return unsafe_cast_helper9<_Ty>(typename std::is_reference<_Ty>::type(), x);
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
			template<typename _Ty, typename _Ty2>
			_Ty unsafe_cast(_Ty2 x[]) {
				return impl::ns_unsafe_cast::unsafe_cast_helper2<_Ty>(std::false_type(), (_Ty2*)x);
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
			_Ty unsafe_cast(const mse::lh::const_void_star_replacement& x) {
				return unsafe_cast<_Ty>(const_cast<void const*>(x.m_shadow_void_const_ptr));
			}
			namespace impl {
				inline auto make_raw_pointer_from_helper2(std::true_type, mse::lh::void_star_replacement const& vsr) -> void* {
					return unsafe_cast<void*>(vsr);
				}
				inline auto make_raw_pointer_from_helper2(std::false_type, mse::lh::const_void_star_replacement const& vsr) -> void const* {
					return unsafe_cast<void const*>(vsr);
				}

				template<typename _Ty>
				auto make_raw_pointer_from_helper1(std::false_type, _Ty const& ptr)/* -> decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))*/ {
					return unsafe_cast<decltype(std::addressof(mse::us::impl::base_type_raw_reference_to(*ptr)))>(ptr);
				}
				template<typename _Ty>
				auto make_raw_pointer_from_helper1(std::true_type, _Ty const& ptr) {
					return make_raw_pointer_from_helper2(typename std::is_base_of<mse::lh::void_star_replacement, _Ty>::type(), ptr);
				}
			}

			template<typename _Ty>
			auto make_raw_pointer_from(_Ty const& ptr) {
				static const bool b1 = (std::is_base_of<mse::lh::void_star_replacement, _Ty>::value || std::is_base_of<mse::lh::const_void_star_replacement, _Ty>::value);
				return impl::make_raw_pointer_from_helper1(typename std::integral_constant<bool, b1>::type(), ptr);
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

			/* An iterator pointing into a container of (other) iterators can't be directly converted to a raw pointer iterator
			pointing into an array of (other) raw pointer iterators. In order to produce such a raw pointer iterator, we first
			need to construct an array of pointer iterators. Once constructed, we can then return (via conversion operator) a
			pointer iterator that points into that array. We make the returned pointer iterator const, as any changes to the 
			constructed array that it targets would not be reflected in the original source array. */
			template<typename _Ty>
			class TXScopeConstArrayOfRawPointersStore : public mse::us::impl::XScopeTagBase {
			public:
				TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > iter1) {
					if (iter1) {
						while (*iter1) {
							auto& pointee_ref = *iter1;
							m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
							++iter1;
						}
						m_pointer_vec.push_back(nullptr);
					}
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
				explicit TXScopeConstArrayOfRawPointersStore(mse::TRegisteredNotNullPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> >(iter1)) {}
				TXScopeConstArrayOfRawPointersStore(_Ty* const* iter1) : m_converted_ptr(iter1) {}

				TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> > iter1) {
					if (iter1) {
						while (*iter1) {
							auto& pointee_ref = *iter1;
							m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
							++iter1;
						}
						m_pointer_vec.push_back(nullptr);
					}
				}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> >(iter1)) {}
				template<size_t N>
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TNativeArrayReplacement<mse::lh::TStrongVectorIterator<_Ty>, N> const& safe_array1) {
					for (auto& iter2 : safe_array1) {
						m_pointer_vec.push_back(make_raw_pointer_from(iter2));
					}
				}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeStrongVectorIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> >(iter1)) {}
				explicit TXScopeConstArrayOfRawPointersStore(mse::lh::TStrongVectorIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> >(iter1)) {}
				explicit TXScopeConstArrayOfRawPointersStore(mse::TRegisteredNotNullPointer<mse::lh::TStrongVectorIterator<_Ty> > const& iter1)
					: TXScopeConstArrayOfRawPointersStore(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> >(iter1)) {}

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

			/* A pointer pointing into an iterator can't be directly converted to a raw pointer pointing to a raw pointer iterator. 
			In order to produce such a raw pointer, we first need to construct a raw pointer iterator that points to the same element 
			as the original target iterator. Once constructed, we can then return (via conversion operator) a raw pointer that 
			points to that constructed pointer iterator. The returned pointer may be used to change which element the target pointer 
			iterator points to. Upon destruction, we attempt to reflect such a change back to the original target (presumably safe) 
			iterator. We don't really have a choice but to assume that the newly targeted element is in the same container as the 
			originally targeted element (if any). */
			template<typename _Ty>
			class TXScopePointerToRawPointersStore : public mse::us::impl::XScopeTagBase {
			public:
				TXScopePointerToRawPointersStore(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TStrongVectorIterator<_Ty> >  ptr1) 
					: m_maybe_sviter_aptr(ptr1) {

					if (ptr1) {
						if (*ptr1) {
							auto& pointee_ref = *ptr1;
							m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
						}
						m_pointer_vec.push_back(nullptr);
					}
				}
				TXScopePointerToRawPointersStore(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> >  ptr1) 
					: m_maybe_aiter_aptr(ptr1) {

					if (ptr1) {
						if (*ptr1) {
							auto& pointee_ref = *ptr1;
							m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
						}
						m_pointer_vec.push_back(nullptr);
					}
				}
				TXScopePointerToRawPointersStore(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyPointer<_Ty> >  ptr1)
					: m_maybe_aptr_aptr(ptr1) {

					if (ptr1) {
						if (*ptr1) {
							auto& pointee_ref = *ptr1;
							m_pointer_vec.push_back(make_raw_pointer_from(pointee_ref));
			}
						m_pointer_vec.push_back(nullptr);
					}
				}

				template<typename TIter>
				void destructor_helper1(TIter iter_ptr) {
					if (iter_ptr) {
						auto iter1 = *iter_ptr;

						if (1 > m_pointer_vec.size()) {
							assert(false);
							return;
						}
						auto* pointee_raw_pointer = m_pointer_vec.front();

						if (m_returned_converted_ptr) {
							if (*m_returned_converted_ptr) {
								const auto ptr_diff1 = ((*m_returned_converted_ptr) - m_returned_converted_pointee_ptr);
								(*iter_ptr) += ptr_diff1;
							}
						}
					}
				}
				template<typename TPtr>
				void destructor_helper2(TPtr ptr_ptr) {
					if (ptr_ptr) {
						auto ptr1 = *ptr_ptr;

						if (1 > m_pointer_vec.size()) {
							assert(false);
							return;
						}
						auto* pointee_raw_pointer = m_pointer_vec.front();

						if (m_returned_converted_ptr) {
							if (*m_returned_converted_ptr) {
								if ((*m_returned_converted_ptr) != m_returned_converted_pointee_ptr) {
									(*ptr_ptr) = *m_returned_converted_ptr;
								}
							}
						}
					}
				}
				~TXScopePointerToRawPointersStore() {
					if (m_maybe_sviter_aptr.has_value()) {
						destructor_helper1(m_maybe_sviter_aptr.value());
					} else if (m_maybe_aiter_aptr.has_value()) {
						destructor_helper1(m_maybe_aiter_aptr.value());
					} else if (m_maybe_aptr_aptr.has_value()) {
						destructor_helper2(m_maybe_aptr_aptr.value());
					}
				}

				_Ty** raw_pointer_to_stored_pointers() {
					if (1 <= m_pointer_vec.size()) {
						m_returned_converted_ptr = std::addressof(m_pointer_vec.front());
						m_returned_converted_pointee_ptr = *m_returned_converted_ptr;
						return m_returned_converted_ptr;
					}
					return nullptr;
				}
				operator _Ty** () {
					return raw_pointer_to_stored_pointers();
				}
				_Ty* const * raw_pointer_to_stored_pointers() const {
					if (1 <= m_pointer_vec.size()) {
						return std::addressof(m_pointer_vec.front());
					}
					return nullptr;
				}
				operator _Ty* const * () const {
					return raw_pointer_to_stored_pointers();
				}

			private:
				std::vector<_Ty*> m_pointer_vec;
				_Ty** m_returned_converted_ptr = nullptr;
				_Ty* m_returned_converted_pointee_ptr = nullptr;

				template<typename T2>
				using my_optional = mse::us::impl::ns_optional::optional_base2<T2, mse::non_thread_safe_shared_mutex, mse::us::impl::ns_optional::optional_base2_not_const_lockable_tag>;
				my_optional<mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TStrongVectorIterator<_Ty> > > m_maybe_sviter_aptr;
				my_optional<mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > > m_maybe_aiter_aptr;
				my_optional<mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyPointer<_Ty> > > m_maybe_aptr_aptr;
			};

			/* An iterator pointing into a container of (other) iterators can't be directly converted to a raw pointer iterator
			pointing into an array of (other) raw pointer iterators. (I.e. for example, an 
			mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<char> > can't be directly 
			converted to a `char**`.) In order to produce such a raw pointer iterator, we firstneed to construct an array of 
			pointer iterators. Once constructed, we can then return (via conversion operator) a pointer iterator that points 
			into that array. We make the returned pointer iterator const, as any changes to the constructed array that it 
			targets would not be reflected in the original source array. */
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty, size_t N>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TNativeArrayReplacement<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty>, N> const& safe_array1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(safe_array1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TStrongVectorIterator<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(_Ty* const* iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}

			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TLHNullableAnyRandomAccessIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty, size_t N>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TNativeArrayReplacement<mse::lh::TStrongVectorIterator<_Ty>, N> const& safe_array1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(safe_array1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeStrongVectorIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TStrongVectorIterator<mse::lh::TStrongVectorIterator<_Ty> > const& iter1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopeConstArrayOfRawPointersStore<_Ty>(iter1);
			}

			/* Even though the (overloaded) function is called "make_temporary_raw_pointer_to_pointer_proxy_from()", it's possible that the given 
			argument ends up being not an iterator, but instead just a pointer. In this case it still makes sense to construct an array of 
			raw pointer iterators, it's just that the array will contain just one element. But the difference is, a pointer to an iterator 
			argument is likely to be used to modify the value of the target iterator, whereas if the argument were an array of iterators 
			we presume that the target iterator values would not be changed. So in the case that the given argument is just a pointer, 
			we need to additionally try to reflect any changes made to the target raw pointer iterator back to the original source 
			target iterator. So we return (two) different class (templates) for the (two) different behaviors. */
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& ptr1) -> TXScopeConstArrayOfRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TLHNullableAnyPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::TRegisteredNotNullPointer<mse::lh::TLHNullableAnyRandomAccessIterator<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TStrongVectorIterator<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TLHNullableAnyPointer<mse::lh::TStrongVectorIterator<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::TRegisteredNotNullPointer<mse::lh::TStrongVectorIterator<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TXScopeLHNullableAnyPointer<mse::lh::TLHNullableAnyPointer<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::lh::TLHNullableAnyPointer<mse::lh::TLHNullableAnyPointer<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(mse::TRegisteredNotNullPointer<mse::lh::TLHNullableAnyPointer<_Ty> > const& ptr1) -> TXScopePointerToRawPointersStore<_Ty> {
				return TXScopePointerToRawPointersStore<_Ty>(ptr1);
			}
			template<typename _Ty, size_t _Size>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(_Ty(*ptr1)[_Size]) {
				/* Regardless of how useful a pointer to native array parameter might be, we have encountered them in the wild. */
				return ptr1;
			}
			template<typename _Ty>
			auto make_temporary_raw_pointer_to_pointer_proxy_from(_Ty** ptr1) {
				return ptr1;
			}

			/* The generic make_raw_pointer_from() implementations will often internally use a const reference to the argument even if 
			the argument reference was not originally const, because is most cases it doesn't matter. But in the case of TXScopePointerToRawPointersStore<>, 
			the (non)constness of the reference makes a functional difference. So we make an overload for the rvalue reference (the only 
			one expected to be used) that avoids reinterpreting it as a a const reference. 
			Note, wrapping a TXScopePointerToRawPointersStore<> with a make_raw_pointer_from() call is generally redundant, but sometimes 
			it might just be easier for auto-conversion tools to add the make_raw_pointer_from() call. */
			template<typename _Ty>
			auto make_raw_pointer_from(TXScopePointerToRawPointersStore<_Ty>&& ptr) { return (_Ty**)ptr; }

			namespace impl {
				namespace ns_fn_wrapper {
					template<typename T>
					struct type_or_raw_pointer_if_dereferenceable_impl {
						typedef typename std::conditional<mse::impl::IsDereferenceable_pb<T>::value
							, typename type_or_raw_pointer_if_dereferenceable_impl<mse::impl::target_or_void_type<T> >::type *, T>::type type;
					};
					template<typename T>
					using type_or_raw_pointer_if_dereferenceable = typename type_or_raw_pointer_if_dereferenceable_impl<T>::type;

					template<typename T>
					auto casted_arg_helper2(std::true_type, const T& arg) {
						return (typename std::conditional<std::is_base_of<mse::lh::void_star_replacement, T>::value, void*, const void*>::type)(arg);
					}
					template<typename T>
					auto casted_arg_helper2(std::false_type, const T& arg) {
						return arg;
					}

					template<typename T>
					auto casted_pointer_arg_helper(std::true_type, const T& arg) {
						return mse::us::lh::make_temporary_raw_pointer_to_pointer_proxy_from(arg);
					}
					template<typename T>
					auto casted_pointer_arg_helper(std::false_type, const T& arg) {
						return mse::us::lh::make_raw_pointer_from(arg);
					}
					template<typename T>
					auto casted_arg_helper1(std::true_type, const T& arg) {
						return casted_pointer_arg_helper(typename mse::impl::IsDereferenceable_pb<mse::impl::target_or_void_type<T> >::type(), arg);
					}
					template<typename T>
					auto casted_arg_helper1(std::false_type, const T& arg) {
						return casted_arg_helper2(typename std::integral_constant<bool, std::is_base_of<mse::lh::void_star_replacement, T>::value || std::is_base_of<mse::lh::const_void_star_replacement, T>::value>::type(), arg);
					}
					template<typename T>
					auto casted_arg(const T& arg) {
						return casted_arg_helper1(typename mse::impl::IsDereferenceable_pb<T>::type(), arg);
					}

					template<typename TWrapperRet, typename T>
					auto casted_pointer_retval_helper(std::true_type, const T& retval) {
						/* TWrapperRet seems to be an iterator */
						return mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(retval);
					}
					template<typename TWrapperRet, typename T>
					auto casted_pointer_retval_helper(std::false_type, const T& retval) {
						return mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(retval);
					}
					template<typename TWrapperRet, typename T>
					auto casted_retval_helper1(std::true_type, const T& retval) {
						return casted_pointer_retval_helper<TWrapperRet>(typename mse::impl::SupportsSubtraction_poly<TWrapperRet>::type(), retval);
					}
					template<typename TWrapperRet, typename T>
					auto casted_retval_helper1(std::false_type, const T& retval) {
						return retval;
					}
					template<typename TWrapperRet, typename T>
					auto casted_retval(const T& retval) {
						return casted_retval_helper1<TWrapperRet>(typename mse::impl::IsDereferenceable_pb<T>::type(), retval);
					}
				}
			}

			/* This function is used to create a "wrapper" (lambda) function (presumably with a safe interface) that just calls the 
			given function (with a presumably unsafe interface). The function signature of the wrapper function is deduced from the 
			(otherwise unused) second (`mse::lh::TNativeFunctionPointerReplacement<>`) parameter. */
			template< typename TWrappee, typename TWrapperRet, typename... TArgs>
			auto unsafe_make_fn_wrapper(const TWrappee& wrappee, mse::lh::TNativeFunctionPointerReplacement<TWrapperRet(TArgs...)>) {
				auto retval = [wrappee](TArgs... args) { return TWrapperRet(impl::ns_fn_wrapper::casted_retval<TWrapperRet>(wrappee(impl::ns_fn_wrapper::casted_arg(args)...))); };
				return retval;
			}

			/* Overload for functions that return void. */
			template< typename TWrappee, typename... TArgs>
			auto unsafe_make_fn_wrapper(const TWrappee& wrappee, mse::lh::TNativeFunctionPointerReplacement<void(TArgs...)>) {
				auto retval = [wrappee](TArgs... args) { wrappee(impl::ns_fn_wrapper::casted_arg(args)...); };
				return retval;
			}

			namespace impl {
				namespace ns_raw_fn_wrapper {
					template<typename T>
					auto casted_arg_helper2(std::true_type, const T& arg) {
						return (typename std::conditional<std::is_same<void*, T>::value, mse::lh::void_star_replacement, mse::lh::const_void_star_replacement>::type)(arg);
					}
					template<typename T>
					auto casted_arg_helper2(std::false_type, const T& arg) {
						return arg;
					}

					template<typename T>
					auto casted_pointer_arg_helper(std::true_type, const T& arg) {
						/* T seems to be an iterator */
						return mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(arg);
					}
					template<typename T>
					auto casted_pointer_arg_helper(std::false_type, const T& arg) {
						return mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(arg);
					}
					template<typename T>
					auto casted_arg_helper1(std::true_type, const T& arg) {
						return casted_pointer_arg_helper<T>(typename mse::impl::SupportsSubtraction_poly<T>::type(), arg);
					}
					template<typename T>
					auto casted_arg_helper1(std::false_type, const T& arg) {
						return casted_arg_helper2(typename std::integral_constant<bool, std::is_same<void*, T>::value || std::is_same<void const*, T>::value>::type(), arg);
					}
					template<typename T>
					auto casted_arg(const T& arg) {
						return casted_arg_helper1(typename mse::impl::IsDereferenceable_pb<T>::type(), arg);
					}

					template<typename TWrapperRet, typename T>
					auto casted_retval_helper1(std::true_type, const T& retval) {
						return TWrapperRet(mse::us::lh::make_raw_pointer_from(retval));
					}
					template<typename TWrapperRet, typename T>
					auto casted_retval_helper1(std::false_type, const T& retval) {
						return TWrapperRet(retval);
					}
					template<typename TWrapperRet, typename T>
					auto casted_retval(const T& retval) {
						return casted_retval_helper1<TWrapperRet>(typename mse::impl::IsDereferenceable_pb<T>::type(), retval);
					}
				}

				/* This function is used to create a "wrapper" (lambda) function (with a potentially unsafe interface) that just calls the
				given function (with a presumably safe interface). The function signature of the wrapper function is deduced from the
				(otherwise unused) second (function pointer) parameter. Each specialization of this function template may only be used to
				wrap one distinct function. If you want to wrap a number of different functions that have the same function signature, you
				should (explicitly) specify a unique TDistinguishingID template argument for each distinct function that you want to wrap. 
				Following is a macro that invokes this function while automatically supplying a unique TDistinguishingID template argument. */
				template<typename TDistinguishingID = void, typename TWrappeeFnPtr, typename TWrapperRet, typename... TArgs>
				auto make_raw_fn_wrapper(TWrappeeFnPtr const& wrappee_fnptr, TWrapperRet(*)(TArgs...)) {
					thread_local TWrappeeFnPtr tl_wrappee_fnptr1;
					thread_local bool tl_first_run = true;
					if (!tl_first_run) {
						if (wrappee_fnptr != tl_wrappee_fnptr1) {
							MSE_THROW(std::logic_error("Apparent attempt to use mse::us::lh::impl::make_raw_fn_wrapper<>() specialization to wrap multiple different functions. Specify a unique TDistinguishingID template argument for each different function to be wrapped."));
							int q = 3;
						}
					}
					tl_first_run = false;
					tl_wrappee_fnptr1 = wrappee_fnptr;

					auto retval = [](TArgs... args) noexcept {
						thread_local TWrappeeFnPtr tl_wrappee_fnptr2 = tl_wrappee_fnptr1;
						return TWrapperRet(ns_raw_fn_wrapper::casted_retval<TWrapperRet>(tl_wrappee_fnptr2(ns_raw_fn_wrapper::casted_arg(args)...)));
						};
					return retval;
				}

				/* Overload for functions that return void. */
				template<typename TDistinguishingID = void, typename TWrappeeFnPtr, typename... TArgs>
				auto make_raw_fn_wrapper(TWrappeeFnPtr const& wrappee_fnptr, void(*)(TArgs...)) {
					thread_local TWrappeeFnPtr tl_wrappee_fnptr1;
					thread_local bool tl_first_run = true;
					if (!tl_first_run) {
						if (wrappee_fnptr != tl_wrappee_fnptr1) {
							MSE_THROW(std::logic_error("Apparent attempt to use mse::us::lh::impl::make_raw_fn_wrapper<>() specialization to wrap multiple different functions. Specify a unique TDistinguishingID template argument for each different function to be wrapped."));
							int q = 3;
						}
					}
					tl_first_run = false;
					tl_wrappee_fnptr1 = wrappee_fnptr;

					auto retval = [](TArgs... args) noexcept {
						thread_local TWrappeeFnPtr tl_wrappee_fnptr2 = tl_wrappee_fnptr1;
						tl_wrappee_fnptr2(ns_raw_fn_wrapper::casted_arg(args)...);
						};
					return retval;
				}

				template<size_t... NumericIDs>
				struct UID1_t {};

				// AI generated constexpr string simple rolling-hash (base 131)
				constexpr std::size_t constexpr_hash1_step(const char* s, std::size_t h = 0u) {
					return *s ? constexpr_hash1_step(s + 1, h * 131u + static_cast<std::size_t>(static_cast<unsigned char>(*s))) : h;
				}
				template<std::size_t N>
				constexpr std::size_t constexpr_hash1(const char(&str)[N]) {
					return constexpr_hash1_step(str, 0);
				}
			}

#if (defined(__GNUC__) || defined(_MSC_VER))

			/* This macro is used to create a "wrapper" (lambda) function (with a potentially unsafe interface) that just calls the
			given function (with a presumably safe interface). The function signature of the wrapper function is deduced from the
			(otherwise unused) second (function pointer) parameter. (And remember that macro arguments with commas need to be 
			wrapped in parentheses.) */
#define MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER(wrappee_fnptr, example_fnptr_with_desired_signature) mse::us::lh::impl::make_raw_fn_wrapper<mse::us::lh::impl::UID1_t<mse::us::lh::impl::constexpr_hash1(__FILE__), __LINE__, __COUNTER__> >(wrappee_fnptr, example_fnptr_with_desired_signature)

#else // (defined(__GNUC__) || defined(_MSC_VER))
#define MSE_LH_MAKE_RAW_FN_WRAPPER(wrappee_fnptr, example_fnptr_with_desired_signature) mse::us::lh::impl::make_raw_fn_wrapper<mse::us::lh::impl::UID1_t<mse::us::lh::impl::constexpr_hash1(__FILE__), __LINE__> >(wrappee_fnptr, example_fnptr_with_desired_signature)
#endif // (defined(__GNUC__) || defined(_MSC_VER))


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


namespace mse {

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

	namespace self_test {
		class CLegacyHelpersTest1 {
		public:
			static void s_test1() {
#ifdef MSE_SELF_TESTS
#if !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
				{
					int i1 = 11;
					mse::TXScopeAnyPointer<int> xsaptr1 = &i1;
					mse::TRegisteredObj<int> i2_reg = 17;
					mse::TAnyPointer<int> aptr2 = &i2_reg;
#if 1
					mse::TAnyPointer<const int> ci_aptr2 = &i2_reg;
					mse::TAnyConstPointer<int> acptr2 = &i2_reg;
					mse::TNullableAnyPointer<int> naptr3 = &i2_reg;
					mse::lh::TLHNullableAnyPointer <int>  lhnaptr1;
					auto ptr2 = lhnaptr1 ? lhnaptr1 : nullptr;
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaraiter1;
					if (xsaptr1 == xsaptr1) {
						int q = 5;
					}
#ifdef MSE_HAS_CXX17
					auto b6 = (aptr2 == xsaptr1);
					auto b7 = (naptr3 == aptr2);
					auto b8 = (aptr2 != naptr3);
					auto b9 = (acptr2 == acptr2);
					auto b10 = (naptr3 == acptr2);
					auto b11 = (acptr2 != naptr3);
					bool b42 = (ci_aptr2 == aptr2);
					bool b43 = (aptr2 == ci_aptr2);
					bool b44 = (aptr2 != ci_aptr2);
#endif //MSE_HAS_CXX17
					mse::lh::TXScopeLHNullableAnyPointer<int> xslhnaptr1 = NULL;
					bool b26 = (xslhnaptr1 == NULL);
					bool b27 = (NULL == xslhnaptr1);
					mse::lh::TLHNullableAnyPointer<int> lhnaptr2 = NULL;
					bool b28 = (lhnaptr2 == NULL);
					bool b29 = (NULL == lhnaptr2);
					bool b30 = (lhnaptr2 == nullptr);
					bool b31 = (nullptr == lhnaptr2);
					bool b32 = (naptr3 == nullptr);
					bool b33 = (nullptr == naptr3);
					bool b34 = (nullptr != naptr3);
					bool b37 = std::is_same<decltype(nullptr), std::nullptr_t>::value;
					bool b38 = (lhnaptr2 == lhnaptr2);
					int q = 5;
#endif // 0
				}
				{
					auto arr1 = mse::nii_vector<int>{ 1, 2 };
					mse::TXScopeAnyRandomAccessIterator<int> xsaiter1 = mse::make_xscope_begin_iterator(&arr1);
					auto arr2_reg = mse::make_registered(mse::nii_vector<int>{ 1, 2 });
					mse::TAnyRandomAccessIterator<int> aiter2 = mse::make_begin_iterator(&arr2_reg);
					mse::TAnyRandomAccessIterator<const int> ci_aiter2 = mse::make_begin_iterator(&arr2_reg);
					mse::TAnyRandomAccessConstIterator<int> aciter2 = mse::make_begin_iterator(&arr2_reg);
					mse::TNullableAnyRandomAccessIterator<int> naiter3;
					mse::lh::TLHNullableAnyRandomAccessIterator <int>  lhnaiter1;
					auto iter2 = lhnaiter1 ? lhnaiter1 : nullptr;
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaraiter1;
					if (xsaiter1 == xsaiter1) {
						int q = 5;
					}
#ifdef MSE_HAS_CXX17
					MSE_TRY{
							auto b6 = (aiter2 == xsaiter1);
					}
					MSE_CATCH_ANY{}
					auto b7 = (naiter3 == aiter2);
					auto b8 = (aiter2 != naiter3);
					auto b9 = (aciter2 == aciter2);
					auto b10 = (naiter3 == aciter2);
					auto b11 = (aciter2 != naiter3);
					bool b42 = (ci_aiter2 == aiter2);
					bool b43 = (aiter2 == ci_aiter2);
					bool b44 = (aiter2 != ci_aiter2);
#endif //MSE_HAS_CXX17
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<int> xslhnaiter1 = NULL;
					bool b26 = (xslhnaiter1 == NULL);
					bool b27 = (NULL == xslhnaiter1);
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaiter2 = NULL;
					bool b28 = (lhnaiter2 == NULL);
					bool b29 = (NULL == lhnaiter2);
					bool b30 = (lhnaiter2 == nullptr);
					bool b31 = (nullptr == lhnaiter2);
					bool b32 = (naiter3 == nullptr);
					bool b33 = (nullptr == naiter3);
					bool b34 = (nullptr != naiter3);
					bool b37 = std::is_same<decltype(nullptr), std::nullptr_t>::value;
					bool b38 = (lhnaiter2 == lhnaiter2);

					int q = 5;
				}
				{
					int i1 = 11;
#if 1
					mse::lh::TXScopeLHNullableAnyPointer<int> xsaptr1 = &i1;
					mse::TRegisteredObj<int> i2_reg = 17;
					mse::lh::TLHNullableAnyPointer<int> aptr2 = &i2_reg;
					mse::lh::TLHNullableAnyPointer<int> acptr2 = &i2_reg;
					mse::TNullableAnyPointer<int> naptr3 = &i2_reg;
					mse::lh::TLHNullableAnyPointer <int>  lhnaptr1;
					auto ptr2 = lhnaptr1 ? lhnaptr1 : nullptr;
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaraiter1;
					if (xsaptr1 == xsaptr1) {
						int q = 5;
					}
#ifdef MSE_HAS_CXX17
					auto b6 = (aptr2 == xsaptr1);
					auto b7 = (naptr3 == aptr2);
					auto b8 = (aptr2 != naptr3);
					auto b9 = (acptr2 == acptr2);
					auto b10 = (naptr3 == acptr2);
					auto b11 = (acptr2 != naptr3);
#endif //MSE_HAS_CXX17
					mse::lh::TXScopeLHNullableAnyPointer<int> xslhnaptr1 = NULL;
					bool b26 = (xslhnaptr1 == NULL);
					bool b27 = (NULL == xslhnaptr1);
					mse::lh::TLHNullableAnyPointer<int> lhnaptr2 = NULL;
					bool b28 = (lhnaptr2 == NULL);
					bool b29 = (NULL == lhnaptr2);
					bool b30 = (lhnaptr2 == nullptr);
					bool b31 = (nullptr == lhnaptr2);
					bool b32 = (naptr3 == nullptr);
					bool b33 = (nullptr == naptr3);
					bool b34 = (nullptr != naptr3);
					bool b38 = (lhnaptr2 == lhnaptr2);
#endif // 0
					int q = 5;
				}
				{
#if 1
					auto arr1 = mse::nii_vector<int>{ 1, 2 };
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<int> xsaiter1 = mse::make_xscope_begin_iterator(&arr1);
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<const int> xsaciter1 = mse::make_xscope_begin_iterator(&arr1);
					auto arr2_reg = mse::make_registered(mse::nii_vector<int>{ 1, 2 });
					mse::lh::TLHNullableAnyRandomAccessIterator<int> aiter2 = mse::make_begin_iterator(&arr2_reg);
					mse::lh::TLHNullableAnyRandomAccessIterator<const int> aciter2 = mse::make_begin_iterator(&arr2_reg);
					mse::TNullableAnyRandomAccessIterator<int> naiter3;
					mse::lh::TLHNullableAnyRandomAccessIterator <int>  lhnaiter1;
					auto iter2 = lhnaiter1 ? lhnaiter1 : nullptr;
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaraiter1;
					if (xsaiter1 == xsaiter1) {
						int q = 5;
					}
#ifdef MSE_HAS_CXX17
					MSE_TRY{
							auto b6 = (aiter2 == xsaiter1);
					}
					MSE_CATCH_ANY{}
					auto b7 = (naiter3 == aiter2);
					auto b8 = (aiter2 != naiter3);
					auto b9 = (aciter2 == aciter2);
					auto b10 = (aiter2 == aciter2);
					auto b11 = (aciter2 != aiter2);
					auto b42 = (aciter2 == aiter2);
					auto b43 = (aiter2 == aciter2);
					auto b44 = (aiter2 != aciter2);
					auto b45 = (xsaciter1 == xsaiter1);
					auto b46 = (xsaiter1 == xsaciter1);
					auto b47 = (xsaiter1 != xsaciter1);
					auto b48 = (xsaciter1 == aiter2);
					auto b49 = (xsaiter1 == aciter2);
#endif //MSE_HAS_CXX17
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<int> xslhnaiter1 = NULL;
					bool b26 = (xslhnaiter1 == NULL);
					bool b27 = (NULL == xslhnaiter1);
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaiter2 = NULL;
					bool b28 = (lhnaiter2 == NULL);
					bool b29 = (NULL == lhnaiter2);
					bool b30 = (lhnaiter2 == nullptr);
					bool b31 = (nullptr == lhnaiter2);
					bool b32 = (naiter3 == nullptr);
					bool b33 = (nullptr == naiter3);
					bool b34 = (nullptr != naiter3);
					bool b38 = (lhnaiter2 == lhnaiter2);
#endif // 0
					int q = 5;
				}
				{
					mse::lh::TNativeArrayReplacement<int, 2> arr1{ 1, 2 };
					auto iter1 = arr1.begin();
					mse::TNullableAnyRandomAccessIterator<int> naraiter2 = iter1;
					mse::lh::TLHNullableAnyRandomAccessIterator<int> lhnaraiter1 = arr1;
					int q = 5;
				}
				{
					mse::TNullableAnyPointer<int> naptr1;
					mse::lh::TLHNullableAnyPointer<int> lhnaptr1;
					lhnaptr1 = naptr1;
				}
				{
					enum direction {
						EAST, NORTH, WEST, SOUTH
					};
					mse::TRegisteredObj<enum direction> direction_reg1 = NORTH;
					if (NORTH == direction_reg1) {
						int q = 5;
					}
					if (direction_reg1 == NORTH) {
						int q = 5;
					}
					auto direction_regptr1 = &direction_reg1;
					if (NORTH == *direction_regptr1) {
						int q = 5;
					}
					auto direction2 = *direction_regptr1;

					//mse::lh::TLHNullableAnyPointer<enum direction> lhnaptr1 = &direction_reg1;
					mse::lh::TLHNullableAnyPointer<enum direction> lhnaptr1 = direction_regptr1;
					lhnaptr1 = direction_regptr1;

					enum direction dir2 = (enum direction)(int)direction_reg1;
					enum direction dir3 = direction_reg1;
					direction_reg1 = WEST;

					mse::TRegisteredObj<int> int_reg1 = 2;
					typedef enum direction direction_t;
					dir2 = direction_t(int_reg1);
					dir2 = (enum direction)int_reg1;
					int i2 = int_reg1;
					bool b3 = (i2 == dir2);
					bool b4 = (int_reg1 == dir2);
					auto b5 = std::numeric_limits<std::string>::is_signed;
					auto digits1 = std::numeric_limits<std::string>::digits;
					size_t szt1 = 7;
					mse::CSize_t cszt2 = szt1;
					mse::CSize_t cszt3 = szt1;
					bool b6 = (cszt2 == cszt3);
					bool b7 = (cszt2 > szt1);
					mse::TFloatingPoint<float> flt1 = 3.14;
					mse::TFloatingPoint<float> flt2 = flt1;
					bool b8 = (flt1 == flt2);
					auto flt3 = flt1 + flt2;
					int q = 5;

					mse::lh::TLHNullableAnyPointer<int> int_lhnaptr1 = &int_reg1;
					int_lhnaptr1 = &int_reg1;
				}
				{
					enum class direction {
						EAST, NORTH, WEST, SOUTH
					};
					mse::TRegisteredObj<direction> direction_reg1 = direction::NORTH;

					bool b11 = std::is_enum<direction>::value;
					bool b12 = std::is_integral<direction>::value;
					//struct CD : direction {};
					direction dir4 = direction::NORTH;
					dir4 = direction_reg1;
					if (direction_reg1 == direction::NORTH) {
						int q = 5;
					}
#if 1
					if (direction::NORTH == direction_reg1) {
						int q = 5;
					}
					auto direction_regptr1 = &direction_reg1;
					if (direction::NORTH == *direction_regptr1) {
						int q = 5;
					}
					auto direction2 = *direction_regptr1;

					//mse::lh::TLHNullableAnyPointer<direction> lhnaptr1 = &direction_reg1;
					mse::lh::TLHNullableAnyPointer<direction> lhnaptr1 = direction_regptr1;
					lhnaptr1 = direction_regptr1;

					//enum direction dir2 = (direction)(int)direction_reg1;
					enum direction dir3 = direction_reg1;
					enum direction dir5 = (direction)(int)dir4;

					mse::TRegisteredObj<int> int_reg1 = 5;
					int i2 = int_reg1;
					int q = 5;

					mse::lh::TLHNullableAnyPointer<int> int_lhnaptr1 = &int_reg1;
					int_lhnaptr1 = &int_reg1;
#endif // 0
				}
				{
					FILE* fhandle1 = nullptr;
#ifdef _MSC_VER
					auto errno1 = fopen_s(&fhandle1, "README.md", "r");
#else // _MSC_VER
					fhandle1 = fopen("README.md", "r");
#endif // _MSC_VER
					if (fhandle1) {
						//char* char_buffer1 = malloc(buffer1_size);
						auto iter1 = mse::make_registered<mse::lh::TStrongVectorIterator<char> >();
						auto buffer1_size = mse::make_registered<size_t>(iter1.size());
						mse::lh::getline(&iter1, &buffer1_size, fhandle1);
						fclose(fhandle1);

						std::string line_that_was_read;
						auto length_of_line_that_was_read_plus_null_terminator = mse::lh::strlen(iter1) + 1;
						for (size_t i = 0; i < length_of_line_that_was_read_plus_null_terminator; ++i) {
							line_that_was_read += iter1[i];
						}
						int q = 5;
					}
					int q = 5;
				}
				{
					mse::lh::TLHNullableAnyRandomAccessIterator<char> lhs;
					mse::TRegisteredObj<mse::lh::TStrongVectorIterator<char> > rhs;
					mse::lh::TStrongVectorIterator<char> rhs2;
					auto b1 = (lhs == rhs);
					auto b2 = (lhs == rhs2);
					auto b7 = (rhs2 == rhs);
					int q = 5;
				}
#ifdef MSE_HAS_CXX17
				{
					mse::TRegisteredObj<int> i1_reg = 10;
					mse::lh::void_star_replacement vsr1 = &i1_reg;
					mse::lh::void_star_replacement vsr2;
					auto b1 = (vsr1 == vsr2);
					mse::lh::TLHNullableAnyPointer<int> lhnaptr1 = &i1_reg;
					auto b2 = (lhnaptr1 == nullptr);
					auto b3 = (lhnaptr1 == vsr1);
					auto b4 = (NULL == vsr1);
					MSE_TRY{
						auto tint2_regptr = (decltype(&std::declval<mse::TRegisteredObj<int> >()))(vsr1);
						typedef decltype(&std::declval<mse::TRegisteredObj<int> >())& type1;
						auto tint3_regptr = type1(vsr1);
						auto tint4_regptr = (decltype(&std::declval<mse::TRegisteredObj<int> >())&)(vsr1);
						auto ctint5_regptr = (decltype(&std::declval<mse::TRegisteredObj<const int> >()))(vsr1);
					} MSE_CATCH_ANY{
						int q = 5;
					}

					vsr1 = lhnaptr1;
					MSE_TRY{
						auto lhnaptr2 = mse::lh::TLHNullableAnyPointer<int>(vsr1);
						auto clhnaptr5 = mse::lh::TLHNullableAnyPointer<const int>(vsr1);
					}
					MSE_CATCH_ANY{
							int q = 5;
					}

					//auto fn_ptr = &std::strlen;
					//void* vs1 = fn_ptr;
					//vsr1 = fn_ptr;

					mse::lh::TNativeFunctionPointerReplacement<void()> nfr1;
					bool b5 = mse::impl::is_instantiation_of<mse::impl::remove_const_t<decltype(nfr1)>, mse::lh::TNativeFunctionPointerReplacement>::value;
					vsr1 = nfr1;
					MSE_TRY{
						auto nfr2 = mse::lh::TNativeFunctionPointerReplacement<void()>(vsr1);
						auto nfr3 = (mse::lh::TNativeFunctionPointerReplacement<void()>)(vsr1);
					}
						MSE_CATCH_ANY{
							int q = 5;
					}

					int q = 5;
				}
#endif // MSE_HAS_CXX17
				{
					int i1 = 7;
					auto int_anyptr1 = mse::us::unsafe_make_any_pointer_to(i1);
					mse::lh::TLHNullableAnyPointer<int> lhnaptr1;
					//lhnaptr1 = mse::us::lh::unsafe_make_lh_nullable_any_pointer_to(i1);
					lhnaptr1 = mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(&i1);
					auto i2 = *lhnaptr1;
					mse::TRegisteredObj<int> i1_reg = 10;
					lhnaptr1 = mse::us::lh::unsafe_make_lh_nullable_any_pointer_from(&i1_reg);
					auto i3 = *lhnaptr1;

					mse::lh::TLHNullableAnyRandomAccessIterator<int>  tok = nullptr;
					auto uslnaiter1 = mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(&i1);
					tok = uslnaiter1;

					int q = 5;
				}
#endif // !defined(MSE_SAFER_SUBSTITUTES_DISABLED)
				{
					auto xs_array1 = mse::make_xscope(mse::nii_array<int, 3>({ 1, 2, 3 }));
					auto xs_section1 = mse::make_xscope(mse::make_xscope_random_access_section(&xs_array1));
					auto xs_iter1 = mse::make_xscope_begin_iterator(&xs_section1);
					auto xs_ptr1 = mse::xscope_pointer(xs_iter1);
					auto xs_vec1 = mse::make_xscope(mse::nii_vector<int>({ 1, 2, 3 }));
					auto xs_section2 = mse::make_xscope(mse::make_xscope_random_access_section(&xs_vec1));
					auto xs_section3 = mse::make_xscope_random_access_section(&xs_vec1);
					auto vec2_reg = mse::make_registered(mse::nii_vector<int>({ 1, 2, 3 }));
					auto array2_reg = mse::make_registered(mse::nii_array<int, 3>({ 1, 2, 3 }));
					auto xs_section4 = mse::make_xscope(mse::make_xscope_random_access_section(&vec2_reg));
					auto xs_iter6 = mse::make_xscope_begin_iterator(&xs_section4);

					auto& i1_ref = xs_array1.at(1);
					//auto& i2_ref = xs_vec1.at(1);
					//auto& i3_ref = vec2_reg.at(1);
					//auto& i4_ref = xs_section2.at(1);
					//auto& i5_ref = xs_section1.at(1);
					//auto& i6_ref = xs_section3.at(1);
					auto xs_iter4 = mse::make_xscope_begin_iterator(&xs_section2);
					auto& i7_ref = *xs_iter4;
					auto xs_iter2 = mse::make_xscope_begin_iterator(&xs_vec1);
					auto& i8_ref = *xs_iter2;
					auto& i9_ref = *xs_iter1;
					auto iter3 = mse::make_begin_iterator(&vec2_reg);
					auto& i10_ref = *iter3;
					auto iter5 = mse::make_begin_iterator(&array2_reg);
					auto& i11_ref = *iter5;
					auto& i12_ref = *xs_iter6;
					vec2_reg.clear();
					int q = 5;
				}
				{
					std::string num_str1 = "-7.3";
					typedef char maybe_const_char;
					mse::lh::TLHNullableAnyRandomAccessIterator<maybe_const_char> str1 = std::addressof(num_str1.at(0));
					mse::TRegisteredObj<mse::lh::TLHNullableAnyRandomAccessIterator<char> > end = (char*)std::addressof(*str1);
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<char> xs_iter1 = end;
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<maybe_const_char> xs_iter2 = xs_iter1;
					mse::lh::TXScopeLHNullableAnyRandomAccessIterator<maybe_const_char> xs_iter3 = end;
					auto val1 = mse::lh::strtol(str1, &end, 10);
					auto len = end ? (std::addressof(*end) - std::addressof(*str1)) : -1;

					mse::TAnyRandomAccessIterator<const char> iter10 = num_str1.data();
					mse::TAnyRandomAccessConstIterator<char> citer11 = num_str1.data();
					citer11 = iter10;
					iter10 = citer11;

					mse::TRegisteredObj<mse::lh::TStrongVectorIterator<char> > sv_iter_reg1 = mse::lh::allocate(sv_iter_reg1, 4);
					mse::lh::strcpy(sv_iter_reg1, "314");
					mse::lh::TStrongVectorIterator<char> sv_iter2 = sv_iter_reg1;
					auto val2 = mse::lh::strtoll(sv_iter_reg1, nullptr, 10);
					auto val4 = mse::lh::strtoll(sv_iter_reg1, NULL, 10);

					mse::TRegisteredObj<mse::lh::TStrongVectorIterator<char> > sv_iter_reg3 = sv_iter_reg1;
					mse::lh::TLHNullableAnyPointer < mse::lh::TStrongVectorIterator<char> > sv_iter_lhnaptr = &sv_iter_reg3;
					auto val3 = mse::lh::strtoll(sv_iter_reg1, sv_iter_lhnaptr, 10);

					int q = 5;
				}
				{
#if 0
					/* Interestingly, this compiles and works under msvc, but not clang. */
					auto val1 = []() { return 7; }();
					typedef decltype([]() { return 7; }) my_type1;
					auto val2 = decltype([]() { return 7; })()();
					std::function<void(int)> fn1;
#endif // 0

					auto lambda1 = mse::us::lh::unsafe_make_fn_wrapper(&std::strlen, mse::lh::TNativeFunctionPointerReplacement<size_t(char const*)>());
					auto slen1 = lambda1("abc");
					mse::lh::TNativeFunctionPointerReplacement<size_t(char const*)> nfr1 = lambda1;
					auto slen2 = nfr1("defg");

					auto lambda2 = mse::us::lh::unsafe_make_fn_wrapper(&std::strlen, mse::lh::TNativeFunctionPointerReplacement<size_t(mse::lh::TLHNullableAnyRandomAccessIterator<char const>)>());
					mse::lh::TLHNullableAnyRandomAccessIterator<char const> nariter1 = "hij";
					auto slen3 = lambda2(nariter1);
					mse::lh::TNativeFunctionPointerReplacement<size_t(mse::lh::TLHNullableAnyRandomAccessIterator<char const>)> nfr2 = lambda2;
					mse::lh::TLHNullableAnyRandomAccessIterator<char const> nariter2 = "klmn";
					auto slen4 = nfr2(nariter2);
					auto lambda3 = [](mse::lh::TLHNullableAnyRandomAccessIterator<char const> x) { return size_t(0); };
					nfr2 = lambda3;
					nfr2 = mse::us::lh::unsafe_make_fn_wrapper(&std::strlen, mse::lh::TNativeFunctionPointerReplacement<size_t(mse::lh::TLHNullableAnyRandomAccessIterator<char const>)>());
					//(slen2 == 4) ? lambda3 : mse::us::lh::unsafe_make_fn_wrapper(&std::strlen, mse::lh::TNativeFunctionPointerReplacement<size_t(mse::lh::TLHNullableAnyRandomAccessIterator<char const>)>());
					//nfr2 = (slen2 == 4) ? lambda3 : mse::us::lh::unsafe_make_fn_wrapper(&std::strlen, mse::lh::TNativeFunctionPointerReplacement<size_t(mse::lh::TLHNullableAnyRandomAccessIterator<char const>)>());

					mse::TRegisteredObj<mse::lh::TStrongVectorIterator<const char>  > s;
					//auto s_ptr1 = mse::us::lh::unsafe_cast<mse::lh::TLHNullableAnyPointer<mse::lh::TStrongVectorIterator<char> >>(&s);
					//mse::lh::TLHNullableAnyPointer<mse::lh::TStrongVectorIterator<const char> > s_ptr2 = &s;
					auto sptr3 = (mse::lh::TLHNullableAnyPointer<mse::lh::TStrongVectorIterator<const char> > const&)(&s);

					mse::lh::void_star_replacement vstar = &s;
					auto uip = (uintptr_t)vstar;

#ifndef MSE_SAFER_SUBSTITUTES_DISABLED
					//typedef int(char) my_fn_type2;
					typedef int my_fn_type1(char);
					my_fn_type1* fn1 = nullptr;
					std::function<int(char)> stdfn2 = fn1;
					auto lambda4 = [](double x) { return (int)(x); };
					stdfn2 = lambda4;
					auto lambda5 = [](long long int x) { return int(x); };
					stdfn2 = lambda5;
					auto lambda6 = [](std::string x) { return int(7); };
					//stdfn2 = lambda6;
					mse::lh::TNativeFunctionPointerReplacement<int(unsigned short int)> nfr3;
					nfr3 = lambda4;
					nfr3 = lambda5;
					//nfr3 = lambda6;
					bool b1 = (NULL == nfr3);
					bool b2 = (nfr3 == nullptr);
					std::vector<int> int_vec1 = { 1, 2, 3 };
					mse::TAnyRandomAccessIterator<int> int_ariter1 = int_vec1.begin();
					auto maybe_stdvec_iter2 = mse::us::impl::maybe_any_cast<typename std::vector<int>::iterator>(int_ariter1);
					int i1 = 0;
					if (maybe_stdvec_iter2.has_value()) {
						auto stdvec_iter2 = maybe_stdvec_iter2.value();
						i1 = *stdvec_iter2;
					}
					//auto stdvec_iter3 = mse::us::impl::any_cast<typename std::vector<int>::iterator>(int_ariter1);
					mse::mstd::any anything1 = i1;
					auto i2 = mse::mstd::any_cast<int>(anything1);

					mse::any anything2 = i1;
					auto i3 = mse::any_cast<int>(anything2);

					mse::us::impl::ns_any::any anything3 = i1;
					auto i4 = mse::us::impl::ns_any::any_cast<int>(anything3);

					auto anyraiter1 = mse::TAnyRandomAccessIterator<int>(int_vec1.begin());
					auto maybe_int_vec_iter1 = mse::maybe_any_cast<decltype(int_vec1.begin())>(anyraiter1);
					if (maybe_int_vec_iter1.has_value()) {
						auto vec_iter = maybe_int_vec_iter1.value();
						i1 = *(vec_iter);
					}

					auto nanyraiter1 = mse::TNullableAnyRandomAccessIterator<int>(int_vec1.begin());
					auto anyraiter2 = mse::not_null_from_nullable(nanyraiter1);
					int i5 = *anyraiter2;

					auto lhnanyraiter1 = mse::lh::TLHNullableAnyRandomAccessIterator<int>(int_vec1.begin());
					auto maybe_int_vec_iter2 = mse::lh::us::impl::maybe_any_cast<decltype(int_vec1.begin())>(lhnanyraiter1);
					if (maybe_int_vec_iter2.has_value()) {
						auto vec_iter = maybe_int_vec_iter2.value();
						i1 = *(vec_iter);
					}

					mse::lh::TStrongVectorIterator<int> sviter1;
					sviter1 = mse::lh::reallocate(sviter1, 2 * sizeof(int));
					*sviter1 = 5;
					lhnanyraiter1 = sviter1;
					lhnanyraiter1 = mse::lh::reallocate(lhnanyraiter1, 3 * sizeof(int));
					int i6 = *lhnanyraiter1;
#endif // !MSE_SAFER_SUBSTITUTES_DISABLED

					const char* cchar_iter = "abc";
					auto lhnanyraiter2 = mse::lh::TLHNullableAnyRandomAccessIterator<const char>(cchar_iter);
					lhnanyraiter2 += 2;
					mse::TRegisteredObj<const char*> cchar_iter_reg = "def";
					auto lhnanyraiter3 = mse::lh::TLHNullableAnyRandomAccessIterator<const char>(cchar_iter_reg);
					lhnanyraiter3 += 2;

					mse::lh::TLHNullableAnyPointer<const void> void_lhnanyptr1 = lhnanyraiter3;
					auto lhnanyraiter4 = (mse::lh::TLHNullableAnyRandomAccessIterator<const char>)void_lhnanyptr1;
					auto ch1 = *lhnanyraiter4;

					int q = 5;
				}
				{
					char input1[] = "one + two * (three - four)!";
					mse::lh::TNativeArrayReplacement<char, sizeof input1> input;
					mse::lh::strcpy(input, input1);
					const char* delimiters = "! +- (*)";

					/*char* */auto token = mse::lh::strtok(input, delimiters);
					while (token)
					{
						std::cout << std::quoted(mse::us::lh::make_raw_pointer_from(token)) << ' ';
						token = mse::lh::strtok(nullptr, delimiters);
					}

					std::cout << "\nContents of the input string now:\n\"";
					for (std::size_t n = 0; n < sizeof input1; ++n)
					{
						const char c = input[n];
						if (c != '\0')
							std::cout << c;
						else
							std::cout << "\\0";
					}
					std::cout << "\"\n";
				}
#ifndef MSE_SAFER_SUBSTITUTES_DISABLED
				{
					char input1[] = "one + two * (three - four)!";
					mse::lh::TNativeArrayReplacement<char, sizeof input1> input;
					mse::lh::strcpy(input, input1);
					const char* delimiters = "! +- (*)";
					mse::TRegisteredObj<mse::lh::TLHNullableAnyRandomAccessIterator<char> > iter1 = input;

					/*char* */auto token = mse::lh::strtok_r(iter1, delimiters, &iter1);
					while (token)
					{
						std::cout << std::quoted(mse::us::lh::make_raw_pointer_from(token)) << ' ';
						token = mse::lh::strtok_r(nullptr, delimiters, &iter1);
					}

					std::cout << "\nContents of the input string now:\n\"";
					for (std::size_t n = 0; n < sizeof input1; ++n)
					{
						const char c = input[n];
						if (c != '\0')
							std::cout << c;
						else
							std::cout << "\\0";
					}
					std::cout << "\"\n";
					int q = 5;
				}
				{
					char input1[] = "one + two * (three - four)!";
					mse::lh::TNativeArrayReplacement<char, sizeof input1> arr1;
					mse::lh::strcpy(arr1, input1);
					mse::lh::TNativeArrayReplacement<char, sizeof input1> arr2;
					mse::lh::strcpy(arr2, input1);
					auto iter1 = mse::lh::TLHNullableAnyRandomAccessIterator<char>(arr1);
					auto iter2 = mse::lh::TLHNullableAnyRandomAccessIterator<char>(arr2);

					*(iter2 + 1) = '1';
					mse::lh::memcpy(iter1, iter2, sizeof input1);
					std::cout << mse::us::lh::make_raw_pointer_from(iter1) << " \n";

					*(iter2 + 1) = '2';
					/* commented out to speed up compile */
					//mse::lh::memcpy(iter1, mse::lh::void_star_replacement(iter2), sizeof input1);
					std::cout << mse::us::lh::make_raw_pointer_from(iter1) << " \n";

					*(iter2 + 1) = '3';
					mse::lh::memcpy(input1, iter2, sizeof input1);
					std::cout << input1 << " \n";

					int q = 5;
				}
				{
					char input1[] = "one + two * (three - four)!";
					mse::lh::TNativeArrayReplacement<char, sizeof input1> arr1;
					mse::lh::strcpy(arr1, input1);

					/* Just exercising lh::TNativeArrayReplacement<>'s overloaded operator&() as an aside. */
					auto iter6 = &arr1;
					mse::lh::strcpy(iter6, input1);
					mse::lh::strcpy(&arr1, input1);

					mse::lh::TNativeArrayReplacement<char, sizeof input1> arr2;
					mse::lh::strcpy(arr2, input1);
					auto iter1 = mse::lh::TLHNullableAnyRandomAccessIterator<char>(arr1);
					auto iter2 = mse::lh::TLHNullableAnyRandomAccessIterator<char>(arr2);

					auto diff1 = mse::lh::memcmp(iter1, iter2, sizeof input1);

					*(iter2 + 1) = '2';
					/* commented out to speed up compile */
					//auto diff2 = mse::lh::memcmp(iter1, mse::lh::void_star_replacement(iter2), sizeof input1);
					//auto diff3 = mse::lh::memcmp(mse::lh::void_star_replacement(iter2), iter1, sizeof input1);
					auto diff4 = mse::lh::memcmp(iter2, iter1, sizeof input1);

#ifdef MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT
					auto diff5 = mse::lh::memcmp(mse::lh::void_star_replacement(iter1), mse::lh::void_star_replacement(iter2), sizeof input1);
#endif // MSE_IMPL_ATTEMPT_TO_GUESS_VOID_STAR_TYPE_WITHOUT_CONTEXT

					auto vsr1 = mse::lh::void_star_replacement(arr1);

					void* iter3 = nullptr;
					vsr1 = iter3;
					auto iter4 = (char*)vsr1;
					bool b1 = std::is_constructible<char *, void* const&>::value;
					typedef char * iter5_t;
					iter5_t iter5 = iter5_t(iter3);
					iter3 = iter5;
					//auto lhnaraiter3 = mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(iter3);
					vsr1 = mse::lh::TLHNullableAnyPointer<void>(nullptr);
					vsr1 = mse::lh::TLHNullableAnyRandomAccessIterator<void>(nullptr);
					auto lhnaptr1 = mse::lh::TLHNullableAnyPointer<void>(vsr1);
					lhnaptr1 = nullptr;
					std::swap(lhnaptr1, lhnaptr1);
					bool b2 = (lhnaptr1 == lhnaptr1);
					int i1 = 7;
					//lhnaptr1 = mse::us::lh::unsafe_make_lh_nullable_any_pointer_from((void*)&i1);

					struct CIncompleteType;
					CIncompleteType* a_ptr1 = nullptr;
					mse::lh::TLHNullableAnyPointer<CIncompleteType> a_lhnaptr3 = nullptr;
					vsr1 = a_lhnaptr3;
					a_lhnaptr3 = mse::lh::TLHNullableAnyPointer<CIncompleteType>(vsr1);

					int q = 5;
				}
				{
					mse::lh::TNativeArrayReplacement<char, 3> buf1;
					mse::TRegisteredObj<mse::lh::TLHNullableAnyRandomAccessIterator<char> > buf_iter1 = buf1;
					mse::TRegisteredObj<size_t> buflen = 3;
					FILE* fhandle = nullptr;
					if (fhandle) {
						auto res = mse::lh::getline(&buf_iter1, &buflen, fhandle);
					}
				}
				{
					const char* s1 = "String";
					mse::lh::TLHNullableAnyRandomAccessIterator<char> s2 = mse::lh::strndup(s1, 2);
					//printf("strndup(\"String\", 2) == %s\n", s2);
					assert(0 == mse::lh::strcmp("St", s2));
					mse::lh::free(s2);
				}
				{
					const char* s1 = "String";
					mse::lh::TLHNullableAnyRandomAccessIterator<char> s2 = mse::lh::strdup(s1);
					assert(mse::lh::strcmp(s1, s2) == 0);
					mse::lh::free(s2);
				}
				{
					const char str[] = "ABCDEFG";
					mse::lh::TNativeArrayReplacement<char, sizeof str> str2;
					mse::lh::strcpy(str2, str);
					const int chars[] = { 'D', 'd' };
					mse::lh::TNativeArrayReplacement<int, sizeof chars> chars2;
					mse::lh::memcpy(chars2, chars, sizeof chars);
					auto iter1 = mse::lh::TLHNullableAnyRandomAccessIterator<char>(str2);
					auto iter2 = mse::lh::TLHNullableAnyRandomAccessIterator<const int>(chars2);

					auto sz1 = sizeof chars;
					auto sz2 = sizeof chars2;

					for (size_t i = 0; i < sizeof chars / (sizeof chars[0]); ++i)
					{
						const int c = chars2[i];
						mse::lh::TLHNullableAnyRandomAccessIterator<const char> ps = mse::lh::memchr(str2, c, mse::lh::strlen(str2));
						ps ? printf("character '%c'(%i) found: %s\n", c, c, mse::us::lh::make_raw_pointer_from(ps))
							: printf("character '%c'(%i) not found\n", c, c);
					}
				}
				{
					const char* str = "Try not. Do, or do not. There is no try.";
					mse::lh::TLHNullableAnyRandomAccessIterator<const char> str2 = mse::lh::strdup(str);
					char target = 'T';
					auto result2 = mse::lh::TLHNullableAnyRandomAccessIterator<const char>(str2);

					while ((result2 = mse::lh::strchr(result2, target)) != NULL)
					{
						printf("Found '%c' starting at '%s'\n", target, mse::us::lh::make_raw_pointer_from(result2));
						++result2; // Increment result, otherwise we'll find target at the same location
					}
				}
				{
					struct CB {
						static size_t strlen1(mse::lh::TLHNullableAnyRandomAccessIterator<const char> str) /*noexcept*/ {
							return std::strlen(mse::us::lh::make_raw_pointer_from(str));
						}
						static size_t strlen2(mse::lh::TLHNullableAnyRandomAccessIterator<const char> str) /*noexcept*/ {
							return std::strlen(mse::us::lh::make_raw_pointer_from(str));
						}
						static void foo1() {};
					};

					auto* fnptr1 = &(std::strlen);

					auto raw_fn2 = MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER((&(CB::strlen1)), fnptr1);

					fnptr1 = &(std::strlen);
					fnptr1 = raw_fn2;
					auto slen1 = raw_fn2("abc");
					auto slen2 = fnptr1("abcd");

					MSE_TRY {
						auto raw_fn3 = MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER((&(CB::strlen2)), fnptr1);
						auto slen3 = raw_fn3("abc");
						auto slen4 = raw_fn2("abc");
					}
					MSE_CATCH_ANY {
						int q = 5;
					}

					auto raw_fn4 = MSE_LH_UNSAFE_MAKE_RAW_FN_WRAPPER((&(CB::strlen2)), fnptr1);
					auto slen5 = raw_fn4("abc");
					auto slen6 = raw_fn2("abc");
				}
				{
					mse::lh::TLHNullableAnyRandomAccessIterator<char> lhnara_iter1;
					mse::TRegisteredObj<char> ch1 = 'a';
					mse::lh::TLHNullableAnyPointer<char> lhna_ptr1 = &ch1;
					lhna_ptr1 = lhnara_iter1;
					lhnara_iter1 = &ch1; /* (Non-iterator) pointers are interpreted as arrays of length 1.  */
					lhnara_iter1 += 1; /* So the iterator is now pointing out-of-bounds and would throw if a dereference were attempted. */
					lhnara_iter1 = lhna_ptr1;
					mse::TRefCountingPointer<char> refcptr1 = mse::make_refcounting<char>('a');
					auto chptr1 = mse::us::lh::unsafe_cast<char*>(refcptr1);
					mse::lh::TLHNullableAnyRandomAccessIterator<const char> lhnara_citer2;
					lhnara_citer2 = lhnara_iter1;

					mse::lh::TStrongVectorIterator<char> sviter1 = mse::lh::allocate_dyn_array1<mse::lh::TStrongVectorIterator<char> >(3);
					lhnara_iter1 = sviter1 + 1;
					auto diff1 = sviter1 - lhnara_iter1;
					auto diff2 = lhnara_iter1 - sviter1;
					bool b1 = (sviter1 == lhnara_iter1);
					bool b2 = (lhnara_iter1 == sviter1);

					mse::lh::void_star_replacement vsr1 = lhnara_iter1;
					//auto z2 = (mse::lh::TLHNullableAnyPointer<int>)lhnara_iter1;
					//mse::lh::TLHNullableAnyPointer<char> z3 = vsr1;
					mse::lh::TLHNullableAnyPointer<char> z6 = (mse::lh::TLHNullableAnyPointer<char>)vsr1;
					auto z4 = (mse::lh::void_star_replacement)lhnara_iter1;
					//auto z5 = (mse::lh::TLHNullableAnyRandomAccessIterator<int> const&)lhnara_iter1;
					//mse::lh::TLHNullableAnyPointer<char> z7 = (mse::lh::TLHNullableAnyPointer<char>)((mse::lh::void_star_replacement)lhnara_iter1);
					//auto z8 = (int*)lhnara_iter1;
					//auto z9 = (int*)vsr1;
					auto z10 = mse::us::lh::unsafe_cast<mse::lh::TLHNullableAnyPointer<int> >(lhna_ptr1);
					auto z11 = mse::us::lh::unsafe_cast<mse::lh::TLHNullableAnyPointer<int>>(lhnara_iter1);

#if (defined(MSE_HAS_CXX17) || (!defined(MSC_VER)))
					bool b3 = (vsr1 == (void*)0);
					bool b4 = (((void*)0) != vsr1);
#endif // (defined(MSE_HAS_CXX17) || (!defined(MSC_VER)))

					int i1 = 7;
					auto lhnara_iter3 = mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(&i1);
					auto lhnara_iter4 = mse::us::lh::unsafe_make_lh_nullable_any_random_access_iterator_from(&((const int&)i1));
					auto diff3 = lhnara_iter3 - lhnara_iter4;

#ifndef MSE_LEGACYHELPERS_DISABLED
					auto maybe_sviter2 = mse::lh::maybe_any_cast<mse::lh::TStrongVectorIterator<char> >(lhnara_iter1);
					if (maybe_sviter2.has_value()) {
						auto& sviter2 = maybe_sviter2.value();
						auto xs_strong_pointer_store1 = mse::make_xscope_strong_pointer_store(sviter2.target_container_ptr());
						auto char_vec_xsptr = xs_strong_pointer_store1.xscope_ptr();
						auto xs_bf_char_nii_vec1 = mse::make_xscope_borrowing_fixed_nii_vector(char_vec_xsptr);
						auto sz1 = xs_bf_char_nii_vec1.size();
						auto const& xs_bf_char_nii_vec1_cref = xs_bf_char_nii_vec1;
						auto& item_ref1 = xs_bf_char_nii_vec1_cref.at(0);
						int q = 5;
					}
#endif // !MSE_LEGACYHELPERS_DISABLED

					lhnara_iter1 = lhnara_iter1 + 1;
					lhnara_iter1 = 1 + lhnara_iter1;

					int q = 5;
				}
				{
					auto testfn1 = [](char * * chpp) {
						**chpp = 'a';
						*((*chpp) + 1) = '\0';
						(*chpp) += 2;
						};
					auto sviter1 = mse::make_registered(mse::lh::allocate_dyn_array1<mse::lh::TStrongVectorIterator<char> >(5));
					*sviter1 = 'b';
					*(sviter1 + 1) = 'c';
					*(sviter1 + 2) = '\0';
					mse::TRegisteredObj<mse::lh::TStrongVectorIterator<char> > temp = sviter1;
					testfn1(mse::us::lh::make_temporary_raw_pointer_to_pointer_proxy_from(&temp));
					std::cout << mse::us::lh::make_raw_pointer_from(sviter1) << " \n";

					testfn1(mse::us::lh::make_raw_pointer_from(mse::us::lh::make_temporary_raw_pointer_to_pointer_proxy_from(&temp)));
					int q = 5;
				}
				{
					auto testfn1 = [](char** chpp) {
						static char s_ch1 = 'b';
						(*chpp) = &s_ch1;
						};
					mse::TRegisteredObj<mse::lh::TLHNullableAnyPointer<char> > naptr1;
					testfn1(mse::us::lh::make_temporary_raw_pointer_to_pointer_proxy_from(&naptr1));
					std::cout << *naptr1 << " \n";
					int q = 5;
				}
#endif // !MSE_SAFER_SUBSTITUTES_DISABLED

#endif // MSE_SELF_TESTS
			}
		};
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
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSELEGACYHELPERS_H_
