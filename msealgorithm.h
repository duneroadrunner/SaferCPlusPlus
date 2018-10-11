
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

#if 0
	namespace impl {
		namespace iterator {
			template<class T, class EqualTo>
			struct SupportsStdBegin_msealgorithm_impl
			{
				template<class U, class V>
				//static auto test(U*) -> decltype(std::declval<U>().begin() == std::declval<V>().begin(), bool(true));
				static auto test(U*) -> decltype(std::begin(std::declval<U>()) == std::begin(std::declval<V>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct SupportsStdBegin_msealgorithm : SupportsStdBegin_msealgorithm_impl<
				typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

			template<class T, class EqualTo>
			struct HasOrInheritsStaticSSBeginMethod_msealgorithm_impl
			{
				template<class U, class V>
				static auto test(U*) -> decltype(U::ss_begin(std::declval<U*>()) == V::ss_begin(std::declval<V*>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct HasOrInheritsStaticSSBeginMethod_msealgorithm : HasOrInheritsStaticSSBeginMethod_msealgorithm_impl<
				typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

			template<typename T, size_t n>
			size_t native_array_size_msemsearray(const T(&)[n]) {
				return n;
			}
			template<class T, class EqualTo>
			struct IsNativeArray_msealgorithm_impl
			{
				template<class U, class V>
				static auto test(U*) -> decltype(native_array_size_msemsearray(std::declval<U>()) == native_array_size_msemsearray(std::declval<V>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct IsNativeArray_msealgorithm : IsNativeArray_msealgorithm_impl<
				typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

			template<class T, class EqualTo>
			struct IsDereferenceable_msealgorithm_impl
			{
				template<class U, class V>
				static auto test(U*) -> decltype((*std::declval<U>()) == (*std::declval<V>()), bool(true));
				template<typename, typename>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			};
			template<class T, class EqualTo = T>
			struct IsDereferenceable_msealgorithm : IsDereferenceable_msealgorithm_impl<
				typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};

			template<typename _Ty, class = typename std::enable_if<(IsDereferenceable_msealgorithm<_Ty>::value), void>::type>
			void T_valid_if_is_dereferenceable() {}


			template <typename _TRAPointer>
			auto begin_iter_from_ptr_helper2(std::true_type, const _TRAPointer& ptr) {
				/* ptr seems to be an xscope pointer.*/
				return mse::make_xscope_random_access_iterator(ptr, 0);
			}
			template <typename _TRAPointer>
			auto begin_iter_from_ptr_helper2(std::false_type, const _TRAPointer& ptr) {
				return mse::make_random_access_iterator(ptr, 0);
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param3(std::false_type, const _TRALoneParam& ptr) {
				/* The parameter doesn't seem to be a container with a "begin()" member function or a native array. Here we'll assume
				that it is a pointer to a supported container. If you get a compile error here, then construction from the given
				parameter type isn't supported. */
				mse::T_valid_if_is_dereferenceable<_TRALoneParam>();
				return begin_iter_from_ptr_helper2(typename std::is_base_of<mse::XScopeTagBase, _TRALoneParam>::type(), ptr);
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param3(std::true_type, const _TRALoneParam& ra_container) {
				/* The parameter seems to be a container with a "begin()" member function. So we'll use that function to obtain the
				iterator we need. */
				return std::begin(ra_container);
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param2(std::false_type, const _TRALoneParam& param) {
				/* The parameter is not a "random access section". */
				return begin_iter_from_lone_param3(typename SupportsStdBegin_msealgorithm<_TRALoneParam>::type(), param);
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param2(std::true_type, const _TRALoneParam& native_array) {
				/* Apparently the lone parameter is a native array. */
				return native_array;
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param1(std::false_type, const _TRALoneParam& ptr) {
				/* The parameter doesn't seem to be a container with a "begin()" member function. */
				return begin_iter_from_lone_param2(typename mse::IsNativeArray_msealgorithm<_TRALoneParam>::type(), ptr);
			}
			template <typename _TRALoneParam>
			auto begin_iter_from_lone_param1(std::true_type, const _TRALoneParam& ra_section) {
				/* The parameter is another "random access section". */
				return ra_section.m_start_iter;
			}
		}
	}
#endif // 0

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
	}

	/* find_if() */

	namespace impl {
		template<class _InIt, class _Pr/* = decltype([](_InIt) { return true; })*/>
		class c_find_if_ptr {
		public:
			typedef typename std::remove_reference<_InIt>::type item_pointer_type;
			typedef typename std::remove_reference<_InIt>::type result_type;
			result_type result;
			c_find_if_ptr(const _InIt& _First, const _InIt& _Last, _Pr _Pred) : result(eval(_First, _Last, _Pred)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Pr _Pred) {
				auto current = _First;
				for (; current != _Last; ++current) {
					if (_Pred(current)) {
						break;
					}
				}
				return current;
			}
		};

		template<class _ContainerPointer, class _Pr>
		class xscope_c_range_get_ref_if_ptr {
		public:
			typedef typename std::remove_reference<decltype(*mse::make_xscope_begin_iterator(std::declval<_ContainerPointer>()))>::type element_t;
			typedef mse::xscope_optional<decltype(mse::us::unsafe_make_xscope_pointer_to(std::declval<element_t>()))> result_type;
			result_type result;
			xscope_c_range_get_ref_if_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (wrapped const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr>(xs_iters.begin(), xs_iters.end(), _Pred).result;
				if (xs_iters.end() == res_it) {
					return result_type{};
				}
				else {
					return result_type(mse::us::unsafe_make_xscope_pointer_to(*res_it));
				}
			}
		};

		template<class _ContainerPointer, class _Pr>
		class xscope_c_range_get_ref_to_element_known_to_be_present_ptr {
		public:
			typedef typename std::remove_reference<decltype(*mse::make_xscope_begin_iterator(std::declval<_ContainerPointer>()))>::type element_t;
			typedef decltype(mse::us::unsafe_make_xscope_pointer_to(std::declval<element_t>())) result_type;
			result_type result;
			xscope_c_range_get_ref_to_element_known_to_be_present_ptr(const _ContainerPointer& _XscpPtr, _Pr _Pred)
				: result(eval(_XscpPtr, _Pred)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _Pr _Pred) {
				/* Note that since we're returning a (const) reference, we need to be careful that it refers to an
				element in the original container, not an (ephemeral) copy. */
				auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				auto res_it = c_find_if_ptr<decltype(xs_iters.begin()), _Pr>(xs_iters.begin(), xs_iters.end(), _Pred).result;
				if (xs_iters.end() == res_it) {
					MSE_THROW(std::logic_error("element not found - xscope_c_range_get_ref_to_element_known_to_be_present"));
				}
				else {
					return mse::us::unsafe_make_xscope_const_pointer_to(*res_it);
				}
			}
		};
	}
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
		template<class _InIt, class _Fn/* = decltype([](_InIt) {})*/>
		class c_for_each_ptr {
		public:
			typedef typename std::remove_reference<_InIt>::type item_pointer_type;
			typedef _Fn result_type;
			result_type result;
			c_for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func) : result(eval(_First, _Last, _Func)) {}
		private:
			static result_type eval(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
				auto current = _First;
				for (; current != _Last; ++current) {
					_Func(current);
				}
				return (_Func);
			}
		};

		template<class _ContainerPointer, class _Fn>
		class xscope_c_range_for_each_ptr {
		public:
			typedef _Fn result_type;
			result_type result;
			xscope_c_range_for_each_ptr(const _ContainerPointer& _XscpPtr, _Fn _Func)
				: result(eval(_XscpPtr, _Func)) {}
		private:
			result_type eval(const _ContainerPointer& _XscpPtr, _Fn _Func) {
				auto xs_iters = TXScopeRangeIterProvider<_ContainerPointer>(_XscpPtr);
				return c_for_each_ptr<decltype(xs_iters.begin()), _Fn>(xs_iters.begin(), xs_iters.end(), _Func).result;
			}
		};
	}
	template<class _InIt, class _Fn>
	inline auto for_each_ptr(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
		return impl::c_for_each_ptr<_InIt, _Fn>(_First, _Last, _Func).result;
	}

	template<class _InIt, class _Fn>
	inline auto for_each(const _InIt& _First, const _InIt& _Last, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		for_each_ptr(_First, _Last, func2);
		return _Func;
	}

	template<class _XScopeContainerPointer, class _Fn>
	inline auto xscope_range_for_each_ptr(const _XScopeContainerPointer& _XscpPtr, _Fn _Func) {
		return impl::xscope_c_range_for_each_ptr<_XScopeContainerPointer, _Fn>(_XscpPtr, _Func).result;
	}

	template<class _XScopeContainerPointer, class _Fn>
	inline auto xscope_range_for_each(const _XScopeContainerPointer& _XscpPtr, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		xscope_range_for_each_ptr(_XscpPtr, func2);
		return _Func;
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

		template<class _Container1, class _InIt2>
		class xscope_c_range_equal_adapter {
		public:
			typedef bool result_type;
			result_type result;

			typedef TXScopeItemFixedConstPointer<_Container1> container_pointer_t;
			xscope_c_range_equal_adapter(const container_pointer_t& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_range_equal<_Container1, _InIt2>(_XscpPtr, _First2).result) {}
			explicit xscope_c_range_equal_adapter(const TXScopeItemFixedPointer<_Container1>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_range_equal<_Container1, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
#ifndef MSE_SCOPEPOINTER_DISABLED
			explicit xscope_c_range_equal_adapter(const TXScopeFixedConstPointer<_Container1>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_range_equal<_Container1, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
			explicit xscope_c_range_equal_adapter(const TXScopeFixedPointer<_Container1>& _XscpPtr, _InIt2 _First2)
				: result(xscope_c_range_equal<_Container1, _InIt2>(container_pointer_t(_XscpPtr), _First2).result) {}
#endif //!MSE_SCOPEPOINTER_DISABLED
		};
	}
	template<class _InIt1, class _InIt2>
	inline _InIt1 equal(const _InIt1& _First1, const _InIt1& _Last1, _InIt2 _First2) {
		return impl::c_equal<_InIt1, _InIt2>(_First1, _Last1, _First2).result;
	}

	template<class _XScopeContainerPointer, class _InIt2>
	inline auto xscope_range_equal(const _XScopeContainerPointer& _XscpPtr, _InIt2 _First2) {
		typedef typename std::remove_reference<decltype(*std::declval<_XScopeContainerPointer>())>::type _Container1;
		return impl::xscope_c_range_equal_adapter<_Container1, _InIt2>(_XscpPtr, _First2).result;
	}
}

#endif /*ndef MSEALGORITHM_H*/
