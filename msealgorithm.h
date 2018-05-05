
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
#include "mseoptional.h"

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	/* The declaration of the "make_xscope_iterator()" template functions were moved to here from "msemsearray.h" because
	they are used by some of the supplementary algorithms. */

	template<class _TArray>
	auto make_xscope_const_iterator(const mse::TXScopeFixedConstPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_const_iterator(owner_ptr);
	}
	template<class _TArray>
	auto make_xscope_const_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_const_iterator(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TArray>
	auto make_xscope_const_iterator(const mse::TXScopeItemFixedConstPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_const_iterator(owner_ptr);
	}
	template<class _TArray>
	auto make_xscope_const_iterator(const mse::TXScopeItemFixedPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_const_iterator(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _TArray>
	auto make_xscope_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_iterator(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TArray>
	auto make_xscope_iterator(const mse::TXScopeItemFixedPointer<_TArray>& owner_ptr) {
		return typename _TArray::xscope_iterator(owner_ptr);
	}
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)

	/* Overloads for us::TReturnableFParam<>. */
	template <typename _Ty>
	auto make_xscope_const_iterator(const us::TReturnableFParam<_Ty>& param) {
		const _Ty& param_base_ref = param;
		typedef decltype(make_xscope_const_iterator(param_base_ref)) base_return_type;
		return us::TReturnableFParam<base_return_type>(make_xscope_const_iterator(param_base_ref));
	}
	template <typename _Ty>
	auto make_xscope_iterator(const us::TReturnableFParam<_Ty>& param) {
		const _Ty& param_base_ref = param;
		typedef decltype(make_xscope_iterator(param_base_ref)) base_return_type;
		return us::TReturnableFParam<base_return_type>(make_xscope_iterator(param_base_ref));
	}

	template<class _TArrayPointer, class size_type = typename std::remove_reference<decltype(*(std::declval<_TArrayPointer>()))>::type::size_type>
	auto make_xscope_const_iterator(const _TArrayPointer& owner_ptr, size_type index) {
		return make_xscope_const_iterator(owner_ptr) + index;
	}
	template<class _TArrayPointer, class size_type = typename std::remove_reference<decltype(*(std::declval<_TArrayPointer>()))>::type::size_type>
	auto make_xscope_iterator(const _TArrayPointer& owner_ptr, size_type index) {
		return make_xscope_iterator(owner_ptr) + index;
	}

	template<class _TArrayPointer>
	auto make_xscope_begin_const_iterator(const _TArrayPointer& owner_ptr) {
		return make_xscope_const_iterator(owner_ptr);
	}
	template<class _TArrayPointer>
	auto make_xscope_begin_iterator(const _TArrayPointer& owner_ptr) {
		return make_xscope_iterator(owner_ptr);
	}

	template<class _TArrayPointer>
	auto make_xscope_end_const_iterator(const _TArrayPointer& owner_ptr) {
		return make_xscope_begin_const_iterator(owner_ptr) + (*owner_ptr).size();
	}
	template<class _TArrayPointer>
	auto make_xscope_end_iterator(const _TArrayPointer& owner_ptr) {
		return make_xscope_begin_iterator(owner_ptr) + (*owner_ptr).size();
	}

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
	}

	/* find_if() */

	namespace impl {
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


	/* equal() */

	namespace impl {
		/* This is just because msvc complains about std::equal() being unsafe. */
		template<class _InIt1, class _InIt2>
		bool m_equal(_InIt1 _First1, const _InIt1& _Last1, _InIt2 _First2) {
#ifdef _MSC_VER
			while (_First1 != _Last1) {
				if (!(*_First1 == *_First2)) {
					return false;
				}
				++_First1; ++_First2;
			}
			return true;
#else // _MSC_VER
			return std::equal(_First1, _Last1, _First2);
#endif // _MSC_VER
		}

		template<class _InIt1, class _InIt2>
		class c_equal {
		public:
			typedef decltype(m_equal(std::declval<_InIt1>(), std::declval<_InIt1>(), std::declval<_InIt2>())) result_type;
			result_type result;
			c_equal(const _InIt1& _First1, const _InIt1& _Last1, _InIt2 _First2) : result(m_equal(_First1, _Last1, _First2)) {}
		};

		template<class _Container, class _InIt2>
		class xscope_c_ra_const_equal {
		public:
			typedef bool result_type;
			result_type result;
			xscope_c_ra_const_equal(const TXScopeItemFixedConstPointer<_Container>& _XscpPtr, _InIt2 _First2)
				: result(eval(_XscpPtr, _First2)) {}
		private:
			result_type eval(const TXScopeItemFixedConstPointer<_Container>& _XscpPtr, _InIt2 _First2) {
				auto xscope_begin_cit = mse::make_xscope_begin_const_iterator(_XscpPtr);
				auto xscope_end_cit = mse::make_xscope_end_const_iterator(_XscpPtr);
				return m_equal(xscope_begin_cit, xscope_end_cit, _First2);
			}
		};

		template<class _Container, class _InIt2>
		class xscope_c_ra_const_equal_adapter {
		public:
			typedef bool result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container> container_pointer_t;
			xscope_c_ra_const_equal_adapter(const container_pointer_t& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_ra_const_equal<_Container, _InIt2>(_XscpPtr, _First2).result) {}
			explicit xscope_c_ra_const_equal_adapter(const TXScopeItemFixedPointer<_Container>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_ra_const_equal<_Container, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
#ifndef MSE_SCOPEPOINTER_DISABLED
			explicit xscope_c_ra_const_equal_adapter(const TXScopeFixedConstPointer<_Container>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_ra_const_equal<_Container, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
			explicit xscope_c_ra_const_equal_adapter(const TXScopeFixedPointer<_Container>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_ra_const_equal<_Container, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
#endif //!MSE_SCOPEPOINTER_DISABLED
		};
	}
	template<class _InIt1, class _InIt2>
	inline _InIt1 equal(const _InIt1& _First1, const _InIt1& _Last1, _InIt2 _First2) {
		return impl::c_equal<_InIt1, _InIt2>(_First1, _Last1, _First2).result;
	}

	template<class _XScopeContainerPointer, class _InIt2>
	inline auto xscope_ra_const_equal(const _XScopeContainerPointer& _XscpPtr, _InIt2 _First2) {
		typedef typename std::remove_reference<decltype(*std::declval<_XScopeContainerPointer>())>::type _Container;
		return impl::xscope_c_ra_const_equal_adapter<_Container, _InIt2>(_XscpPtr, _First2).result;
	}
}

#endif /*ndef MSEALGORITHM_H*/
