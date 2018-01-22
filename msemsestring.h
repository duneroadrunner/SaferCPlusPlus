
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSESTRING_H
#define MSEMSESTRING_H

#include <string>

/* for string_view */
//include <__config>
//include <__string>
#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>
//include <__debug>


namespace mse {

	/* This macro roughly simulates constructor inheritance.*/
#define MSE_MSESTRING_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}


	/* These do not have safe implementations yet. Presumably, safe implementations will resemble those of the vectors. */

	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class nii_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		typedef std::basic_string<_Elem, _Traits, _Alloc> base_class;
		MSE_MSESTRING_USING(nii_basic_string, base_class);

		/* This placeholder implementation is actually not safe to share asynchronously (due to its unsafe iterators), but
		the eventual implementation will be.*/
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};

	class nii_string : public nii_basic_string<char> {
	public:
		typedef nii_basic_string<char> base_class;
		MSE_MSESTRING_USING(nii_string, base_class);

		/* This placeholder implementation is actually not safe to share asynchronously (due to its unsafe iterators), but
		the eventual implementation will be.*/
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};

	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class mse_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		typedef std::basic_string<_Elem, _Traits, _Alloc> base_class;
		MSE_MSESTRING_USING(mse_basic_string, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	class msestring : public mse_basic_string<char> {
	public:
		typedef mse_basic_string<char> base_class;
		MSE_MSESTRING_USING(msestring, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};


	//===------------------------ string_view ---------------------------------===//
	//
	//                     The LLVM Compiler Infrastructure
	//
	// This file is distributed under the University of Illinois Open Source
	// License.
	//
	//===----------------------------------------------------------------------===//

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif //_NOEXCEPT

#define _LIBCPP_TEMPLATE_VIS
#define _LIBCPP_INLINE_VISIBILITY
#define _LIBCPP_CONSTEXPR
#define _LIBCPP_CONSTEXPR_AFTER_CXX11
#define _LIBCPP_CONSTEXPR_AFTER_CXX14
#define _LIBCPP_CONSTEXPR_AFTER_CXX17
#define _LIBCPP_NODISCARD
#define _LIBCPP_NODISCARD_AFTER_CXX11
#define _LIBCPP_NODISCARD_AFTER_CXX14
#define _LIBCPP_NODISCARD_AFTER_CXX17
#define _LIBCPP_ASSERT(x, y) assert(x)
#define _VSTD std
#define __throw_out_of_range(x) throw(x)

	namespace us {
		namespace impl {
			typedef decltype(std::declval<int*>() - std::declval<int*>()) ptrdiff_t;

			// __str_find
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos >= __sz)
					return __npos;
				const _CharT* __r = _Traits::find(__p + __pos, __sz - __pos, __c);
				if (__r == 0)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}
			template <class _CharT, class _Traits>
			inline _LIBCPP_CONSTEXPR_AFTER_CXX11 const _CharT *
				__search_substring(const _CharT *__first1, const _CharT *__last1,
					const _CharT *__first2, const _CharT *__last2) {
				// Take advantage of knowing source and pattern lengths.
				// Stop short when source is smaller than pattern.
				const ptrdiff_t __len2 = __last2 - __first2;
				if (__len2 == 0)
					return __first1;

				ptrdiff_t __len1 = __last1 - __first1;
				if (__len1 < __len2)
					return __last1;

				// First element of __first2 is loop invariant.
				_CharT __f2 = *__first2;
				while (true) {
					__len1 = __last1 - __first1;
					// Check whether __first1 still has at least __len2 bytes.
					if (__len1 < __len2)
						return __last1;

					// Find __f2 the first byte matching in __first1.
					__first1 = _Traits::find(__first1, __len1 - __len2 + 1, __f2);
					if (__first1 == 0)
						return __last1;

					// It is faster to compare from the first byte of __first1 even if we
					// already know that it matches the first byte of __first2: this is because
					// __first2 is most likely aligned, as it is user's "pattern" string, and
					// __first1 + 1 is most likely not aligned, as the match is in the middle of
					// the string.
					if (_Traits::compare(__first1, __first2, __len2) == 0)
						return __first1;

					++__first1;
				}
			}

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos > __sz)
					return __npos;

				if (__n == 0) // There is nothing to search, just return __pos.
					return __pos;

				const _CharT *__r = __search_substring<_CharT, _Traits>(
					__p + __pos, __p + __sz, __s, __s + __n);

				if (__r == __p + __sz)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}


			// __str_rfind

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_rfind(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__sz < 1)
					return __npos;
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
				{
					if (_Traits::eq(*--__ps, __c))
						return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_rfind(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				__pos = _VSTD::min(__pos, __sz);
				if (__n < __sz - __pos)
					__pos += __n;
				else
					__pos = __sz;
				const _CharT* __r = _VSTD::__find_end(
					__p, __p + __pos, __s, __s + __n, _Traits::eq,
					std::random_access_iterator_tag(), std::random_access_iterator_tag());
				if (__n > 0 && __r == __p + __pos)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}

			template <class _ForwardIterator1, class _ForwardIterator2, class _BinaryPredicate>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _ForwardIterator1
				__find_first_of_ce(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
					_ForwardIterator2 __first2, _ForwardIterator2 __last2, _BinaryPredicate __pred)
			{
				for (; __first1 != __last1; ++__first1)
					for (_ForwardIterator2 __j = __first2; __j != __last2; ++__j)
						if (__pred(*__first1, *__j))
							return __first1;
				return __last1;
			}

			// __str_find_first_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos >= __sz || __n == 0)
					return __npos;
				const _CharT* __r = /*_VSTD::*/__find_first_of_ce
				(__p + __pos, __p + __sz, __s, __s + __n, _Traits::eq);
				if (__r == __p + __sz)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}


			// __str_find_last_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__n != 0)
				{
					if (__pos < __sz)
						++__pos;
					else
						__pos = __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __p;)
					{
						const _CharT* __r = _Traits::find(__s, __n, *--__ps);
						if (__r)
							return static_cast<_SizeT>(__ps - __p);
					}
				}
				return __npos;
			}


			// __str_find_first_not_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos < __sz)
				{
					const _CharT* __pe = __p + __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
						if (_Traits::find(__s, __n, *__ps) == 0)
							return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}


			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos < __sz)
				{
					const _CharT* __pe = __p + __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
						if (!_Traits::eq(*__ps, __c))
							return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}


			// __str_find_last_not_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
					if (_Traits::find(__s, __n, *--__ps) == 0)
						return static_cast<_SizeT>(__ps - __p);
				return __npos;
			}


			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
					if (!_Traits::eq(*--__ps, __c))
						return static_cast<_SizeT>(__ps - __p);
				return __npos;
			}

			template<class _Ptr>
			inline _LIBCPP_INLINE_VISIBILITY
				size_t __do_string_hash(_Ptr __p, _Ptr __e)
			{
				typedef typename std::iterator_traits<_Ptr>::value_type value_type;
				//return __murmur2_or_cityhash<size_t>()(__p, (__e - __p) * sizeof(value_type));

				/* Rather than include a hash implementation, we'll just use the existing one for std::string. */
				/* todo: check if C++17 is available, and if so, use std::basic_string_view's hash instead. */
				typedef decltype(sizeof(value_type)) l_size_type;
				l_size_type num_bytes = (__e - __p) * sizeof(value_type);
				std::string string1;
				string1.resize(num_bytes);
				char * bytes__p = reinterpret_cast<char *>(__p);
				for (l_size_type i = 0; i < num_bytes; i += 1) {
					string1[i] = *bytes__p;
					++bytes__p;
				}
				std::hash<std::string> str_hash;
				return str_hash(string1);
			}

			template<class _CharT, class _Traits = std::char_traits<_CharT> >
			class _LIBCPP_TEMPLATE_VIS basic_string_view {
			public:
				// types
				typedef _Traits                                    traits_type;
				typedef _CharT                                     value_type;
				typedef const _CharT*                              pointer;
				typedef const _CharT*                              const_pointer;
				typedef const _CharT&                              reference;
				typedef const _CharT&                              const_reference;
				typedef const_pointer                              const_iterator; // See [string.view.iterators]
				typedef const_iterator                             iterator;
				typedef _VSTD::reverse_iterator<const_iterator>    const_reverse_iterator;
				typedef const_reverse_iterator                     reverse_iterator;
				typedef size_t                                     size_type;
				typedef ptrdiff_t                                  difference_type;
				//static _LIBCPP_CONSTEXPR const size_type npos = -1; // size_type(-1);
				static _LIBCPP_CONSTEXPR const size_type npos = size_type(-1);

				static_assert(std::is_pod<value_type>::value, "Character type of basic_string_view must be a POD");
				static_assert((std::is_same<_CharT, typename traits_type::char_type>::value),
					"traits_type::char_type must be the same type as CharT");

				// [string.view.cons], construct/copy
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view() _NOEXCEPT : __data(nullptr), __size(0) {}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const basic_string_view&) _NOEXCEPT = default;

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					basic_string_view& operator=(const basic_string_view&) _NOEXCEPT = default;

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const _CharT* __s, size_type __len) _NOEXCEPT
					: __data(__s), __size(__len)
				{
					// #if _LIBCPP_STD_VER > 11
					//         _LIBCPP_ASSERT(__len == 0 || __s != nullptr, "string_view::string_view(_CharT *, size_t): received nullptr");
					// #endif
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const _CharT* __s)
					: __data(__s), __size(_Traits::length(__s)) {}

				// [string.view.iterators], iterators
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator begin()  const _NOEXCEPT { return cbegin(); }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator end()    const _NOEXCEPT { return cend(); }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator cbegin() const _NOEXCEPT { return __data; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator cend()   const _NOEXCEPT { return __data + __size; }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator rbegin()   const _NOEXCEPT { return const_reverse_iterator(cend()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator rend()     const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator crbegin()  const _NOEXCEPT { return const_reverse_iterator(cend()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator crend()    const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

				// [string.view.capacity], capacity
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type size()     const _NOEXCEPT { return __size; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type length()   const _NOEXCEPT { return __size; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type max_size() const _NOEXCEPT { return std::numeric_limits<size_type>::max(); }

				_LIBCPP_NODISCARD_AFTER_CXX17 _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
					bool empty()         const _NOEXCEPT { return __size == 0; }

				// [string.view.access], element access
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference operator[](size_type __pos) const _NOEXCEPT { return __data[__pos]; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference at(size_type __pos) const
				{
					return __pos >= size()
						? (__throw_out_of_range("string_view::at"), __data[0])
						: __data[__pos];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference front() const
				{
					return _LIBCPP_ASSERT(!empty(), "string_view::front(): string is empty"), __data[0];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference back() const
				{
					return _LIBCPP_ASSERT(!empty(), "string_view::back(): string is empty"), __data[__size - 1];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_pointer data() const _NOEXCEPT { return __data; }

				// [string.view.modifiers], modifiers:
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void remove_prefix(size_type __n) _NOEXCEPT
				{
					_LIBCPP_ASSERT(__n <= size(), "remove_prefix() can't remove more than size()");
					__data += __n;
					__size -= __n;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void remove_suffix(size_type __n) _NOEXCEPT
				{
					_LIBCPP_ASSERT(__n <= size(), "remove_suffix() can't remove more than size()");
					__size -= __n;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void swap(basic_string_view& __other) _NOEXCEPT
				{
					const value_type *__p = __data;
					__data = __other.__data;
					__other.__data = __p;

					size_type __sz = __size;
					__size = __other.__size;
					__other.__size = __sz;
				}

				_LIBCPP_INLINE_VISIBILITY
					size_type copy(_CharT* __s, size_type __n, size_type __pos = 0) const
				{
					if (__pos > size())
						__throw_out_of_range("string_view::copy");
					size_type __rlen = _VSTD::min(__n, size() - __pos);
					_Traits::copy(__s, data() + __pos, __rlen);
					return __rlen;
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view substr(size_type __pos = 0, size_type __n = npos) const
				{
					return __pos > size()
						? (__throw_out_of_range("string_view::substr"), basic_string_view())
						: basic_string_view(data() + __pos, _VSTD::min(__n, size() - __pos));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 int compare(basic_string_view __sv) const _NOEXCEPT
				{
					size_type __rlen = _VSTD::min(size(), __sv.size());
					int __retval = _Traits::compare(data(), __sv.data(), __rlen);
					if (__retval == 0) // first __rlen chars matched
						__retval = size() == __sv.size() ? 0 : (size() < __sv.size() ? -1 : 1);
					return __retval;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, basic_string_view __sv) const
				{
					return substr(__pos1, __n1).compare(__sv);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1,
						basic_string_view __sv, size_type __pos2, size_type __n2) const
				{
					return substr(__pos1, __n1).compare(__sv.substr(__pos2, __n2));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(const _CharT* __s) const _NOEXCEPT
				{
					return compare(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, const _CharT* __s) const
				{
					return substr(__pos1, __n1).compare(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, const _CharT* __s, size_type __n2) const
				{
					return substr(__pos1, __n1).compare(basic_string_view(__s, __n2));
				}

				// find
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// rfind
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::rfind(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::rfind(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_first_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return find(__c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_last_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return rfind(__c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_first_not_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_last_not_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

#if 1//_LIBCPP_STD_VER > 17
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(basic_string_view __s) const _NOEXCEPT
				{
					return size() >= __s.size() && compare(0, __s.size(), __s) == 0;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(value_type __c) const _NOEXCEPT
				{
					return !empty() && _Traits::eq(front(), __c);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(const value_type* __s) const _NOEXCEPT
				{
					return starts_with(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(basic_string_view __s) const _NOEXCEPT
				{
					return size() >= __s.size() && compare(size() - __s.size(), npos, __s) == 0;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(value_type __c) const _NOEXCEPT
				{
					return !empty() && _Traits::eq(back(), __c);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(const value_type* __s) const _NOEXCEPT
				{
					return ends_with(basic_string_view(__s));
				}
#endif

			private:
				const   value_type* __data;
				size_type           __size;
			};


			// [string.view.comparison]
			// operator ==
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(basic_string_view<_CharT, _Traits> __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}


			// operator !=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}


			// operator <
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}


			// operator >
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator> (basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}


			// operator <=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}


			// operator >=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}


			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}

			typedef basic_string_view<char>     string_view;
			typedef basic_string_view<char16_t> u16string_view;
			typedef basic_string_view<char32_t> u32string_view;
			typedef basic_string_view<wchar_t>  wstring_view;

#if 0//_LIBCPP_STD_VER > 11 
			inline namespace literals
			{
				inline namespace string_view_literals
				{
					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char> operator "" sv(const char *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<wchar_t> operator "" sv(const wchar_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<wchar_t>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char16_t> operator "" sv(const char16_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char16_t>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char32_t> operator "" sv(const char32_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char32_t>(__str, __len);
					}
				}
			}
#endif
		}
	}
}

namespace std {
	// [string.view.hash]
	template<class _CharT, class _Traits>
	struct _LIBCPP_TEMPLATE_VIS hash<mse::us::impl::basic_string_view<_CharT, _Traits> >
		: public unary_function<mse::us::impl::basic_string_view<_CharT, _Traits>, size_t>
	{
		size_t operator()(const mse::us::impl::basic_string_view<_CharT, _Traits> __val) const _NOEXCEPT;
	};

	template<class _CharT, class _Traits>
	size_t
		hash<mse::us::impl::basic_string_view<_CharT, _Traits> >::operator()(
			const mse::us::impl::basic_string_view<_CharT, _Traits> __val) const _NOEXCEPT
	{
		return __do_string_hash(__val.data(), __val.data() + __val.size());
	}
}

namespace mse {
	template <typename _TRandomAccessSection>
	class TStringSectionFunctionaltyWrapper : public _TRandomAccessSection {
	public:
	};
}

#endif /*ndef MSEMSESTRING_H*/
