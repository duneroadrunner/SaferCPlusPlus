
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSESTRING_H
#define MSEMSESTRING_H

#include <string>

namespace mse {

	/* These do not have safe implementations yet. Presumably, safe implementations will resemble those of the vectors. */

	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class nii_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		/* This placeholder implementation is actually not safe to share asybchronously (due to its unsafe iterators), but
		the eventual implementation will be.*/
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};

	class nii_string : public nii_basic_string<char> {
	public:
		/* This placeholder implementation is actually not safe to share asybchronously (due to its unsafe iterators), but
		the eventual implementation will be.*/
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};

	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class mse_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	class msestring : public mse_basic_string<char> {
	public:
		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};
}

#endif /*ndef MSEMSESTRING_H*/
