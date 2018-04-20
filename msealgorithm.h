
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEALGORITHM_H
#define MSEALGORITHM_H

#include <algorithm>

namespace mse {

	namespace impl {
		namespace us {
			template<class _InIt>
			auto iterator_pair_to_raw_pointers_checked(const _InIt& first, const _InIt& last) {
				typedef decltype(std::addressof(*(std::declval<_InIt>()))) raw_pointer_type;
				typedef std::pair<raw_pointer_type, raw_pointer_type> return_type;
				auto distance = last - first; /* Safe iterators will ensure that both iterators point into the same container. */
				if (1 <= distance) {
					/* Safe iterators will ensure that dereferences are valid. */
					return return_type(std::addressof(*first), std::addressof(*(last - 1)) + 1);
				}
				else {
					return return_type(nullptr, nullptr);
				}
			}
		}

		template<class _InIt, class _Pr>
		class c_find_if {
		public:
			typedef decltype(std::find_if(std::declval<_InIt>(), std::declval<_InIt>(), std::declval<_Pr>())) result_type;
			result_type result;
			c_find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred) : result(std::find_if(_First, _Last, _Pred)) {}
		};
	}
	template<class _InIt, class _Pr>
	inline _InIt find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
		return impl::c_find_if<_InIt, _Pr>(_First, _Last, _Pred).result;
	}

}

#endif /*ndef MSEALGORITHM_H*/
