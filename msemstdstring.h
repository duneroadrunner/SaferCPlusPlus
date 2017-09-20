
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSTDSTRING_H
#define MSEMSTDSTRING_H

#include "msemsestring.h"

namespace mse {

	namespace mstd {

		/* These do not have safe implementations yet. Presumably, safe implementations will resemble those of the vectors. */

		template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
		class basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
		public:
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		};

		class string : public basic_string<char> {
		public:
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		};
	}
}

#endif /*ndef MSEMSTDSTRING_H*/
