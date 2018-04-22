
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEALGORITHM_H
#define MSEALGORITHM_H

#include <algorithm>
#include <stdexcept>
#include "msescope.h"
#include "msemsearray.h"
#include "mseoptional.h"

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

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

		template<class _Container, class _Pr>
		class xscope_c_ra_const_find_if {
		public:
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef mse::xscope_optional<TXScopeItemFixedConstPointer<element_t> > result_type;
			result_type result;
			xscope_c_ra_const_find_if(const TXScopeItemFixedConstPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const TXScopeItemFixedConstPointer<_Container>& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				auto xscope_begin_cit = mse::make_xscope_begin_const_iterator(_XscpPtr);
				auto xscope_end_cit = mse::make_xscope_end_const_iterator(_XscpPtr);
				auto res_cit = std::find_if(xscope_begin_cit, xscope_end_cit, _Pred);
				if (xscope_end_cit == res_cit) {
					return result_type{};
				}
				else {
					return result_type(mse::us::unsafe_make_xscope_const_pointer_to(*res_cit));
				}
			}
		};

		template<class _Container, class _Pr>
		class xscope_c_ra_const_find_if_adapter {
		public:
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef mse::xscope_optional<TXScopeItemFixedConstPointer<element_t> > result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container> container_pointer_t;
			xscope_c_ra_const_find_if_adapter(const container_pointer_t& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_if<_Container, _Pr>(_XscpPtr, _Pred).result) {}
			explicit xscope_c_ra_const_find_if_adapter(const TXScopeItemFixedPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_if<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
#ifndef MSE_SCOPEPOINTER_DISABLED
			explicit xscope_c_ra_const_find_if_adapter(const TXScopeFixedConstPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_if<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
			explicit xscope_c_ra_const_find_if_adapter(const TXScopeFixedPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_if<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
#endif //!MSE_SCOPEPOINTER_DISABLED
		};

		template<class _Container, class _Pr>
		class xscope_c_ra_const_find_element_known_to_be_present {
		public:
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef TXScopeItemFixedConstPointer<element_t> result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container> container_pointer_t;
			xscope_c_ra_const_find_element_known_to_be_present(const container_pointer_t& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const container_pointer_t& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				auto xscope_begin_cit = mse::make_xscope_begin_const_iterator(_XscpPtr);
				auto xscope_end_cit = mse::make_xscope_end_const_iterator(_XscpPtr);
				auto res_it = std::find_if(xscope_begin_cit, xscope_end_cit, _Pred);
				if (xscope_end_cit == res_it) {
					MSE_THROW(std::logic_error("element not found - xscope_c_ra_const_find_element_known_to_be_present"));
				}
				else {
					return mse::us::unsafe_make_xscope_const_pointer_to(*res_it);
				}
			}
		};

		template<class _Container, class _Pr>
		class xscope_c_ra_const_find_element_known_to_be_present_adapter {
		public:
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef TXScopeItemFixedConstPointer<element_t> result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container> container_pointer_t;
			xscope_c_ra_const_find_element_known_to_be_present_adapter(const container_pointer_t& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_element_known_to_be_present<_Container, _Pr>(_XscpPtr, _Pred).result) {}
			explicit xscope_c_ra_const_find_element_known_to_be_present_adapter(const TXScopeItemFixedPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_element_known_to_be_present<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
#ifndef MSE_SCOPEPOINTER_DISABLED
			explicit xscope_c_ra_const_find_element_known_to_be_present_adapter(const TXScopeFixedConstPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_element_known_to_be_present<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
			explicit xscope_c_ra_const_find_element_known_to_be_present_adapter(const TXScopeFixedPointer<_Container>& _XscpPtr, _Pr _Pred)
				: result(xscope_c_ra_const_find_element_known_to_be_present<_Container, _Pr>(container_pointer_t(_XscpPtr), _Pred).result) {}
#endif //!MSE_SCOPEPOINTER_DISABLED
		};
	}
	template<class _InIt, class _Pr>
	inline _InIt find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
		return impl::c_find_if<_InIt, _Pr>(_First, _Last, _Pred).result;
	}

	/* This function returns a (scope) optional that contains a scope pointer to the found element. */
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_ra_const_find_if(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		typedef typename std::remove_reference<decltype(*std::declval<_XScopeContainerPointer>())>::type _Container;
		return impl::xscope_c_ra_const_find_if_adapter<_Container, _Pr>(_XscpPtr, _Pred).result;
	}

	/* This function returns a scope pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_ra_const_find_element_known_to_be_present(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		typedef typename std::remove_reference<decltype(*std::declval<_XScopeContainerPointer>())>::type _Container;
		return impl::xscope_c_ra_const_find_element_known_to_be_present_adapter<_Container, _Pr>(_XscpPtr, _Pred).result;
	}


	namespace impl {

		/* Some implementation specializations for nii_array<>.  */

		template<class _Pr, class _Ty, size_t _Size, class _TStateMutex>
		class c_find_if<Tnii_array_xscope_ss_const_iterator_type<_Ty, _Size, _TStateMutex>, _Pr> {
		public:
			typedef Tnii_array_xscope_ss_const_iterator_type<_Ty, _Size, _TStateMutex> _InIt;
			typedef decltype(std::find_if(std::declval<_InIt>(), std::declval<_InIt>(), std::declval<_Pr>())) result_type;
			result_type result;
			c_find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred)
				: result(eval(_First, _Last, _Pred)) {}
		private:
			auto eval(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
				auto raw_pair = us::iterator_pair_to_raw_pointers_checked(_First, _Last);
				/* If (_Last <= _First) the returned raw pointers will both have nullptr value. The C++ spec suggests this'll
				work just fine. Apparently. */
				auto raw_result = std::find_if(raw_pair.first, raw_pair.second, _Pred);
				return _First + (raw_result - raw_pair.first);
			}
		};

		template<class _Pr, class _Ty, size_t _Size, class _TStateMutex>
		class xscope_c_ra_const_find_if<nii_array<_Ty, _Size, _TStateMutex>, _Pr> {
		public:
			typedef nii_array<_Ty, _Size, _TStateMutex> _Container;
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef mse::xscope_optional<TXScopeItemFixedConstPointer<element_t> > result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<nii_array<_Ty, _Size, _TStateMutex> > container_pointer_t;
			xscope_c_ra_const_find_if(const container_pointer_t& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const container_pointer_t& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto& array1 = (*_XscpPtr);
				if (0 >= array1.size()) {
					return result_type{};
				}
				else {
					auto raw_begin_cit = std::addressof(array1[0]);
					auto raw_end_cit = raw_begin_cit + msear_as_a_size_t(array1.size());
					auto raw_result_cit = std::find_if(raw_begin_cit, raw_end_cit, _Pred);
					if (raw_end_cit == raw_result_cit) {
						return result_type{};
					}
					else {
						return result_type(mse::us::unsafe_make_xscope_const_pointer_to(*raw_result_cit));
					}
				}
			}
		};

		template<class _Pr, class _Ty, size_t _Size, class _TStateMutex>
		class xscope_c_ra_const_find_element_known_to_be_present<nii_array<_Ty, _Size, _TStateMutex>, _Pr> {
		public:
			typedef nii_array<_Ty, _Size, _TStateMutex> _Container;
			typedef typename std::remove_reference<decltype(std::declval<const _Container>()[0])>::type element_t;
			typedef TXScopeItemFixedConstPointer<element_t> result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container> container_pointer_t;
			xscope_c_ra_const_find_element_known_to_be_present(const container_pointer_t& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const container_pointer_t& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto& array1 = (*_XscpPtr);
				if (0 >= array1.size()) {
					MSE_THROW(std::logic_error("element not found - xscope_c_ra_const_find_element_known_to_be_present"));
				}
				else {
					auto raw_begin_cit = std::addressof(array1[0]);
					auto raw_end_cit = raw_begin_cit + msear_as_a_size_t(array1.size());
					auto raw_result_cit = std::find_if(raw_begin_cit, raw_end_cit, _Pred);
					if (raw_end_cit == raw_result_cit) {
						MSE_THROW(std::logic_error("element not found - xscope_c_ra_const_find_element_known_to_be_present"));
					}
					else {
						return mse::us::unsafe_make_xscope_const_pointer_to(*raw_result_cit);
					}
				}
			}
		};
	}
}

#endif /*ndef MSEALGORITHM_H*/
