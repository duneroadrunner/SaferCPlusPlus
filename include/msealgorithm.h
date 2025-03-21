
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
#include "msemsearray.h"

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	namespace impl {
		namespace us {
			MSE_SUPPRESS_CHECK_IN_DECLSCOPE
			template<class TElement>
			class basic_raw_ptr_iterator1_t {
			public:
				typedef mse::impl::random_access_iterator_base_from_ra_container<mse::rsv::xslta_array<TElement, 1> > historical_base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(historical_base_class);

				basic_raw_ptr_iterator1_t(const basic_raw_ptr_iterator1_t& src) = default;
				basic_raw_ptr_iterator1_t(basic_raw_ptr_iterator1_t&& src) = default;

				MSE_SUPPRESS_CHECK_IN_DECLSCOPE basic_raw_ptr_iterator1_t(std::nullptr_t) : m_raw_ptr_iter(nullptr) {}
				basic_raw_ptr_iterator1_t(TElement* raw_ptr_iter MSE_ATTR_PARAM_STR("mse::lifetime_label(99)")) : m_raw_ptr_iter(raw_ptr_iter) {}
				MSE_SUPPRESS_CHECK_IN_DECLSCOPE
				template<class TIter, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<TElement*, mse::impl::remove_reference_t<TIter> >::value) && (!std::is_same<std::nullptr_t, mse::impl::remove_reference_t<TIter> >::value)> MSE_IMPL_EIS >
				basic_raw_ptr_iterator1_t(TIter const & iter MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : m_raw_ptr_iter(std::addressof(*iter)) {}

				reference operator*() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
					return (*m_raw_ptr_iter);
				}
				pointer operator->() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") {
					return m_raw_ptr_iter;
				}

				basic_raw_ptr_iterator1_t& operator +=(difference_type x) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
					MSE_SUPPRESS_CHECK_IN_XSCOPE m_raw_ptr_iter += (x);
					return (*this);
				}
				basic_raw_ptr_iterator1_t& operator -=(difference_type x) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") { operator +=(-x); return (*this); }
				basic_raw_ptr_iterator1_t& operator ++() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") { MSE_SUPPRESS_CHECK_IN_XSCOPE ++m_raw_ptr_iter; return (*this); }
				basic_raw_ptr_iterator1_t operator ++(int) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") { auto _Tmp = *this; MSE_SUPPRESS_CHECK_IN_XSCOPE m_raw_ptr_iter++; return (_Tmp); }
				basic_raw_ptr_iterator1_t& operator --() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") { MSE_SUPPRESS_CHECK_IN_XSCOPE --m_raw_ptr_iter; return (*this); }
				basic_raw_ptr_iterator1_t operator --(int) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") { auto _Tmp = *this; MSE_SUPPRESS_CHECK_IN_XSCOPE m_raw_ptr_iter--; return (_Tmp); }
				basic_raw_ptr_iterator1_t operator+(difference_type n) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") { auto retval = (*this); retval += n; return retval; }
				basic_raw_ptr_iterator1_t operator-(difference_type n) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }") { return ((*this) + (-n)); }
				difference_type operator-(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return (m_raw_ptr_iter - _Right_cref.m_raw_ptr_iter);
				}

				bool operator==(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					return (m_raw_ptr_iter == _Right_cref.m_raw_ptr_iter);
				}
				bool operator!=(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					return !((*this) == _Right_cref);
				}
				bool operator<(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return (m_raw_ptr_iter < _Right_cref.m_raw_ptr_iter);
				}
				bool operator>(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return (m_raw_ptr_iter > _Right_cref.m_raw_ptr_iter);
				}
				bool operator<=(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return (m_raw_ptr_iter <= _Right_cref.m_raw_ptr_iter);
				}
				bool operator>=(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) const {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return (m_raw_ptr_iter >= _Right_cref.m_raw_ptr_iter);
				}

				basic_raw_ptr_iterator1_t& operator=(const basic_raw_ptr_iterator1_t& _Right_cref MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
					m_raw_ptr_iter = _Right_cref.m_raw_ptr_iter;
					return (*this);
				}

				TElement* m_raw_ptr_iter MSE_ATTR_STR("mse::lifetime_labels(99)");
			} MSE_ATTR_STR("mse::lifetime_labels(99)");

			template<class T>
			struct lta_basic_pair_of_same {
				lta_basic_pair_of_same(const lta_basic_pair_of_same& src) = default;
				lta_basic_pair_of_same(lta_basic_pair_of_same&& src) = default;
				lta_basic_pair_of_same(T const& first_param MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"), T const& second_param MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) : first(first_param), second(second_param) {}
				T first MSE_ATTR_STR("mse::lifetime_label(99)");
				T second MSE_ATTR_STR("mse::lifetime_label(99)");
			} MSE_ATTR_STR("mse::lifetime_label(99)");

			MSE_SUPPRESS_CHECK_IN_DECLSCOPE
			template<class _InIt>
			auto iterator_pair_to_raw_pointers_checked(const _InIt& first MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), const _InIt& last MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); return_value(99) }")
			{
				MSE_SUPPRESS_CHECK_IN_XSCOPE {
					typedef basic_raw_ptr_iterator1_t<mse::impl::remove_reference_t<decltype(*(std::declval<_InIt>()))> > unchecked_iterator_type;
					typedef lta_basic_pair_of_same<unchecked_iterator_type> return_type;
					auto distance = last - first; /* Safe iterators will ensure that both iterators point into the same container. */
					if (1 <= distance) {
						/* Safe iterators will ensure that dereferences are valid. */
						return return_type(unchecked_iterator_type{ std::addressof(*first) }, unchecked_iterator_type{ std::addressof(*(last - 1)) + 1 });
					}
					else {
						return return_type(nullptr, nullptr);
					}
				}
			}
		}

		template<class _InIt>
		class TXScopeSpecializedFirstAndLast : public mse::us::impl::XScopeTagBase {
		public:
			TXScopeSpecializedFirstAndLast(_InIt _First MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), _InIt _Last MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)")) : m_first(std::move(_First)), m_last(std::move(_Last)) {}
			const auto& first() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return m_first;
			}
			const auto& last() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return m_last;
			}

		private:
			_InIt m_first MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)");
			_InIt m_last MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_InIt, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class T, class EqualTo>
		struct IsSupportedByMakeXScopeSpecializedFirstAndLastOverloaded_impl
		{
			template<class U, class V>
			static auto test(U* u) -> decltype(make_xscope_specialized_first_and_last_overloaded(*u, *u), std::declval<V>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct IsSupportedByMakeXScopeSpecializedFirstAndLastOverloaded : IsSupportedByMakeXScopeSpecializedFirstAndLastOverloaded_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

		template<class _InIt>
		auto make_xscope_specialized_first_and_last_helper1(std::true_type, const _InIt& _First, const _InIt& _Last) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_specialized_first_and_last_overloaded(_First, _Last);
		}
		template<class _InIt>
		auto make_xscope_specialized_first_and_last_helper1(std::false_type, const _InIt& _First, const _InIt& _Last) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return TXScopeSpecializedFirstAndLast<_InIt>(_First, _Last);
		}

		template<class _InIt>
		using make_xscope_specialized_first_and_last_return_type = decltype(make_xscope_specialized_first_and_last_helper1(
			typename IsSupportedByMakeXScopeSpecializedFirstAndLastOverloaded<_InIt>::type(), std::declval<_InIt>(), std::declval<_InIt>()));

		template<class _InIt, typename _TReturn = make_xscope_specialized_first_and_last_return_type<_InIt> >
		auto make_xscope_specialized_first_and_last(const _InIt& _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _InIt& _Last MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_TReturn, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(alias_11$); return_value(alias_11$) }")
		{
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_specialized_first_and_last_helper1(
				typename IsSupportedByMakeXScopeSpecializedFirstAndLastOverloaded<_InIt>::type(), _First, _Last);
		}

		template<class _InIt>
		using item_pointer_type_from_iterator = typename std::add_const<mse::impl::remove_reference_t<
				decltype(make_xscope_specialized_first_and_last(std::declval<_InIt>(), std::declval<_InIt>()).first())
			> >::type;


		template<class _ContainerPointer>
		class TXScopeRangeIterProvider {
		public:
			typedef decltype(mse::make_xscope_begin_iterator(std::declval<_ContainerPointer>())) iter_t;
			typedef decltype(make_xscope_specialized_first_and_last(std::declval<iter_t>(), std::declval<iter_t>())) specialized_first_and_last_t;

			TXScopeRangeIterProvider(const _ContainerPointer& _XscpPtr)
				: m_specialized_first_and_last(mse::make_xscope_begin_iterator(_XscpPtr), mse::make_xscope_end_iterator(_XscpPtr)) {}
			const auto& begin() const {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return m_specialized_first_and_last.first();
			}
			const auto& end() const {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return m_specialized_first_and_last.last();
			}

		private:
			specialized_first_and_last_t m_specialized_first_and_last;
		};

		template<class _ContainerPointer>
		using item_pointer_type_from_container_pointer = item_pointer_type_from_iterator<
			mse::impl::remove_const_t<mse::impl::remove_reference_t<
				decltype(make_xscope_range_iter_provider(std::declval<_ContainerPointer>()).begin())
			> > >;

		template<class T, class EqualTo>
		struct IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded_impl
		{
			template<class U, class V>
			static auto test(U* u) -> decltype(make_xscope_specialized_range_iter_provider_overloaded(*u, *u), std::declval<V>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded : IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

		template<class _ContainerPointer>
		auto make_xscope_range_iter_provider_helper1(std::true_type, const _ContainerPointer& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return mse::impl::make_xscope_specialized_range_iter_provider_overloaded(*ptr);
		}
		template<class _ContainerPointer>
		auto make_xscope_range_iter_provider_helper1(std::false_type, const _ContainerPointer& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return TXScopeRangeIterProvider<decltype(ptr)>(ptr);
		}

		template<class _ContainerPointer>
		auto make_xscope_range_iter_provider_helper2(const _ContainerPointer& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return TXScopeRangeIterProvider<decltype(ptr)>(ptr);
		}
		template<class _Container>
		auto make_xscope_range_iter_provider_helper2(const mse::TXScopeFixedConstPointer<_Container>& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_range_iter_provider_helper1(typename IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded<_Container>::type(), ptr);
		}
		template<class _Container>
		auto make_xscope_range_iter_provider_helper2(const mse::TXScopeFixedPointer<_Container>& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_range_iter_provider_helper1(typename IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded<_Container>::type(), ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Container>
		auto make_xscope_range_iter_provider_helper2(const mse::TXScopeObjFixedConstPointer<_Container>& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_range_iter_provider_helper1(typename IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded<_Container>::type(), ptr);
		}
		template<class _Container>
		auto make_xscope_range_iter_provider_helper2(const mse::TXScopeObjFixedPointer<_Container>& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_range_iter_provider_helper1(typename IsSupportedByMakeXScopeSpecializedRangeIterProviderOverloaded<_Container>::type(), ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

		template<class _ContainerPointer>
		auto make_xscope_range_iter_provider(const _ContainerPointer& ptr) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE return make_xscope_range_iter_provider_helper2(ptr);
		}
	}

	/* find_if() */

	namespace impl {
		inline auto find_if_ptr_placeholder_predicate() {
			auto retval = [](auto) { return true; };
			return retval;
		};
		template<class _InIt, class _Pr, class... Args>
		class c_find_if_ptr {
		public:
			typedef item_pointer_type_from_iterator<_InIt> item_pointer_type;
			typedef mse::impl::remove_reference_t<_InIt> result_type;
			result_type result;
			c_find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) : result(eval(_First, _Last, _Pred, args...)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) {
				const auto xs_iters = make_xscope_specialized_first_and_last(_First, _Last);
				auto current = xs_iters.first();
				MSE_SUPPRESS_CHECK_IN_XSCOPE auto unsafe_increment = [](auto& x) { ++x; };
				for (; current != xs_iters.last(); unsafe_increment(current)) {
					if (_Pred(current, args...)) {
						break;
					}
				}
				MSE_SUPPRESS_CHECK_IN_XSCOPE return _First + (current - xs_iters.first());
			}
		};

		template<class _ContainerPointer, class _Pr, class... Args>
		class xscope_c_range_get_ref_if_ptr {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef mse::xscope_optional<decltype(mse::us::unsafe_make_xscope_pointer_to(*std::declval<item_pointer_type>()))> result_type;
			result_type result;
			xscope_c_range_get_ref_if_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args)
				: result(eval(_XscpPtr, _Pred, args...)) {}
		private:
			static result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
				/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto xs_iters = make_xscope_range_iter_provider(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr, Args...>(xs_iters.begin(), xs_iters.end(), _Pred, args...).result;
				if (xs_iters.end() == res_it) {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return result_type{};
				}
				else {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return result_type(mse::us::unsafe_make_xscope_pointer_to(*res_it));
				}
			}
		};

		template<class _ContainerPointer, class _Pr, class... Args>
		class xscope_c_range_get_ref_to_element_known_to_be_present_ptr {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef decltype(mse::us::unsafe_make_xscope_pointer_to(*std::declval<item_pointer_type>())) result_type;
			result_type result;
			xscope_c_range_get_ref_to_element_known_to_be_present_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args)
				: result(eval(_XscpPtr, _Pred, args...)) {}
		private:
			static result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
				/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				const auto xs_iters = make_xscope_range_iter_provider(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr, Args...>(xs_iters.begin(), xs_iters.end(), _Pred, args...).result;
				if (xs_iters.end() == res_it) {
					MSE_SUPPRESS_CHECK_IN_XSCOPE MSE_THROW(std::logic_error("element not found - xscope_c_range_get_ref_to_element_known_to_be_present"));
				}
				else {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return mse::us::unsafe_make_xscope_pointer_to(*res_it);
				}
			}
		};
	}
	template<class _InIt> using find_if_ptr_type = typename impl::c_find_if_ptr<_InIt, decltype(impl::find_if_ptr_placeholder_predicate())>::item_pointer_type;
	template<class _InIt, class _Pr, class... Args>
	inline _InIt find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE return impl::c_find_if_ptr<_InIt, _Pr, Args...>(_First, _Last, _Pred, args...).result;
	}

	template<class _InIt, class _Pr, class... Args>
	inline _InIt find_if(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE auto pred2 = mse::rsv::make_xscope_reference_or_pointer_capture_lambda([&_Pred](auto ptr) { return _Pred(*ptr); });
		auto retval = find_if_ptr(_First, _Last, pred2);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	template<class _XScopeContainerPointer> using xscope_range_get_ref_if_ptr_type = typename impl::xscope_c_range_get_ref_if_ptr<_XScopeContainerPointer, decltype(impl::find_if_ptr_placeholder_predicate())>::item_pointer_type;
	/* This function returns a (scope) optional that contains a scope pointer to the found element. */
	template<class _XScopeContainerPointer, class _Pr, class... Args>
	inline auto xscope_range_get_ref_if_ptr(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
		auto retval = impl::xscope_c_range_get_ref_if_ptr<_XScopeContainerPointer, _Pr, Args...>(_XscpPtr, _Pred, args...).result;
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	/* This function returns a (scope) optional that contains a scope pointer to the found element. */
	template<class _XScopeContainerPointer, class _Pr, class... Args>
	inline auto xscope_range_get_ref_if(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE auto pred2 = mse::rsv::make_xscope_reference_or_pointer_capture_lambda([&_Pred](auto ptr) { return _Pred(*ptr); });
		auto retval = xscope_range_get_ref_if_ptr(_XscpPtr, pred2);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	template<class _XScopeContainerPointer> using xscope_range_get_ref_to_element_known_to_be_present_ptr_type = typename impl::xscope_c_range_get_ref_to_element_known_to_be_present_ptr<_XScopeContainerPointer, decltype(impl::find_if_ptr_placeholder_predicate())>::item_pointer_type;
	/* This function returns a scope pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
	template<class _XScopeContainerPointer, class _Pr, class... Args>
	inline auto xscope_range_get_ref_to_element_known_to_be_present_ptr(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
		auto retval = impl::xscope_c_range_get_ref_to_element_known_to_be_present_ptr<_XScopeContainerPointer, _Pr, Args...>(_XscpPtr, _Pred, args...).result;
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	/* This function returns a scope pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
	template<class _XScopeContainerPointer, class _Pr, class... Args>
	inline auto xscope_range_get_ref_to_element_known_to_be_present(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE auto pred2 = mse::rsv::make_xscope_reference_or_pointer_capture_lambda([&_Pred](auto ptr) { return _Pred(*ptr); });
		auto retval = xscope_range_get_ref_to_element_known_to_be_present_ptr(_XscpPtr, pred2);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	/* deprecated alias */
	template<class _XScopeContainerPointer, class _Pr, class... Args>
	MSE_DEPRECATED inline auto xscope_ra_const_find_if(const _XScopeContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
		auto retval = xscope_range_get_ref_if(_XscpPtr, _Pred, args...);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

#if 0
	namespace rsv {
		namespace impl {
			inline auto find_if_ptr_placeholder_predicate() {
				auto retval = [](auto) { return true; };
				return retval;
			};
			template<class _InIt, class _Pr, class... Args>
			class c_find_if_ptr {
			public:
				//typedef item_pointer_type_from_iterator<_InIt> item_pointer_type;
				typedef mse::impl::remove_reference_t<_InIt> result_type;
				result_type result;
				c_find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) : result(eval(_First, _Last, _Pred, args...)) {}
			private:
				static result_type eval(const _InIt& _First, const _InIt& _Last, _Pr _Pred, const Args&... args) {
					const auto xs_iters = make_xslta_specialized_first_and_last(_First, _Last);
					auto current = xs_iters.first();
					for (; current != xs_iters.last(); ++current) {
						if (_Pred(current, args...)) {
							break;
						}
					}
					return _First + (current - xs_iters.first());
				}
			};

			template<class _ContainerPointer, class _Pr, class... Args>
			class xslta_c_range_get_ref_if_ptr {
			public:
				//typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
				typedef mse::rsv::xslta_optional<decltype(mse::rsv::us::unsafe_make_xslta_pointer_to(*std::declval<item_pointer_type>()))> result_type;
				result_type result;
				xslta_c_range_get_ref_if_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args)
					: result(eval(_XscpPtr, _Pred, args...)) {}
			private:
				static result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
					/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
					element in the original container, not an (ephemeral) copy. */
					const auto xs_iters = make_xslta_range_iter_provider(_XscpPtr);
					auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr, Args...>(xs_iters.begin(), xs_iters.end(), _Pred, args...).result;
					if (xs_iters.end() == res_it) {
						return result_type{};
					}
					else {
						return result_type(mse::rsv::us::unsafe_make_xslta_pointer_to(*res_it));
					}
				}
			};

			template<class _ContainerPointer, class _Pr, class... Args>
			class xslta_c_range_get_ref_to_element_known_to_be_present_ptr {
			public:
				typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
				typedef decltype(mse::rsv::us::unsafe_make_xslta_pointer_to(*std::declval<item_pointer_type>())) result_type;
				result_type result;
				xslta_c_range_get_ref_to_element_known_to_be_present_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args)
					: result(eval(_XscpPtr, _Pred, args...)) {}
			private:
				static result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
					/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
					element in the original container, not an (ephemeral) copy. */
					const auto xs_iters = make_xslta_range_iter_provider(_XscpPtr);
					auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr, Args...>(xs_iters.begin(), xs_iters.end(), _Pred, args...).result;
					if (xs_iters.end() == res_it) {
						MSE_THROW(std::logic_error("element not found - xslta_c_range_get_ref_to_element_known_to_be_present"));
					}
					else {
						return mse::rsv::us::unsafe_make_xslta_pointer_to(*res_it);
					}
				}
			};
		}

		template<class _XSLTAContainerPointer> using xslta_range_get_ref_if_ptr_type = typename impl::xslta_c_range_get_ref_if_ptr<_XSLTAContainerPointer, decltype(impl::find_if_ptr_placeholder_predicate())>::item_pointer_type;
		/* This function returns a (slta) optional that contains a slta pointer to the found element. */
		template<class _XSLTAContainerPointer, class _Pr, class... Args>
		inline auto xslta_range_get_ref_if_ptr(const _XSLTAContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
			return impl::xslta_c_range_get_ref_if_ptr<_XSLTAContainerPointer, _Pr, Args...>(_XscpPtr, _Pred, args...).result;
		}

		/* This function returns a (slta) optional that contains a slta pointer to the found element. */
		template<class _XSLTAContainerPointer, class _Pr, class... Args>
		inline auto xslta_range_get_ref_if(const _XSLTAContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE auto pred2 = mse::rsv::make_xslta_reference_or_pointer_capture_lambda([&_Pred](auto ptr) { return _Pred(*ptr); });
			return xslta_range_get_ref_if_ptr(_XscpPtr, pred2);
		}

		template<class _XSLTAContainerPointer> using xslta_range_get_ref_to_element_known_to_be_present_ptr_type = typename impl::xslta_c_range_get_ref_to_element_known_to_be_present_ptr<_XSLTAContainerPointer, decltype(impl::find_if_ptr_placeholder_predicate())>::item_pointer_type;
		/* This function returns a slta pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
		template<class _XSLTAContainerPointer, class _Pr, class... Args>
		inline auto xslta_range_get_ref_to_element_known_to_be_present_ptr(const _XSLTAContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
			return impl::xslta_c_range_get_ref_to_element_known_to_be_present_ptr<_XSLTAContainerPointer, _Pr, Args...>(_XscpPtr, _Pred, args...).result;
		}

		/* This function returns a slta pointer to the element. (Or throws an exception if it a suitable element isn't found.) */
		template<class _XSLTAContainerPointer, class _Pr, class... Args>
		inline auto xslta_range_get_ref_to_element_known_to_be_present(const _XSLTAContainerPointer& _XscpPtr, _Pr _Pred, const Args&... args) {
			MSE_SUPPRESS_CHECK_IN_XSCOPE auto pred2 = mse::rsv::make_xslta_reference_or_pointer_capture_lambda([&_Pred](auto ptr) { return _Pred(*ptr); });
			return xslta_range_get_ref_to_element_known_to_be_present_ptr(_XscpPtr, pred2);
		}
	}
