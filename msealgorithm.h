
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

	/* Forward declarations of the "make_xscope_iterator()" template functions. */

	template<class _TArray> auto make_xscope_const_iterator(const mse::TXScopeFixedConstPointer<_TArray>& owner_ptr);
	template<class _TArray> auto make_xscope_const_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr);
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TArray> auto make_xscope_const_iterator(const mse::TXScopeItemFixedConstPointer<_TArray>& owner_ptr);
	template<class _TArray> auto make_xscope_const_iterator(const mse::TXScopeItemFixedPointer<_TArray>& owner_ptr);
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _TArray> auto make_xscope_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr);
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TArray> auto make_xscope_iterator(const mse::TXScopeItemFixedPointer<_TArray>& owner_ptr);
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)

	/* Overloads for rsv::TReturnableFParam<>. */
	//template <typename _Ty> auto make_xscope_const_iterator(const rsv::TReturnableFParam<_Ty>& param);
	// template <typename _Ty> auto make_xscope_iterator(const rsv::TReturnableFParam<_Ty>& param);

	template<class _TArrayPointer, class size_type> auto make_xscope_const_iterator(const _TArrayPointer& owner_ptr, size_type index);
	template<class _TArrayPointer, class size_type> auto make_xscope_iterator(const _TArrayPointer& owner_ptr, size_type index);

	template<class _TArrayPointer> auto make_xscope_begin_const_iterator(const _TArrayPointer& owner_ptr);
	template<class _TArrayPointer> auto make_xscope_begin_iterator(const _TArrayPointer& owner_ptr);

	template<class _TArrayPointer> auto make_xscope_end_const_iterator(const _TArrayPointer& owner_ptr);
	template<class _TArrayPointer> auto make_xscope_end_iterator(const _TArrayPointer& owner_ptr);


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

		template<class _InIt>
		class TXScopeSpecializedFirstAndLast {
		public:
			TXScopeSpecializedFirstAndLast(const _InIt& _First, const _InIt& _Last) : m_first(_First), m_last(_Last) {}
			const auto& first() const {
				return m_first;
			}
			const auto& last() const {
				return m_last;
			}

		private:
			const _InIt& m_first;
			const _InIt& m_last;
		};

		template<class _InIt>
		using item_pointer_type_from_iterator = typename std::remove_const<typename std::remove_reference<
				decltype(impl::TXScopeSpecializedFirstAndLast<_InIt>(std::declval<_InIt>(), std::declval<_InIt>()).first())
			>::type>::type;

		template<class _ContainerPointer>
		class TXScopeRangeIterProvider {
		public:
			typedef decltype(mse::make_xscope_begin_iterator(std::declval<_ContainerPointer>())) iter_t;
			TXScopeRangeIterProvider(const _ContainerPointer& _XscpPtr) : m_begin(mse::make_xscope_begin_iterator(_XscpPtr))
				, m_end(mse::make_xscope_end_iterator(_XscpPtr)) {}
			const auto& begin() const {
				return m_begin;
			}
			const auto& end() const {
				return m_end;
			}

		private:
			iter_t m_begin;
			iter_t m_end;
		};

		template<class _ContainerPointer>
		using item_pointer_type_from_container_pointer = item_pointer_type_from_iterator<
			typename std::remove_const<typename std::remove_reference<
				decltype(TXScopeRangeIterProvider<_ContainerPointer>(std::declval<_ContainerPointer>()).begin())
			>::type>::type>;
	}

	/* find_if() */

	namespace impl {
		inline auto find_if_ptr_placeholder_predicate() {
			auto retval = [](auto) { return true; };
			return retval;
		};
		template<class _InIt, class _Pr = decltype(find_if_ptr_placeholder_predicate())>
		class c_find_if_ptr {
		public:
			typedef item_pointer_type_from_iterator<_InIt> item_pointer_type;
			typedef typename std::remove_reference<_InIt>::type result_type;
			result_type result;
			c_find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred) : result(eval(_First, _Last, _Pred)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
				const auto xs_iters = TXScopeSpecializedFirstAndLast<_InIt>(_First, _Last);
				auto current = xs_iters.first();
				for (; current != xs_iters.last(); ++current) {
					if (_Pred(current)) {
						break;
					}
				}
				return _First + (current - xs_iters.first());
			}
		};

		template<class _ContainerPointer, class _Pr = decltype(find_if_ptr_placeholder_predicate())>
		class xscope_c_range_get_ref_if_ptr {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef mse::xscope_optional<decltype(mse::us::unsafe_make_xscope_pointer_to(*std::declval<item_pointer_type>()))> result_type;
			result_type result;
			xscope_c_range_get_ref_if_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			static result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr>(xs_iters.begin(), xs_iters.end(), _Pred).result;
				if (xs_iters.end() == res_it) {
					return result_type{};
				}
				else {
					return result_type(mse::us::unsafe_make_xscope_pointer_to(*res_it));
				}
			}
		};

		template<class _ContainerPointer, class _Pr = decltype(find_if_ptr_placeholder_predicate())>
		class xscope_c_range_get_ref_to_element_known_to_be_present_ptr {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef decltype(mse::us::unsafe_make_xscope_pointer_to(*std::declval<item_pointer_type>())) result_type;
			result_type result;
			xscope_c_range_get_ref_to_element_known_to_be_present_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr>(xs_iters.begin(), xs_iters.end(), _Pred).result;
				if (xs_iters.end() == res_it) {
					MSE_THROW(std::logic_error("element not found - xscope_c_range_get_ref_to_element_known_to_be_present"));
				}
				else {
					return mse::us::unsafe_make_xscope_pointer_to(*res_it);
				}
			}
		};
	}
	template<class _InIt> using find_if_ptr_type = typename impl::c_find_if_ptr<_InIt>::item_pointer_type;
	template<class _InIt, class _Pr>
	inline _InIt find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
		return impl::c_find_if_ptr<_InIt, _Pr>(_First, _Last, _Pred).result;
	}

	template<class _InIt, class _Pr>
	inline _InIt find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
		auto pred2 = [&_Pred](auto ptr) { return _Pred(*ptr); };
		return find_if_ptr(_First, _Last, pred2);
	}

	/* This function returns a (scope) optional that contains a scope pointer to the found element. */
	template<class _XScopeContainerPointer> using xscope_range_get_ref_if_ptr_type = typename impl::xscope_c_range_get_ref_if_ptr<_XScopeContainerPointer>::item_pointer_type;
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_range_get_ref_if_ptr(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		return impl::xscope_c_range_get_ref_if_ptr<_XScopeContainerPointer, _Pr>(_XscpPtr, _Pred).result;
	}

	/* This function returns a (scope) optional that contains a scope pointer to the found element. */
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_range_get_ref_if(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		auto pred2 = [&_Pred](auto ptr) { return _Pred(*ptr); };
		return xscope_range_get_ref_if_ptr(_XscpPtr, pred2);
	}

	/* This function returns a scope pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
	template<class _XScopeContainerPointer> using xscope_range_get_ref_to_element_known_to_be_present_ptr_type = typename impl::xscope_c_range_get_ref_to_element_known_to_be_present_ptr<_XScopeContainerPointer>::item_pointer_type;
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_range_get_ref_to_element_known_to_be_present_ptr(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		return impl::xscope_c_range_get_ref_to_element_known_to_be_present_ptr<_XScopeContainerPointer, _Pr>(_XscpPtr, _Pred).result;
	}

	/* This function returns a scope pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_range_get_ref_to_element_known_to_be_present(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		auto pred2 = [&_Pred](auto ptr) { return _Pred(*ptr); };
		return xscope_range_get_ref_to_element_known_to_be_present_ptr(_XscpPtr, pred2);
	}

	/* deprecated alias */
	template<class _XScopeContainerPointer, class _Pr>
	inline auto xscope_ra_const_find_if(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred) {
		return xscope_range_get_ref_if(_XscpPtr, _Pred);
	}

	/* for_each() */

	namespace impl {
		inline auto for_each_ptr_placeholder_function() {
			auto retval = [](auto) {};
			return retval;
		};
		template<class _InIt, class _Fn = decltype(for_each_ptr_placeholder_function())>
		class c_for_each_ptr {
		public:
			typedef item_pointer_type_from_iterator<_InIt> item_pointer_type;
			typedef _Fn result_type;
			result_type result;
			c_for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func) : result(eval(_First, _Last, _Func)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
				const auto xs_iters = TXScopeSpecializedFirstAndLast<_InIt>(_First, _Last);
				auto current = xs_iters.first();
				for (; current != xs_iters.last(); ++current) {
					_Func(current);
				}
				return (_Func);
			}
		};

		template<class _ContainerPointer, class _Fn = decltype(for_each_ptr_placeholder_function())>
		class xscope_c_range_for_each_ptr {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef _Fn result_type;
			result_type result;
			xscope_c_range_for_each_ptr(const _ContainerPointer& _XscpPtr, _Fn _Func)
				: result(eval(_XscpPtr, _Func)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _Fn _Func) {
				const auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				return c_for_each_ptr<decltype(xs_iters.begin()), _Fn>(xs_iters.begin(), xs_iters.end(), _Func).result;
			}
		};
	}
	template<class _InIt> using for_each_ptr_type = typename impl::c_for_each_ptr<_InIt>::item_pointer_type;
	template<class _InIt, class _Fn>
	inline auto for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
		return impl::c_for_each_ptr<_InIt, _Fn>(_First, _Last, _Func).result;
	}

	template<class _InIt, class _Fn>
	inline auto for_each(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		for_each_ptr(_First, _Last, func2);
		return (_Func);
	}

	template<class _XScopeContainerPointer> using xscope_range_for_each_ptr_type = typename impl::xscope_c_range_for_each_ptr<_XScopeContainerPointer>::item_pointer_type;
	template<class _XScopeContainerPointer, class _Fn>
	inline auto xscope_range_for_each_ptr(const _XScopeContainerPointer& _XscpPtr, _Fn _Func) {
		return impl::xscope_c_range_for_each_ptr<_XScopeContainerPointer, _Fn>(_XscpPtr, _Func).result;
	}

	template<class _XScopeContainerPointer, class _Fn>
	inline auto xscope_range_for_each(const _XScopeContainerPointer& _XscpPtr, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		xscope_range_for_each_ptr(_XscpPtr, func2);
		return (_Func);
	}

	/* sort() */

	namespace impl {
		template<class _RanIt>
		class c_sort {
		public:
			c_sort(const _RanIt& _First, const _RanIt& _Last) { eval(_First, _Last); }
		private:
			static void eval(const _RanIt& _First, const _RanIt& _Last) {
				const auto xs_iters = TXScopeSpecializedFirstAndLast<_RanIt>(_First, _Last);
				std::sort(xs_iters.first(), xs_iters.first());
			}
		};

		template<class _ContainerPointer>
		class xscope_c_range_sort {
		public:
			xscope_c_range_sort(const _ContainerPointer& _XscpPtr) { eval(_XscpPtr); }
		private:
			static void eval(const _ContainerPointer& _XscpPtr) {
				const auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				std::sort(xs_iters.begin(), xs_iters.end());
			}
		};
	}
	template<class _RanIt>
	inline void sort(const _RanIt& _First, const _RanIt& _Last) {
		impl::c_sort<_RanIt>(_First, _Last);
	}

	template<class _XScopeContainerPointer>
	inline void xscope_range_sort(const _XScopeContainerPointer& _XscpPtr) {
		auto tmp_vexing_parse = impl::xscope_c_range_sort<_XScopeContainerPointer>(_XscpPtr);
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

		template<class _ContainerPointer, class _InIt2>
		class xscope_c_range_equal {
		public:
			typedef bool result_type;
			result_type result;
			xscope_c_range_equal(const _ContainerPointer& _XscpPtr, _InIt2 _First2)
				: result(eval(_XscpPtr, _First2)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _InIt2 _First2) {
				auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				return m_equal(xs_iters.begin(), xs_iters.end(), _First2);
			}
		};
	}
	template<class _InIt1, class _InIt2>
	inline _InIt1 equal(const _InIt1& _First1, const _InIt1& _Last1, _InIt2 _First2) {
		return impl::c_equal<_InIt1, _InIt2>(_First1, _Last1, _First2).result;
	}

	template<class _XScopeContainerPointer, class _InIt2>
	inline auto xscope_range_equal(const _XScopeContainerPointer& _XscpPtr, _InIt2 _First2) {
		return impl::xscope_c_range_equal<_XScopeContainerPointer, _InIt2>(_XscpPtr, _First2).result;
	}
}

#endif /*ndef MSEALGORITHM_H*/