#endif // 0


	/* for_each() */

	namespace impl {
		inline auto for_each_ptr_placeholder_function() {
			auto retval = [](auto) {};
			return retval;
		};
		template<class _InIt, class _Fn, class... Args>
		class c_for_each_ptr : public mse::rsv::XScopeContainsNonOwningScopeReferenceTagBase, public mse::rsv::ReferenceableByScopePointerTagBase {
		public:
			typedef item_pointer_type_from_iterator<_InIt> item_pointer_type;
			typedef _Fn result_type;
			result_type result MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			c_for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args) : result(eval(_First, _Last, _Func, args...)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }")
			{
				const auto xs_iters = make_xscope_specialized_first_and_last(_First, _Last);
				auto current = xs_iters.first();
				/* Note that `current` could be a raw pointer iterator here. */
				MSE_SUPPRESS_CHECK_IN_XSCOPE auto unsafe_increment = [](auto& x) { ++x; };
				for (; current != xs_iters.last(); unsafe_increment(current)) {
					_Func(current, args...);
				}
				MSE_SUPPRESS_CHECK_IN_XSCOPE return (_Func);
			}
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class _ContainerPointer, class _Fn, class... Args>
		class xscope_c_range_for_each_ptr : public mse::rsv::XScopeContainsNonOwningScopeReferenceTagBase, public mse::rsv::ReferenceableByScopePointerTagBase {
		public:
			typedef item_pointer_type_from_container_pointer<_ContainerPointer> item_pointer_type;
			typedef _Fn result_type;
			result_type result MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			xscope_c_range_for_each_ptr(const _ContainerPointer& _XscpPtr, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
				: result(eval(_XscpPtr, _Func, args...)) {}
		private:
			static result_type eval(const _ContainerPointer& _XscpPtr, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args) {
				const auto xs_iters = make_xscope_range_iter_provider(_XscpPtr);
				auto retval = c_for_each_ptr<decltype(xs_iters.begin()), _Fn>(xs_iters.begin(), xs_iters.end(), _Func, args...).result;
				MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
			}
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
	}
	template<class _InIt> using for_each_ptr_type = typename impl::c_for_each_ptr<_InIt, decltype(impl::for_each_ptr_placeholder_function())>::item_pointer_type;
	template<class _InIt, class _Fn, class... Args>
	inline auto for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
		MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
		MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
	{
		auto retval = impl::c_for_each_ptr<_InIt, _Fn, Args...>(_First, _Last, _Func, args...).result;
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	template<class _InIt, class _Fn, class... Args>
	inline auto for_each(const _InIt& _First, const _InIt& _Last, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
		MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
		MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
	{
		auto func2 = mse::rsv::make_xscope_reference_or_pointer_capture_lambda([&_Func](auto ptr) { _Func(*ptr); });
		for_each_ptr(_First, _Last, func2, args...);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return (_Func);
	}

	template<class _XScopeContainerPointer> using xscope_range_for_each_ptr_type = typename impl::xscope_c_range_for_each_ptr<_XScopeContainerPointer, decltype(impl::for_each_ptr_placeholder_function())>::item_pointer_type;
	template<class _XScopeContainerPointer, class _Fn, class... Args>
	inline auto xscope_range_for_each_ptr(const _XScopeContainerPointer& _XscpPtr, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
		MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
		MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
	{
		auto retval = impl::xscope_c_range_for_each_ptr<_XScopeContainerPointer, _Fn>(_XscpPtr, _Func, args...).result;
		MSE_SUPPRESS_CHECK_IN_XSCOPE return std::move(retval);
	}

	template<class _XScopeContainerPointer, class _Fn, class... Args>
	inline auto xscope_range_for_each(const _XScopeContainerPointer& _XscpPtr, _Fn _Func MSE_ATTR_PARAM_STR("mse::lifetime_labels(alias_11$)"), const Args&... args)
		MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Fn, alias_11$)")
		MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
	{
		auto func2 = mse::rsv::make_xscope_reference_or_pointer_capture_lambda([&_Func](auto ptr) { _Func(*ptr); });
		xscope_range_for_each_ptr(_XscpPtr, func2, args...);
		MSE_SUPPRESS_CHECK_IN_XSCOPE return (_Func);
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
				MSE_SUPPRESS_CHECK_IN_XSCOPE std::sort(xs_iters.first(), xs_iters.first());
			}
		};

		template<class _ContainerPointer>
		class xscope_c_range_sort {
		public:
			xscope_c_range_sort(const _ContainerPointer& _XscpPtr) { eval(_XscpPtr); }
		private:
			static void eval(const _ContainerPointer& _XscpPtr) {
				const auto xs_iters = make_xscope_range_iter_provider(_XscpPtr);
				MSE_SUPPRESS_CHECK_IN_XSCOPE std::sort(xs_iters.begin(), xs_iters.end());
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
			result_type result = false;
			xscope_c_range_equal(const _ContainerPointer& _XscpPtr, _InIt2 _First2)
				: result(eval(_XscpPtr, _First2)) {}
		private:
			static result_type eval(const _ContainerPointer& _XscpPtr, _InIt2 _First2) {
				auto xs_iters = make_xscope_range_iter_provider(_XscpPtr);
				MSE_SUPPRESS_CHECK_IN_XSCOPE return m_equal(xs_iters.begin(), xs_iters.end(), _First2);
			}
		};
	}
	template<class _InIt1, class _InIt2>
	inline _InIt1 equal(const _InIt1& _First1, const _InIt1& _Last1, _InIt2 _First2) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE return impl::c_equal<_InIt1, _InIt2>(_First1, _Last1, _First2).result;
	}

	template<class _XScopeContainerPointer, class _InIt2>
	inline auto xscope_range_equal(const _XScopeContainerPointer& _XscpPtr, _InIt2 _First2) {
		MSE_SUPPRESS_CHECK_IN_XSCOPE return impl::xscope_c_range_equal<_XScopeContainerPointer, _InIt2>(_XscpPtr, _First2).result;
	}
}

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma pop_macro("MSE_THROW")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#endif /*ndef MSEALGORITHM_H*/
