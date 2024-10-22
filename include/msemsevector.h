
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSEVECTOR_H
#define MSEMSEVECTOR_H

/*compiler specific defines*/
#ifdef _MSC_VER
#if (1700 > _MSC_VER)
#define MSVC2010_COMPATIBLE 1
#endif /*(1700 > _MSC_VER)*/
#if (1900 > _MSC_VER)
#define MSVC2013_COMPATIBLE 1
#endif /*(1900 > _MSC_VER)*/
#else /*_MSC_VER*/
#if (defined(__GNUC__) || defined(__GNUG__))
#define GPP_COMPATIBLE 1
#if ((5 > __GNUC__) && (!defined(__clang__)))
#define GPP4P8_COMPATIBLE 1
#endif /*((5 > __GNUC__) && (!defined(__clang__)))*/
#endif
#endif /*_MSC_VER*/

//define MSE_MSEVECTOR_USE_MSE_PRIMITIVES 1
#ifdef MSE_MSEVECTOR_USE_MSE_PRIMITIVES
#include "mseprimitives.h"
#endif // MSE_MSEVECTOR_USE_MSE_PRIMITIVES

#include "msemsearray.h"
#include <vector>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <functional>
#include <climits>       // ULONG_MAX
#include <stdexcept>
#include <atomic>

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDVECTOR_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifndef NDEBUG
#ifndef MSE_SUPPRESS_MSTD_VECTOR_CHECK_USE_AFTER_FREE
#define MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE
#endif // !MSE_SUPPRESS_MSTD_VECTOR_CHECK_USE_AFTER_FREE
#endif // !NDEBUG

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4522 4189)
#endif /*_MSC_VER*/

#ifndef MSE_PUSH_MACRO_NOT_SUPPORTED
#pragma push_macro("MSE_THROW")
#pragma push_macro("_NOEXCEPT")
#pragma push_macro("_NOEXCEPT_OP")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifndef _NOEXCEPT_OP
#define _NOEXCEPT_OP(x)	noexcept(x)
#endif /*_NOEXCEPT_OP*/

namespace mse {

#ifdef MSE_MSEVECTOR_USE_MSE_PRIMITIVES
	typedef mse::CSize_t msev_size_t;
	typedef mse::CInt msev_int;
	typedef bool msev_bool; // no added safety benefit to using mse::CBool in this case
	#define msev_as_a_size_t as_a_size_t
#else // MSE_MSEVECTOR_USE_MSE_PRIMITIVES

#ifndef MSE_MSEVECTOR_BASE_INTEGER_TYPE
#if SIZE_MAX <= UINT_MAX
#define MSE_MSEVECTOR_BASE_INTEGER_TYPE int
#else // SIZE_MAX <= INT_MAX
#if SIZE_MAX <= ULONG_MAX
#define MSE_MSEVECTOR_BASE_INTEGER_TYPE long int
#else // SIZE_MAX <= ULONG_MAX
#define MSE_MSEVECTOR_BASE_INTEGER_TYPE long long int
#endif // SIZE_MAX <= ULONG_MAX
#endif // SIZE_MAX <= INT_MAX
#endif // !MSE_MSEVECTOR_BASE_INTEGER_TYPE

	typedef size_t msev_size_t;
	typedef MSE_MSEVECTOR_BASE_INTEGER_TYPE msev_int;
	typedef bool msev_bool;
	typedef size_t msev_as_a_size_t;
#endif // MSE_MSEVECTOR_USE_MSE_PRIMITIVES

	class gnii_vector_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};
	class gnii_vector_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};
	class msevector_range_error : public std::range_error { public:
		using std::range_error::range_error;
	};
	class msevector_null_dereference_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	/* msev_pointer behaves similar to native pointers. It's a bit safer in that it initializes to
	nullptr by default and checks for attempted dereference of null pointers. */
	template<typename _Ty>
	class msev_pointer {
	public:
		msev_pointer() : m_ptr(nullptr) {}
		msev_pointer(_Ty* ptr) : m_ptr(ptr) {}
		//msev_pointer(const msev_pointer<_Ty>& src) : m_ptr(src.m_ptr) {}
		template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<
			std::is_same<_Ty2, _Ty>::value || ((!std::is_const<_Ty2>::value) && std::is_same<const _Ty2, _Ty>::value)
			> MSE_IMPL_EIS >
			msev_pointer(const msev_pointer<_Ty2>& src) : m_ptr(src.m_ptr) {}

		_Ty& operator*() const {
#ifndef MSE_DISABLE_MSEAR_POINTER_CHECKS
			if (nullptr == m_ptr) { MSE_THROW(msevector_null_dereference_error("attempt to dereference null pointer - mse::msev_pointer")); }
#endif /*MSE_DISABLE_MSEAR_POINTER_CHECKS*/
			return (*m_ptr);
		}
		_Ty* operator->() const {
#ifndef MSE_DISABLE_MSEAR_POINTER_CHECKS
			if (nullptr == m_ptr) { MSE_THROW(msevector_null_dereference_error("attempt to dereference null pointer - mse::msev_pointer")); }
#endif /*MSE_DISABLE_MSEAR_POINTER_CHECKS*/
			return m_ptr;
		}
		msev_pointer<_Ty>& operator=(_Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		bool operator==(const msev_pointer _Right_cref) const { return (_Right_cref.m_ptr == m_ptr); }
		bool operator!=(const msev_pointer _Right_cref) const { return (!((*this) == _Right_cref)); }
		//bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
		//bool operator!=(const _Ty* _Right_cref) const { return (!((*this) == _Right_cref)); }

		bool operator!() const { return (!m_ptr); }
		explicit operator bool() const { return (m_ptr != nullptr); }

		explicit operator _Ty*() const { return m_ptr; }

		void async_not_shareable_and_not_passable_tag() const {}

		_Ty* m_ptr;
	};

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

	namespace impl {
		template<class... _Types> using void_t = void;

		template<class T, class EqualTo>
		struct SupportsPreIncrement_msemsevector_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype((++std::declval<U>()), (++std::declval<V>()), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
			static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
		};
		template<>
		struct SupportsPreIncrement_msemsevector_impl<void*, void*> : std::false_type {};
		template<class T, class EqualTo = T>
		struct SupportsPreIncrement_msemsevector : SupportsPreIncrement_msemsevector_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

		template<class _Iter>
		using _mse_Iter_cat_t = typename std::iterator_traits<_Iter>::iterator_category;
		template<class _Ty, class = void>
		MSE_INLINE_VAR MSE_CONSTEXPR bool _mse_Is_iterator_v = false;
		template<class _Ty>
		MSE_INLINE_VAR MSE_CONSTEXPR bool _mse_Is_iterator_v<_Ty, void_t<_mse_Iter_cat_t<_Ty> > > = true;

		template<class _Iter>
		struct _mse_Is_iterator : public std::integral_constant<bool, /*(!std::is_integral<_Iter>::value) && */
				mse::impl::IsDereferenceable_pb<_Iter>::value && mse::impl::SupportsPreIncrement_msemsevector<_Iter>::value>
		{	// tests for reasonable iterator candidate
		};

		template<typename _InIter>
		using _mse_RequireInputIter = mse::impl::enable_if_t<
			std::is_convertible<typename std::iterator_traits<_InIter>::iterator_category, std::input_iterator_tag>::value
			//mse::impl::_mse_Is_iterator<_InIter>::value
		>;

#ifdef MSE_HAS_CXX17
		template<class _Ty, class = void>
		struct _mse_Is_allocator : std::false_type {};

		template<class _Ty>
		struct _mse_Is_allocator < _Ty, std::void_t<typename _Ty::value_type,
			decltype(std::declval<_Ty&>().deallocate(std::declval<_Ty&>().allocate(size_t{ 1 }), size_t{ 1 }))
		> > : std::true_type {};

		template<class _Alloc>
		using _mse_Guide_size_type_t = typename std::allocator_traits<std::conditional_t<_mse_Is_allocator<_Alloc>::value, _Alloc, std::allocator<int>>>::size_type;
#endif /* MSE_HAS_CXX17 */

		//template <typename _Ty> struct is_contiguous_sequence_container<std::vector<_Ty> > : std::true_type {};
		template <typename _Ty, typename _A> struct is_contiguous_sequence_container<std::vector<_Ty, _A> > : std::true_type {};

		template<class T, class EqualTo>
		struct HasOrInheritsLessThanOperator_msemsevector_impl
		{
			template<class U, class V>
			static auto test(U*) -> decltype(std::declval<U>() < std::declval<V>(), bool(true));
			template<typename, typename>
			static auto test(...)->std::false_type;

			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct HasOrInheritsLessThanOperator_msemsevector : HasOrInheritsLessThanOperator_msemsevector_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};
	}


	namespace impl {
		namespace ns_gnii_vector {
			template<typename _TVector>
			class Tgnii_vector_xscope_ss_const_iterator_type;
		}
	}

	namespace us {
		namespace impl {
			struct gnii_vector_const_lockable_tag {}; struct gnii_vector_not_const_lockable_tag {};

			template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex, template<typename> class _TTXScopeConstIterator = mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type, class TConstLockableIndicator = gnii_vector_not_const_lockable_tag>
			class gnii_vector;
		}
	}

	namespace us {
		template<class _Ty, class _A/* = std::allocator<_Ty>*/, class _TStateMutex/* = mse::non_thread_safe_shared_mutex*/>
		class msevector;
	}
	namespace rsv {
		template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
		class xslta_accessing_fixed_vector;
		namespace us {
			namespace impl {
				template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
				class xslta_accessing_fixed_vector_base;
			}
		}
		namespace impl {
			namespace ns_xslta_accessing_fixed_vector {
				template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
				class xslta_accessing_fixed_vector_base2;
			}
		}
	}
	template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
	class xscope_accessing_fixed_nii_vector;
	namespace us {
		namespace impl {
			template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
			class xscope_accessing_fixed_nii_vector_base;
		}
	}
	namespace impl {
		namespace ns_xscope_accessing_fixed_nii_vector {
			template<class _TPointer, class _TLender/* = mse::impl::target_type<_TPointer>*/, class _Ty/* = mse::impl::container_element_type<_TLender> */>
			class xscope_accessing_fixed_nii_vector_base2;
		}
	}

	namespace impl {
		namespace ns_gnii_vector {
			template<class _TContainer>
			class xscope_shared_structure_lock_guard;
			template<class _TContainer, class _TAccessMutex/* = mse::non_thread_safe_shared_mutex*/>
			class xscope_ewconst_structure_lock_guard;
		}
	}
	namespace us {
		namespace impl {
			namespace ns_gnii_vector {
				template<class _TContainer>
				class xscope_shared_const_structure_lock_guard;
			}
		}
	}

	//template<class _Ty> auto make_xscope_shared_structure_lock_guard(const _Ty& owner_ptr);
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >& owner_ptr)->mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >;
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >& owner_ptr)->mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >;
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator, class _TAccessMutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TAccessControlledConstPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>, _TAccessMutex>& owner_ptr)->mse::impl::ns_gnii_vector::xscope_ewconst_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>, _TAccessMutex>;

	namespace us {
		namespace impl {
			template <typename _TRAContainerPointer, typename _TStructureLockPointer> class TXScopeCSLSStrongRAConstIterator;

			template <typename _TRAContainerPointer, typename _TStructureLockPointer>
			class TXScopeCSLSStrongRAIterator : public TXScopeRAIterator<_TStructureLockPointer> {
			public:
				typedef TXScopeRAIterator<_TStructureLockPointer> base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				TXScopeCSLSStrongRAIterator(const TXScopeCSLSStrongRAIterator& src) : base_class(src) {}
				TXScopeCSLSStrongRAIterator(TXScopeCSLSStrongRAIterator&& src) : base_class(MSE_FWD(src)) {}

				TXScopeCSLSStrongRAIterator(const _TRAContainerPointer& ra_container_pointer, size_type index = 0)
					: base_class(ra_container_pointer, index) {}
				TXScopeCSLSStrongRAIterator(_TRAContainerPointer&& ra_container_pointer, size_type index = 0)
					: base_class(MSE_FWD(ra_container_pointer), index) {}

				TXScopeCSLSStrongRAIterator(const TXScopeRAIterator<_TRAContainerPointer>& ra_iterator)
					: base_class(ra_iterator.target_container_ptr(), ra_iterator.position()) {}
				TXScopeCSLSStrongRAIterator(TXScopeRAIterator<_TRAContainerPointer>&& ra_iterator)
					: base_class(MSE_FWD(ra_iterator).target_container_ptr(), ra_iterator.position()) {}

				auto& operator=(const TXScopeCSLSStrongRAIterator& _Right_cref) & { base_class::operator=(_Right_cref); return (*this); }
				auto& operator=(TXScopeCSLSStrongRAIterator&& _Right_cref) & { base_class::operator=(MSE_FWD(_Right_cref)); return (*this); }
				TXScopeCSLSStrongRAIterator operator=(TXScopeCSLSStrongRAIterator&& _X) && { base_class::operator=(MSE_FWD(_X)); return std::move(*this); }
				TXScopeCSLSStrongRAIterator operator=(const TXScopeCSLSStrongRAIterator _X) && { base_class::operator=(_X); return std::move(*this); }
				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeCSLSStrongRAIterator);

				typedef mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > _TRAContainer;
				TXScopeCSSSXSRAIterator<_TRAContainer> xscope_csssxsra_iterator() const & {
					auto xs_strong_pointer_store = mse::make_xscope_strong_pointer_store((*this).target_container_ptr());
					return mse::us::unsafe_make_xscope_csss_strong_ra_iterator<mse::TXScopeFixedPointer<_TRAContainer> >(xs_strong_pointer_store.xscope_ptr(), (*this).position());
				}
				void xscope_csssxsra_iterator() const && = delete;
				operator TXScopeCSSSXSRAIterator<_TRAContainer>() const & {
					return mse::us::unsafe_make_xscope_csss_strong_ra_iterator<mse::TXScopeFixedPointer<_TRAContainer> >((*this).target_container_ptr(), (*this).position());
				}
				operator TXScopeCSSSXSRAIterator<_TRAContainer>() const && = delete;

				TXScopeCSSSStrongRAIterator<_TStructureLockPointer> xscope_csss_strong_ra_iterator() const & {
					return mse::us::unsafe_make_xscope_csss_strong_ra_iterator<_TStructureLockPointer>((*this).target_container_ptr(), (*this).position());
				}
				void xscope_csss_strong_ra_iterator() const && = delete;

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_iterator_tag() const {}
				void xscope_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				template <typename _TRAContainerPointer2, typename _TStructureLockPointer2>
				friend class TXScopeCSLSStrongRAConstIterator;
			};

			template <typename _TRAContainerPointer, typename _TStructureLockPointer>
			class TXScopeCSLSStrongRAConstIterator : public TXScopeRAConstIterator<_TStructureLockPointer> {
			public:
				typedef TXScopeRAConstIterator<_TStructureLockPointer> base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				TXScopeCSLSStrongRAConstIterator(const TXScopeCSLSStrongRAConstIterator& src) : base_class(src) {}
				TXScopeCSLSStrongRAConstIterator(TXScopeCSLSStrongRAConstIterator&& src) : base_class(MSE_FWD(src)) {}

				template <typename _TRAContainerPointer2, typename _TStructureLockPointer2>
				TXScopeCSLSStrongRAConstIterator(const TXScopeCSLSStrongRAIterator<_TRAContainerPointer2, _TStructureLockPointer2>& src) : base_class(src) {}
				template <typename _TRAContainerPointer2, typename _TStructureLockPointer2>
				TXScopeCSLSStrongRAConstIterator(TXScopeCSLSStrongRAIterator<_TRAContainerPointer2, _TStructureLockPointer2>&& src) : base_class(MSE_FWD(src)) {}

				TXScopeCSLSStrongRAConstIterator(const _TRAContainerPointer& ra_container_pointer, size_type index = 0)
					: base_class(ra_container_pointer, index) {}
				TXScopeCSLSStrongRAConstIterator(_TRAContainerPointer&& ra_container_pointer, size_type index = 0)
					: base_class(MSE_FWD(ra_container_pointer), index) {}

				TXScopeCSLSStrongRAConstIterator(const TXScopeRAConstIterator<_TRAContainerPointer>& ra_iterator)
					: base_class(ra_iterator.target_container_ptr(), ra_iterator.position()) {}
				TXScopeCSLSStrongRAConstIterator(TXScopeRAConstIterator<_TRAContainerPointer>&& ra_iterator)
					: base_class(MSE_FWD(ra_iterator).target_container_ptr(), ra_iterator.position()) {}

				auto& operator=(const TXScopeCSLSStrongRAConstIterator& _Right_cref) & { base_class::operator=(_Right_cref); return (*this); }
				auto& operator=(TXScopeCSLSStrongRAConstIterator&& _Right_cref) & { base_class::operator=(MSE_FWD(_Right_cref)); return (*this); }
				TXScopeCSLSStrongRAConstIterator operator=(TXScopeCSLSStrongRAConstIterator&& _X) && { base_class::operator=(MSE_FWD(_X)); return std::move(*this); }
				TXScopeCSLSStrongRAConstIterator operator=(const TXScopeCSLSStrongRAConstIterator _X) && { base_class::operator=(_X); return std::move(*this); }
				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				MSE_INHERIT_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXScopeCSLSStrongRAConstIterator);

				typedef mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > _TRAContainer;
				TXScopeCSSSXSRAConstIterator<_TRAContainer> xscope_csssxsra_iterator() const & {
					auto xs_strong_pointer_store = mse::make_xscope_strong_const_pointer_store((*this).target_container_ptr());
					return mse::us::unsafe_make_xscope_csss_strong_ra_const_iterator<mse::TXScopeFixedConstPointer<_TRAContainer> >(xs_strong_pointer_store.xscope_ptr(), (*this).position());
				}
				void xscope_csssxsra_iterator() const && = delete;
				operator TXScopeCSSSXSRAConstIterator<_TRAContainer>() const & {
					return mse::us::unsafe_make_xscope_csss_strong_ra_const_iterator<mse::TXScopeFixedConstPointer<_TRAContainer> >((*this).target_container_ptr(), (*this).position());
				}
				operator TXScopeCSSSXSRAConstIterator<_TRAContainer>() const && = delete;

				TXScopeCSSSStrongRAConstIterator<_TStructureLockPointer> xscope_csss_strong_ra_iterator() const & {
					return mse::us::unsafe_make_xscope_csss_strong_ra_const_iterator<_TStructureLockPointer>((*this).target_container_ptr(), (*this).position());
				}
				void xscope_csss_strong_ra_iterator() const && = delete;

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_iterator_tag() const {}
				void xscope_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
			};
		}
	}

	namespace impl {

		/* Specializations of TXScopeRawPointerRAFirstAndLast<> that replace certain iterators with fast (raw pointer) iterators
		when it's safe to do so. In this case mse::us::impl::TXScopeCSLSStrongRA(Const)Iterator<>s. */
		template <typename _TRAContainerPointer, typename _TStructureLockPointer>
		class TXScopeSpecializedFirstAndLast<mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer> >
			: public TXScopeSpecializedFirstAndLast<TXScopeCSSSXSRAConstIterator<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > > > {
		public:
			typedef TXScopeSpecializedFirstAndLast<TXScopeCSSSXSRAConstIterator<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > > > base_class;
			typedef mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer> iter_t;
			TXScopeSpecializedFirstAndLast(iter_t first, const iter_t& last) : base_class(first, last), m_first(std::move(first)) {}

		private:
			/* We need to store one of the given iterators (or a copy of it) as it holds, while it exists, a "structure lock"
			on the container (ensuring that it is not resized). */
			iter_t m_first;
		};

		template <typename _TRAContainerPointer, typename _TStructureLockPointer>
		auto make_xscope_specialized_first_and_last_overloaded(const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last) {
			return TXScopeSpecializedFirstAndLast<mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer> >(_First, _Last);
		}

		template <typename _TRAContainerPointer, typename _TStructureLockPointer>
		class TXScopeSpecializedFirstAndLast<mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer> >
			: public TXScopeSpecializedFirstAndLast<TXScopeCSSSXSRAIterator<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > > > {
		public:
			typedef TXScopeSpecializedFirstAndLast<TXScopeCSSSXSRAIterator<mse::impl::remove_const_t<mse::impl::remove_reference_t<decltype(*std::declval<_TRAContainerPointer>())> > > > base_class;
			typedef mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer> iter_t;
			TXScopeSpecializedFirstAndLast(iter_t first, const iter_t& last) : base_class(first, last), m_first(std::move(first)) {}

		private:
			/* We need to store one of the given iterators (or a copy of it) as it holds, while it exists, a "structure lock"
			on the container (ensuring that it is not resized). */
			iter_t m_first;
		};

		template <typename _TRAContainerPointer, typename _TStructureLockPointer>
		auto make_xscope_specialized_first_and_last_overloaded(const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last) {
			return TXScopeSpecializedFirstAndLast<mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer> >(_First, _Last);
		}
	}
}

namespace std {

	/* Overloads of standard algorithm functions for mse::us::impl::TXScopeCSLSStrongRA(Const)Iterator<>s. */

	template<class _Pr, typename _TRAContainerPointer, typename _TStructureLockPointer>
	inline auto find_if(const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last, _Pr _Pred) -> mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer> {
		auto pred2 = [&_Pred](auto ptr) { return _Pred(*ptr); };
		return mse::find_if_ptr(_First, _Last, pred2);
	}
	template<class _Pr, typename _TRAContainerPointer, typename _TStructureLockPointer>
	inline auto find_if(const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last, _Pr _Pred) -> mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer> {
		auto pred2 = [&_Pred](auto ptr) { return _Pred(*ptr); };
		return mse::find_if_ptr(_First, _Last, pred2);
	}

	template<class _Fn, typename _TRAContainerPointer, typename _TStructureLockPointer>
	inline _Fn for_each(const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		mse::for_each_ptr(_First, _Last, func2);
		return (_Func);
	}
	template<class _Fn, typename _TRAContainerPointer, typename _TStructureLockPointer>
	inline _Fn for_each(const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last, _Fn _Func) {
		auto func2 = [&_Func](auto ptr) { _Func(*ptr); };
		mse::for_each_ptr(_First, _Last, func2);
		return (_Func);
	}

	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	inline void sort(const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _First, const mse::us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& _Last) {
		mse::sort(_First, _Last);
	}
}

namespace mse {

	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	auto xscope_pointer(const us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& iter_cref) {
		//return xscope_pointer(static_cast<const TXScopeCSSSStrongRAIterator<_TStructureLockPointer>&>(iter_cref));
		return xscope_pointer(static_cast<const TXScopeCSSSStrongRAIterator<_TStructureLockPointer>&>(iter_cref));
	}
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	void xscope_pointer(const us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>&& iter_cref) = delete;
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	auto xscope_pointer(const us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& iter_cref) {
		return xscope_pointer(static_cast<const TXScopeCSSSStrongRAConstIterator<_TStructureLockPointer>&>(iter_cref));
	}
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	void xscope_pointer(const us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>&& iter_cref) = delete;
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	auto xscope_const_pointer(const us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>& iter_cref) {
		return xscope_const_pointer(static_cast<const TXScopeCSSSStrongRAIterator<_TStructureLockPointer>&>(iter_cref));
	}
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	void xscope_const_pointer(const us::impl::TXScopeCSLSStrongRAIterator<_TRAContainerPointer, _TStructureLockPointer>&& iter_cref) = delete;
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	auto xscope_const_pointer(const us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>& iter_cref) {
		return xscope_const_pointer(static_cast<const TXScopeCSSSStrongRAConstIterator<_TStructureLockPointer>&>(iter_cref));
	}
	template <typename _TRAContainerPointer, typename _TStructureLockPointer>
	void xscope_const_pointer(const us::impl::TXScopeCSLSStrongRAConstIterator<_TRAContainerPointer, _TStructureLockPointer>&& iter_cref) = delete;

	namespace impl {
		namespace ns_gnii_vector {
			/* Following are a bunch of template (iterator) classes that, organizationally, should be members of mse::us::impl::gnii_vector<>. (And they
			used to be.) However, being a member of mse::us::impl::gnii_vector<> makes them "dependent types", and dependent types do not participate
			in automatic template parameter type deduction. So we had to haul them here outside of mse::us::impl::gnii_vector<>. */

			template<typename _TVectorPointer>
			class Tgnii_vector_ss_iterator_type;

			/* Tgnii_vector_ss_const_iterator_type is a bounds checked const_iterator. */
			template<typename _TVectorConstPointer>
			class Tgnii_vector_ss_const_iterator_type : public mse::TFriendlyAugmentedRAConstIterator<mse::TRAConstIterator<_TVectorConstPointer> > {
			public:
				typedef mse::TFriendlyAugmentedRAConstIterator<mse::TRAConstIterator<_TVectorConstPointer> > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				typedef mse::impl::remove_reference_t<mse::impl::remove_const_t<decltype(*std::declval<_TVectorConstPointer>())> > _TVector;

				template<class _TVectorConstPointer2 = _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_TVectorConstPointer2, _TVectorConstPointer>::value) && (std::is_default_constructible<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
				Tgnii_vector_ss_const_iterator_type() {}

				Tgnii_vector_ss_const_iterator_type(const _TVectorConstPointer& owner_cptr, size_type index) : base_class(owner_cptr, index) {}
				Tgnii_vector_ss_const_iterator_type(_TVectorConstPointer&& owner_cptr, size_type index) : base_class(MSE_FWD(owner_cptr), index) {}

				Tgnii_vector_ss_const_iterator_type(Tgnii_vector_ss_const_iterator_type&& src) = default;
				Tgnii_vector_ss_const_iterator_type(const Tgnii_vector_ss_const_iterator_type& src) = default;
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TVectorConstPointer>::value> MSE_IMPL_EIS >
				Tgnii_vector_ss_const_iterator_type(const Tgnii_vector_ss_const_iterator_type<_Ty2>& src) : base_class(src.target_container_ptr(), src.position()) {}
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TVectorConstPointer>::value> MSE_IMPL_EIS >
				Tgnii_vector_ss_const_iterator_type(const Tgnii_vector_ss_iterator_type<_Ty2>& src) : base_class(src.target_container_ptr(), src.position()) {}

				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				auto& operator=(Tgnii_vector_ss_const_iterator_type&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_ss_const_iterator_type& _X) { base_class::operator=(_X); return (*this); }

				Tgnii_vector_ss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
				Tgnii_vector_ss_const_iterator_type operator++(int) { Tgnii_vector_ss_const_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
				Tgnii_vector_ss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
				Tgnii_vector_ss_const_iterator_type operator--(int) { Tgnii_vector_ss_const_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }

				Tgnii_vector_ss_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
				Tgnii_vector_ss_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
				Tgnii_vector_ss_const_iterator_type operator+(difference_type n) const {
					Tgnii_vector_ss_const_iterator_type retval(*this);
					retval += (n);
					return retval;
				}
				Tgnii_vector_ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const base_class& _Right_cref) const {
					return base_class::operator-(_Right_cref);
				}

				void set_to_const_item_pointer(const Tgnii_vector_ss_const_iterator_type& _Right_cref) {
					(*this) = _Right_cref;
				}

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_TVectorConstPointer);
				void iterator_tag() const {}

			private:

				friend _TVector;
			};
			/* Tgnii_vector_ss_iterator_type is a bounds checked iterator. */
			template<typename _TVectorPointer>
			class Tgnii_vector_ss_iterator_type : public mse::TFriendlyAugmentedRAIterator<mse::TRAIterator<_TVectorPointer> > {
			public:
				typedef mse::TFriendlyAugmentedRAIterator<mse::TRAIterator<_TVectorPointer> > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				typedef mse::impl::remove_reference_t<mse::impl::remove_const_t<decltype(*std::declval<_TVectorPointer>())> > _TVector;

				template<class _TVectorPointer2 = _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_TVectorPointer2, _TVectorPointer>::value) && (std::is_default_constructible<_TVectorPointer>::value)> MSE_IMPL_EIS >
				Tgnii_vector_ss_iterator_type() {}

				Tgnii_vector_ss_iterator_type(const _TVectorPointer& owner_ptr, size_type index) : base_class(owner_ptr, index) {}
				Tgnii_vector_ss_iterator_type(_TVectorPointer&& owner_ptr, size_type index) : base_class(MSE_FWD(owner_ptr), index) {}

				Tgnii_vector_ss_iterator_type(Tgnii_vector_ss_iterator_type&& src) = default;
				Tgnii_vector_ss_iterator_type(const Tgnii_vector_ss_iterator_type& src) = default;
				template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TVectorPointer>::value> MSE_IMPL_EIS >
				Tgnii_vector_ss_iterator_type(const Tgnii_vector_ss_iterator_type<_Ty2>& src) : base_class(src.target_container_ptr(), src.position()) {}

				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				auto& operator=(Tgnii_vector_ss_iterator_type&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_ss_iterator_type& _X) { base_class::operator=(_X); return (*this); }

				Tgnii_vector_ss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
				Tgnii_vector_ss_iterator_type operator++(int) { Tgnii_vector_ss_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
				Tgnii_vector_ss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
				Tgnii_vector_ss_iterator_type operator--(int) { Tgnii_vector_ss_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }

				Tgnii_vector_ss_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
				Tgnii_vector_ss_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
				Tgnii_vector_ss_iterator_type operator+(difference_type n) const {
					Tgnii_vector_ss_iterator_type retval(*this);
					retval += (n);
					return retval;
				}
				Tgnii_vector_ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const base_class& _Right_cref) const {
					return base_class::operator-(_Right_cref);
				}

				void set_to_item_pointer(const Tgnii_vector_ss_iterator_type& _Right_cref) {
					(*this) = _Right_cref;
				}

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_TVectorPointer);
				void iterator_tag() const {}

			private:

				friend _TVector;
				template<typename _TVectorConstPointer2>
				friend class Tgnii_vector_ss_const_iterator_type;
			};

			template<typename _TVectorPointer>
			using Tgnii_vector_ss_reverse_iterator_type = std::reverse_iterator<Tgnii_vector_ss_iterator_type<_TVectorPointer> >;
			template<typename _TVectorConstPointer>
			using Tgnii_vector_ss_const_reverse_iterator_type = std::reverse_iterator<Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer> >;

			template<typename _TVector>
			using Tgnii_vector_rp_ss_iterator_type = Tgnii_vector_ss_iterator_type<msev_pointer<_TVector> >;
			template<typename _TVector>
			using Tgnii_vector_rp_ss_const_iterator_type = Tgnii_vector_ss_const_iterator_type<msev_pointer<const _TVector> >;
			template<typename _TVector>
			using Tgnii_vector_rp_ss_reverse_iterator_type = Tgnii_vector_ss_iterator_type<msev_pointer<_TVector> >;
			template<typename _TVector>
			using Tgnii_vector_rp_ss_const_reverse_iterator_type = Tgnii_vector_ss_const_reverse_iterator_type<msev_pointer<const _TVector> >;

			template<typename _TVector>
			using TXScopeVectorPointer = mse::TXScopeFixedPointer<_TVector>;
			template<typename _TVector>
			using TXScopeVectorConstPointer = mse::TXScopeFixedConstPointer<_TVector>;

			template<typename _TVector>
			class Tgnii_vector_xscope_cslsstrong_iterator_type;

			template<typename _TVector>
			class Tgnii_vector_xscope_cslsstrong_const_iterator_type : public mse::TFriendlyAugmentedRAConstIterator<mse::us::impl::TXScopeCSLSStrongRAConstIterator<TXScopeVectorConstPointer<_TVector>, mse::us::impl::ns_gnii_vector::xscope_shared_const_structure_lock_guard<_TVector> > >
				/*, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::AsyncNotShareableAndNotPassableTagBase*/ {
			public:
				typedef mse::TFriendlyAugmentedRAConstIterator<mse::us::impl::TXScopeCSLSStrongRAConstIterator<TXScopeVectorConstPointer<_TVector>, mse::us::impl::ns_gnii_vector::xscope_shared_const_structure_lock_guard<_TVector> > > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Tgnii_vector_xscope_cslsstrong_const_iterator_type, base_class);

				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				auto& operator=(Tgnii_vector_xscope_cslsstrong_const_iterator_type&& _X) & { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_xscope_cslsstrong_const_iterator_type& _X) & { base_class::operator=(_X); return (*this); }

				Tgnii_vector_xscope_cslsstrong_const_iterator_type& operator ++() & { base_class::operator ++(); return (*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator++(int) { Tgnii_vector_xscope_cslsstrong_const_iterator_type _Tmp = *this; base_class::operator++(); return (_Tmp); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type& operator --() & { base_class::operator --(); return (*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator--(int) { Tgnii_vector_xscope_cslsstrong_const_iterator_type _Tmp = *this; base_class::operator--(); return (_Tmp); }

				Tgnii_vector_xscope_cslsstrong_const_iterator_type& operator +=(difference_type n) & { base_class::operator +=(n); return (*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type& operator -=(difference_type n) & { base_class::operator -=(n); return (*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const Tgnii_vector_xscope_cslsstrong_const_iterator_type& _Right_cref) const { return base_class::operator-(_Right_cref); }
				const_reference operator*() const { return base_class::operator*(); }

				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator=(Tgnii_vector_xscope_cslsstrong_const_iterator_type&& _X) && { base_class::operator=(MSE_FWD(_X)); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator=(const Tgnii_vector_xscope_cslsstrong_const_iterator_type& _X) && { base_class::operator=(_X); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator ++() && { base_class::operator ++(); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator --() && { base_class::operator --(); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator +=(difference_type n) && { base_class::operator +=(n); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_const_iterator_type operator -=(difference_type n) && { base_class::operator -=(n); return std::move(*this); }

				void set_to_const_item_pointer(const Tgnii_vector_xscope_cslsstrong_const_iterator_type& _Right_cref) { base_class::set_to_item_pointer(_Right_cref); }

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_cslsstrong_iterator_type_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				friend _TVector;
				template<class _TVector2>
				friend class Tgnii_vector_xscope_cslsstrong_iterator_type;
			};

			template<typename _TVector>
			class Tgnii_vector_xscope_cslsstrong_iterator_type : public mse::TFriendlyAugmentedRAIterator<mse::us::impl::TXScopeCSLSStrongRAIterator<TXScopeVectorPointer<_TVector>, mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<_TVector> > >
				/*, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::AsyncNotShareableAndNotPassableTagBase*/ {
			public:
				typedef mse::TFriendlyAugmentedRAIterator<mse::us::impl::TXScopeCSLSStrongRAIterator<TXScopeVectorPointer<_TVector>, mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<_TVector> > > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Tgnii_vector_xscope_cslsstrong_iterator_type, base_class);

				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				auto& operator=(Tgnii_vector_xscope_cslsstrong_iterator_type&& _X) & { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_xscope_cslsstrong_iterator_type& _X) & { base_class::operator=(_X); return (*this); }

				Tgnii_vector_xscope_cslsstrong_iterator_type& operator ++() & { base_class::operator ++(); return (*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator++(int) { Tgnii_vector_xscope_cslsstrong_iterator_type _Tmp = *this; base_class::operator++(); return (_Tmp); }
				Tgnii_vector_xscope_cslsstrong_iterator_type& operator --() & { base_class::operator --(); return (*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator--(int) { Tgnii_vector_xscope_cslsstrong_iterator_type _Tmp = *this; base_class::operator--(); return (_Tmp); }

				Tgnii_vector_xscope_cslsstrong_iterator_type& operator +=(difference_type n) & { base_class::operator +=(n); return (*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type& operator -=(difference_type n) & { base_class::operator -=(n); return (*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const Tgnii_vector_xscope_cslsstrong_iterator_type& _Right_cref) const { return base_class::operator-(_Right_cref); }
				const_reference operator*() const { return base_class::operator*(); }

				Tgnii_vector_xscope_cslsstrong_iterator_type operator=(Tgnii_vector_xscope_cslsstrong_iterator_type&& _X) && { base_class::operator=(MSE_FWD(_X)); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator=(const Tgnii_vector_xscope_cslsstrong_iterator_type& _X) && { base_class::operator=(_X); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator ++() && { base_class::operator ++(); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator --() && { base_class::operator --(); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator +=(difference_type n) && { base_class::operator +=(n); return std::move(*this); }
				Tgnii_vector_xscope_cslsstrong_iterator_type operator -=(difference_type n) && { base_class::operator -=(n); return std::move(*this); }

				void set_to_item_pointer(const Tgnii_vector_xscope_cslsstrong_iterator_type& _Right_cref) { base_class::set_to_item_pointer(_Right_cref); }

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_cslsstrong_iterator_type_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				friend _TVector;
			};

			template<typename _TVector>
			class Tgnii_vector_xscope_ss_iterator_type;

			template<typename _TVector>
			class Tgnii_vector_xscope_ss_const_iterator_type : public mse::TFriendlyAugmentedRAConstIterator<mse::TXScopeRAConstIterator<mse::TXScopeFixedConstPointer<_TVector> > >
				/*, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::AsyncNotShareableAndNotPassableTagBase*/ {
			public:
				typedef mse::TFriendlyAugmentedRAConstIterator<mse::TXScopeRAConstIterator<mse::TXScopeFixedConstPointer<_TVector> > > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Tgnii_vector_xscope_ss_const_iterator_type, base_class);

				/* gnii_vector<> uses this iterator class as its "xscope const" iterator, but uses the Tgnii_vector_xscope_cslsstrong_iterator_type<>
				iterator class for its "xscope non-const" iterator. So this class needs to support construction from that class.
				This class requires a TXScopeFixedConstPointer<> to the container, but the other class instead holds an
				xscope_shared_structure_lock_guard<>, which can be converted to the needed TXScopeFixedConstPointer<> with an
				explicit intermediary conversion (to a TXScopeFixedPointer<>). */
				Tgnii_vector_xscope_ss_const_iterator_type(const Tgnii_vector_xscope_cslsstrong_iterator_type<_TVector>& src)
					: base_class(mse::TXScopeFixedPointer<_TVector>(src.target_container_ptr()), src.position()) {}

				//MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				/* Normally we would just use the macro for inheriting assignment operators, but here we need to exclude any
				that handle Tgnii_vector_xscope_cslsstrong_iterator_type<>s, because that class needs to be handled a bit
				differently. */
				template<class _Ty2mse_uao, class _Tbase_class2 = base_class, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::HasOrInheritsAssignmentOperator_pb<_Tbase_class2>::value \
					&& (!std::is_same<Tgnii_vector_xscope_cslsstrong_iterator_type<_TVector>, mse::impl::remove_reference_t<_Ty2mse_uao> >::value) \
					&& ((!mse::impl::is_a_pair_with_the_first_a_base_of_the_second<_Tbase_class2, _Ty2mse_uao>::value) || std::is_same<_Tbase_class2, mse::impl::remove_reference_t<_Ty2mse_uao> >::value)> MSE_IMPL_EIS > \
				auto& operator=(_Ty2mse_uao&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); } \
				template<class _Ty2mse_uao, class _Tbase_class2 = base_class, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::HasOrInheritsAssignmentOperator_pb<_Tbase_class2>::value \
					&& (!std::is_same<Tgnii_vector_xscope_cslsstrong_iterator_type<_TVector>, mse::impl::remove_reference_t<_Ty2mse_uao> >::value) \
					&& ((!mse::impl::is_a_pair_with_the_first_a_base_of_the_second<_Tbase_class2, _Ty2mse_uao>::value) || std::is_same<_Tbase_class2, mse::impl::remove_reference_t<_Ty2mse_uao> >::value)> MSE_IMPL_EIS > \
				auto& operator=(const _Ty2mse_uao& _X) { base_class::operator=(_X); return (*this); }

				auto& operator=(const Tgnii_vector_xscope_cslsstrong_iterator_type<_TVector>& _X) { base_class::operator=(Tgnii_vector_xscope_ss_const_iterator_type(_X)); return (*this); }

				auto& operator=(Tgnii_vector_xscope_ss_const_iterator_type&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_xscope_ss_const_iterator_type& _X) { base_class::operator=(_X); return (*this); }

				Tgnii_vector_xscope_ss_const_iterator_type& operator ++() { base_class::operator ++(); return (*this); }
				Tgnii_vector_xscope_ss_const_iterator_type operator++(int) { Tgnii_vector_xscope_ss_const_iterator_type _Tmp = *this; base_class::operator++(); return (_Tmp); }
				Tgnii_vector_xscope_ss_const_iterator_type& operator --() { base_class::operator --(); return (*this); }
				Tgnii_vector_xscope_ss_const_iterator_type operator--(int) { Tgnii_vector_xscope_ss_const_iterator_type _Tmp = *this; base_class::operator--(); return (_Tmp); }

				Tgnii_vector_xscope_ss_const_iterator_type& operator +=(difference_type n) { base_class::operator +=(n); return (*this); }
				Tgnii_vector_xscope_ss_const_iterator_type& operator -=(difference_type n) { base_class::operator -=(n); return (*this); }
				Tgnii_vector_xscope_ss_const_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				Tgnii_vector_xscope_ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const Tgnii_vector_xscope_ss_const_iterator_type& _Right_cref) const { return base_class::operator-(_Right_cref); }
				const_reference operator*() const { return base_class::operator*(); }

				void set_to_const_item_pointer(const Tgnii_vector_xscope_ss_const_iterator_type& _Right_cref) { base_class::set_to_item_pointer(_Right_cref); }

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_ss_iterator_type_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				friend _TVector;
				template<class _TVector2>
				friend class Tgnii_vector_xscope_ss_iterator_type;
			};
			template<typename _TVector>
			class Tgnii_vector_xscope_ss_iterator_type : public mse::TFriendlyAugmentedRAIterator<mse::TXScopeRAIterator<mse::TXScopeFixedPointer<_TVector> > >
				/*, public mse::us::impl::XScopeContainsNonOwningScopeReferenceTagBase, public mse::us::impl::AsyncNotShareableAndNotPassableTagBase*/ {
			public:
				typedef mse::TFriendlyAugmentedRAIterator<mse::TXScopeRAIterator<mse::TXScopeFixedPointer<_TVector> > > base_class;
				MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

				MSE_USING_AND_DEFAULT_COPY_AND_MOVE_CONSTRUCTOR_DECLARATIONS(Tgnii_vector_xscope_ss_iterator_type, base_class);

				MSE_USING_ASSIGNMENT_OPERATOR(base_class);
				auto& operator=(Tgnii_vector_xscope_ss_iterator_type&& _X) { base_class::operator=(MSE_FWD(_X)); return (*this); }
				auto& operator=(const Tgnii_vector_xscope_ss_iterator_type& _X) { base_class::operator=(_X); return (*this); }

				Tgnii_vector_xscope_ss_iterator_type& operator ++() { base_class::operator ++(); return (*this); }
				Tgnii_vector_xscope_ss_iterator_type operator++(int) { Tgnii_vector_xscope_ss_iterator_type _Tmp = *this; base_class::operator++(); return (_Tmp); }
				Tgnii_vector_xscope_ss_iterator_type& operator --() { base_class::operator --(); return (*this); }
				Tgnii_vector_xscope_ss_iterator_type operator--(int) { Tgnii_vector_xscope_ss_iterator_type _Tmp = *this; base_class::operator--(); return (_Tmp); }

				Tgnii_vector_xscope_ss_iterator_type& operator +=(difference_type n) { base_class::operator +=(n); return (*this); }
				Tgnii_vector_xscope_ss_iterator_type& operator -=(difference_type n) { base_class::operator -=(n); return (*this); }
				Tgnii_vector_xscope_ss_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				Tgnii_vector_xscope_ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const Tgnii_vector_xscope_ss_iterator_type& _Right_cref) const { return base_class::operator-(_Right_cref); }

				void set_to_item_pointer(const Tgnii_vector_xscope_ss_iterator_type& _Right_cref) { base_class::set_to_item_pointer(_Right_cref); }

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
				void xscope_ss_iterator_type_tag() const {}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				friend _TVector;
			};
		}
	}

	namespace us {
		namespace impl {
			namespace ns_gnii_vector {
			}

			/* mse::us::impl::gnii_vector<> is essentially a generic implementation of a memory-safe vector that does not expose
			(unprotected) non-static member functions like begin() or end() which return (memory) unsafe iterators. It does provide
			static member function templates like ss_begin<>(...) and ss_end<>(...) (and emplace<>(...), insert<>(...) and
			erase<>(...)) which take a pointer parameter and return a (bounds-checked) iterator that inherits the safety of the given
			pointer. mse::us::impl::gnii_vector<> may or may not be safe to share between asynchronous threads depending on the
			template arguments. */
			template<class _Ty, class _A/* = std::allocator<_Ty>*/, class _TStateMutex/* = mse::non_thread_safe_shared_mutex*/, template<typename> class _TTXScopeConstIterator/* = mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type*/, class TConstLockableIndicator/* = gnii_vector_not_const_lockable_tag*/>
			class gnii_vector : private mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> >, public us::impl::ContiguousSequenceContainerTagBase, public us::impl::LockableStructureContainerTagBase {
			public:
				typedef _TStateMutex state_mutex_type;
				static const bool s_is_const_lockable = std::is_same<TConstLockableIndicator, mse::us::impl::gnii_vector_const_lockable_tag>::value;

				/* We (privately) inherit the underlying data type rather than make it a data member to ensure it's the "first" component in the structure.*/
				typedef mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> > base_class;
				typedef std::vector<_Ty, _A> std_vector;
				typedef std_vector _MV;
				typedef gnii_vector _Myt;

				typedef typename std_vector::allocator_type allocator_type;
				typedef typename std_vector::value_type value_type;
				//typedef typename std_vector::size_type size_type;
				typedef msev_size_t size_type;
				//typedef typename std_vector::difference_type difference_type;
				typedef msev_int difference_type;
				typedef typename std_vector::pointer pointer;
				typedef typename std_vector::const_pointer const_pointer;
				typedef typename std_vector::reference reference;
				typedef typename std_vector::const_reference const_reference;

				typedef typename std_vector::iterator iterator;
				typedef typename std_vector::const_iterator const_iterator;

				typedef typename std_vector::reverse_iterator reverse_iterator;
				typedef typename std_vector::const_reverse_iterator const_reverse_iterator;

			private:
				const _MV& contained_vector() const& { return (*this).value(); }
				//const _MV& contained_vector() const&& { return (*this).value(); }
				_MV& contained_vector() & { return (*this).value(); }
				_MV&& contained_vector() && {
					/* We're making sure that the vector is not "structure locked", because in that case it might not be
					safe to to allow the contained vector to be moved from (when made movable with std::move()). */
					//structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					return std::move(*this).value();
				}
				const _MV& unchecked_contained_vector() const& { return (*this).value(); }
				const _MV&& unchecked_contained_vector() const&& { return (*this).value(); }
				_MV& unchecked_contained_vector()& { return (*this).value(); }
				_MV&& unchecked_contained_vector()&& { return std::move(*this).value(); }

			public:
				explicit gnii_vector(const _A& _Al = _A())
					: base_class(_Al) {
					/*m_debug_size = size();*/
				}
				explicit gnii_vector(size_type _N)
					: base_class(msev_as_a_size_t(_N)) {
					/*m_debug_size = size();*/
				}
				explicit gnii_vector(size_type _N, const _Ty& _V, const _A& _Al = _A())
					: base_class(msev_as_a_size_t(_N), _V, _Al) {
					/*m_debug_size = size();*/
				}
				gnii_vector(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
				gnii_vector(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
				gnii_vector(_Myt&& _X) : base_class(CWriteLockedSrc<_Myt>(MSE_FWD(_X)).ref().contained_vector()) { /*m_debug_size = size();*/ }
				gnii_vector(_Myt& _X) : base_class(CNoopOrReadLockedSrcRefHolder<typename std::is_trivially_copy_constructible<_Ty>::type, _Myt>(_X).ref().contained_vector()) { /*m_debug_size = size();*/ }
				gnii_vector(const _Myt& _X) : base_class(CNoopOrReadLockedSrcRefHolder<typename std::is_trivially_copy_constructible<_Ty>::type, _Myt>(_X).ref().contained_vector()) {
					/* If you get a compile error here: User-defined copy constructors are potentially dangerous. We can ensure safety by
					"structure locking" the source vector, but some vector types (like nii_vector<>) do not support being locked through a
					const reference, so this (const reference) copy constructor is not supported for those vectors. */
					mse::impl::T_valid_if_true_type_pb<typename mse::impl::disjunction<std::is_trivially_copy_constructible<_Ty>, std::is_same<TConstLockableIndicator, gnii_vector_const_lockable_tag> >::type>(); /*m_debug_size = size();*/ }
				typedef typename std_vector::const_iterator _It;
				/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
				gnii_vector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
				gnii_vector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
				template<class _Iter
					//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
				gnii_vector(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
				template<class _Iter
					//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					//gnii_vector(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
				gnii_vector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
				_Myt& operator=(_Myt&& _X) {
					if (std::addressof(_X) == this) { return (*this); }
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					structure_change_guard<decltype(_X.m_structure_change_mutex)> lock(_X.m_structure_change_mutex);
					contained_vector().operator=(MSE_FWD(_X).contained_vector());
					return (*this);
				}
				_Myt& operator=(_Myt& _X) {
					if (std::addressof(_X) == this) { return (*this); }
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					noop_or_structure_no_change_guard<typename mse::impl::conjunction<std::is_trivially_copy_constructible<_Ty>, std::is_trivially_move_constructible<_Ty>, std::is_trivially_destructible<_Ty> >::type, decltype(_X.m_structure_change_mutex)> lock(_X.m_structure_change_mutex);
					contained_vector().operator=(_X.contained_vector());
					return (*this);
				}
				_Myt& operator=(const _Myt& _X) {
					if (std::addressof(_X) == this) { return (*this); }
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);

					/* If you get a compile error here: User-defined copy constructors are potentially dangerous. We can ensure safety by
					"structure locking" the source vector, but some vector types (like nii_vector<>) do not support being locked through a
					const reference, so this (const reference) copy assignment operator is not supported for those vectors. */
					mse::impl::T_valid_if_true_type_pb<typename mse::impl::disjunction<mse::impl::conjunction<std::is_trivially_copy_constructible<_Ty>, std::is_trivially_move_constructible<_Ty>, std::is_trivially_destructible<_Ty> >
						, std::is_same<TConstLockableIndicator, gnii_vector_const_lockable_tag> >::type>();
					noop_or_structure_no_change_guard<typename mse::impl::conjunction<std::is_trivially_copy_constructible<_Ty>, std::is_trivially_move_constructible<_Ty>, std::is_trivially_destructible<_Ty> >::type, decltype(_X.m_structure_change_mutex)> lock(_X.m_structure_change_mutex);
					contained_vector().operator=(_X.contained_vector());
					return (*this);
				}

				~gnii_vector() {
					mse::impl::destructor_lock_guard1<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);

					/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
					//valid_if_Ty_is_not_an_xscope_type();
					valid_if_Ty_is_not_bool();
				}

				operator _MV() const & { return _MV(CNoopOrReadLockedSrcRefHolder<typename std::is_trivially_copy_constructible<_Ty>::type, _Myt>(*this).ref().contained_vector()); }
				operator _MV()&& { return _MV(CWriteLockedSrc<_Myt>(std::move(*this)).ref().contained_vector()); }

				void reserve(size_type _Count)
				{	// determine new minimum length of allocated storage
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().reserve(msev_as_a_size_t(_Count));
				}
				size_type capacity() const _NOEXCEPT
				{	// return current length of allocated storage
					return contained_vector().capacity();
				}
				void shrink_to_fit() {	// reduce capacity
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().shrink_to_fit();
				}
				void resize(size_type _N, const _Ty& _X = _Ty()) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().resize(msev_as_a_size_t(_N), _X);
				}
				typename std_vector::const_reference operator[](msev_size_t _P) const {
					return (*this).at(msev_as_a_size_t(_P));
				}
				typename std_vector::reference operator[](msev_size_t _P) {
					return (*this).at(msev_as_a_size_t(_P));
				}
				typename std_vector::reference front() {	// return first element of mutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::reference front() - gnii_vector")); }
					return contained_vector().front();
				}
				typename std_vector::const_reference front() const {	// return first element of nonmutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::const_reference front() - gnii_vector")); }
					return contained_vector().front();
				}
				typename std_vector::reference back() {	// return last element of mutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::reference back() - gnii_vector")); }
					return contained_vector().back();
				}
				typename std_vector::const_reference back() const {	// return last element of nonmutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::const_reference back() - gnii_vector")); }
					return contained_vector().back();
				}
				void push_back(_Ty&& _X) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().push_back(MSE_FWD(_X));
				}
				void push_back(const _Ty& _X) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().push_back(_X);
				}
				void pop_back() {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().pop_back();
				}
				void assign(_It _F, _It _L) {
					smoke_check_source_iterators(_F, _L);
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().assign(_F, _L);
					/*m_debug_size = size();*/
				}
				template<class _Iter>
				void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
					smoke_check_source_iterators(_First, _Last);
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().assign(_First, _Last);
					/*m_debug_size = size();*/
				}
				void assign(size_type _N, const _Ty& _X = _Ty()) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().assign(msev_as_a_size_t(_N), _X);
					/*m_debug_size = size();*/
				}

				template<class _Iter>
				static void smoke_check_source_iterators_helper(std::true_type, const _Iter& _First, const _Iter& _Last) {
					if (_Last < _First)/*comparison operations should also verify that safe iterators point to the same container*/ {
						MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - gnii_vector"));
					}
				}
				template<class _Iter>
				static void smoke_check_source_iterators_helper(std::false_type, const _Iter&, const _Iter&) {}

				template<class _Iter>
				static void smoke_check_source_iterators(const gnii_vector& target_cref, const _Iter& _First, const _Iter& _Last) {
#ifndef MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK
					if (!(target_cref.empty())) {
						/* check if the source sequence is part of target (target) container */
						auto start_of_target_ptr = std::addressof(*(target_cref.cbegin()));
						auto end_of_target_ptr = std::addressof(*(target_cref.cend() - 1)) + 1;
						auto _First_ptr = std::addressof(*_First);
						if ((end_of_target_ptr > _First_ptr) && (start_of_target_ptr <= _First_ptr)) {
							MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - gnii_vector"));
						}
					}
#endif // !MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK

#ifdef MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
					smoke_check_source_iterators_helper(typename mse::impl::HasOrInheritsLessThanOperator_msemsevector<_Iter>::type(), _First, _Last);
#endif // MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
				}
				template<class _Iter>
				void smoke_check_source_iterators(const _Iter& _First, const _Iter& _Last) {
					smoke_check_source_iterators(*this, _First, _Last);
				}

				template<class ..._Valty>
				void emplace_back(_Valty&& ..._Val)
				{	// insert by moving into element at end
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
				}
				void clear() {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().clear();
					/*m_debug_size = size();*/
				}

				void swap(_Myt& _Other) {	// swap contents with _Other
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().swap(_Other.contained_vector());
				}
				void swap(_MV& _Other) {	// swap contents with _Other
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().swap(_Other);
				}
				template<typename _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2>
				void swap(mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex2, _TTXScopeConstIterator2, TConstLockableIndicator2>& _Other) {	// swap contents with _Other
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					contained_vector().swap(_Other.contained_vector());
				}

				size_type size() const _NOEXCEPT
				{	// return length of sequence
					return contained_vector().size();
				}

				size_type max_size() const _NOEXCEPT
				{	// return maximum possible length of sequence
					return contained_vector().max_size();
				}

				bool empty() const _NOEXCEPT
				{	// test if sequence is empty
					return contained_vector().empty();
				}
				_A get_allocator() const _NOEXCEPT
				{	// return allocator object for values
					return contained_vector().get_allocator();
				}

				reference at(msev_size_t _Pos)
				{	// subscript mutable sequence with checking
					return contained_vector().at(msev_as_a_size_t(_Pos));
				}

				const_reference at(msev_size_t _Pos) const
				{	// subscript nonmutable sequence with checking
					return contained_vector().at(msev_as_a_size_t(_Pos));
				}

				gnii_vector(_XSTD initializer_list<typename std_vector::value_type> _Ilist, const _A& _Al = _A())
					: base_class(_Ilist, _Al) {	// construct from initializer_list
					/*m_debug_size = size();*/
				}
				_Myt& operator=(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
					contained_vector().operator=(static_cast<std_vector>(_Ilist));
					return (*this);
				}
				void assign(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
					contained_vector().assign(_Ilist);
					/*m_debug_size = size();*/
				}

				MSE_DEPRECATED value_type *data() _NOEXCEPT
				{	// return pointer to mutable data vector
					return contained_vector().data();
				}

				MSE_DEPRECATED const value_type *data() const _NOEXCEPT
				{	// return pointer to nonmutable data vector
					return contained_vector().data();
				}

				//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
				//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

				typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
				typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

				template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
				using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
				template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
				using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

				template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
				using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
				template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
				using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

			private:
				/* ss_iterator_type is bounds checked, but not safe against "use-after-free", so the member functions that
				involve ss_iterator_type are made publicly inaccessible. They are used by friend type us::msevector<>,
				which is in turn used by mstd::vector<>. */

				ss_iterator_type ss_begin() {	// return std_vector::iterator for beginning of mutable sequence
					ss_iterator_type retval(this, 0);
					retval.set_to_beginning();
					return retval;
				}
				ss_const_iterator_type ss_begin() const {	// return std_vector::iterator for beginning of nonmutable sequence
					ss_const_iterator_type retval(this, 0);
					retval.set_to_beginning();
					return retval;
				}
				ss_iterator_type ss_end() {	// return std_vector::iterator for end of mutable sequence
					ss_iterator_type retval(this, 0);
					retval.set_to_end_marker();
					return retval;
				}
				ss_const_iterator_type ss_end() const {	// return std_vector::iterator for end of nonmutable sequence
					ss_const_iterator_type retval(this, 0);
					retval.set_to_end_marker();
					return retval;
				}
				ss_const_iterator_type ss_cbegin() const {	// return std_vector::iterator for beginning of nonmutable sequence
					ss_const_iterator_type retval(this, 0);
					retval.set_to_beginning();
					return retval;
				}
				ss_const_iterator_type ss_cend() const {	// return std_vector::iterator for end of nonmutable sequence
					ss_const_iterator_type retval(this, 0);
					retval.set_to_end_marker();
					return retval;
				}

				ss_const_reverse_iterator_type ss_crbegin() const {	// return std_vector::iterator for beginning of reversed nonmutable sequence
					return (ss_rbegin());
				}
				ss_const_reverse_iterator_type ss_crend() const {	// return std_vector::iterator for end of reversed nonmutable sequence
					return (ss_rend());
				}
				ss_reverse_iterator_type ss_rbegin() {	// return std_vector::iterator for beginning of reversed mutable sequence
					return (reverse_iterator(ss_end()));
				}
				ss_const_reverse_iterator_type ss_rbegin() const {	// return std_vector::iterator for beginning of reversed nonmutable sequence
					return (const_reverse_iterator(ss_end()));
				}
				ss_reverse_iterator_type ss_rend() {	// return std_vector::iterator for end of reversed mutable sequence
					return (reverse_iterator(ss_begin()));
				}
				ss_const_reverse_iterator_type ss_rend() const {	// return std_vector::iterator for end of reversed nonmutable sequence
					return (const_reverse_iterator(ss_begin()));
				}

				gnii_vector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
					: base_class(_Al) {
					/*m_debug_size = size();*/
					assign(start, end);
				}
				void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
					if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(gnii_vector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					if (start > end) { MSE_THROW(gnii_vector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					typename std_vector::const_iterator _F = start;
					typename std_vector::const_iterator _L = end;
					(*this).assign(_F, _L);
				}
				void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
					auto end = last;
					end++; // this should include some checks
					(*this).assign(first, end);
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid argument - void insert_before() - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).insert(_P, _M, _X);
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid argument - void insert_before() - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).insert(_P, MSE_FWD(_X));
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
				template<class _Iter
					//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid argument - ss_iterator_type insert_before() - gnii_vector")); }
					//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(gnii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).insert(_P, start, end);
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type& start, const ss_const_iterator_type &end) {
					if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(gnii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					end.assert_valid_index();
					if (start > end) { MSE_THROW(gnii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					typename std_vector::const_iterator _S = start;
					typename std_vector::const_iterator _E = end;
					return (*this).insert_before(pos, _S, _E);
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* end) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - ss_iterator_type insert_before() - gnii_vector")); }
					//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(gnii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - gnii_vector")); }
					if (start > end) { MSE_THROW(gnii_vector_range_error("invalid arguments - ss_iterator_type insert_before() - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).insert(_P, start, end);
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				template<class _Iter
					//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					ss_iterator_type insert_before_inclusive(const ss_iterator_type &pos, const _Iter &first, const _Iter &last) {
					auto end = last;
					end++; // this may include some checks
					return (*this).insert_before(pos, first, end);
				}
				ss_iterator_type insert_before(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// insert initializer_list
					if (pos.m_owner_ptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void insert_before() - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).insert(_P, _Ilist);
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				/* These insert() functions are just aliases for their corresponding insert_before() functions. */
				/*
				ss_iterator_type insert(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
				ss_iterator_type insert(const ss_const_iterator_type &pos, _Ty&& _X) { return insert_before(pos, MSE_FWD(_X)); }
				ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
				template<class _Iter
					//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					ss_iterator_type insert(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
				ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
				ss_iterator_type insert(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) { return insert_before(pos, _Ilist); }
				*/
				/*
				template<class ..._Valty>
				ss_iterator_type emplace(const ss_const_iterator_type &pos, _Valty&& ..._Val)
				{	// insert by moving _Val at pos
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void emplace() - gnii_vector")); }
					pos.assert_valid_index();
					msev_size_t original_pos = pos.position();
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).emplace(_P, std::forward<_Valty>(_Val)...);
					ss_iterator_type retval = ss_begin();
					retval += (msev_int(original_pos));
					return retval;
				}
				ss_iterator_type erase(const ss_const_iterator_type &pos) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					if (!pos.points_to_an_item()) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					auto pos_index = pos.position();

					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					(*this).erase(_P);

					ss_iterator_type retval = (*this).ss_begin();
					retval += (typename ss_const_iterator_type::difference_type(pos_index));
					return retval;
				}
				ss_iterator_type erase(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
					if (start.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					if (end.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					if (start.position() > end.position()) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					auto pos_index = start.position();

					typename std_vector::const_iterator _F = start;
					typename std_vector::const_iterator _L = end;
					(*this).erase(_F, _L);

					ss_iterator_type retval = (*this).ss_begin();
					retval += (typename ss_const_iterator_type::difference_type(pos_index));
					return retval;
				}
				*/
				ss_iterator_type erase_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
					auto end = last; end.set_to_next();
					return erase(first, end);
				}
				void erase_previous_item(const ss_const_iterator_type &pos) {
					if (pos.m_owner_cptr != this) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase_previous_item() - gnii_vector")); }
					if (!(pos.has_previous())) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase_previous_item() - gnii_vector")); }
					typename std_vector::const_iterator _P = pos.target_container_ptr()->contained_vector().cbegin() + pos.position();
					_P--;
					(*this).erase(_P);
				}

			public:

				typedef _Myt& _Myt_ref;

				/* Here we provide static versions of the member functions that return iterators. As static member functions do
				not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
				argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

				template<typename _TVectorPointer>
				static auto ss_begin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
					return_type retval(owner_ptr, 0);
					retval.set_to_beginning();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_end(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
					return_type retval(owner_ptr, 0);
					retval.set_to_end_marker();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_cbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
					retval.set_to_beginning();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto data(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					return ss_begin(owner_ptr);
				}

				template<typename _TVectorPointer>
				static auto ss_cend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
					retval.set_to_end_marker();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_rbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
					return return_type(ss_end<_TVectorPointer>(owner_ptr));
				}

				template<typename _TVectorPointer>
				static auto ss_rend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
					return return_type(ss_begin<_TVectorPointer>(owner_ptr));
				}

				template<typename _TVectorPointer>
				static auto ss_crbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_cend<_TVectorPointer>(owner_ptr)));
				}

				template<typename _TVectorPointer>
				static auto ss_crend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_crbegin<_TVectorPointer>(owner_ptr)));
				}

				template<typename _TVectorPointer1>
				static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
					if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - gnii_vector")); }
				}

				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, size_type pos, _Ty&& _X) {
					return (emplace(this_ptr, pos, MSE_FWD(_X)));
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, size_type pos, const _Ty& _X = _Ty()) {
					s_assert_valid_index(this_ptr, pos);
					msev_size_t original_pos = pos;
					typename std_vector::const_iterator _P = (*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).insert(_P, _X);
					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (msev_int(original_pos));
					return retval;
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, size_type pos, size_type _M, const _Ty& _X) {
					s_assert_valid_index(this_ptr, pos);
					msev_size_t original_pos = pos;
					typename std_vector::const_iterator _P = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).insert(_P, _M, _X);
					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (msev_int(original_pos));
					return retval;
				}
				template<typename _TVectorPointer1, class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
				static auto insert(_TVectorPointer1 this_ptr, size_type pos, const _Iter& _First, const _Iter& _Last) {
					s_assert_valid_index(this_ptr, pos);
					msev_size_t original_pos = pos;
					typename std_vector::const_iterator _P = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).insert(_P, _First, _Last);
					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (msev_int(original_pos));
					return retval;
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, size_type pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
					s_assert_valid_index(this_ptr, pos);
					msev_size_t original_pos = pos;
					typename std_vector::const_iterator _P = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).insert(_P, _Ilist);
					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (msev_int(original_pos));
					return retval;
				}
				template<typename _TVectorPointer1, class ..._Valty>
				static auto emplace(_TVectorPointer1 this_ptr, size_type pos, _Valty&& ..._Val)
				{	// insert by moving _Val at _Where
					s_assert_valid_index(this_ptr, pos);
					msev_size_t original_pos = pos;
					typename std_vector::const_iterator _P = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).emplace(_P, std::forward<_Valty>(_Val)...);
					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (msev_int(original_pos));
					return retval;
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, size_type pos) {
					s_assert_valid_index(this_ptr, pos);
					auto pos_index = pos;

					typename std_vector::const_iterator _P = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(pos);
					_Myt_ref(*this_ptr).erase(_P);

					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (typename decltype(retval)::difference_type(pos_index));
					return retval;
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, size_type start, size_type end) {
					if (start > end) { MSE_THROW(gnii_vector_range_error("invalid arguments - void erase() - gnii_vector")); }
					auto pos_index = start;

					typename std_vector::const_iterator _F = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(start);
					typename std_vector::const_iterator _L = _Myt_ref(*this_ptr).contained_vector().cbegin() + difference_type(end);
					_Myt_ref(*this_ptr).erase(_F, _L);

					auto retval = mse::make_begin_iterator(this_ptr);
					retval += (typename decltype(retval)::difference_type(pos_index));
					return retval;
				}

				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _Ty&& _X) {
					return insert(this_ptr, pos.position(), MSE_FWD(_X));
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, const _Ty& _X = _Ty()) {
					return insert(this_ptr, pos.position(), _X);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, size_type _M, const _Ty& _X) {
					return insert(this_ptr, pos.position(), _M, _X);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2, class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
				static auto insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, const _Iter& _First, const _Iter& _Last) {
					return insert(this_ptr, pos.position(), _First, _Last);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
					return insert(this_ptr, pos.position(), _Ilist);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2, class ..._Valty>
				static auto emplace(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _Valty&& ..._Val) {
					return emplace(this_ptr, pos.position(), std::forward<_Valty>(_Val)...);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto erase(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos) {
					return erase(this_ptr, pos.position());
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto erase(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& start, const Tss_const_iterator_type<_TVectorPointer2>& end) {
					return erase(this_ptr, start.position(), end.position());
				}

				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _Ty&& _X) {
					return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), MSE_FWD(_X));
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, const _Ty& _X = _Ty()) {
					return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _X);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, size_type _M, const _Ty& _X) {
					return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _M, _X);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2, class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
				static auto insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, const _Iter& _First, const _Iter& _Last) {
					return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _First, _Last);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
					return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _Ilist);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2, class ..._Valty>
				static auto emplace(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _Valty&& ..._Val) {
					return emplace(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), std::forward<_Valty>(_Val)...);
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto erase(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos) {
					return erase(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos));
				}
				template<typename _TVectorPointer1, typename _TVectorPointer2>
				static auto erase(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& start, const Tss_iterator_type<_TVectorPointer2>& end) {
					return erase(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(start), Tss_const_iterator_type<_TVectorPointer2>(end));
				}


				typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type<_Myt> xscope_ss_const_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_iterator_type<_Myt> xscope_ss_iterator_type;

				//typedef xscope_ss_const_iterator_type xscope_const_iterator;
				typedef _TTXScopeConstIterator<_Myt> xscope_const_iterator;
				//typedef xscope_ss_iterator_type xscope_iterator;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_iterator_type<_Myt> xscope_iterator;

				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, _Ty&& _X) {
					return insert(this_ptr, pos.position(), MSE_FWD(_X));
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, const _Ty& _X = _Ty()) {
					return insert(this_ptr, pos.position(), _X);
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, size_type _M, const _Ty& _X) {
					return insert(this_ptr, pos.position(), _M, _X);
				}
				template<typename _TVectorPointer1, class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
				static auto insert(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, const _Iter& _First, const _Iter& _Last) {
					return insert(this_ptr, pos.position(), _First, _Last);
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
					return insert(this_ptr, pos.position(), _Ilist);
				}
				template<typename _TVectorPointer1, class ..._Valty>
				static auto emplace(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos, _Valty&& ..._Val) {
					return emplace(this_ptr, pos.position(), std::forward<_Valty>(_Val)...);
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, const xscope_const_iterator& pos) {
					return erase(this_ptr, pos.position());
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, const xscope_const_iterator& start, const xscope_const_iterator& end) {
					return erase(this_ptr, start.position(), end.position());
				}

				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_iterator& pos, _Ty&& _X) {
					return insert(this_ptr, xscope_const_iterator(pos), MSE_FWD(_X));
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_iterator& pos, const _Ty& _X = _Ty()) {
					return insert(this_ptr, xscope_const_iterator(pos), _X);
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_iterator& pos, size_type _M, const _Ty& _X) {
					return insert(this_ptr, xscope_const_iterator(pos), _M, _X);
				}
				template<typename _TVectorPointer1, class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
				static auto insert(_TVectorPointer1 this_ptr, const xscope_iterator& pos, const _Iter& _First, const _Iter& _Last) {
					return insert(this_ptr, xscope_const_iterator(pos), _First, _Last);
				}
				template<typename _TVectorPointer1>
				static auto insert(_TVectorPointer1 this_ptr, const xscope_iterator& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
					return insert(this_ptr, xscope_const_iterator(pos), _Ilist);
				}
				template<typename _TVectorPointer1, class ..._Valty>
				static auto emplace(_TVectorPointer1 this_ptr, const xscope_iterator& pos, _Valty&& ..._Val) {
					return emplace(this_ptr, xscope_const_iterator(pos), std::forward<_Valty>(_Val)...);
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, const xscope_iterator& pos) {
					return erase(this_ptr, xscope_const_iterator(pos));
				}
				template<typename _TVectorPointer1>
				static auto erase(_TVectorPointer1 this_ptr, const xscope_iterator& start, const xscope_iterator& end) {
					return erase(this_ptr, xscope_const_iterator(start), xscope_const_iterator(end));
				}


				/* Note that uncommonly, the comparison operators have distinct const and non-const implementaions.
				This is because the container needs to be "locked" during the operation, and locking support is not
				necessarily the same between const and non-const function calls. */

				bool operator==(_Myt& _Right) {	// test for vector equality
					structure_no_change_guard<decltype(_Right.m_structure_change_mutex)> lock1(_Right.m_structure_change_mutex);
					structure_no_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
					return (_Right.contained_vector() == contained_vector());
				}
				bool operator==(const _Myt& _Right) const {	// test for vector equality
					/* If you get a compile error here: User-defined comparison operators are potentially dangerous. We can ensure safety by
					"structure locking" the source vector, but some vector types (like nii_vector<>) do not support being locked through a
					const reference, so this (const reference) comparison operator is not supported for those vectors. */
					mse::impl::T_valid_if_true_type_pb<typename std::is_same<TConstLockableIndicator, gnii_vector_const_lockable_tag>::type>();
					structure_no_change_guard<decltype(_Right.m_structure_change_mutex)> lock1(_Right.m_structure_change_mutex);
					structure_no_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);

					return (_Right.contained_vector() == contained_vector());
				}
				bool operator<(_Myt& _Right) {	// test if _Left < _Right for vectors
					structure_no_change_guard<decltype(_Right.m_structure_change_mutex)> lock1(_Right.m_structure_change_mutex);
					structure_no_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
					return (contained_vector() < _Right.contained_vector());
				}
				bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for vectors
					/* If you get a compile error here: User-defined comparison operators are potentially dangerous. We can ensure safety by
					"structure locking" the source vector, but some vector types (like nii_vector<>) do not support being locked through a
					const reference, so this (const reference) comparison operator is not supported for those vectors. */
					mse::impl::T_valid_if_true_type_pb<typename std::is_same<TConstLockableIndicator, gnii_vector_const_lockable_tag>::type>();
					structure_no_change_guard<decltype(_Right.m_structure_change_mutex)> lock1(_Right.m_structure_change_mutex);
					structure_no_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);

					return (contained_vector() < _Right.contained_vector());
				}
#ifndef MSE_HAS_CXX20
				bool operator!=(const _Myt& _Right_cref) const { return !((*this) == _Right_cref); }
				bool operator>(const _Myt& _Right_cref) const { return (_Right_cref < (*this)); }
				bool operator<=(const _Myt& _Right_cref) const { return !((*this) > _Right_cref); }
				bool operator>=(const _Myt& _Right_cref) const { return !((*this) < _Right_cref); }
#else // !MSE_HAS_CXX20
				std::strong_ordering operator<=>(const _Myt& _Right_cref) const = delete;
#endif // !MSE_HAS_CXX20

				template<class _Ty2, class _Traits2>
				std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr, size_type byte_count, const size_type byte_start_offset = 0) const {
					const auto array_size_in_bytes = mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size());
					auto byte_ptr = reinterpret_cast<const char *>((*this).contained_vector().data());
					if ((array_size_in_bytes <= byte_start_offset) || (0 >= byte_count)) {
						return _Ostr;
					}
					else {
						byte_ptr += mse::msev_as_a_size_t(byte_start_offset);
						return _Ostr.write(byte_ptr, std::min(mse::msev_as_a_size_t(array_size_in_bytes - byte_start_offset), mse::msev_as_a_size_t(byte_count)));
					}
				}
				template<class _Ty2, class _Traits2>
				std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr) const {
					return write_bytes(_Ostr, mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size()));
				}

				MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

			private:

				/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
				(intended) compile error. */
				template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
				void valid_if_Ty_is_not_an_xscope_type() const {}

				template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (!std::is_same<bool, _Ty2>::value)> MSE_IMPL_EIS >
				void valid_if_Ty_is_not_bool() const {}

				auto begin() { return contained_vector().begin(); }
				auto end() { return contained_vector().end(); }
				auto begin() const { return contained_vector().begin(); }
				auto end() const { return contained_vector().end(); }
				auto cbegin() const { return contained_vector().cbegin(); }
				auto cend() const { return contained_vector().cend(); }

				auto rbegin() { return contained_vector().rbegin(); }
				auto rend() { return contained_vector().rend(); }
				auto rbegin() const { return contained_vector().rbegin(); }
				auto rend() const { return contained_vector().rend(); }
				auto crbegin() const { return contained_vector().crbegin(); }
				auto crend() const { return contained_vector().crend(); }


				typename std_vector::iterator insert(typename std_vector::const_iterator _P, _Ty&& _X) {
					return (emplace(_P, MSE_FWD(_X)));
				}
				typename std_vector::iterator insert(typename std_vector::const_iterator _P, const _Ty& _X = _Ty()) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					typename std_vector::iterator retval = contained_vector().insert(_P, _X);
					/*m_debug_size = size();*/
					return retval;
				}
				typename std_vector::iterator insert(typename std_vector::const_iterator _P, size_type _M, const _Ty& _X) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					typename std_vector::iterator retval = contained_vector().insert(_P, msev_as_a_size_t(_M), _X);
					/*m_debug_size = size();*/
					return retval;
				}
				template<class _Iter
					//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					typename std_vector::iterator insert(typename std_vector::const_iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
					smoke_check_source_iterators(_First, _Last);
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					auto retval = contained_vector().insert(_Where, _First, _Last);
					/*m_debug_size = size();*/
					return retval;
				}
				template<class ..._Valty>
				typename std_vector::iterator emplace(typename std_vector::const_iterator _Where, _Valty&& ..._Val)
				{	// insert by moving _Val at _Where
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					auto retval = contained_vector().emplace(_Where, std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
					return retval;
				}
				typename std_vector::iterator erase(typename std_vector::const_iterator _P) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					typename std_vector::iterator retval = contained_vector().erase(_P);
					/*m_debug_size = size();*/
					return retval;
				}
				typename std_vector::iterator erase(typename std_vector::const_iterator _F, typename std_vector::const_iterator _L) {
					structure_change_guard<decltype(m_structure_change_mutex)> lock1(m_structure_change_mutex);
					typename std_vector::iterator retval = contained_vector().erase(_F, _L);
					/*m_debug_size = size();*/
					return retval;
				}

				typename std_vector::iterator insert(typename std_vector::const_iterator _Where, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// insert initializer_list
					auto retval = contained_vector().insert(_Where, _Ilist);
					/*m_debug_size = size();*/
					return retval;
				}

				template<class _Mutex>
				class structure_change_guard {
				public:
					structure_change_guard(_Mutex& _Mtx) MSE_FUNCTION_TRY : m_lock_guard(_Mtx) {}
					MSE_FUNCTION_CATCH_ANY {
						MSE_THROW(mse::structure_lock_violation_error("structure lock violation - Attempting to modify \
							the structure (size/capacity) of a container while a reference (iterator) to one of its elements \
							still exists?"));
					}
				private:
					std::lock_guard<_Mutex> m_lock_guard;
				};
				template<class _Mutex>
				class structure_no_change_guard {
				public:
					structure_no_change_guard(_Mutex& _Mtx) MSE_FUNCTION_TRY : m_lock_guard(_Mtx) {}
					MSE_FUNCTION_CATCH_ANY{
						MSE_THROW(mse::structure_lock_violation_error("structure lock violation - Attempting to reference \
							the contents of a container during an operation that potentially modifies its structure (size/capacity)?"));
					}
				private:
					std::shared_lock<_Mutex> m_lock_guard;
				};
				/* noop_or_structure_no_change_guard<> either does essentially nothing, or obtains and holds a lock that prevents the
				container's "structure" (size/capacity/location) from being modified. */
				template<class TNoopIndicator, class _Mutex>
				class noop_or_structure_no_change_guard : public structure_no_change_guard<_Mutex> {
				public:
					typedef structure_no_change_guard<_Mutex> base_class;
					noop_or_structure_no_change_guard(_Mutex& _Mtx) : base_class(_Mtx) {}
				};
				template<class _Mutex>
				class noop_or_structure_no_change_guard<std::true_type, _Mutex> {
				public:
					noop_or_structure_no_change_guard(_Mutex&) {}
				};
				/* The purpose of the CNoopOrReadLockedSrcRefHolder class is to either do essentially nothing (aka "no-op"), or obtain (and
				hold) a "structure" lock on the source object. Used in the copy constructor. */
				template<class TNoopIndicator, class _Ty2, class _TWrappedAccessMutex = decltype(std::declval<_Ty2>().m_structure_change_mutex)>
				class CNoopOrReadLockedSrcRefHolder : private noop_or_structure_no_change_guard<TNoopIndicator, _TWrappedAccessMutex> {
				public:
					CNoopOrReadLockedSrcRefHolder(const _Ty2& src) : noop_or_structure_no_change_guard<TNoopIndicator, _TWrappedAccessMutex>(src.m_structure_change_mutex), m_ptr(&src) {}
					const _Ty2& ref() const { return *m_ptr; }
				private:
					const _Ty2* m_ptr = nullptr;
				};
				/* The purpose of the CWriteLockedSrc class is to obtain (and hold) a "structure" lock to a source object about to be
				moved from. Used in the move constructor. */
				template<class _Ty2, class _TWrappedAccessMutex = decltype(std::declval<_Ty2>().m_structure_change_mutex)>
				class CWriteLockedSrc : private structure_change_guard<_TWrappedAccessMutex> {
				public:
					CWriteLockedSrc(_Ty2&& src) : structure_change_guard<_TWrappedAccessMutex>(src.m_structure_change_mutex), m_ref(MSE_FWD(src)) {}
					_Ty2&& ref() const { return MSE_FWD(m_ref); }
				private:
					_Ty2&& m_ref;
				};

				auto structure_change_lock() const { m_structure_change_mutex.lock(); }
				auto structure_change_unlock() const { m_structure_change_mutex.unlock(); }

				/* The "mutability" of m_structure_change_mutex is not actually required or utilized by this class, and thus
				doesn't compromise the safety of sharing this class among asynchronous thread. The mutability is utilized by
				derived (friend) classes (that will identify themselves as not safely shareable). */
				mutable _TStateMutex m_structure_change_mutex;

				typedef mse::impl::conditional_t<s_is_const_lockable, mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>
					, mse::us::impl::Txscope_shared_structure_lock_guard<_Myt> > xscope_shared_structure_lock_guard_t;
				static auto s_make_xscope_shared_structure_lock_guard(_Myt& vec_ref) -> xscope_shared_structure_lock_guard_t {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_shared_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(vec_ref));
				}
				template<class _Myt2 = _Myt, MSE_IMPL_EIP mse::impl::enable_if_t<(!std::is_same<_Myt2 const, _Myt>::value) && (s_is_const_lockable)> MSE_IMPL_EIS >
				static auto s_make_xscope_shared_structure_lock_guard(_Myt2 const & vec_ref) -> xscope_shared_structure_lock_guard_t {
					MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_shared_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(vec_ref));
				}
				//typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt> xscope_shared_const_structure_lock_guard_t;

				friend class mse::us::impl::Txscope_shared_structure_lock_guard<_Myt>;
				friend class mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>;
				template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::rsv::impl::ns_xslta_accessing_fixed_vector::xslta_accessing_fixed_vector_base2;
				template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::rsv::us::impl::xslta_accessing_fixed_vector_base;
				template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::impl::ns_xscope_accessing_fixed_nii_vector::xscope_accessing_fixed_nii_vector_base2;
				template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::us::impl::xscope_accessing_fixed_nii_vector_base;
				template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::xscope_accessing_fixed_nii_vector;

				friend /*class */xscope_ss_const_iterator_type;
				friend /*class */xscope_ss_iterator_type;
				template<class _Ty2, class _A2, class _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2> friend class gnii_vector;
				//friend class us::msevector<_Ty, _A>;
				template<class _Ty2, class _A2, class _TStateMutex2> friend class us::msevector;

				friend void swap(_Myt& a, _Myt& b) _NOEXCEPT_OP(_NOEXCEPT_OP(a.swap(b))) { a.swap(b); }
				friend void swap(_Myt& a, _MV& b) _NOEXCEPT_OP(_NOEXCEPT_OP(a.swap(b))) { a.swap(b); }
				friend void swap(_MV& a, _Myt& b) _NOEXCEPT_OP(_NOEXCEPT_OP(b.swap(a))) { b.swap(a); }
			};

#ifdef MSE_HAS_CXX17
			/* deduction guides */
			template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
			gnii_vector(_Iter, _Iter, _Alloc = _Alloc())
				->gnii_vector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */

			template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator> inline bool operator!=(const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left,
				const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) {	// test for vector inequality
				return (!(_Left == _Right));
			}

			template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator> inline bool operator>(const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left,
				const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) {	// test if _Left > _Right for vectors
				return (_Right < _Left);
			}

			template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator> inline bool operator<=(const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left,
				const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) {	// test if _Left <= _Right for vectors
				return (!(_Right < _Left));
			}

			template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator> inline bool operator>=(const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left,
				const mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) {	// test if _Left >= _Right for vectors
				return (!(_Left < _Right));
			}

			namespace ns_gnii_vector {
				/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
				individual elements in the vector do not become invalid by preventing any operation that might resize the vector
				or increase its capacity. Any attempt to execute such an operation would result in an exception. */
				/* Constructing an xscope_shared_structure_lock_guard involves a (shared) lock operation on the gnii_vector<>'s
				mutable m_structure_change_mutex. So in cases where m_structure_change_mutex is not thread safe, neither is
				the construction of xscope_shared_structure_lock_guard from a const reference. So while it's safe to expose
				generally xscope_shared_structure_lock_guards constructed from non-const references, it's up to the specific
				specializations and derived classes of gnii_vector<> to (optionally) expose xscope_shared_structure_lock_guards
				constructed from const references and ensure their safety by either indicating that they are not eligible to
				be shared between threads (like stnii_vector<> does), or ensuring that m_structure_change_mutex is thread safe
				(like mtnii_vector<> does). */
				template<class _TContainer>
				class xscope_shared_const_structure_lock_guard : public mse::us::impl::Txscope_shared_const_structure_lock_guard<_TContainer> {
				public:
					typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_TContainer> base_class;
					using base_class::base_class;

					operator mse::TXScopeFixedConstPointer<_TContainer>() const {
						return static_cast<const base_class&>(*this);
					}
					MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
				private:
					MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
				};
			}
		}
	}

	namespace impl {
		namespace ns_gnii_vector {
			/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			template<class _TContainer>
			class xscope_shared_structure_lock_guard : public mse::us::impl::Txscope_shared_structure_lock_guard<_TContainer> {
			public:
				typedef mse::us::impl::Txscope_shared_structure_lock_guard<_TContainer> base_class;
				typedef _TContainer TDynamicContainer;
				using base_class::base_class;

				operator mse::TXScopeFixedPointer<TDynamicContainer>() const {
					return static_cast<const base_class&>(*this);
				}
				/*
				template<class TDynamicContainer2 = TDynamicContainer, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::TXScopeFixedConstPointer<TDynamicContainer2>, mse::TXScopeFixedPointer<TDynamicContainer> >::value> MSE_IMPL_EIS >
				explicit operator mse::TXScopeFixedConstPointer<TDynamicContainer2>() const {
					return mse::TXScopeFixedConstPointer<TDynamicContainer2>(static_cast<const base_class&>(*this));
				}
				*/
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
			};

			/* For objects that are access controlled under an "exclusive writer" access policy, the object is immutable
			while a const pointer to the object exists. So given an "exclusive writer" const pointer to a vector, it is
			safe to provide a direct scope const pointer to any of its elements. */
			template<class _TContainer, class _TAccessMutex = mse::non_thread_safe_shared_mutex>
			class xscope_ewconst_structure_lock_guard : public mse::us::impl::Txscope_ewconst_structure_lock_guard<_TContainer, _TAccessMutex> {
			public:
				typedef mse::us::impl::Txscope_ewconst_structure_lock_guard<_TContainer, _TAccessMutex> base_class;
				using base_class::base_class;

				typedef mse::TAccessControlledConstPointer<_TContainer, _TAccessMutex> exclusive_writer_const_pointer_t;

				operator exclusive_writer_const_pointer_t() const {
					return static_cast<const base_class&>(*this);
				}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION
			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;
			};
		}
	}

	/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
	individual elements in the vector do not become invalid by preventing any operation that might resize the vector
	or increase its capacity. Any attempt to execute such an operation would result in an exception. */
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >& owner_ptr) -> mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> > {
		return mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >& owner_ptr) -> mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> > {
		return mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator> >(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_shared_structure_lock_guard)

	/* For objects that are access controlled under an "exclusive writer" access policy, the object is immutable
	while a const pointer to the object exists. So given an "exclusive writer" const pointer to a vector, it is
	safe to provide a direct scope const pointer to any of its elements. */
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator, class _TAccessMutex = mse::non_thread_safe_shared_mutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TAccessControlledConstPointer<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>, _TAccessMutex>& owner_ptr) -> mse::impl::ns_gnii_vector::xscope_ewconst_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>, _TAccessMutex> {
		return mse::impl::ns_gnii_vector::xscope_ewconst_structure_lock_guard<mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>, _TAccessMutex>(owner_ptr);
	}

	template<class _TDynamicContainerPointer>
	auto make_xscope_vector_size_change_lock_guard(const _TDynamicContainerPointer& owner_ptr) -> decltype(make_xscope_shared_structure_lock_guard(owner_ptr)) {
		return make_xscope_shared_structure_lock_guard(owner_ptr);
	}
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_vector_size_change_lock_guard)
	namespace mstd {
		template<class _TDynamicContainerPointer>
		auto make_xscope_vector_size_change_lock_guard(const _TDynamicContainerPointer& owner_ptr) -> decltype(make_xscope_shared_structure_lock_guard(owner_ptr)) {
			return make_xscope_shared_structure_lock_guard(owner_ptr);
		}
		/* Overloads for rsv::TReturnableFParam<>. */
		MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_vector_size_change_lock_guard)
	}
	namespace us {
		template<class _TDynamicContainerPointer>
		auto make_xscope_vector_size_change_lock_guard(const _TDynamicContainerPointer& owner_ptr) -> decltype(make_xscope_shared_structure_lock_guard(owner_ptr)) {
			return make_xscope_shared_structure_lock_guard(owner_ptr);
		}
		/* Overloads for rsv::TReturnableFParam<>. */
		MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_vector_size_change_lock_guard)
	}
}

namespace std {

	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	void swap(mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left, mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{
		_Left.swap(_Right);
	}
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator, class _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<_TStateMutex, _TStateMutex2>::value> MSE_IMPL_EIS >
	void swap(mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left, mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex2, _TTXScopeConstIterator2, TConstLockableIndicator2>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{
		_Left.swap(_Right);
	}
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	void swap(vector<_Ty, _A>& _Left, mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap vectors
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _A, class _TStateMutex, template<typename> class _TTXScopeConstIterator, class TConstLockableIndicator>
	void swap(mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex, _TTXScopeConstIterator, TConstLockableIndicator>& _Left, vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{
		_Left.swap(_Right);
	}
}

namespace mse {

	/* This macro provides an implementation of a free function that just calls the corresponding member function. The implementation
	favors static member functions (over non-static ones) when available. */
#define MSE_IMPL_FREE_FUNCTION_INVOKING_POSSIBLY_STATIC_MEMBER_FUNCTION(function_name) \
	namespace impl { \
		template<class T, class EqualTo, class ..._Valty> \
		struct HasOrInheritsStatic##function_name##Method_impl \
		{ \
			template<class U, class V> \
			static auto test(U* u) -> decltype(U::function_name(std::declval<_Valty>()...), std::declval<V*>(), bool(true)); \
			template<typename, typename> \
			static auto test(...)->std::false_type; \
			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type; \
		}; \
		template<class T, class ..._Valty> \
		struct HasOrInheritsStatic##function_name##Method : HasOrInheritsStatic##function_name##Method_impl< \
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<T>, _Valty...>::type {}; \
 \
		template<typename _TContainerPointer1, class ..._Valty> \
		static auto function_name##_helper1(std::true_type, const _TContainerPointer1& this_ptr, _Valty&& ..._Val) \
		{ \
			return this_ptr->function_name(this_ptr, std::forward<_Valty>(_Val)...); \
		} \
		template<typename _TContainerPointer1, class ..._Valty> \
		static auto function_name##_helper1(std::false_type, const _TContainerPointer1& this_ptr, _Valty&& ..._Val) \
		{ \
			return this_ptr->function_name(std::forward<_Valty>(_Val)...); \
		} \
	} \
	template<typename _TContainerPointer1, class ..._Valty> \
	static auto function_name(const _TContainerPointer1& this_ptr, _Valty&& ..._Val) \
	{ \
		return impl::function_name##_helper1(typename impl::HasOrInheritsStatic##function_name##Method<impl::target_type<_TContainerPointer1>, _TContainerPointer1, _Valty...>::type(), this_ptr, std::forward<_Valty>(_Val)...); \
	} \
	template<typename _TContainerPointer1, class _Ty> \
	static auto function_name(const _TContainerPointer1& this_ptr, const std::initializer_list<_Ty>& il) \
	{ \
		return impl::function_name##_helper1(typename impl::HasOrInheritsStatic##function_name##Method<impl::target_type<_TContainerPointer1>, _TContainerPointer1, const std::initializer_list<_Ty> >::type(), this_ptr, il); \
	} \
	template<typename _TContainerPointer1, class _TPos, class _Ty> \
	static auto function_name(const _TContainerPointer1& this_ptr, const _TPos& pos, const std::initializer_list<_Ty>& il) \
	{ \
		return impl::function_name##_helper1(typename impl::HasOrInheritsStatic##function_name##Method<impl::target_type<_TContainerPointer1>, _TContainerPointer1, _TPos, const std::initializer_list<_Ty> >::type(), this_ptr, pos, il); \
	}

	/* The emplace(), insert() and erase() member functions (often) return an iterator. Some of the safe vectors don't support "implicit"
	iterators (i.e. iterators generated from the implicit "this" pointer) and so make them static member functions that take an explicit
	(safe) "this" pointer parameter in order to generate and return an "explicit" iterator. */
	MSE_IMPL_FREE_FUNCTION_INVOKING_POSSIBLY_STATIC_MEMBER_FUNCTION(emplace);
	MSE_IMPL_FREE_FUNCTION_INVOKING_POSSIBLY_STATIC_MEMBER_FUNCTION(insert);
	MSE_IMPL_FREE_FUNCTION_INVOKING_POSSIBLY_STATIC_MEMBER_FUNCTION(erase);

	template<typename _TContainerPointer1, typename size_type, typename _Ty>
	void resize(const _TContainerPointer1& this_ptr, size_type _N, const _Ty& _X = _Ty()) {
		this_ptr->resize(_N, _X);
	}
	template<typename _TContainerPointer1, typename _Ty>
	void push_back(const _TContainerPointer1& this_ptr, _Ty&& _X) {
		this_ptr->push_back(MSE_FWD(_X));
	}
	template<typename _TContainerPointer1>
	void pop_back(const _TContainerPointer1& this_ptr) {
		this_ptr->pop_back();
	}
	template<typename _TContainerPointer1, class _Iter>
	void assign(const _TContainerPointer1& this_ptr, const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
		this_ptr->assign(_First, _Last);
	}
	template<typename _TContainerPointer1, typename size_type, typename _Ty>
	void assign(const _TContainerPointer1& this_ptr, size_type _N, const _Ty& _X = _Ty()) {
		this_ptr->assign(_N, _X);
	}

	template<typename _TContainerPointer1, class ..._Valty>
	void emplace_back(const _TContainerPointer1& this_ptr, _Valty&& ..._Val)
	{	// insert by moving into element at end
		this_ptr->emplace_back(std::forward<_Valty>(_Val)...);
	}
	template<typename _TContainerPointer1>
	void clear(const _TContainerPointer1& this_ptr) {
		this_ptr->clear();
	}
	template<typename _TContainerPointer1, typename _Ty>
	void assign(const _TContainerPointer1& this_ptr, _XSTD initializer_list<_Ty> _Ilist) {	// assign initializer_list
		this_ptr->assign(_Ilist);
	}


	/* nii_vector<> is a vector that is eligible to be shared among threads and does not support implicit iterators. */
	template<class _Ty, class _A = std::allocator<_Ty> >
	//using nii_vector = mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex>;
	class nii_vector : public mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex> {
	public:
		typedef mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex> base_class;
		typedef mse::non_thread_safe_shared_mutex _TStateMutex;
		typedef nii_vector _Myt;

		typedef typename base_class::allocator_type allocator_type;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		typedef typename base_class::iterator iterator;
		typedef typename base_class::const_iterator const_iterator;
		typedef typename base_class::reverse_iterator reverse_iterator;
		typedef typename base_class::const_reverse_iterator const_reverse_iterator;

		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_iterator_type = typename base_class::template Tss_const_iterator_type<_TVectorConstPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_iterator_type = typename base_class::template Tss_iterator_type<_TVectorPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_reverse_iterator_type = typename base_class::template Tss_reverse_iterator_type<_TVectorPointer>;
		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_reverse_iterator_type = typename base_class::template Tss_const_reverse_iterator_type<_TVectorConstPointer>;
		typedef typename base_class::ss_iterator_type ss_iterator_type;
		typedef typename base_class::ss_const_iterator_type ss_const_iterator_type;
		typedef typename base_class::ss_reverse_iterator_type ss_reverse_iterator_type;
		typedef typename base_class::ss_const_reverse_iterator_type ss_const_reverse_iterator_type;

		typedef typename base_class::xscope_ss_const_iterator_type xscope_ss_const_iterator_type;
		typedef typename base_class::xscope_ss_iterator_type xscope_ss_iterator_type;

		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
		typedef typename base_class::xscope_const_iterator xscope_const_iterator;
		typedef typename base_class::xscope_iterator xscope_iterator;

		MSE_USING(nii_vector, base_class);

		nii_vector(_XSTD initializer_list<value_type> _Ilist, const _A& _Al = _A()) : base_class(_Ilist, _Al) {}

		nii_vector(const nii_vector&) = default;
		nii_vector(nii_vector&&) = default;

		~nii_vector() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			mse::impl::T_valid_if_not_an_xscope_type<_Ty>();
		}

		nii_vector& operator=(nii_vector&& _X) = default;
		nii_vector& operator=(const nii_vector& _X) = default;

		MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)");


	/* mtnii_vector<> is a vector that is eligible to be shared among threads and does not support implicit iterators. */
	template<class _Ty, class _A = std::allocator<_Ty> >
	//using mtnii_vector = mse::us::impl::gnii_vector<_Ty, _A, mse::shareable_dynamic_container_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag>;
	class mtnii_vector : public mse::us::impl::gnii_vector<_Ty, _A, mse::shareable_dynamic_container_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag> {
	public:
		typedef mse::us::impl::gnii_vector<_Ty, _A, mse::shareable_dynamic_container_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag> base_class;
		typedef mse::shareable_dynamic_container_mutex _TStateMutex;
		typedef mtnii_vector _Myt;

		typedef typename base_class::allocator_type allocator_type;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		typedef typename base_class::iterator iterator;
		typedef typename base_class::const_iterator const_iterator;
		typedef typename base_class::reverse_iterator reverse_iterator;
		typedef typename base_class::const_reverse_iterator const_reverse_iterator;

		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_iterator_type = typename base_class::template Tss_const_iterator_type<_TVectorConstPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_iterator_type = typename base_class::template Tss_iterator_type<_TVectorPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_reverse_iterator_type = typename base_class::template Tss_reverse_iterator_type<_TVectorPointer>;
		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_reverse_iterator_type = typename base_class::template Tss_const_reverse_iterator_type<_TVectorConstPointer>;
		typedef typename base_class::ss_iterator_type ss_iterator_type;
		typedef typename base_class::ss_const_iterator_type ss_const_iterator_type;
		typedef typename base_class::ss_reverse_iterator_type ss_reverse_iterator_type;
		typedef typename base_class::ss_const_reverse_iterator_type ss_const_reverse_iterator_type;

		typedef typename base_class::xscope_ss_const_iterator_type xscope_ss_const_iterator_type;
		typedef typename base_class::xscope_ss_iterator_type xscope_ss_iterator_type;

		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
		typedef typename base_class::xscope_const_iterator xscope_const_iterator;
		typedef typename base_class::xscope_iterator xscope_iterator;

		MSE_USING(mtnii_vector, base_class);

		mtnii_vector(_XSTD initializer_list<value_type> _Ilist, const _A& _Al = _A()) : base_class(_Ilist, _Al) {}

		mtnii_vector(const mtnii_vector&) = default;
		mtnii_vector(mtnii_vector&&) = default;

		~mtnii_vector() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			mse::impl::T_valid_if_not_an_xscope_type<_Ty>();
		}

		mtnii_vector& operator=(mtnii_vector&& _X) = default;
		mtnii_vector& operator=(const mtnii_vector& _X) = default;

		MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)");

	namespace impl {
		namespace ns_mtnii_vector {
			/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			/* The following xscope_shared_const_structure_lock_guard constructed from a const reference is thread safe because
			mtnii_vector<> is uses an atomic "state mutex". */
			template<class _TContainer>
			using xscope_shared_const_structure_lock_guard = mse::us::impl::ns_gnii_vector::xscope_shared_const_structure_lock_guard<typename _TContainer::base_class>;

			template<class _TContainer>
			using xscope_shared_structure_lock_guard = mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<typename _TContainer::base_class>;
		}

		template<class _Ty, class _A>
		struct can_be_structure_locked_as_const<mse::mtnii_vector<_Ty, _A> > : std::true_type {};
	}

	/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
	individual elements in the vector do not become invalid by preventing any operation that might resize the vector
	or increase its capacity. Any attempt to execute such an operation would result in an exception. */
	/* The returned xscope_shared_structure_lock_guard constructed from a const reference is only safe because
	mtnii_vector<> is not eligible to be shared between threads. */
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<mtnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_mtnii_vector::xscope_shared_const_structure_lock_guard<mtnii_vector<_Ty, _A> >(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedConstPointer<mtnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_mtnii_vector::xscope_shared_const_structure_lock_guard<mtnii_vector<_Ty, _A> >(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<mtnii_vector<mtnii_vector<_Ty, _A> > >& owner_ptr) {
		return mse::impl::ns_mtnii_vector::xscope_shared_structure_lock_guard<mtnii_vector<_Ty, _A> >(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<mtnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_mtnii_vector::xscope_shared_structure_lock_guard<mtnii_vector<_Ty, _A> >(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#ifdef MSE_HAS_CXX20
	template<typename _Ty>
	std::strong_ordering spaceship_operator_equivalent(const _Ty& _Left_cref, const _Ty& _Right_cref) {
		signed char res = 0;
		if (!(_Left_cref == _Right_cref)) {
			if (_Left_cref < _Right_cref) {
				res = -1;
			}
			else {
				res = 1;
			}
		}
		return (res <=> 0); /* that's the right order, right? */
	}
#endif // MSE_HAS_CXX20

	/* stnii_basic_vector<> is a "low-overhead" vector that is not eligible to be shared among threads and does not
	support implicit iterators. */
	template<class _Ty, class _A = std::allocator<_Ty> >
	class stnii_vector : public mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag>, public us::impl::AsyncNotShareableTagBase {
	public:
		typedef mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag> base_class;
		typedef mse::non_thread_safe_shared_mutex _TStateMutex;
		typedef stnii_vector _Myt;

		typedef typename base_class::allocator_type allocator_type;
		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

		typedef typename base_class::iterator iterator;
		typedef typename base_class::const_iterator const_iterator;
		typedef typename base_class::reverse_iterator reverse_iterator;
		typedef typename base_class::const_reverse_iterator const_reverse_iterator;

		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_iterator_type = typename base_class::template Tss_const_iterator_type<_TVectorConstPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_iterator_type = typename base_class::template Tss_iterator_type<_TVectorPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_reverse_iterator_type = typename base_class::template Tss_reverse_iterator_type<_TVectorPointer>;
		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_reverse_iterator_type = typename base_class::template Tss_const_reverse_iterator_type<_TVectorConstPointer>;
		typedef typename base_class::ss_iterator_type ss_iterator_type;
		typedef typename base_class::ss_const_iterator_type ss_const_iterator_type;
		typedef typename base_class::ss_reverse_iterator_type ss_reverse_iterator_type;
		typedef typename base_class::ss_const_reverse_iterator_type ss_const_reverse_iterator_type;

		typedef typename base_class::xscope_ss_const_iterator_type xscope_ss_const_iterator_type;
		typedef typename base_class::xscope_ss_iterator_type xscope_ss_iterator_type;

		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
		typedef typename base_class::xscope_const_iterator xscope_const_iterator;
		typedef typename base_class::xscope_iterator xscope_iterator;

		MSE_USING(stnii_vector, base_class);

		stnii_vector(_XSTD initializer_list<value_type> _Ilist, const _A& _Al = _A()) : base_class(_Ilist, _Al) {}

		stnii_vector(const stnii_vector&) = default;
		stnii_vector(stnii_vector&&) = default;

		~stnii_vector() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			mse::impl::T_valid_if_not_an_xscope_type<_Ty>();
		}

		stnii_vector& operator=(stnii_vector&& _X) = default;
		stnii_vector& operator=(const stnii_vector& _X) = default;

		/* We don't want just to inherit the comparison operators because the base class (template) has a deleted
		<=> operator and distinct const and non-const implementations that aren't entirely appropriate for this
		class. */
		friend bool operator==(const _Myt& _Left_cref, const _Myt& _Right_cref) {
			const base_class& lhs = _Left_cref;
			const base_class& rhs = _Right_cref;
			return (lhs == rhs);
		}
		MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(stnii_vector)
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

	namespace impl {
		namespace ns_stnii_vector {
			/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			/* The following xscope_shared_const_structure_lock_guard constructed from a const reference is only safe because
			stnii_vector<> is not eligible to be shared between threads. */
			template<class _TContainer>
			using xscope_shared_const_structure_lock_guard = mse::us::impl::ns_gnii_vector::xscope_shared_const_structure_lock_guard<typename _TContainer::base_class>;

			template<class _TContainer>
			using xscope_shared_structure_lock_guard = mse::impl::ns_gnii_vector::xscope_shared_structure_lock_guard<typename _TContainer::base_class>;
		}

		template<class _Ty, class _A>
		struct can_be_structure_locked_as_const<mse::stnii_vector<_Ty, _A> > : std::true_type {};
	}

	/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
	individual elements in the vector do not become invalid by preventing any operation that might resize the vector
	or increase its capacity. Any attempt to execute such an operation would result in an exception. */
	/* The returned xscope_shared_structure_lock_guard constructed from a const reference is only safe because
	stnii_vector<> is not eligible to be shared between threads. */
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<stnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_stnii_vector::xscope_shared_const_structure_lock_guard<stnii_vector<_Ty, _A> >(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedConstPointer<stnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_stnii_vector::xscope_shared_const_structure_lock_guard<stnii_vector<_Ty, _A> >(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<stnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_stnii_vector::xscope_shared_structure_lock_guard<stnii_vector<_Ty, _A> >(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<stnii_vector<_Ty, _A> >& owner_ptr) {
		return mse::impl::ns_stnii_vector::xscope_shared_structure_lock_guard<stnii_vector<_Ty, _A> >(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	namespace impl {
		template<class T, class EqualTo>
		struct HasAllocatorMemberType_vc_impl
		{
			template<class U, class V>
			static auto test(U* u) -> decltype(std::declval<typename U::allocator_type>(), std::declval<V>(), bool(true));
			template<typename, typename>
			static auto test(...) -> std::false_type;

			static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
			using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
		};
		template<class T, class EqualTo = T>
		struct HasAllocatorMemberType_vc : HasAllocatorMemberType_vc_impl<
			mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

		template<class TF, class _TContainer>
		struct container_allocator_type_helper {
			typedef std::allocator<container_element_type<_TContainer> > type;
		};
		template<class _TContainer>
		struct container_allocator_type_helper<std::true_type, _TContainer> {
			typedef typename _TContainer::allocator_type type;
		};

		template<class _TContainer>
		using container_allocator_type_if_available = typename container_allocator_type_helper<typename HasAllocatorMemberType_vc<_TContainer>::type, _TContainer>::type;
	}

	namespace rsv {
		template<class _Ty, class _A = std::allocator<_Ty> > class xslta_vector;
	}

	template<class _Ty, class _A>
	class fixed_nii_vector;
	template<class _Ty, class _A>
	class xscope_fixed_nii_vector;

	namespace us {
		namespace impl {
			template<class _Ty, class _A = std::allocator<_Ty> >
			class fixed_nii_vector_base : private mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> >, private container_adjusted_default_state_mutex<_Ty>, public us::impl::ContiguousSequenceStaticStructureContainerTagBase {
			public:
				typedef container_adjusted_default_state_mutex<_Ty> state_mutex_t;
				typedef state_mutex_t _TStateMutex;

				/* We (privately) inherit the underlying data type rather than make it a data member to ensure it's the "first" component in the structure.*/
				typedef mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> > base_class;
				typedef std::vector<_Ty, _A> std_vector;
				typedef std_vector _MV;
				typedef fixed_nii_vector_base _Myt;

				typedef typename std_vector::allocator_type allocator_type;
				typedef typename std_vector::value_type value_type;
				//typedef typename std_vector::size_type size_type;
				typedef msev_size_t size_type;
				//typedef typename std_vector::difference_type difference_type;
				typedef msev_int difference_type;
				typedef typename std_vector::pointer pointer;
				typedef typename std_vector::const_pointer const_pointer;
				typedef typename std_vector::reference reference;
				typedef typename std_vector::const_reference const_reference;

				typedef typename std_vector::iterator iterator;
				typedef typename std_vector::const_iterator const_iterator;

				typedef typename std_vector::reverse_iterator reverse_iterator;
				typedef typename std_vector::const_reverse_iterator const_reverse_iterator;

			private:
				const _MV& contained_vector() const& { return (*this).value(); }
				//const _MV& contained_vector() const&& { return (*this).value(); }
				_MV& contained_vector()& { return (*this).value(); }
				_MV&& contained_vector()&& { return std::move(*this).value(); }

				state_mutex_t& state_mutex1()& { return (*this); }

			public:
				explicit fixed_nii_vector_base(const _A& _Al = _A())
					: base_class(_Al) {
					/*m_debug_size = size();*/
				}
				explicit fixed_nii_vector_base(size_type _N)
					: base_class(msev_as_a_size_t(_N)) {
					/*m_debug_size = size();*/
				}
				explicit fixed_nii_vector_base(size_type _N, const _Ty& _V, const _A& _Al = _A())
					: base_class(msev_as_a_size_t(_N), _V, _Al) {
					/*m_debug_size = size();*/
				}
				fixed_nii_vector_base(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
				fixed_nii_vector_base(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
				//fixed_nii_vector_base(_Myt&& _X) : base_class(_X.contained_vector()) { /*m_debug_size = size();*/ }
				fixed_nii_vector_base(const _Myt& _X) : base_class(_X.contained_vector()) { /*m_debug_size = size();*/ }
				typedef typename std_vector::const_iterator _It;
				/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
				fixed_nii_vector_base(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
				fixed_nii_vector_base(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
				template<class _Iter
					//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					fixed_nii_vector_base(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
				template<class _Iter
					//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
					, class = mse::impl::_mse_RequireInputIter<_Iter> >
					//fixed_nii_vector_base(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
					fixed_nii_vector_base(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

				~fixed_nii_vector_base() {
					mse::impl::destructor_lock_guard1<decltype(state_mutex1())> lock1(state_mutex1());

					/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
					//valid_if_Ty_is_not_an_xscope_type();
					valid_if_Ty_is_not_bool();
				}

				/* The returned vector should be a copy (rather than a move) even when this is an rvalue, as this vector is not allowed to be resized. */
				operator _MV() const { return contained_vector(); }

				typename std_vector::const_reference operator[](msev_size_t _P) const {
					return (*this).at(msev_as_a_size_t(_P));
				}
				typename std_vector::reference operator[](msev_size_t _P) {
					return (*this).at(msev_as_a_size_t(_P));
				}
				typename std_vector::reference front() {	// return first element of mutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::reference front() - fixed_nii_vector_base")); }
					return contained_vector().front();
				}
				typename std_vector::const_reference front() const {	// return first element of nonmutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::const_reference front() - fixed_nii_vector_base")); }
					return contained_vector().front();
				}
				typename std_vector::reference back() {	// return last element of mutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::reference back() - fixed_nii_vector_base")); }
					return contained_vector().back();
				}
				typename std_vector::const_reference back() const {	// return last element of nonmutable sequence
					if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::const_reference back() - fixed_nii_vector_base")); }
					return contained_vector().back();
				}

				size_type size() const _NOEXCEPT
				{	// return length of sequence
					return contained_vector().size();
				}

				bool empty() const _NOEXCEPT
				{	// test if sequence is empty
					return contained_vector().empty();
				}
				_A get_allocator() const _NOEXCEPT
				{	// return allocator object for values
					return contained_vector().get_allocator();
				}

				reference at(msev_size_t _Pos)
				{	// subscript mutable sequence with checking
					return contained_vector().at(msev_as_a_size_t(_Pos));
				}

				const_reference at(msev_size_t _Pos) const
				{	// subscript nonmutable sequence with checking
					return contained_vector().at(msev_as_a_size_t(_Pos));
				}

				fixed_nii_vector_base(_XSTD initializer_list<typename std_vector::value_type> _Ilist, const _A& _Al = _A())
					: base_class(_Ilist, _Al) {	// construct from initializer_list
					/*m_debug_size = size();*/
				}

				value_type* data() _NOEXCEPT
				{	// return pointer to mutable data vector
					return contained_vector().data();
				}

				const value_type* data() const _NOEXCEPT
				{	// return pointer to nonmutable data vector
					return contained_vector().data();
				}

				size_type max_size() const _NOEXCEPT
				{	// return maximum possible length of sequence
					return contained_vector().max_size();
				}

			private:
				_Myt& operator=(const std_vector& _X) {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().operator =(_X);
					/*m_debug_size = size();*/
					return (*this);
				}
				_Myt& operator=(_Myt&& _X) {
					if (std::addressof(_X) == this) { return (*this); }
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().operator=(MSE_FWD(_X).contained_vector());
					return (*this);
				}
				_Myt& operator=(const _Myt& _X) {
					if (std::addressof(_X) == this) { return (*this); }
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().operator=(_X.contained_vector());
					return (*this);
				}

				void reserve(size_type _Count)
				{	// determine new minimum length of allocated storage
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().reserve(msev_as_a_size_t(_Count));
				}
				size_type capacity() const _NOEXCEPT
				{	// return current length of allocated storage
					return contained_vector().capacity();
				}
				void shrink_to_fit() {	// reduce capacity
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().shrink_to_fit();
				}
				void resize(size_type _N, const _Ty& _X = _Ty()) {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().resize(msev_as_a_size_t(_N), _X);
				}

				void push_back(_Ty&& _X) {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().push_back(MSE_FWD(_X));
				}
				void push_back(const _Ty& _X) {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().push_back(_X);
				}
				void pop_back() {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().pop_back();
				}
				void assign(_It _F, _It _L) {
					smoke_check_source_iterators(_F, _L);
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().assign(_F, _L);
					/*m_debug_size = size();*/
				}
				template<class _Iter>
				void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
					smoke_check_source_iterators(_First, _Last);
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().assign(_First, _Last);
					/*m_debug_size = size();*/
				}
				void assign(size_type _N, const _Ty& _X = _Ty()) {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().assign(msev_as_a_size_t(_N), _X);
					/*m_debug_size = size();*/
				}


				template<class _Iter>
				static void smoke_check_source_iterators_helper(std::true_type, const _Iter& _First, const _Iter& _Last) {
					if (_Last < _First)/*comparison operations should also verify that safe iterators point to the same container*/ {
						MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - fixed_nii_vector_base"));
					}
				}
				template<class _Iter>
				static void smoke_check_source_iterators_helper(std::false_type, const _Iter&, const _Iter&) {}

				template<class _Iter>
				static void smoke_check_source_iterators(const fixed_nii_vector_base& target_cref, const _Iter& _First, const _Iter& _Last) {
#ifndef MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK
					if (!(target_cref.empty())) {
						/* check if the source sequence is part of target (target) container */
						auto start_of_target_ptr = std::addressof(*(target_cref.cbegin()));
						auto end_of_target_ptr = std::addressof(*(target_cref.cend() - 1)) + 1;
						auto _First_ptr = std::addressof(*_First);
						if ((end_of_target_ptr > _First_ptr) && (start_of_target_ptr <= _First_ptr)) {
							MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - fixed_nii_vector_base"));
						}
					}
#endif // !MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK

#ifdef MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
					smoke_check_source_iterators_helper(typename mse::impl::HasOrInheritsLessThanOperator_msemsevector<_Iter>::type(), _First, _Last);
#endif // MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
				}
				template<class _Iter>
				void smoke_check_source_iterators(const _Iter& _First, const _Iter& _Last) {
					smoke_check_source_iterators(*this, _First, _Last);
				}

				template<class ..._Valty>
				void emplace_back(_Valty&& ..._Val)
				{	// insert by moving into element at end
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
				}
				void clear() {
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().clear();
					/*m_debug_size = size();*/
				}

				void swap(_Myt& _Other) {	// swap contents with _Other
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().swap(_Other.contained_vector());
				}
				void swap(_MV& _Other) {	// swap contents with _Other
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().swap(_Other);
				}
				/*
				template<typename _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2>
				void swap(mse::us::impl::fixed_nii_vector_base<_Ty, _A, _TStateMutex2, _TTXScopeConstIterator2, TConstLockableIndicator2>& _Other) {	// swap contents with _Other
					//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
					contained_vector().swap(_Other.contained_vector());
				}
				*/

				_Myt& operator=(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
					contained_vector().operator=(static_cast<std_vector>(_Ilist));
					return (*this);
				}
				void assign(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
					contained_vector().assign(_Ilist);
					/*m_debug_size = size();*/
				}
			public:

				//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
				//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

				typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
				typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

				template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
				using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
				template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
				using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

				template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
				using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
				template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
				using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

				typedef _Myt& _Myt_ref;

				/* Here we provide static versions of the member functions that return iterators. As static member functions do
				not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
				argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

				template<typename _TVectorPointer>
				static auto ss_begin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
					return_type retval(owner_ptr, 0);
					retval.set_to_beginning();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_end(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
					return_type retval(owner_ptr, 0);
					retval.set_to_end_marker();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_cbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
					retval.set_to_beginning();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_cend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
					retval.set_to_end_marker();
					return retval;
				}

				template<typename _TVectorPointer>
				static auto ss_rbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
					return return_type(ss_end<_TVectorPointer>(owner_ptr));
				}

				template<typename _TVectorPointer>
				static auto ss_rend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
						, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
					return return_type(ss_begin<_TVectorPointer>(owner_ptr));
				}

				template<typename _TVectorPointer>
				static auto ss_crbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_cend<_TVectorPointer>(owner_ptr)));
				}

				template<typename _TVectorPointer>
				static auto ss_crend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
					mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
					return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_crbegin<_TVectorPointer>(owner_ptr)));
				}

				template<typename _TVectorPointer1>
				static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
					if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - fixed_nii_vector_base")); }
				}

				typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type<_Myt> xscope_ss_const_iterator_type;
				typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_iterator_type<_Myt> xscope_ss_iterator_type;

				//typedef xscope_ss_const_iterator_type xscope_const_iterator;
				//typedef _TTXScopeConstIterator<_Myt> xscope_const_iterator;
				//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
				//typedef xscope_ss_iterator_type xscope_iterator;
				//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_iterator_type<_Myt> xscope_iterator;
				typedef TXScopeCSSSXSRAConstIterator<_Myt> xscope_const_iterator;
				typedef TXScopeCSSSXSRAIterator<_Myt> xscope_iterator;

				friend bool operator==(const _Myt& _Left, const _Myt& _Right) {
					return (_Left.contained_vector() == _Right.contained_vector());
				}
				MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(fixed_nii_vector_base)
#ifndef MSE_HAS_CXX20
				friend bool operator<(const _Myt& _Left, const _Myt& _Right) {
					return (_Left.contained_vector() < _Right.contained_vector());
				}
#else // !MSE_HAS_CXX20
				friend std::strong_ordering operator<=>(const _Myt& _Left, const _Myt& _Right) {
					return (_Left.contained_vector() <=> _Right.contained_vector());
				}
#endif // !MSE_HAS_CXX20

				template<class _Ty2, class _Traits2>
				std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr, size_type byte_count, const size_type byte_start_offset = 0) const {
					const auto array_size_in_bytes = mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size());
					auto byte_ptr = reinterpret_cast<const char*>((*this).contained_vector().data());
					if ((array_size_in_bytes <= byte_start_offset) || (0 >= byte_count)) {
						return _Ostr;
					}
					else {
						byte_ptr += mse::msev_as_a_size_t(byte_start_offset);
						return _Ostr.write(byte_ptr, std::min(mse::msev_as_a_size_t(array_size_in_bytes - byte_start_offset), mse::msev_as_a_size_t(byte_count)));
					}
				}
				template<class _Ty2, class _Traits2>
				std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr) const {
					return write_bytes(_Ostr, mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size()));
				}

				MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

			private:

				/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
				(intended) compile error. */
				template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
				void valid_if_Ty_is_not_an_xscope_type() const {}

				template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (!std::is_same<bool, _Ty2>::value)> MSE_IMPL_EIS >
				void valid_if_Ty_is_not_bool() const {}


				friend class mse::fixed_nii_vector<_Ty, _A>;
				friend class mse::xscope_fixed_nii_vector<_Ty, _A>;

				friend /*class */xscope_ss_const_iterator_type;
				friend /*class */xscope_ss_iterator_type;
			};
		}
	}

	template<class _Ty, class _A = std::allocator<_Ty> >
	class fixed_nii_vector : public mse::us::impl::fixed_nii_vector_base<_Ty, _A> {
	public:
		typedef mse::us::impl::fixed_nii_vector_base<_Ty, _A> base_class;
		typedef std::vector<_Ty, _A> std_vector;
		typedef std_vector _MV;
		typedef fixed_nii_vector _Myt;

		typedef typename std_vector::allocator_type allocator_type;
		typedef typename std_vector::value_type value_type;
		//typedef typename std_vector::size_type size_type;
		typedef msev_size_t size_type;
		//typedef typename std_vector::difference_type difference_type;
		typedef msev_int difference_type;
		typedef typename std_vector::pointer pointer;
		typedef typename std_vector::const_pointer const_pointer;
		typedef typename std_vector::reference reference;
		typedef typename std_vector::const_reference const_reference;

		typedef typename std_vector::iterator iterator;
		typedef typename std_vector::const_iterator const_iterator;

		typedef typename std_vector::reverse_iterator reverse_iterator;
		typedef typename std_vector::const_reverse_iterator const_reverse_iterator;

	private:
		const _MV& contained_vector() const& { return base_class::contained_vector(); }
		//const _MV& contained_vector() const&& { return base_class::contained_vector(); }
		_MV& contained_vector()& { return base_class::contained_vector(); }
		_MV&& contained_vector()&& { return std::move(base_class::contained_vector()); }

	public:
		explicit fixed_nii_vector(const _A& _Al = _A()) : base_class(_Al) {}
		explicit fixed_nii_vector(size_type _N) : base_class(_N) {}
		explicit fixed_nii_vector(size_type _N, const _Ty& _V, const _A& _Al = _A()) : base_class(_N, _V, _Al) {}
		fixed_nii_vector(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
		fixed_nii_vector(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
		fixed_nii_vector(_Myt&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
		fixed_nii_vector(const _Myt& _X) : base_class(_X) { /*m_debug_size = size();*/ }
		typedef typename std_vector::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		fixed_nii_vector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
		fixed_nii_vector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
			, class = mse::impl::_mse_RequireInputIter<_Iter> >
		fixed_nii_vector(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
			, class = mse::impl::_mse_RequireInputIter<_Iter> >
			//fixed_nii_vector(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
		fixed_nii_vector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

		~fixed_nii_vector() {
			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_not_an_xscope_type();
		}

		fixed_nii_vector(_XSTD initializer_list<typename std_vector::value_type> _Ilist, const _A& _Al = _A())
			: base_class(_Ilist, _Al) {	// construct from initializer_list
			/*m_debug_size = size();*/
		}

		//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

		typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
		typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

		/* Here we provide static versions of the member functions that return iterators. As static member functions do
		not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
		argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

		typedef _Myt& _Myt_ref;

		template<typename _TArrayPointer>
		static auto ss_begin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_begin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_end(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_end(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_cbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_cbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_cend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_cend(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_rbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_rbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_rend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_rend(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_crbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_crbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_crend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_crend(owner_ptr);
		}

		template<typename _TVectorPointer1>
		static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
			if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - fixed_nii_vector")); }
		}

		typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type<_Myt> xscope_ss_const_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_iterator_type<_Myt> xscope_ss_iterator_type;

		//typedef xscope_ss_const_iterator_type xscope_const_iterator;
		//typedef _TTXScopeConstIterator<_Myt> xscope_const_iterator;
		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
		//typedef xscope_ss_iterator_type xscope_iterator;
		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_iterator_type<_Myt> xscope_iterator;
		typedef TXScopeCSSSXSRAConstIterator<_Myt> xscope_const_iterator;
		typedef TXScopeCSSSXSRAIterator<_Myt> xscope_iterator;

		MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

	private:

		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
		void valid_if_Ty_is_not_an_xscope_type() const {}

		friend /*class */xscope_ss_const_iterator_type;
		friend /*class */xscope_ss_iterator_type;
	} MSE_ATTR_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_Ty)");

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
	fixed_nii_vector(_Iter, _Iter, _Alloc = _Alloc())
		->fixed_nii_vector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */

	template<class _TLender, class _Ty, class _A>
	class xscope_borrowing_fixed_nii_vector;

	template<class _Ty, class _A = std::allocator<_Ty> >
	class xscope_fixed_nii_vector : public mse::us::impl::fixed_nii_vector_base<_Ty, _A>, public mse::us::impl::XScopeTagBase
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ReferenceableByScopePointerTagBase, _Ty, xscope_fixed_nii_vector<_Ty, _A> >
		, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty, xscope_fixed_nii_vector<_Ty, _A> >
	{
	public:
		typedef mse::us::impl::fixed_nii_vector_base<_Ty, _A> base_class;
		typedef std::vector<_Ty, _A> std_vector;
		typedef std_vector _MV;
		typedef xscope_fixed_nii_vector _Myt;

		typedef typename std_vector::allocator_type allocator_type;
		typedef typename std_vector::value_type value_type;
		//typedef typename std_vector::size_type size_type;
		typedef msev_size_t size_type;
		//typedef typename std_vector::difference_type difference_type;
		typedef msev_int difference_type;
		typedef typename std_vector::pointer pointer;
		typedef typename std_vector::const_pointer const_pointer;
		typedef typename std_vector::reference reference;
		typedef typename std_vector::const_reference const_reference;

		typedef typename std_vector::iterator iterator;
		typedef typename std_vector::const_iterator const_iterator;

		typedef typename std_vector::reverse_iterator reverse_iterator;
		typedef typename std_vector::const_reverse_iterator const_reverse_iterator;

	private:
		const _MV& contained_vector() const& { return base_class::contained_vector(); }
		//const _MV& contained_vector() const&& { return base_class::contained_vector(); }
		_MV& contained_vector()& { return base_class::contained_vector(); }
		_MV&& contained_vector()&& { return std::move(base_class::contained_vector()); }

	public:
		explicit xscope_fixed_nii_vector(const _A& _Al = _A()) : base_class(_Al) {}
		explicit xscope_fixed_nii_vector(size_type _N) : base_class(_N) {}
		explicit xscope_fixed_nii_vector(size_type _N, const _Ty& _V, const _A& _Al = _A()) : base_class(_N, _V, _Al) {}
		xscope_fixed_nii_vector(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
		xscope_fixed_nii_vector(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
		xscope_fixed_nii_vector(_Myt&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
		xscope_fixed_nii_vector(const _Myt& _X) : base_class(_X) { /*m_debug_size = size();*/ }
		typedef typename std_vector::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		xscope_fixed_nii_vector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
		xscope_fixed_nii_vector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
		, class = mse::impl::_mse_RequireInputIter<_Iter> >
		xscope_fixed_nii_vector(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
			, class = mse::impl::_mse_RequireInputIter<_Iter> >
		//xscope_fixed_nii_vector(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
		xscope_fixed_nii_vector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

		xscope_fixed_nii_vector(_XSTD initializer_list<typename std_vector::value_type> _Ilist, const _A& _Al = _A())
			: base_class(_Ilist, _Al) {	// construct from initializer_list
			/*m_debug_size = size();*/
		}

		//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

		typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
		typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

		template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
		using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
		template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
		using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

		typedef _Myt& _Myt_ref;

		/* Here we provide static versions of the member functions that return iterators. As static member functions do
		not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
		argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

		template<typename _TArrayPointer>
		static auto ss_begin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_begin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_end(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_end(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_cbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_cbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_cend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_cend(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_rbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_rbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_rend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_rend(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_crbegin(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_crbegin(owner_ptr);
		}
		template<typename _TArrayPointer>
		static auto ss_crend(const _TArrayPointer& owner_ptr) {
			return base_class::template ss_crend(owner_ptr);
		}

		template<typename _TVectorPointer1>
		static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
			if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - xscope_fixed_nii_vector")); }
		}

		typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type<_Myt> xscope_ss_const_iterator_type;
		typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_iterator_type<_Myt> xscope_ss_iterator_type;

		//typedef xscope_ss_const_iterator_type xscope_const_iterator;
		//typedef _TTXScopeConstIterator<_Myt> xscope_const_iterator;
		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_const_iterator_type<_Myt> xscope_const_iterator;
		//typedef xscope_ss_iterator_type xscope_iterator;
		//typedef mse::impl::ns_gnii_vector::Tgnii_vector_xscope_cslsstrong_iterator_type<_Myt> xscope_iterator;
		typedef TXScopeCSSSXSRAConstIterator<_Myt> xscope_const_iterator;
		typedef TXScopeCSSSXSRAIterator<_Myt> xscope_iterator;

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

	private:
		/* If _Ty is "marked" as containing a scope reference, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
			&& (mse::impl::potentially_does_not_contain_non_owning_scope_reference<_Ty2>::value)> MSE_IMPL_EIS >
		void valid_if_Ty_is_not_marked_as_containing_a_scope_reference() const {}

		/* If _Ty is "marked" as containing an accessible "scope address of" operator, then the following member function
		will not instantiate, causing an (intended) compile error. */
		template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
			&& (mse::impl::is_potentially_not_referenceable_by_scope_pointer<_Ty2>::value)
			> MSE_IMPL_EIS >
		void valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_addressof_operator() const {}

		friend /*class */xscope_ss_const_iterator_type;
		friend /*class */xscope_ss_iterator_type;
		template<class _TLender2, class _Ty2, class _A2> friend class xscope_borrowing_fixed_nii_vector;
	};

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
	xscope_fixed_nii_vector(_Iter, _Iter, _Alloc = _Alloc())
		->xscope_fixed_nii_vector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */

	template<class _TLender, class _Ty = mse::impl::container_element_type<_TLender>, class _A = std::allocator<_Ty>/*todo: replace with an mse::impl::container_allocator_type_if_available<_TLender>*/>
	class xscope_borrowing_fixed_nii_vector : public xscope_fixed_nii_vector<_Ty, _A>
		, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, xscope_fixed_nii_vector<_Ty, _A>, xscope_borrowing_fixed_nii_vector<_TLender, _Ty, _A> >
	{
	public:
		typedef xscope_fixed_nii_vector<_Ty, _A> base_class;

		typedef typename base_class::std_vector std_vector;
		typedef typename base_class::_MV _MV;
		typedef xscope_borrowing_fixed_nii_vector _Myt;

		typedef typename base_class::allocator_type allocator_type;
		typedef typename base_class::value_type value_type;
		typedef typename base_class::size_type size_type;
		typedef typename base_class::difference_type difference_type;
		typedef typename base_class::pointer pointer;
		typedef typename base_class::const_pointer const_pointer;
		typedef typename base_class::reference reference;
		typedef typename base_class::const_reference const_reference;

		typedef typename base_class::iterator iterator;
		typedef typename base_class::const_iterator const_iterator;

		typedef typename base_class::reverse_iterator reverse_iterator;
		typedef typename base_class::const_reverse_iterator const_reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_borrowing_fixed_nii_vector(xscope_borrowing_fixed_nii_vector&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_borrowing_fixed_nii_vector(xscope_borrowing_fixed_nii_vector&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		xscope_borrowing_fixed_nii_vector(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) : m_src_ref(*src_xs_ptr) {
			(*this).contained_vector() = std::move(m_src_ref);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		xscope_borrowing_fixed_nii_vector(_TLender* src_xs_ptr) : m_src_ref(*src_xs_ptr) {
			(*this).contained_vector() = std::move(m_src_ref);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		~xscope_borrowing_fixed_nii_vector() {
			m_src_ref = std::move((*this).contained_vector());
		}

		MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);

	private:
		xscope_borrowing_fixed_nii_vector(const xscope_borrowing_fixed_nii_vector&) = delete;

		_TLender& m_src_ref;
	};

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _TLender>
	xscope_borrowing_fixed_nii_vector(mse::TXScopeFixedPointer<_TLender>)->xscope_borrowing_fixed_nii_vector<_TLender>;
#endif /* MSE_HAS_CXX17 */

	template<class _TLender>
	auto make_xscope_borrowing_fixed_nii_vector(const mse::TXScopeFixedPointer<_TLender>& src_xs_ptr) {
		return mse::TXScopeObj<xscope_borrowing_fixed_nii_vector<_TLender> >(src_xs_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TLender>
	auto make_xscope_borrowing_fixed_nii_vector(_TLender* src_xs_ptr) {
		return mse::TXScopeObj<xscope_borrowing_fixed_nii_vector<_TLender> >(src_xs_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	namespace us {
		namespace impl {
			template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
			class xscope_accessing_fixed_nii_vector_base : public mse::us::impl::ContainsNonOwningScopeReferenceTagBase {
			public:
				typedef typename std::conditional<std::is_const<_TLender>::value, const _Ty, _Ty>::type const_adjusted_Ty;
				typedef mse::impl::target_type<_TPointerToLender> unchecked_contained_vector_t;

				typedef xscope_accessing_fixed_nii_vector_base _Myt;

				//typedef typename unchecked_contained_vector_t::allocator_type allocator_type;
				typedef typename unchecked_contained_vector_t::value_type value_type;
				typedef typename unchecked_contained_vector_t::size_type size_type;
				typedef typename unchecked_contained_vector_t::difference_type difference_type;
				typedef typename unchecked_contained_vector_t::const_pointer const_pointer;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_pointer, typename unchecked_contained_vector_t::pointer>::type pointer;
				typedef typename unchecked_contained_vector_t::const_reference const_reference;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_reference, typename unchecked_contained_vector_t::reference>::type reference;

				typedef typename unchecked_contained_vector_t::const_iterator const_iterator;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename unchecked_contained_vector_t::iterator>::type iterator;

				typedef typename unchecked_contained_vector_t::const_reverse_iterator const_reverse_iterator;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename unchecked_contained_vector_t::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				xscope_accessing_fixed_nii_vector_base(xscope_accessing_fixed_nii_vector_base&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				xscope_accessing_fixed_nii_vector_base(xscope_accessing_fixed_nii_vector_base&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

				typedef mse::impl::remove_const_t<_TLender> ncTLender;
				/* Some vectors support structure locking even via const reference. */
				typedef typename std::conditional<mse::impl::can_be_structure_locked_as_const<ncTLender>::value
					, ncTLender, _TLender>::type maybe_remove_const_lender_t;

				xscope_accessing_fixed_nii_vector_base(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : m_src_ptr(src_xs_ptr) {}

				auto& operator[](msev_size_t _P) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
					return (*this).at(msev_as_a_size_t(_P));
				}
				auto& front() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return first element of nonmutable sequence
					//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - const_reference front() - xslta_fixed_vector_base")); }
					return (*this).at(0);
				}
				auto& back() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return last element of nonmutable sequence
					//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - const_reference back() - xslta_fixed_vector_base")); }
					return (*this).at((*this).size() - 1);
				}

				size_type size() const _NOEXCEPT
				{	// return length of sequence
					return contained_vector().size();
				}

				bool empty() const _NOEXCEPT
				{	// test if sequence is empty
					return contained_vector().empty();
				}

				auto& at(msev_size_t _Pos) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
				{	// subscript nonmutable sequence with checking
					return unchecked_contained_vector().at(msev_as_a_size_t(_Pos));
				}

				/*
				auto* data() const _NOEXCEPT MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
				{	// return pointer to nonmutable data vector
					return contained_vector().data();
				}
				*/

				size_type max_size() const _NOEXCEPT
				{	// return maximum possible length of sequence
					return contained_vector().max_size();
				}

				//MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(corresponding_xslta_fixed_vector_t);

			private:
				xscope_accessing_fixed_nii_vector_base(const xscope_accessing_fixed_nii_vector_base&) = delete;

				auto& contained_vector() const { return (*m_src_ptr); }

				template<class T, class EqualTo>
				struct HasUncheckedContainedVector_impl
				{
					template<class U, class V>
					static auto test(U*) -> decltype(std::declval<U>().unchecked_contained_vector(), std::declval<V>(), bool(true));
					template<typename, typename>
					static auto test(...) -> std::false_type;

					using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				};
				template<class T, class EqualTo = T>
				struct HasUncheckedContainedVector : HasUncheckedContainedVector_impl<
					mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

				/*
				template<class HasUncheckedContainedVector_t>
				struct CB {
					template<class _Ty2>
					static auto& s_unchecked_contained_vector_helper1(_Ty2& this_obj) { return this_obj.contained_vector().unchecked_contained_vector(); }
				};
				template<>
				struct CB<std::false_type> {
					template<class _Ty2>
					static auto& s_unchecked_contained_vector_helper1(_Ty2& this_obj) { return this_obj.contained_vector(); }
				};
				*/

				auto& unchecked_contained_vector_helper1(const std::true_type&) const {
					return contained_vector().unchecked_contained_vector();
				}
				auto& unchecked_contained_vector_helper1(const std::false_type&) const {
					return contained_vector();
				}

				auto& unchecked_contained_vector() const {
					return unchecked_contained_vector_helper1(typename HasUncheckedContainedVector<decltype(contained_vector())>::type());
					//return CB<typename HasUncheckedContainedVector<decltype(contained_vector())>::type>::s_unchecked_contained_vector_helper1(*this);
				}

				_TPointerToLender m_src_ptr;

			} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
				MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
				MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");
		}
	}

	namespace impl {
		namespace ns_xscope_accessing_fixed_nii_vector {
			template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
			class xscope_accessing_fixed_nii_vector_base2 : public mse::us::impl::xscope_accessing_fixed_nii_vector_base<_TPointerToLender, _TLender, _Ty> {
				typedef mse::us::impl::xscope_accessing_fixed_nii_vector_base<_TPointerToLender, _TLender, _Ty> base_class;
			public:
				typedef typename std::conditional<std::is_const<_TLender>::value, const _Ty, _Ty>::type const_adjusted_Ty;
				typedef xscope_accessing_fixed_nii_vector_base2 _Myt;

				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class)
				//typedef typename base_class::allocator_type allocator_type;

				typedef typename base_class::const_iterator const_iterator;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename base_class::iterator>::type iterator;

				typedef typename base_class::const_reverse_iterator const_reverse_iterator;
				typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename base_class::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				xscope_accessing_fixed_nii_vector_base2(xscope_accessing_fixed_nii_vector_base2&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
				xscope_accessing_fixed_nii_vector_base2(xscope_accessing_fixed_nii_vector_base2&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

				typedef mse::impl::remove_const_t<_TLender> ncTLender;
				/* Some vectors support structure locking even via const reference. */
				typedef typename std::conditional<mse::impl::can_be_structure_locked_as_const<ncTLender>::value
					, ncTLender, _TLender>::type maybe_remove_const_lender_t;

				xscope_accessing_fixed_nii_vector_base2(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : base_class(src_xs_ptr)
					, m_xs_structure_lock_guard(_TLender::s_make_xscope_shared_structure_lock_guard(*src_xs_ptr)) {}

			private:
				xscope_accessing_fixed_nii_vector_base2(const xscope_accessing_fixed_nii_vector_base2&) = delete;

				template<class T, class EqualTo>
				struct IsSupportedLenderType_impl
				{
					template<class U, class V>
					static auto test(U* u) -> decltype(U::s_make_xscope_shared_structure_lock_guard(std::declval<U&>()), std::declval<V>(), bool(true));
					template<typename, typename>
					static auto test(...) -> std::false_type;

					using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				};
				template<class T, class EqualTo = T>
				struct IsSupportedLenderType : IsSupportedLenderType_impl<
					mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

				static_assert(IsSupportedLenderType<_TLender>::value, "xscope_accessing_fixed_nii_vector_base2<> does not support the given "
					"pointer to vector argument. This may be due to the underlying vector type not being supported "
					"or it may be due to the const qualification of the pointer target. A non-const pointer argument is "
					"required for certain vector types.");

				static_assert(mse::impl::is_strong_ptr<_TPointerToLender>::value, "xscope_accessing_fixed_nii_vector_base2<> does not support the given "
					"pointer to vector argument. This may be due to the pointer not being recognized as a 'strong' "
					"pointer.");

				typedef decltype(_TLender::s_make_xscope_shared_structure_lock_guard(std::declval<_TLender&>())) xscope_shared_structure_lock_guard_t;

				xscope_shared_structure_lock_guard_t m_xs_structure_lock_guard;

			} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
				MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
				MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

			template<class T>
			void takes_aco_const_pointer(mse::TXScopeAccessControlledConstPointer<T, mse::non_thread_safe_shared_mutex> const&) {}

			template<class T, class EqualTo>
			struct IsTXScopeAccessControlledConstPointer_impl
			{
				template<class U, class V>
				static auto test(U* u) -> decltype(takes_aco_const_pointer(std::declval<U>()), std::declval<V>(), bool(true));
				template<typename, typename>
				static auto test(...) -> std::false_type;

				using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
				static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
			};
			template<class T, class EqualTo = T>
			struct IsTXScopeAccessControlledConstPointer : IsTXScopeAccessControlledConstPointer_impl<
				mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

			/* xscope_accessing_fixed_nii_vector_base2<> requires that the lending vector support a specific API
			for locking the structure of its contents, while mse::us::impl::xscope_accessing_fixed_nii_vector_base
			doesn't do any locking and has no such API requirement. If _TPointerToLender is a
			TXScopeExclusiveWriterObj<> const pointer, then the lending vector doesn't need to be locked as
			the pointer itself takes care of that. So we will choose the base class for
			mse::xscope_accessing_fixed_nii_vector<> based on whether locking is required or not. */
			template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
			using xscope_accessing_fixed_nii_vector_base3 = mse::impl::conditional_t<IsTXScopeAccessControlledConstPointer<_TPointerToLender>::value, mse::us::impl::xscope_accessing_fixed_nii_vector_base<_TPointerToLender, _TLender, _Ty>, xscope_accessing_fixed_nii_vector_base2<_TPointerToLender, _TLender, _Ty> >;
		}
	}

	template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
	class xscope_accessing_fixed_nii_vector : public impl::ns_xscope_accessing_fixed_nii_vector::xscope_accessing_fixed_nii_vector_base3<_TPointerToLender, _TLender, _Ty> {
		typedef impl::ns_xscope_accessing_fixed_nii_vector::xscope_accessing_fixed_nii_vector_base3<_TPointerToLender, _TLender, _Ty> base_class;
	public:
		typedef xscope_accessing_fixed_nii_vector _Myt;

		MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class)
		//typedef typename base_class::allocator_type allocator_type;

		typedef typename base_class::const_iterator const_iterator;
		typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename base_class::iterator>::type iterator;

		typedef typename base_class::const_reverse_iterator const_reverse_iterator;
		typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename base_class::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_accessing_fixed_nii_vector(xscope_accessing_fixed_nii_vector&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
		xscope_accessing_fixed_nii_vector(xscope_accessing_fixed_nii_vector&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

		xscope_accessing_fixed_nii_vector(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : base_class(src_xs_ptr) {}

	private:
		xscope_accessing_fixed_nii_vector(const xscope_accessing_fixed_nii_vector&) = delete;
	} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
		MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
		MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

#ifdef MSE_HAS_CXX17
	/* deduction guides */
	template<class _TPointerToLender>
	xscope_accessing_fixed_nii_vector(_TPointerToLender) -> xscope_accessing_fixed_nii_vector<_TPointerToLender>;
#endif /* MSE_HAS_CXX17 */

	template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
	auto make_xscope_accessing_fixed_nii_vector(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"))
		MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
		MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
	{
		return xscope_accessing_fixed_nii_vector<_TPointerToLender, _TLender, _Ty>(src_xs_ptr);
	}


	namespace rsv {
		template<class _Ty, class _A>
		class xslta_fixed_vector;

		namespace us {
			namespace impl {
				template<class _Ty, class _A = std::allocator<_Ty> >
				class xslta_fixed_vector_base : private mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> >, private container_adjusted_default_state_mutex<_Ty>, public mse::us::impl::ContiguousSequenceStaticStructureContainerTagBase {
				public:
					typedef container_adjusted_default_state_mutex<_Ty> state_mutex_t;
					typedef state_mutex_t _TStateMutex;

					/* We (privately) inherit the underlying data type rather than make it a data member to ensure it's the "first" component in the structure.*/
					typedef mse::impl::TOpaqueWrapper<std::vector<_Ty, _A> > base_class;
					typedef std::vector<_Ty, _A> std_vector;
					typedef std_vector _MV;
					typedef xslta_fixed_vector_base _Myt;

					typedef typename std_vector::allocator_type allocator_type;
					typedef typename std_vector::value_type value_type;
					//typedef typename std_vector::size_type size_type;
					typedef msev_size_t size_type;
					//typedef typename std_vector::difference_type difference_type;
					typedef msev_int difference_type;
					typedef typename std_vector::pointer pointer;
					typedef typename std_vector::const_pointer const_pointer;
					typedef typename std_vector::reference reference;
					typedef typename std_vector::const_reference const_reference;

				private:
					const _MV& contained_vector() const& { return (*this).value(); }
					//const _MV& contained_vector() const&& { return (*this).value(); }
					_MV& contained_vector()& { return (*this).value(); }
					_MV&& contained_vector()&& { return std::move(*this).value(); }

					state_mutex_t& state_mutex1()& { return (*this); }

				public:
					explicit xslta_fixed_vector_base(const _A& _Al = _A())
						: base_class(_Al) {
						/*m_debug_size = size();*/
					}
					explicit xslta_fixed_vector_base(size_type _N)
						: base_class(msev_as_a_size_t(_N)) {
						/*m_debug_size = size();*/
					}
					explicit xslta_fixed_vector_base(size_type _N, const _Ty& _V MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"), const _A& _Al = _A())
						: base_class(msev_as_a_size_t(_N), _V, _Al) {
						/*m_debug_size = size();*/
					}
					xslta_fixed_vector_base(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
					xslta_fixed_vector_base(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
					xslta_fixed_vector_base(xslta_vector<_Ty, _A>&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(_MV(MSE_FWD(_X))) { /*m_debug_size = size();*/ }
					xslta_fixed_vector_base(const xslta_vector<_Ty, _A>& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(_MV(_X)) { /*m_debug_size = size();*/ }
					//xslta_fixed_vector_base(_Myt&& _X) : base_class(_X.contained_vector()) { /*m_debug_size = size();*/ }
					xslta_fixed_vector_base(const _Myt& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)")) : base_class(_X.contained_vector()) { /*m_debug_size = size();*/ }
					typedef typename std_vector::const_iterator _It;
					/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
					xslta_fixed_vector_base(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
					//xslta_fixed_vector_base(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
					template<class _Iter
						//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
						, class = mse::impl::_mse_RequireInputIter<_Iter> >
					xslta_fixed_vector_base(const _Iter& _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter& _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
					template<class _Iter
						//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
						, class = mse::impl::_mse_RequireInputIter<_Iter> >
					//xslta_fixed_vector_base(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
					xslta_fixed_vector_base(const _Iter& _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

					~xslta_fixed_vector_base() {
						mse::impl::destructor_lock_guard1<decltype(state_mutex1())> lock1(state_mutex1());

						/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
						//valid_if_Ty_is_not_an_xslta_type();
						valid_if_Ty_is_not_bool();
					}

					/* The returned vector should be a copy (rather than a move) even when this is an rvalue, as this vector is not allowed to be resized. */
					operator _MV() const { return contained_vector(); }

					typename std_vector::const_reference operator[](msev_size_t _P) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
						return (*this).at(msev_as_a_size_t(_P));
					}
					typename std_vector::reference operator[](msev_size_t _P) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
						return (*this).at(msev_as_a_size_t(_P));
					}
					typename std_vector::reference front() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return first element of mutable sequence
						if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::reference front() - xslta_fixed_vector_base")); }
						return contained_vector().front();
					}
					typename std_vector::const_reference front() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return first element of nonmutable sequence
						if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - typename std_vector::const_reference front() - xslta_fixed_vector_base")); }
						return contained_vector().front();
					}
					typename std_vector::reference back() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return last element of mutable sequence
						if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::reference back() - xslta_fixed_vector_base")); }
						return contained_vector().back();
					}
					typename std_vector::const_reference back() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return last element of nonmutable sequence
						if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - typename std_vector::const_reference back() - xslta_fixed_vector_base")); }
						return contained_vector().back();
					}

					size_type size() const _NOEXCEPT
					{	// return length of sequence
						return contained_vector().size();
					}

					bool empty() const _NOEXCEPT
					{	// test if sequence is empty
						return contained_vector().empty();
					}
					_A get_allocator() const _NOEXCEPT
					{	// return allocator object for values
						return contained_vector().get_allocator();
					}

					reference at(msev_size_t _Pos) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// subscript mutable sequence with checking
						return contained_vector().at(msev_as_a_size_t(_Pos));
					}

					const_reference at(msev_size_t _Pos) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// subscript nonmutable sequence with checking
						return contained_vector().at(msev_as_a_size_t(_Pos));
					}

					xslta_fixed_vector_base(_XSTD initializer_list<typename std_vector::value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"), const _A& _Al = _A())
						: base_class(_Ilist, _Al) {	// construct from initializer_list
						/*m_debug_size = size();*/
					}

					value_type* data() _NOEXCEPT MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// return pointer to mutable data vector
						return contained_vector().data();
					}

					const value_type* data() const _NOEXCEPT MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// return pointer to nonmutable data vector
						return contained_vector().data();
					}

					size_type max_size() const _NOEXCEPT
					{	// return maximum possible length of sequence
						return contained_vector().max_size();
					}

				private:
					_Myt& operator=(const std_vector& _X) {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().operator =(_X);
						/*m_debug_size = size();*/
						return (*this);
					}
					_Myt& operator=(_Myt&& _X) {
						if (std::addressof(_X) == this) { return (*this); }
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().operator=(MSE_FWD(_X).contained_vector());
						return (*this);
					}
					_Myt& operator=(const _Myt& _X) {
						if (std::addressof(_X) == this) { return (*this); }
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().operator=(_X.contained_vector());
						return (*this);
					}

					void reserve(size_type _Count)
					{	// determine new minimum length of allocated storage
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().reserve(msev_as_a_size_t(_Count));
					}
					size_type capacity() const _NOEXCEPT
					{	// return current length of allocated storage
						return contained_vector().capacity();
					}
					void shrink_to_fit() {	// reduce capacity
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().shrink_to_fit();
					}
					void resize(size_type _N, const _Ty& _X = _Ty()) {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().resize(msev_as_a_size_t(_N), _X);
					}

					void push_back(_Ty&& _X) {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().push_back(MSE_FWD(_X));
					}
					void push_back(const _Ty& _X) {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().push_back(_X);
					}
					void pop_back() {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().pop_back();
					}
					void assign(_It _F, _It _L) {
						smoke_check_source_iterators(_F, _L);
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().assign(_F, _L);
						/*m_debug_size = size();*/
					}
					template<class _Iter>
					void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
						smoke_check_source_iterators(_First, _Last);
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().assign(_First, _Last);
						/*m_debug_size = size();*/
					}
					void assign(size_type _N, const _Ty& _X = _Ty()) {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().assign(msev_as_a_size_t(_N), _X);
						/*m_debug_size = size();*/
					}


					template<class _Iter>
					static void smoke_check_source_iterators_helper(std::true_type, const _Iter& _First, const _Iter& _Last) {
						if (_Last < _First)/*comparison operations should also verify that safe iterators point to the same container*/ {
							MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - xslta_fixed_vector_base"));
						}
					}
					template<class _Iter>
					static void smoke_check_source_iterators_helper(std::false_type, const _Iter&, const _Iter&) {}

					template<class _Iter>
					static void smoke_check_source_iterators(const xslta_fixed_vector_base& target_cref, const _Iter& _First, const _Iter& _Last) {
#ifndef MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK
						if (!(target_cref.empty())) {
							/* check if the source sequence is part of target (target) container */
							auto start_of_target_ptr = std::addressof(*(target_cref.cbegin()));
							auto end_of_target_ptr = std::addressof(*(target_cref.cend() - 1)) + 1;
							auto _First_ptr = std::addressof(*_First);
							if ((end_of_target_ptr > _First_ptr) && (start_of_target_ptr <= _First_ptr)) {
								MSE_THROW(gnii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - xslta_fixed_vector_base"));
							}
						}
#endif // !MSE_NII_VECTOR_SUPRESS_SOURCE_ITER_ALIAS_CHECK

#ifdef MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
						smoke_check_source_iterators_helper(typename mse::impl::HasOrInheritsLessThanOperator_msemsevector<_Iter>::type(), _First, _Last);
#endif // MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ORDER_CHECK
					}
					template<class _Iter>
					void smoke_check_source_iterators(const _Iter& _First, const _Iter& _Last) {
						smoke_check_source_iterators(*this, _First, _Last);
					}

					template<class ..._Valty>
					void emplace_back(_Valty&& ..._Val)
					{	// insert by moving into element at end
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().emplace_back(std::forward<_Valty>(_Val)...);
						/*m_debug_size = size();*/
					}
					void clear() {
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().clear();
						/*m_debug_size = size();*/
					}

					void swap(_Myt& _Other) {	// swap contents with _Other
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().swap(_Other.contained_vector());
					}
					void swap(_MV& _Other) {	// swap contents with _Other
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().swap(_Other);
					}
					/*
					template<typename _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2>
					void swap(mse::rsv::us::impl::xslta_fixed_vector_base<_Ty, _A, _TStateMutex2, _TTXScopeConstIterator2, TConstLockableIndicator2>& _Other) {	// swap contents with _Other
						//structure_change_guard<decltype(state_mutex1())> lock1(state_mutex1());
						contained_vector().swap(_Other.contained_vector());
					}
					*/

					_Myt& operator=(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
						contained_vector().operator=(static_cast<std_vector>(_Ilist));
						return (*this);
					}
					void assign(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
						contained_vector().assign(_Ilist);
						/*m_debug_size = size();*/
					}
				public:

					//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
					//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

					typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
					typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

					template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
					using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
					template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
					using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

					template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
					using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
					template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
					using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

					typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
					typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
					typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
					typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

					typedef _Myt& _Myt_ref;

					/* Here we provide static versions of the member functions that return iterators. As static member functions do
					not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
					argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

					template<typename _TVectorPointer>
					static auto ss_begin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
							, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
						return_type retval(owner_ptr, 0);
						retval.set_to_beginning();
						return retval;
					}

					template<typename _TVectorPointer>
					static auto ss_end(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
							, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> > return_type;
						return_type retval(owner_ptr, 0);
						retval.set_to_end_marker();
						return retval;
					}

					template<typename _TVectorPointer>
					static auto ss_cbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
						retval.set_to_beginning();
						return retval;
					}

					template<typename _TVectorPointer>
					static auto ss_cend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr, 0);
						retval.set_to_end_marker();
						return retval;
					}

					template<typename _TVectorPointer>
					static auto ss_rbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
							, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
						return return_type(ss_end<_TVectorPointer>(owner_ptr));
					}

					template<typename _TVectorPointer>
					static auto ss_rend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						typedef mse::impl::conditional_t<std::is_const<mse::impl::remove_reference_t<decltype(*owner_ptr)> >::value
							, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> > return_type;
						return return_type(ss_begin<_TVectorPointer>(owner_ptr));
					}

					template<typename _TVectorPointer>
					static auto ss_crbegin(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_cend<_TVectorPointer>(owner_ptr)));
					}

					template<typename _TVectorPointer>
					static auto ss_crend(const _TVectorPointer& owner_ptr) MSE_ATTR_FUNC_STR("mse::lifetime_scope_types_prohibited_for_template_parameter_by_name(_TVectorPointer)") {
						mse::impl::T_valid_if_not_an_xscope_type<_TVectorPointer>();
						return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_crbegin<_TVectorPointer>(owner_ptr)));
					}

					template<typename _TVectorPointer1>
					static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
						if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - xslta_fixed_vector_base")); }
					}

					friend bool operator==(const _Myt& _Left, const _Myt& _Right) {
						return (_Left.contained_vector() == _Right.contained_vector());
					}
					MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_IF_ANY(xslta_fixed_vector_base)
#ifndef MSE_HAS_CXX20
						friend bool operator<(const _Myt& _Left, const _Myt& _Right) {
						return (_Left.contained_vector() < _Right.contained_vector());
					}
#else // !MSE_HAS_CXX20
						friend std::strong_ordering operator<=>(const _Myt& _Left, const _Myt& _Right) {
						return (_Left.contained_vector() <=> _Right.contained_vector());
					}
#endif // !MSE_HAS_CXX20

					template<class _Ty2, class _Traits2>
					std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr, size_type byte_count, const size_type byte_start_offset = 0) const {
						const auto array_size_in_bytes = mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size());
						auto byte_ptr = reinterpret_cast<const char*>((*this).contained_vector().data());
						if ((array_size_in_bytes <= byte_start_offset) || (0 >= byte_count)) {
							return _Ostr;
						}
						else {
							byte_ptr += mse::msev_as_a_size_t(byte_start_offset);
							return _Ostr.write(byte_ptr, std::min(mse::msev_as_a_size_t(array_size_in_bytes - byte_start_offset), mse::msev_as_a_size_t(byte_count)));
						}
					}
					template<class _Ty2, class _Traits2>
					std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr) const {
						return write_bytes(_Ostr, mse::msev_as_a_size_t(sizeof(_Ty) * (*this).size()));
					}

					MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

				private:

					/* If _Ty is an xslta type, then the following member function will not instantiate, causing an
					(intended) compile error. */
					template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (mse::impl::is_potentially_not_xscope<_Ty2>::value)> MSE_IMPL_EIS >
					void valid_if_Ty_is_not_an_xslta_type() const {}

					template<class _Ty2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value) && (!std::is_same<bool, _Ty2>::value)> MSE_IMPL_EIS >
					void valid_if_Ty_is_not_bool() const {}


					//friend class mse::rsv::fixed_nii_vector<_Ty, _A>;
					friend class mse::rsv::xslta_fixed_vector<_Ty, _A>;
				} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
					MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			}
		}

		template<class _TLender, class _Ty, class _A>
		class xslta_borrowing_fixed_vector;

		template<class _Ty, class _A = std::allocator<_Ty> >
		class xslta_fixed_vector : public mse::rsv::us::impl::xslta_fixed_vector_base<_Ty, _A>, public mse::us::impl::XScopeTagBase
			, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ReferenceableByScopePointerTagBase, _Ty, xslta_fixed_vector<_Ty, _A> >
			, public mse::impl::first_or_placeholder_if_not_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, _Ty, xslta_fixed_vector<_Ty, _A> >
		{
		public:
			typedef mse::rsv::us::impl::xslta_fixed_vector_base<_Ty, _A> base_class;
			typedef std::vector<_Ty, _A> std_vector;
			typedef std_vector _MV;
			typedef xslta_fixed_vector _Myt;

			typedef typename std_vector::allocator_type allocator_type;
			typedef typename std_vector::value_type value_type;
			//typedef typename std_vector::size_type size_type;
			typedef msev_size_t size_type;
			//typedef typename std_vector::difference_type difference_type;
			typedef msev_int difference_type;
			typedef typename std_vector::pointer pointer;
			typedef typename std_vector::const_pointer const_pointer;
			typedef typename std_vector::reference reference;
			typedef typename std_vector::const_reference const_reference;

		private:
			const _MV& contained_vector() const& { return base_class::contained_vector(); }
			//const _MV& contained_vector() const&& { return base_class::contained_vector(); }
			_MV& contained_vector()& { return base_class::contained_vector(); }
			_MV&& contained_vector()&& { return std::move(base_class::contained_vector()); }

		public:
			explicit xslta_fixed_vector(const _A& _Al = _A()) : base_class(_Al) {}
			explicit xslta_fixed_vector(size_type _N) : base_class(_N) {}
			explicit xslta_fixed_vector(size_type _N, const _Ty& _V MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _A& _Al = _A()) : base_class(_N, _V, _Al) {}
			xslta_fixed_vector(std_vector&& _X) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(const std_vector& _X) : base_class(_X) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(xslta_vector<_Ty, _A>&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(const xslta_vector<_Ty, _A>& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(_X) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(_Myt&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(MSE_FWD(_X)) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(const _Myt& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(_X) { /*m_debug_size = size();*/ }
			typedef typename std_vector::const_iterator _It;
			/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
			xslta_fixed_vector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : base_class(_F, _L, _Al) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			xslta_fixed_vector(const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last) : base_class(_First, _Last) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			//xslta_fixed_vector(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }
			xslta_fixed_vector(const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

			xslta_fixed_vector(_XSTD initializer_list<typename std_vector::value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"), const _A& _Al = _A())
				: base_class(_Ilist, _Al) {	// construct from initializer_list
				/*m_debug_size = size();*/
			}

			//class nv_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
			//class nv_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

			typedef mse::impl::random_access_const_iterator_base<_Ty> nv_const_iterator_base;
			typedef mse::impl::random_access_iterator_base<_Ty> nv_iterator_base;

			template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
			using Tss_const_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_iterator_type<_TVectorConstPointer>;
			template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
			using Tss_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_iterator_type<_TVectorPointer>;

			template<typename _TVectorPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorPointer>::value)> MSE_IMPL_EIS >
			using Tss_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_reverse_iterator_type<_TVectorPointer>;
			template<typename _TVectorConstPointer, MSE_IMPL_EIP mse::impl::enable_if_t<(mse::impl::is_potentially_not_xscope<_TVectorConstPointer>::value)> MSE_IMPL_EIS >
			using Tss_const_reverse_iterator_type = mse::impl::ns_gnii_vector::Tgnii_vector_ss_const_reverse_iterator_type<_TVectorConstPointer>;

			typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_iterator_type<_Myt> ss_iterator_type;
			typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_iterator_type<_Myt> ss_const_iterator_type;
			typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_reverse_iterator_type<_Myt> ss_reverse_iterator_type;
			typedef mse::impl::ns_gnii_vector::Tgnii_vector_rp_ss_const_reverse_iterator_type<_Myt> ss_const_reverse_iterator_type;

			typedef TXSLTACSSSXSRAConstIterator<_Myt> xslta_const_iterator;
			typedef TXSLTACSSSXSRAIterator<_Myt> xslta_iterator;
			typedef xslta_const_iterator const_iterator;
			typedef xslta_iterator iterator;
			typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
			typedef std::reverse_iterator<iterator> reverse_iterator;

			iterator begin() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return mse::rsv::us::unsafe_make_xslta_csss_strong_ra_iterator(mse::rsv::TXSLTAPointer<_Myt>(this), 0/*index*/);
			}
			const_iterator begin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cbegin();
			}
			const_iterator cbegin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return mse::rsv::us::unsafe_make_xslta_csss_strong_ra_const_iterator(mse::rsv::TXSLTAConstPointer<_Myt>(this), 0/*index*/);
			}
			iterator end() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return mse::rsv::us::unsafe_make_xslta_csss_strong_ra_iterator(mse::rsv::TXSLTAPointer<_Myt>(this), 0/*index*/) + (*this).size();
			}
			const_iterator end() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cend();
			}
			const_iterator cend() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return mse::rsv::us::unsafe_make_xslta_csss_strong_ra_const_iterator(mse::rsv::TXSLTAConstPointer<_Myt>(this), 0/*index*/) + (*this).size();
			}

			reverse_iterator rbegin() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return end();
			}
			const_reverse_iterator rbegin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return crbegin();
			}
			const_reverse_iterator crbegin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cend();
			}
			reverse_iterator rend() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return begin();
			}
			const_reverse_iterator rend() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return crend();
			}
			const_reverse_iterator crend() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cbegin();
			}

			typedef _Myt& _Myt_ref;

			/* Here we provide static versions of the member functions that return iterators. As static member functions do
			not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
			argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

			template<typename _TArrayPointer>
			static auto ss_begin(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_begin(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_end(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_end(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_cbegin(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_cbegin(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_cend(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_cend(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_rbegin(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_rbegin(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_rend(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_rend(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_crbegin(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_crbegin(owner_ptr);
			}
			template<typename _TArrayPointer>
			static auto ss_crend(const _TArrayPointer& owner_ptr) {
				return base_class::template ss_crend(owner_ptr);
			}

			template<typename _TArrayPointer, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TArrayPointer, mse::rsv::TXSLTAPointer<_Myt> >::value || std::is_convertible<_TArrayPointer, mse::rsv::TXSLTAConstPointer<_Myt> >::value> MSE_IMPL_EIS >
			static auto xslta_ss_begin(const _TArrayPointer& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_12$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_TArrayPointer, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return base_class::template xslta_ss_begin(owner_ptr);
			}
			template<typename _TArrayPointer, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_TArrayPointer, mse::rsv::TXSLTAPointer<_Myt> >::value || std::is_convertible<_TArrayPointer, mse::rsv::TXSLTAConstPointer<_Myt> >::value> MSE_IMPL_EIS >
			static auto xslta_ss_end(const _TArrayPointer& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_12$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_TArrayPointer, alias_12$); labels(alias_12$); return_value(alias_12$) }") {
				return base_class::template xslta_ss_end(owner_ptr);
			}

			static auto xslta_ss_cbegin(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				xslta_const_iterator retval(owner_ptr, 0);
				retval.set_to_beginning();
				return retval;
			}
			static auto xslta_ss_cend(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				xslta_const_iterator retval(owner_ptr, 0);
				retval.set_to_end_marker();
				return retval;
			}

			static auto xslta_ss_rbegin(const mse::rsv::TXSLTAPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				return xslta_ss_reverse_iterator_type(xslta_ss_end(owner_ptr));
			}
			static auto xslta_ss_rend(const mse::rsv::TXSLTAPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				return xslta_ss_reverse_iterator_type(xslta_ss_begin(owner_ptr));
			}
			static auto xslta_ss_crbegin(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				return (xslta_ss_const_reverse_iterator_type(xslta_ss_cend(owner_ptr)));
			}
			static auto xslta_ss_crend(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") {

				return (xslta_ss_const_reverse_iterator_type(xslta_ss_crbegin(owner_ptr)));
			}
			static auto xslta_ss_rbegin(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") { return xslta_ss_crbegin(owner_ptr); }
			static auto xslta_ss_rend(const mse::rsv::TXSLTAConstPointer<_Myt>& owner_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[99])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(99); return_value(99) }") { return xslta_ss_crend(owner_ptr); }

			template<typename _TVectorPointer1>
			static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
				if (_Myt_ref(*this_ptr).size() < index) { MSE_THROW(gnii_vector_range_error("invalid index - void assert_valid_index() const - xslta_fixed_vector")); }
			}

			MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);

		private:
			/* If _Ty is "marked" as containing a scope reference, then the following member function
			will not instantiate, causing an (intended) compile error. */
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (mse::impl::potentially_does_not_contain_non_owning_scope_reference<_Ty2>::value)> MSE_IMPL_EIS >
			void valid_if_Ty_is_not_marked_as_containing_a_scope_reference() const {}

			/* If _Ty is "marked" as containing an accessible "scope address of" operator, then the following member function
			will not instantiate, causing an (intended) compile error. */
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<_Ty2, _Ty>::value)
				&& (mse::impl::is_potentially_not_referenceable_by_scope_pointer<_Ty2>::value)
			> MSE_IMPL_EIS >
			void valid_if_Ty_is_not_marked_as_containing_an_accessible_scope_addressof_operator() const {}

			template<class _TLender2, class _Ty2, class _A2> friend class xslta_borrowing_fixed_vector;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
			MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
		xslta_fixed_vector(_Iter, _Iter, _Alloc = _Alloc())
			-> xslta_fixed_vector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */

		template<class _TLender, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		class xslta_borrowing_fixed_vector : public xslta_fixed_vector<_Ty, _A>
			, public mse::impl::first_or_placeholder_if_base_of_second<mse::us::impl::ContainsNonOwningScopeReferenceTagBase, xslta_fixed_vector<_Ty, _A>, xslta_borrowing_fixed_vector<_TLender, _Ty, _A> >
		{
		public:
			typedef xslta_fixed_vector<_Ty, _A> base_class;

			typedef typename base_class::std_vector std_vector;
			typedef typename base_class::_MV _MV;
			typedef xslta_borrowing_fixed_vector _Myt;

			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class)
			typedef typename base_class::allocator_type allocator_type;

			typedef typename base_class::iterator iterator;
			typedef typename base_class::const_iterator const_iterator;

			typedef typename base_class::reverse_iterator reverse_iterator;
			typedef typename base_class::const_reverse_iterator const_reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_borrowing_fixed_vector(xslta_borrowing_fixed_vector&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_borrowing_fixed_vector(xslta_borrowing_fixed_vector&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

			xslta_borrowing_fixed_vector(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
			xslta_borrowing_fixed_vector(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) : base_class(std::move(*src_xs_ptr)), m_src_ref(*src_xs_ptr) {}
			~xslta_borrowing_fixed_vector() {
				m_src_ref = std::move((*this).contained_vector());
			}

			MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(base_class);

		private:
			xslta_borrowing_fixed_vector(const xslta_borrowing_fixed_vector&) = delete;

			_TLender& m_src_ref;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
			MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _TLender>
		xslta_borrowing_fixed_vector(mse::rsv::TXSLTAPointer<_TLender>) -> xslta_borrowing_fixed_vector<_TLender>;
#endif /* MSE_HAS_CXX17 */

		template<class _TLender, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		auto make_xslta_borrowing_fixed_vector(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_vector<_TLender>(src_xs_ptr);
		}
		template<class _TLender, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		auto make_xslta_borrowing_fixed_vector(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_borrowing_fixed_vector<_TLender>(src_xs_ptr);
		}

		namespace us {
			namespace impl {
				template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
				class xslta_accessing_fixed_vector_base : public mse::us::impl::ContainsNonOwningScopeReferenceTagBase {
				public:
					typedef typename std::conditional<std::is_const<_TLender>::value, const _Ty, _Ty>::type const_adjusted_Ty;
					typedef mse::impl::target_type<_TPointerToLender> unchecked_contained_vector_t;

					typedef xslta_accessing_fixed_vector_base _Myt;

					//typedef typename unchecked_contained_vector_t::allocator_type allocator_type;
					typedef typename unchecked_contained_vector_t::value_type value_type;
					typedef typename unchecked_contained_vector_t::size_type size_type;
					typedef typename unchecked_contained_vector_t::difference_type difference_type;
					typedef typename unchecked_contained_vector_t::const_pointer const_pointer;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_pointer, typename unchecked_contained_vector_t::pointer>::type pointer;
					typedef typename unchecked_contained_vector_t::const_reference const_reference;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_reference, typename unchecked_contained_vector_t::reference>::type reference;

					typedef typename unchecked_contained_vector_t::const_iterator const_iterator;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename unchecked_contained_vector_t::iterator>::type iterator;

					typedef typename unchecked_contained_vector_t::const_reverse_iterator const_reverse_iterator;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename unchecked_contained_vector_t::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
					xslta_accessing_fixed_vector_base(xslta_accessing_fixed_vector_base&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
					xslta_accessing_fixed_vector_base(xslta_accessing_fixed_vector_base&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

					typedef mse::impl::remove_const_t<_TLender> ncTLender;
					/* Some vectors support structure locking even via const reference. */
					typedef typename std::conditional<mse::impl::can_be_structure_locked_as_const<ncTLender>::value
						, ncTLender, _TLender>::type maybe_remove_const_lender_t;

					xslta_accessing_fixed_vector_base(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : m_src_ptr(src_xs_ptr) {}

					auto& operator[](msev_size_t _P) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
						return (*this).at(msev_as_a_size_t(_P));
					}
					auto& front() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return first element of nonmutable sequence
						//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - const_reference front() - xslta_fixed_vector_base")); }
						return (*this).at(0);
					}
					auto& back() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return last element of nonmutable sequence
						//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - const_reference back() - xslta_fixed_vector_base")); }
						return (*this).at((*this).size() - 1);
					}

					size_type size() const _NOEXCEPT
					{	// return length of sequence
						return contained_vector().size();
					}

					bool empty() const _NOEXCEPT
					{	// test if sequence is empty
						return contained_vector().empty();
					}

					auto& at(msev_size_t _Pos) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// subscript nonmutable sequence with checking
						return unchecked_contained_vector().at(msev_as_a_size_t(_Pos));
					}

					/*
					auto* data() const _NOEXCEPT MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// return pointer to nonmutable data vector
						return contained_vector().data();
					}
					*/

					size_type max_size() const _NOEXCEPT
					{	// return maximum possible length of sequence
						return contained_vector().max_size();
					}

					//MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_OF(corresponding_xslta_fixed_vector_t);

				private:
					xslta_accessing_fixed_vector_base(const xslta_accessing_fixed_vector_base&) = delete;

					auto& contained_vector() const { return (*m_src_ptr); }

					template<class T, class EqualTo>
					struct HasUncheckedContainedVector_impl
					{
						template<class U, class V>
						static auto test(U*) -> decltype(std::declval<U>().unchecked_contained_vector(), std::declval<V>(), bool(true));
						template<typename, typename>
						static auto test(...) -> std::false_type;

						using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
					};
					template<class T, class EqualTo = T>
					struct HasUncheckedContainedVector : HasUncheckedContainedVector_impl<
						mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

					/*
					template<class HasUncheckedContainedVector_t>
					struct CB {
						template<class _Ty2>
						static auto& s_unchecked_contained_vector_helper1(_Ty2& this_obj) { return this_obj.contained_vector().unchecked_contained_vector(); }
					};
					template<>
					struct CB<std::false_type> {
						template<class _Ty2>
						static auto& s_unchecked_contained_vector_helper1(_Ty2& this_obj) { return this_obj.contained_vector(); }
					};
					*/

					auto& unchecked_contained_vector_helper1(const std::true_type&) const {
						return contained_vector().unchecked_contained_vector();
					}
					auto& unchecked_contained_vector_helper1(const std::false_type&) const {
						return contained_vector();
					}

					auto& unchecked_contained_vector() const {
						return unchecked_contained_vector_helper1(typename HasUncheckedContainedVector<decltype(contained_vector())>::type());
						//return CB<typename HasUncheckedContainedVector<decltype(contained_vector())>::type>::s_unchecked_contained_vector_helper1(*this);
					}

					_TPointerToLender m_src_ptr;

				} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
					MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			}
		}

		namespace impl {
			namespace ns_xslta_accessing_fixed_vector {
				template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
				class xslta_accessing_fixed_vector_base2 : public mse::rsv::us::impl::xslta_accessing_fixed_vector_base<_TPointerToLender, _TLender, _Ty> {
					typedef mse::rsv::us::impl::xslta_accessing_fixed_vector_base<_TPointerToLender, _TLender, _Ty> base_class;
				public:
					typedef typename std::conditional<std::is_const<_TLender>::value, const _Ty, _Ty>::type const_adjusted_Ty;
					typedef xslta_accessing_fixed_vector_base2 _Myt;

					MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class)
					//typedef typename base_class::allocator_type allocator_type;

					typedef typename base_class::const_iterator const_iterator;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename base_class::iterator>::type iterator;

					typedef typename base_class::const_reverse_iterator const_reverse_iterator;
					typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename base_class::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
					xslta_accessing_fixed_vector_base2(xslta_accessing_fixed_vector_base2&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
					xslta_accessing_fixed_vector_base2(xslta_accessing_fixed_vector_base2&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

					typedef mse::impl::remove_const_t<_TLender> ncTLender;
					/* Some vectors support structure locking even via const reference. */
					typedef typename std::conditional<mse::impl::can_be_structure_locked_as_const<ncTLender>::value
						, ncTLender, _TLender>::type maybe_remove_const_lender_t;

					xslta_accessing_fixed_vector_base2(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : base_class(src_xs_ptr)
						, m_xs_structure_lock_guard(_TLender::s_make_xscope_shared_structure_lock_guard(*src_xs_ptr)) {}

					/* At the time of writing, lifetimes are only guaranteed to be transmitted properly to the immediate base class, so we override these
					member functions with lifetime annotations in order to reexepress them for potential use by an immediately derived class. (Although, 
					since these return value lifetimes all happen to be the same as the default implied ones, they actually don't really need to be 
					explicitly expressed here or in the (immediate) base class. These redundant lifetime annotations are only present because they were
					originally used before the implied defaults were established.) */

					auto& operator[](msev_size_t _P) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
						return base_class::operator[](_P);
					}
					auto& front() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return first element of nonmutable sequence
						//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("front() on empty - const_reference front() - xslta_fixed_vector_base")); }
						return base_class::front();
					}
					auto& back() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// return last element of nonmutable sequence
						//if (0 == (*this).size()) { MSE_THROW(gnii_vector_range_error("back() on empty - const_reference back() - xslta_fixed_vector_base")); }
						return base_class::back();
					}

					auto& at(msev_size_t _Pos) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// subscript nonmutable sequence with checking
						return base_class::at(_Pos);
					}

					/*
					auto* data() const _NOEXCEPT MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }")
					{	// return pointer to nonmutable data vector
						return base_class::data();
					}
					*/

				private:
					xslta_accessing_fixed_vector_base2(const xslta_accessing_fixed_vector_base2&) = delete;

					template<class T, class EqualTo>
					struct IsSupportedLenderType_impl
					{
						template<class U, class V>
						static auto test(U* u) -> decltype(U::s_make_xscope_shared_structure_lock_guard(std::declval<U&>()), std::declval<V>(), bool(true));
						template<typename, typename>
						static auto test(...) -> std::false_type;

						using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
					};
					template<class T, class EqualTo = T>
					struct IsSupportedLenderType : IsSupportedLenderType_impl<
						mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

					static_assert(IsSupportedLenderType<_TLender>::value, "xslta_accessing_fixed_vector_base2<> does not support the given "
						"pointer to vector argument. This may be due to the underlying vector type not being supported "
						"or it may be due to the const qualification of the pointer target. A non-const pointer argument is "
						"required for certain vector types.");

					static_assert(mse::impl::is_strong_ptr<_TPointerToLender>::value, "xscope_accessing_fixed_nii_vector_base2<> does not support the given "
						"pointer to vector argument. This may be due to the pointer not being recognized as a 'strong' "
						"pointer.");

					typedef decltype(_TLender::s_make_xscope_shared_structure_lock_guard(std::declval<_TLender&>())) xscope_shared_structure_lock_guard_t;

					xscope_shared_structure_lock_guard_t m_xs_structure_lock_guard;

				} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
					MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
					MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

				template<class T>
				void takes_aco_const_pointer(mse::TXScopeAccessControlledConstPointer<T, mse::non_thread_safe_shared_mutex> const &) {}

				template<class T, class EqualTo>
				struct IsTXScopeAccessControlledConstPointer_impl
				{
					template<class U, class V>
					static auto test(U* u) -> decltype(takes_aco_const_pointer(std::declval<U>()), std::declval<V>(), bool(true));
					template<typename, typename>
					static auto test(...) -> std::false_type;

					using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
					static const bool value = std::is_same<bool, decltype(test<T, EqualTo>(0))>::value;
				};
				template<class T, class EqualTo = T>
				struct IsTXScopeAccessControlledConstPointer : IsTXScopeAccessControlledConstPointer_impl<
					mse::impl::remove_reference_t<T>, mse::impl::remove_reference_t<EqualTo> >::type {};

				/* xslta_accessing_fixed_vector_base2<> requires that the lending vector support a specific API 
				for locking the structure of its contents, while mse::rsv::us::impl::xslta_accessing_fixed_vector_base 
				doesn't do any locking and has no such API requirement. If _TPointerToLender is a 
				TXScopeExclusiveWriterObj<> const pointer, then the lending vector doesn't need to be locked as 
				the pointer itself takes care of that. So we will choose the base class for 
				mse::rsv::xslta_accessing_fixed_vector<> based on whether locking is required or not. */
				template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
				using xslta_accessing_fixed_vector_base3 = mse::impl::conditional_t<IsTXScopeAccessControlledConstPointer<_TPointerToLender>::value, mse::rsv::us::impl::xslta_accessing_fixed_vector_base<_TPointerToLender, _TLender, _Ty>, xslta_accessing_fixed_vector_base2<_TPointerToLender, _TLender, _Ty> >;
			}
		}

		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
		class xslta_accessing_fixed_vector : public impl::ns_xslta_accessing_fixed_vector::xslta_accessing_fixed_vector_base3<_TPointerToLender, _TLender, _Ty> {
			typedef impl::ns_xslta_accessing_fixed_vector::xslta_accessing_fixed_vector_base3<_TPointerToLender, _TLender, _Ty> base_class;
		public:
			typedef xslta_accessing_fixed_vector _Myt;

			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class)
			//typedef typename corresponding_xslta_fixed_vector_t::allocator_type allocator_type;

			typedef typename base_class::const_iterator const_iterator;
			typedef typename std::conditional<std::is_const<_TLender>::value, const_iterator, typename base_class::iterator>::type iterator;

			typedef typename base_class::const_reverse_iterator const_reverse_iterator;
			typedef typename std::conditional<std::is_const<_TLender>::value, const_reverse_iterator, typename base_class::reverse_iterator>::type reverse_iterator;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_accessing_fixed_vector(xslta_accessing_fixed_vector&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			xslta_accessing_fixed_vector(xslta_accessing_fixed_vector&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED

			xslta_accessing_fixed_vector(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])")) : base_class(src_xs_ptr) {}

		private:
			xslta_accessing_fixed_vector(const xslta_accessing_fixed_vector&) = delete;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)")
			MSE_ATTR_STR("mse::lifetime_label_for_base_class(alias_11$)");

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _TPointerToLender>
		xslta_accessing_fixed_vector(_TPointerToLender) -> xslta_accessing_fixed_vector<_TPointerToLender>;
#endif /* MSE_HAS_CXX17 */

		template<class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender> >
		auto make_xslta_accessing_fixed_vector(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"))
			MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); return_value(alias_11$) }")
		{
			return xslta_accessing_fixed_vector<_TPointerToLender, _TLender, _Ty>(src_xs_ptr);
		}


		template<class TAccessingFixed, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		class TXSLTADynamicRAContainerElementProxyRef : public mse::us::impl::XSLTATagBase {
		public:
			typedef TAccessingFixed accessing_fixed_type;
			typedef _TPointerToLender pointer_to_lender_type;
			typedef _TLender lender_type;
			typedef _Ty element_type;
			typedef _A allocator_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyRef(TXSLTADynamicRAContainerElementProxyRef&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyRef(TXSLTADynamicRAContainerElementProxyRef&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyRef(const TXSLTADynamicRAContainerElementProxyRef&) = delete;

#if 0
			TXSLTADynamicRAContainerElementProxyRef(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}
			TXSLTADynamicRAContainerElementProxyRef(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}
#endif // 0
			TXSLTADynamicRAContainerElementProxyRef(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}


			operator _Ty() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return m_bf_container[m_index];
			}
			void operator=(const _Ty& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) && {
				m_bf_container[m_index] = src_ref;
			}
			void operator=(_Ty&& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) && {
				m_bf_container[m_index] = MSE_FWD(src_ref);
			}
			void operator=(const _Ty& src_ref)& = delete;
			void operator=(_Ty&& src_ref)& = delete;
			void operator=(const TXSLTADynamicRAContainerElementProxyRef& src_ref MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))&& {
				m_bf_container[m_index] = _Ty(src_ref);
			}

		private:
			TAccessingFixed m_bf_container MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			size_t m_index = 0;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TAccessingFixed, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		class TXSLTADynamicRAContainerElementProxyConstRef : public mse::us::impl::XSLTATagBase {
		public:
			typedef TAccessingFixed accessing_fixed_type;
			typedef _TPointerToLender pointer_to_lender_type;
			typedef _TLender lender_type;
			typedef _Ty element_type;
			typedef _A allocator_type;

#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstRef(TXSLTADynamicRAContainerElementProxyConstRef&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstRef(TXSLTADynamicRAContainerElementProxyConstRef&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstRef(const TXSLTADynamicRAContainerElementProxyConstRef&) = delete;

#if 0
			TXSLTADynamicRAContainerElementProxyConstRef(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}
			TXSLTADynamicRAContainerElementProxyConstRef(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}
#endif // 0
			TXSLTADynamicRAContainerElementProxyConstRef(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"), size_t index = 0) : m_bf_container(src_xs_ptr), m_index(index) {}


			operator _Ty() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return m_bf_container[m_index];
			}
			template<typename _Ty2>
			void operator=(const _Ty2& src_ref) = delete;

		private:
			TAccessingFixed m_bf_container MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
			size_t m_index = 0;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TElementProxyRef, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		class TXSLTADynamicRAContainerElementProxyPtr : public mse::us::impl::XSLTATagBase {
		public:
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyPtr(TXSLTADynamicRAContainerElementProxyPtr&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyPtr(TXSLTADynamicRAContainerElementProxyPtr&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyPtr(const TXSLTADynamicRAContainerElementProxyPtr&) = delete;

#if 0
			TXSLTADynamicRAContainerElementProxyPtr(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}
			TXSLTADynamicRAContainerElementProxyPtr(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}
#endif // 0
			TXSLTADynamicRAContainerElementProxyPtr(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}

			auto operator->() const && MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}
			auto operator->() && MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}

		private:
			TElementProxyRef m_proxy_ref MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

		template<class TElementProxyConstRef, class _TPointerToLender, class _TLender = mse::impl::target_type<_TPointerToLender>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		class TXSLTADynamicRAContainerElementProxyConstPtr : public mse::us::impl::XSLTATagBase {
		public:
#ifndef MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstPtr(TXSLTADynamicRAContainerElementProxyConstPtr&&) = delete;
#else // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstPtr(TXSLTADynamicRAContainerElementProxyConstPtr&&) = default;
#endif // !MSE_IMPL_MOVE_ENABLED_FOR_BORROWING_FIXED
			TXSLTADynamicRAContainerElementProxyConstPtr(const TXSLTADynamicRAContainerElementProxyConstPtr&) = delete;

#if 0
			TXSLTADynamicRAContainerElementProxyConstPtr(const mse::rsv::TXSLTAPointer<_TLender> src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}
			TXSLTADynamicRAContainerElementProxyConstPtr(_TLender* src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}
#endif // 0
			TXSLTADynamicRAContainerElementProxyConstPtr(const _TPointerToLender& src_xs_ptr MSE_ATTR_PARAM_STR("mse::lifetime_label(_[_[alias_11$]])"), size_t index = 0) : m_proxy_ref(src_xs_ptr, index) {}

			auto operator->() const&& MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				return std::addressof(m_proxy_ref);
			}

		private:
			TElementProxyConstRef m_proxy_ref MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");


		template <class TElementProxyConstRef, typename _TRAContainerPointer> class TXSLTADynamicContainerRAConstIterator;
		template <class TElementProxyRef, typename _TRAContainerPointer> class TXSLTADynamicContainerRAIterator;
		namespace us {
			/* A couple of unsafe functions for internal use. */
			template<class TElementProxyRef, class _TRAContainerPointer>
			TXSLTADynamicContainerRAIterator<TElementProxyRef, _TRAContainerPointer> unsafe_make_xslta_dynamic_container_ra_iterator(const _TRAContainerPointer& ra_container_pointer, typename TXSLTADynamicContainerRAIterator<TElementProxyRef, _TRAContainerPointer>::size_type index/* = 0*/);
			template<class TElementProxyConstRef, class _TRAContainerPointer>
			TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef, _TRAContainerPointer> unsafe_make_xslta_dynamic_container_ra_const_iterator(const _TRAContainerPointer& ra_container_pointer, typename TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef, _TRAContainerPointer>::size_type index/* = 0*/);
		}

		template <class TElementProxyRef, typename _TRAContainerPointer>
		class TXSLTADynamicContainerRAIterator : public TXSLTARAIterator<_TRAContainerPointer> {
		public:
			typedef TXSLTARAIterator<_TRAContainerPointer> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TXSLTADynamicContainerRAIterator(const TXSLTADynamicContainerRAIterator& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}
			TXSLTADynamicContainerRAIterator(TXSLTADynamicContainerRAIterator&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}

			template<class TElementProxyRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAIterator(TXSLTADynamicContainerRAIterator<TElementProxyRef2, _Ty2>&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}
			template<class TElementProxyRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAIterator(const TXSLTADynamicContainerRAIterator<TElementProxyRef2, _Ty2>& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}

			TXSLTADynamicContainerRAIterator(const _TRAContainerPointer& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index) : base_class(ra_container_pointer, index) {}
			TXSLTADynamicContainerRAIterator(_TRAContainerPointer&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index) : base_class(MSE_FWD(ra_container_pointer), index) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAIterator(const _Ty2& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index) : base_class(ra_container_pointer, index) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAIterator(_Ty2&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index) : base_class(MSE_FWD(ra_container_pointer), index) {}

			TXSLTADynamicContainerRAIterator(const TXSLTARAIterator<_TRAContainerPointer>& xs_ra_iter MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(xs_ra_iter) {}
			TXSLTADynamicContainerRAIterator(TXSLTARAIterator<_TRAContainerPointer>&& xs_ra_iter MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(xs_ra_iter)) {}

			MSE_INHERIT_LTA_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXSLTADynamicContainerRAIterator, "99");
			MSE_INHERIT_LTA_ASSIGNMENT_OPERATOR_FROM(base_class, TXSLTADynamicContainerRAIterator, "99");
			//MSE_USING_ASSIGNMENT_OPERATOR(base_class);

			/* Dereference operators of dynamic container iterators, by default, can't provide (minimum) lifetime
			guarantees for their return values. It could be argued that they shouldn't even have dereference operators. */
			TElementProxyRef operator*() const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyRef(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), this->position());
				//return base_class::operator*();
			}
			typedef typename std::add_pointer<value_type>::type pointer_t;
			typedef TXSLTADynamicRAContainerElementProxyPtr<TElementProxyRef, typename TElementProxyRef::pointer_to_lender_type, typename TElementProxyRef::lender_type, typename TElementProxyRef::element_type, typename TElementProxyRef::allocator_type> TElementProxyPtr;
			TElementProxyPtr operator->() const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyPtr(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), this->position());
				//return base_class::operator->();
			}
			TElementProxyRef operator[](difference_type _Off) const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyRef(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), difference_type(this->position()) + _Off);
				//return base_class::operator[](_Off);
			}

			bool is_valid() const {
				return ((0 <= (*this).position()) && (difference_type(mse::container_size(*(*this).target_container_ptr())) >= (*this).position()));
			}

			MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
			void xslta_iterator_tag() const {}
			void xslta_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			class unsafe_t {};
			TXSLTADynamicContainerRAIterator(unsafe_t, const _TRAContainerPointer& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(ra_container_pointer, index) {}
			TXSLTADynamicContainerRAIterator(unsafe_t, _TRAContainerPointer&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(MSE_FWD(ra_container_pointer), index) {}

			MSE_DEFAULT_OPERATOR_NEW_DECLARATION;

			friend class TXSLTADynamicContainerRAConstIterator<TElementProxyRef, _TRAContainerPointer>;
			template<class TElementProxyRef2, class _TRAContainerPointer2> friend TXSLTADynamicContainerRAIterator<TElementProxyRef2, _TRAContainerPointer2> mse::rsv::us::unsafe_make_xslta_dynamic_container_ra_iterator(const _TRAContainerPointer2& ra_container_pointer, typename TXSLTADynamicContainerRAIterator<TElementProxyRef2, _TRAContainerPointer2>::size_type index);
		} MSE_ATTR_STR("mse::lifetime_labels(99)") MSE_ATTR_STR("mse::lifetime_label_for_base_class(99)");

		template <class TElementProxyConstRef, typename _TRAContainerPointer>
		class TXSLTADynamicContainerRAConstIterator : public TXSLTARAConstIterator<_TRAContainerPointer> {
		public:
			typedef TXSLTARAConstIterator<_TRAContainerPointer> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TXSLTADynamicContainerRAConstIterator(const TXSLTADynamicContainerRAConstIterator& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}
			TXSLTADynamicContainerRAConstIterator(TXSLTADynamicContainerRAConstIterator&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}

			//TXSLTADynamicContainerRAConstIterator(const TXSLTADynamicContainerRAIterator<_TRAContainerPointer>& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}
			//TXSLTADynamicContainerRAConstIterator(TXSLTADynamicContainerRAIterator<_TRAContainerPointer>&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}

			template<class TElementProxyConstRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef2, _Ty2>&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}
			template<class TElementProxyConstRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(const TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef2, _Ty2>& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}

			template<class TElementProxyConstRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(TXSLTADynamicContainerRAIterator<TElementProxyConstRef2, _Ty2>&& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(src)) {}
			template<class TElementProxyConstRef2, class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(const TXSLTADynamicContainerRAIterator<TElementProxyConstRef2, _Ty2>& src MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(src) {}

			TXSLTADynamicContainerRAConstIterator(const _TRAContainerPointer& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(ra_container_pointer, index) {}
			TXSLTADynamicContainerRAConstIterator(_TRAContainerPointer&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(MSE_FWD(ra_container_pointer), index) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(const _Ty2& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(ra_container_pointer, index) {}
			template<class _Ty2, MSE_IMPL_EIP mse::impl::enable_if_t<std::is_convertible<_Ty2, _TRAContainerPointer>::value> MSE_IMPL_EIS >
			TXSLTADynamicContainerRAConstIterator(_Ty2&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(MSE_FWD(ra_container_pointer), index) {}

			TXSLTADynamicContainerRAConstIterator(const TXSLTARAConstIterator<_TRAContainerPointer>& xs_ra_iter MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(xs_ra_iter) {}
			TXSLTADynamicContainerRAConstIterator(TXSLTARAConstIterator<_TRAContainerPointer>&& xs_ra_iter MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])")) : base_class(MSE_FWD(xs_ra_iter)) {}

			MSE_INHERIT_LTA_ITERATOR_ARITHMETIC_OPERATORS_FROM(base_class, TXSLTADynamicContainerRAConstIterator, "99");
			MSE_INHERIT_LTA_ASSIGNMENT_OPERATOR_FROM(base_class, TXSLTADynamicContainerRAConstIterator, "99");
			//MSE_USING_ASSIGNMENT_OPERATOR(base_class);

			/* Dereference operators of dynamic container iterators, by default, can't provide (minimum) lifetime
			guarantees for their return values. It could be argued that they shouldn't even have dereference operators. */
			TElementProxyConstRef operator*() const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyConstRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyConstRef(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), this->position());
				//return base_class::operator*();
			}
			typedef typename std::add_pointer<value_type>::type pointer_t;
			typedef TXSLTADynamicRAContainerElementProxyPtr<TElementProxyConstRef, typename TElementProxyConstRef::pointer_to_lender_type, typename TElementProxyConstRef::lender_type, typename TElementProxyConstRef::element_type, typename TElementProxyConstRef::allocator_type> TElementProxyConstPtr;
			TElementProxyConstPtr operator->() const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyConstRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyPtr(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), this->position());
				//return base_class::operator->();
			}
			TElementProxyConstRef operator[](difference_type _Off) const /*MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(99) }")*/
				MSE_ATTR_FUNC_STR("mse::lifetime_set_alias_from_template_parameter_by_name(TElementProxyConstRef, alias_11$)")
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ labels(alias_11$); this(_[_[alias_11$]]); return_value(alias_11$) }")
			{
				return TElementProxyConstRef(mse::rsv::xslta_ptr_to(*(this->target_container_ptr())), difference_type(this->position()) + _Off);
				//return base_class::operator[](_Off);
			}

			bool is_valid() const {
				return ((0 <= (*this).position()) && (difference_type(mse::container_size(*(*this).target_container_ptr())) >= (*this).position()));
			}

			MSE_INHERIT_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(base_class);
			void xslta_iterator_tag() const {}
			void xslta_tag() const {}
			MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

		private:
			class unsafe_t {};
			TXSLTADynamicContainerRAConstIterator(unsafe_t, const _TRAContainerPointer& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(ra_container_pointer, index) {}
			TXSLTADynamicContainerRAConstIterator(unsafe_t, _TRAContainerPointer&& ra_container_pointer MSE_ATTR_PARAM_STR("mse::lifetime_labels(_[99])"), size_type index = 0) : base_class(MSE_FWD(ra_container_pointer), index) {}

			MSE_DEFAULT_OPERATOR_NEW_DECLARATION;

			template<class TElementProxyConstRef2, class _TRAContainerPointer2> friend TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef2, _TRAContainerPointer2> mse::rsv::us::unsafe_make_xslta_dynamic_container_ra_const_iterator(const _TRAContainerPointer2& ra_container_pointer, typename TXSLTADynamicContainerRAConstIterator<TElementProxyConstRef2, _TRAContainerPointer2>::size_type index);
		} MSE_ATTR_STR("mse::lifetime_labels(99)") MSE_ATTR_STR("mse::lifetime_label_for_base_class(99)");

		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		using TXSLTAVectorElementProxyRef = mse::rsv::TXSLTADynamicRAContainerElementProxyRef<mse::rsv::xslta_accessing_fixed_vector<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty, _A>;
		template<class _TPointer, class _TLender = mse::impl::target_type<_TPointer>, class _Ty = mse::impl::container_element_type<_TLender>, class _A = mse::impl::container_allocator_type_if_available<_TLender> >
		using TXSLTAVectorElementProxyConstRef = mse::rsv::TXSLTADynamicRAContainerElementProxyConstRef<mse::rsv::xslta_accessing_fixed_vector<_TPointer, _TLender, _Ty>, _TPointer, _TLender, _Ty, _A>;


		template<class _Ty, class _A/* = std::allocator<_Ty>*/>
		class xslta_vector : public mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag>, public mse::us::impl::XSLTATagBase, public mse::us::impl::AsyncNotShareableTagBase {
		public:
			typedef mse::us::impl::gnii_vector<_Ty, _A, mse::non_thread_safe_shared_mutex, mse::impl::ns_gnii_vector::Tgnii_vector_xscope_ss_const_iterator_type, mse::us::impl::gnii_vector_const_lockable_tag> base_class;
			typedef mse::non_thread_safe_shared_mutex _TStateMutex;
			typedef xslta_vector _Myt;
			typedef typename base_class::std_vector std_vector;

			typedef typename base_class::allocator_type allocator_type;
			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

			explicit xslta_vector(size_type _N, const _Ty& _V MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _A& _Al = _A()) : base_class(_N, _V, _Al) {}

			/* If an initialization value is not given, any lifetimes will be "deduced" to be a (minimum) default value. */
			explicit xslta_vector(const _A& _Al = _A()) : base_class(_Al) {}
			/* Constructs an empty container, uses the second argument only to deduce lifetime. */
			explicit xslta_vector(nullopt_t, const _Ty& MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _A& _Al = _A()) : base_class(_Al) {}
			template<class _Iter
				//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			xslta_vector(const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last, const _A& _Al) : base_class(_First, _Last, _Al) { /*m_debug_size = size();*/ }

			xslta_vector(const xslta_vector& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) = default;
			xslta_vector(xslta_vector&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) = default;

			xslta_vector(std_vector&& _X) : base_class(MSE_FWD(_X)) {}
			xslta_vector(const std_vector& _X) : base_class(_X) {}

			xslta_vector& operator=(xslta_vector&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") = default;
			xslta_vector& operator=(const xslta_vector& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") = default;

			xslta_vector& operator=(nullopt_t) noexcept MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {
				base_class::clear();
				return *this;
			}

			void resize(size_type _N, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) {
				base_class::resize(_N, _X);
			}
			void push_back(_Ty&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) {
				base_class::push_back(MSE_FWD(_X));
			}
			void push_back(const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) {
				base_class::push_back(_X);
			}
			template<class _Iter>
			void assign(const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last) {	// assign [_First, _Last)
				base_class::assign(_First, _Last);
			}
			void assign(size_type _N, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])")) {
				base_class::assign(_N, _X);
			}

			void swap(xslta_vector& rhs MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_12$])")) noexcept(std::is_nothrow_move_constructible<_Ty>::value && noexcept(std::swap(std::declval<_Ty&>(), std::declval<_Ty&>())))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ set_alias_from_template_parameter_by_name(_Ty, alias_12$); labels(alias_12$); encompasses(alias_11$, alias_12$); encompasses(alias_12$, alias_11$) }")
			{
				base_class::swap(rhs);
			}

			xslta_vector(_XSTD initializer_list<value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"), const _A& _Al = _A())
				: base_class(_Ilist, _Al) {	// construct from initializer_list
				/*m_debug_size = size();*/
			}
			_Myt& operator=(_XSTD initializer_list<value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)")) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {	// assign initializer_list
				base_class::operator=(_Ilist);
				return (*this);
			}
			void assign(_XSTD initializer_list<value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)")) {	// assign initializer_list
				base_class::assign(_Ilist);
				/*m_debug_size = size();*/
			}

			auto operator[](msev_size_t _P) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return TXSLTAVectorElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), _P);
				//return (*this).at(msev_as_a_size_t(_P));
			}
			auto operator[](msev_size_t _P) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {
				return TXSLTAVectorElementProxyConstRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), _P);
				//return (*this).at(msev_as_a_size_t(_P));
			}
			auto front() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {	// return first element of mutable sequence
				return TXSLTAVectorElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), 0/*index*/);
				//return base_class::front();
			}
			auto front() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {	// return first element of mutable sequence
				return TXSLTAVectorElementProxyConstRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), 0/*index*/);
				//return base_class::front();
			}
			auto back() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {	// return last element of mutable sequence
				return TXSLTAVectorElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), (*this).size() - 1);
				//return base_class::back();
			}
			auto back() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }") {	// return last element of mutable sequence
				return TXSLTAVectorElementProxyConstRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), (*this).size() - 1);
				//return base_class::back();
			}

			auto at(msev_size_t _Pos) MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }")
			{	// subscript mutable sequence with checking
				return TXSLTAVectorElementProxyRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), _Pos);
				//return base_class::at(msev_as_a_size_t(_Pos));
			}
			auto at(msev_size_t _Pos) const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ return_value(alias_11$) }")
			{	// subscript mutable sequence with checking
				return TXSLTAVectorElementProxyConstRef<decltype(mse::rsv::xslta_ptr_to(*this)), _Myt, _Ty, _A>(mse::rsv::xslta_ptr_to(*this), _Pos);
				//return base_class::at(msev_as_a_size_t(_Pos));
			}

			typedef _Myt& _Myt_ref;


			typedef mse::TXScopeRAConstIterator<mse::TXScopeFixedConstPointer<_Myt> > xscope_const_iterator;
			typedef mse::TXScopeRAIterator<mse::TXScopeFixedPointer<_Myt> > xscope_iterator;

			typedef TXSLTADynamicContainerRAConstIterator<TXSLTAVectorElementProxyConstRef<mse::rsv::TXSLTAConstPointer<_Myt>, _Myt, _Ty, _A>, mse::rsv::TXSLTAConstPointer<_Myt> > xslta_const_iterator;
			typedef TXSLTADynamicContainerRAIterator<TXSLTAVectorElementProxyRef<mse::rsv::TXSLTAPointer<_Myt>, _Myt, _Ty, _A>, mse::rsv::TXSLTAPointer<_Myt> > xslta_iterator;
			typedef xslta_const_iterator const_iterator;
			typedef xslta_iterator iterator;

			xslta_iterator begin() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return xslta_iterator(mse::rsv::TXSLTAPointer<_Myt>(this), 0/*index*/);
			}
			xslta_const_iterator begin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cbegin();
			}
			xslta_const_iterator cbegin() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return xslta_const_iterator(mse::rsv::TXSLTAConstPointer<_Myt>(this), 0/*index*/);
			}
			xslta_iterator end() MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return xslta_iterator(mse::rsv::TXSLTAPointer<_Myt>(this), 0/*index*/) + (*this).size();
			}
			xslta_const_iterator end() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return cend();
			}
			xslta_const_iterator cend() const MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(99); this(99); return_value(99) }") {
				return xslta_const_iterator(mse::rsv::TXSLTAConstPointer<_Myt>(this), 0/*index*/) + (*this).size();
			}

			auto insert(const xslta_const_iterator& pos, _Ty&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), MSE_FWD(_X));
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_const_iterator& pos, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), _X);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_const_iterator& pos, size_type _M, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), _M, _X);
				return xslta_iterator(this, xs_iter.position());
			}
			template<class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
			auto insert(const xslta_const_iterator& pos, const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), _First, _Last);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_const_iterator& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), _Ilist);
				return xslta_iterator(this, xs_iter.position());
			}
			auto emplace(const xslta_const_iterator& pos, _Ty&& _Val MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::emplace(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position(), std::forward<_Ty>(_Val));
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(const xslta_const_iterator& pos)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::erase(mse::us::unsafe_make_xscope_pointer_to(*this), pos.position());
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(const xslta_const_iterator& start, const xslta_const_iterator& end)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::erase(mse::us::unsafe_make_xscope_pointer_to(*this), start.position(), end.position());
				return xslta_iterator(this, xs_iter.position());
			}

			auto insert(const xslta_iterator& pos, _Ty&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = insert(xslta_const_iterator(pos), MSE_FWD(_X));
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_iterator& pos, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = insert(xslta_const_iterator(pos), _X);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_iterator& pos, size_type _M, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = insert(xslta_const_iterator(pos), _M, _X);
				return xslta_iterator(this, xs_iter.position());
			}
			template<class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
			auto insert(const xslta_iterator& pos, const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = insert(xslta_const_iterator(pos), _First, _Last);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(const xslta_iterator& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = insert(xslta_const_iterator(pos), _Ilist);
				return xslta_iterator(this, xs_iter.position());
			}
			auto emplace(const xslta_iterator& pos, _Ty&& _Val MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = emplace(xslta_const_iterator(pos), std::forward<_Ty>(_Val));
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(const xslta_iterator& pos)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = erase(xslta_const_iterator(pos));
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(const xslta_iterator& start, const xslta_iterator& end)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = erase(xslta_const_iterator(start), xslta_const_iterator(end));
				return xslta_iterator(this, xs_iter.position());
			}


			auto insert(size_type pos, _Ty&& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos, MSE_FWD(_X));
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(size_type pos, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos, _X);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(size_type pos, size_type _M, const _Ty& _X MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos, _M, _X);
				return xslta_iterator(this, xs_iter.position());
			}
			template<class _Iter, class = mse::impl::_mse_RequireInputIter<_Iter> >
			auto insert(size_type pos, const _Iter _First MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"), const _Iter _Last)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos, _First, _Last);
				return xslta_iterator(this, xs_iter.position());
			}
			auto insert(size_type pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist MSE_ATTR_PARAM_STR("mse::lifetime_label(alias_11$)"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::insert(mse::us::unsafe_make_xscope_pointer_to(*this), pos, _Ilist);
				return xslta_iterator(this, xs_iter.position());
			}
			auto emplace(size_type pos, _Ty&& _Val MSE_ATTR_PARAM_STR("mse::lifetime_label(_[alias_11$])"))
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::emplace(mse::us::unsafe_make_xscope_pointer_to(*this), pos, std::forward<_Ty>(_Val));
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(size_type pos)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::erase(mse::us::unsafe_make_xscope_pointer_to(*this), pos);
				return xslta_iterator(this, xs_iter.position());
			}
			auto erase(size_type start, size_type end)
				MSE_ATTR_FUNC_STR("mse::lifetime_notes{ label(42); this(42); return_value(42) }") {

				auto xs_iter = base_class::erase(mse::us::unsafe_make_xscope_pointer_to(*this), start, end);
				return xslta_iterator(this, xs_iter.position());
			}


			/* This type can be safely used as a function return value if the element it contains is also safely returnable. */
			template<class T2 = _Ty, MSE_IMPL_EIP mse::impl::enable_if_t<(std::is_same<T2, _Ty>::value) && (
				(std::integral_constant<bool, mse::impl::HasXScopeReturnableTagMethod<T2>::Has>()) || (mse::impl::is_potentially_not_xscope<T2>::value)
				)> MSE_IMPL_EIS >
			void xscope_returnable_tag() const {} /* Indication that this type can be used as a function return value. */

			MSE_INHERIT_XSCOPE_ASYNC_SHAREABILITY_AND_PASSABILITY_OF(_Ty);
			MSE_DEFAULT_OPERATOR_AMPERSAND_DECLARATION;

		private:

			typedef base_class _MV;

			typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<base_class> xscope_shared_structure_lock_guard_t;
			static auto s_make_xscope_shared_structure_lock_guard(_Myt const & vec_ref) -> xscope_shared_structure_lock_guard_t {
				MSE_SUPPRESS_CHECK_IN_XSCOPE return xscope_shared_structure_lock_guard_t(mse::us::unsafe_make_xscope_pointer_to(vec_ref));
			}
			//typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<base_class> xscope_shared_const_structure_lock_guard_t;

			friend class mse::us::impl::Txscope_shared_structure_lock_guard<_Myt>;
			friend class mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>;
			template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::rsv::impl::ns_xslta_accessing_fixed_vector::xslta_accessing_fixed_vector_base2;
			template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::rsv::us::impl::xslta_accessing_fixed_vector_base;
			template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::impl::ns_xscope_accessing_fixed_nii_vector::xscope_accessing_fixed_nii_vector_base2;
			template<class _TPointer2, class _TLender2, class _Ty2> friend class mse::us::impl::xscope_accessing_fixed_nii_vector_base;
		} MSE_ATTR_STR("mse::lifetime_set_alias_from_template_parameter_by_name(_Ty, alias_11$)")
			MSE_ATTR_STR("mse::lifetime_labels(alias_11$)");

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
		xslta_vector(_Iter, _Iter, _Alloc = _Alloc())
			-> xslta_vector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */
	}

	namespace impl {
		template<class _Ty, class _A/* = std::allocator<_Ty>*/>
		struct can_be_structure_locked_as_const<mse::rsv::xslta_vector<_Ty, _A> > : std::true_type {};
	}

	namespace mstd {
#ifndef MSE_MSTDVECTOR_DISABLED
		/* forward declaration because mstd::vector<> needs to be declared a friend of us::msevector<> (in its current
		implementation) */
		template<class _Ty, class _A/* = std::allocator<_Ty> */>
		class vector;
#endif /*!MSE_MSTDVECTOR_DISABLED*/
	}

	namespace us {

		namespace ns_msevector {
			template<class _TContainer> class xscope_shared_structure_lock_guard;
			template<class _TContainer> class xscope_shared_const_structure_lock_guard;
		}

		/* msevector<> is an unsafe extension of stnii_vector<> that provides the traditional begin() and end() (non-static)
		member functions that return unsafe iterators. It also provides ss_begin() and ss_end() (non-static) member
		functions which return bounds-checked, but still technically unsafe iterators. */
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
		class msevector : public stnii_vector<_Ty, _A>/*, public us::impl::AsyncNotShareableTagBase*/ {
		public:
			typedef stnii_vector<_Ty, _A> base_class;
			typedef std::vector<_Ty, _A> std_vector;
			typedef msevector _Myt;

			typedef typename base_class::allocator_type allocator_type;
			MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(base_class);

			typedef typename base_class::_MV::iterator iterator;
			typedef typename base_class::_MV::const_iterator const_iterator;
			typedef typename base_class::_MV::reverse_iterator reverse_iterator;
			typedef typename base_class::_MV::const_reverse_iterator const_reverse_iterator;

			explicit msevector(const _A& _Al = _A())
				: base_class(_Al), m_mmitset(*this) {
				/*m_debug_size = size();*/
			}
			explicit msevector(size_type _N)
				: base_class(msev_as_a_size_t(_N)), m_mmitset(*this) {
				/*m_debug_size = size();*/
			}
			explicit msevector(size_type _N, const _Ty& _V, const _A& _Al = _A())
				: base_class(msev_as_a_size_t(_N), _V, _Al), m_mmitset(*this) {
				/*m_debug_size = size();*/
			}
			msevector(std_vector&& _X) : base_class(MSE_FWD(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const std_vector& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(base_class&& _X) : base_class(MSE_FWD(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const base_class& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(_Myt&& _X) : base_class(MSE_FWD(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const _Myt& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			typedef typename base_class::const_iterator _It;
			/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
			msevector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				msevector(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last), m_mmitset(*this) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, MSE_IMPL_EIP mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value> MSE_IMPL_EIS
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				//msevector(const _Iter& _First, const _Iter& _Last, const typename base_class::_Alloc& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
				msevector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			_Myt& operator=(_Myt&& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::operator=(MSE_FWD(_X));
				m_mmitset.reset();
				return (*this);
			}
			_Myt& operator=( _Myt& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::operator=(_X);
				m_mmitset.reset();
				return (*this);
			}
			_Myt& operator=(const _Myt& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::operator=(_X);
				m_mmitset.reset();
				return (*this);
			}
			void reserve(size_type _Count)
			{	// determine new minimum length of allocated storage
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				auto original_capacity = msev_size_t((*this).capacity());

				base_class::reserve(msev_as_a_size_t(_Count));

				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
			}
			void shrink_to_fit() {	// reduce capacity
				auto original_capacity = msev_size_t((*this).capacity());

				base_class::shrink_to_fit();

				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());
				bool shrinking = (_N < original_size);

				base_class::resize(msev_as_a_size_t(_N), _X);
				/*m_debug_size = size();*/

				if (shrinking) {
					m_mmitset.invalidate_inclusive_range(_N, msev_size_t(original_size - 1));
				}
				m_mmitset.shift_inclusive_range(original_size, original_size, msev_size_t(_N) - original_size); /*shift the end markers*/
				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
			}
			typename base_class::const_reference operator[](size_type _P) const {
				return (*this).at(msev_as_a_size_t(_P));
			}
			typename base_class::reference operator[](size_type _P) {
				return (*this).at(msev_as_a_size_t(_P));
			}
			typename base_class::reference front() {	// return first element of mutable sequence
				if (0 == (*this).size()) { MSE_THROW(msevector_range_error("front() on empty - typename base_class::reference front() - msevector")); }
				return base_class::front();
			}
			typename base_class::const_reference front() const {	// return first element of nonmutable sequence
				if (0 == (*this).size()) { MSE_THROW(msevector_range_error("front() on empty - typename base_class::const_reference front() - msevector")); }
				return base_class::front();
			}
			typename base_class::reference back() {	// return last element of mutable sequence
				if (0 == (*this).size()) { MSE_THROW(msevector_range_error("back() on empty - typename base_class::reference back() - msevector")); }
				return base_class::back();
			}
			typename base_class::const_reference back() const {	// return last element of nonmutable sequence
				if (0 == (*this).size()) { MSE_THROW(msevector_range_error("back() on empty - typename base_class::const_reference back() - msevector")); }
				return base_class::back();
			}
			void push_back(_Ty&& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::push_back(MSE_FWD(_X));
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::push_back(MSE_FWD(_X));
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + 1) == msev_size_t((*this).size()));
					m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			void push_back(const _Ty& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::push_back(_X);
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::push_back(_X);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + 1) == msev_size_t((*this).size()));
					m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			void pop_back() {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::pop_back();
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					if (0 == original_size) { MSE_THROW(msevector_range_error("pop_back() on empty - void pop_back() - msevector")); }
					base_class::pop_back();
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size - 1) == msev_size_t((*this).size()));
					m_mmitset.invalidate_inclusive_range(msev_size_t(original_size - 1), msev_size_t(original_size - 1));
					m_mmitset.shift_inclusive_range(original_size, original_size, -1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			void assign(_It _F, _It _L) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::assign(_F, _L);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			template<class _Iter>
			void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::assign(_First, _Last);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void assign(size_type _N, const _Ty& _X = _Ty()) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::assign(msev_as_a_size_t(_N), _X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			typename base_class::iterator insert(typename base_class::const_iterator _P, _Ty&& _X) {
				return (emplace(_P, MSE_FWD(_X)));
			}
			typename base_class::iterator insert(typename base_class::const_iterator _P, const _Ty& _X = _Ty()) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					typename base_class::iterator retval = base_class::insert(_P, _X);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _P);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					typename base_class::iterator retval = base_class::insert(_P, _X);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + 1) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					m_mmitset.shift_inclusive_range(d, original_size, 1);
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}

	#if !(defined(GPP4P8_COMPATIBLE))
			typename base_class::iterator insert(typename base_class::const_iterator _P, size_type _M, const _Ty& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					typename base_class::iterator retval = base_class::insert(_P, msev_as_a_size_t(_M), _X);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _P);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					typename base_class::iterator retval = base_class::insert(_P, msev_as_a_size_t(_M), _X);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					m_mmitset.shift_inclusive_range(d, original_size, msev_int(_M));
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				typename base_class::iterator insert(typename base_class::const_iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					auto retval = base_class::insert(_Where, _First, _Last);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _Where);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

					auto _M = msev_int(std::distance(_First, _Last));
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					//if (0 > _M) { MSE_THROW(msevector_range_error("invalid argument - typename base_class::iterator insert() - msevector")); }
					auto retval = base_class::insert(_Where, _First, _Last);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					m_mmitset.shift_inclusive_range(d, original_size, _M);
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}

	#else /*!(defined(GPP4P8_COMPATIBLE))*/

			/*typename base_class::iterator*/
			void
				/* g++4.8 seems to be using the c++98 version of this insert function instead of the c++11 version. */
				insert(typename base_class::/*const_*/iterator _P, size_t _M, const _Ty& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				msev_int di = std::distance(base_class::/*c*/begin(), _P);
				msev_size_t d = msev_size_t(di);
				if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				/*typename base_class::iterator retval =*/
				base_class::insert(_P, _M, _X);
				/*m_debug_size = size();*/

				assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
				/*assert(di == std::distance(base_class::begin(), retval));*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
				/*return retval;*/
			}
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value>
				, class = mse::impl::_mse_RequireInputIter<_Iter> > void
				insert(typename base_class::/*const_*/iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				msev_int di = std::distance(base_class::/*c*/begin(), _Where);
				msev_size_t d = msev_size_t(di);
				if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

				auto _M = msev_int(std::distance(_First, _Last));
				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				//if (0 > _M) { MSE_THROW(msevector_range_error("invalid argument - typename base_class::iterator insert() - msevector")); }
				/*auto retval =*/
				base_class::insert(_Where, _First, _Last);
				/*m_debug_size = size();*/

				assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
				/*assert(di == std::distance(base_class::begin(), retval));*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
				/*return retval;*/
			}
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/

			template<class ..._Valty>
			void emplace_back(_Valty&& ..._Val)
			{	// insert by moving into element at end
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + 1) == msev_size_t((*this).size()));
					m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			template<class ..._Valty>
	#if !(defined(GPP4P8_COMPATIBLE))
			typename base_class::iterator emplace(typename base_class::const_iterator _Where, _Valty&& ..._Val)
			{	// insert by moving _Val at _Where
	#else /*!(defined(GPP4P8_COMPATIBLE))*/
			typename base_class::iterator emplace(typename base_class::/*const_*/iterator _Where, _Valty&& ..._Val)
			{	// insert by moving _Val at _Where
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/

				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					auto retval = base_class::emplace(_Where, std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
					return retval;
				}
				else {

	#if !(defined(GPP4P8_COMPATIBLE))
					msev_int di = std::distance(base_class::cbegin(), _Where);
	#else /*!(defined(GPP4P8_COMPATIBLE))*/
					msev_int di = std::distance(base_class::/*c*/begin(), _Where);
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/

					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator emplace() - msevector")); }

					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					auto retval = base_class::emplace(_Where, std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + 1) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					m_mmitset.shift_inclusive_range(d, original_size, 1);
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}
			typename base_class::iterator erase(typename base_class::const_iterator _P) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					typename base_class::iterator retval = base_class::erase(_P);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _P);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator erase() - msevector")); }

					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					if (base_class::end() == _P) { MSE_THROW(msevector_range_error("invalid argument - typename base_class::iterator erase(typename base_class::const_iterator _P) - msevector")); }
					typename base_class::iterator retval = base_class::erase(_P);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size - 1) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					{
						m_mmitset.invalidate_inclusive_range(d, d);
						m_mmitset.shift_inclusive_range(msev_size_t(d + 1), original_size, -1);
					}
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}
			typename base_class::iterator erase(typename base_class::const_iterator _F, typename base_class::const_iterator _L) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					typename base_class::iterator retval = base_class::erase(_F, _L);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _F);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator erase() - msevector")); }
					msev_int di2 = std::distance(base_class::cbegin(), _L);
					if ((0 > di2) || ((*this).size() < msev_size_t(di2))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator erase() - msevector")); }

					auto _M = msev_int(std::distance(_F, _L));
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					if ((base_class::end() == _F)/* || (0 > _M)*/) { MSE_THROW(msevector_range_error("invalid argument - typename base_class::iterator erase(typename base_class::iterator _F, typename base_class::iterator _L) - msevector")); }
					typename base_class::iterator retval = base_class::erase(_F, _L);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size - _M) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					{
						if (1 <= _M) {
							m_mmitset.invalidate_inclusive_range(d, msev_size_t(d + _M - 1));
						}
						m_mmitset.shift_inclusive_range(msev_size_t(d + _M), original_size, -_M);
					}
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}
			void clear() {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::clear();
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(std::vector<_Ty, _A>& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::swap(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(base_class& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::swap(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(_Myt& _X) {
				(*this).swap(static_cast<base_class&>(_X));
				m_mmitset.reset();
			}
			template<typename _TStateMutex2, template<typename> class _TTXScopeConstIterator2, class TConstLockableIndicator2>
			void swap(mse::us::impl::gnii_vector<_Ty, _A, _TStateMutex2, _TTXScopeConstIterator2, TConstLockableIndicator2>& _X) {
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::swap(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}

			msevector(_XSTD initializer_list<typename base_class::value_type> _Ilist,
				const _A& _Al = _A())
				: base_class(_Ilist, _Al), m_mmitset(*this) {	// construct from initializer_list
																/*m_debug_size = size();*/
			}
			_Myt& operator=(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::operator=(_Ilist);
				m_mmitset.reset();
				return (*this);
			}
			void assign(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				base_class::assign(_Ilist);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
	#if defined(GPP4P8_COMPATIBLE)
			/* g++4.8 seems to be (incorrectly) using the c++98 version of this insert function instead of the c++11 version. */
			/*typename base_class::iterator*/void insert(typename base_class::/*const_*/iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				msev_int di = std::distance(base_class::/*c*/begin(), _Where);
				msev_size_t d = msev_size_t(di);
				if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

				auto _M = _Ilist.size();
				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				/*auto retval = */base_class::insert(_Where, _Ilist);
				/*m_debug_size = size();*/

				assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
				/*assert(di == std::distance(base_class::begin(), retval));*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = msev_size_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
				/*return retval;*/
			}
	#else /*defined(GPP4P8_COMPATIBLE)*/
			typename base_class::iterator insert(typename base_class::const_iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				structure_change_guard<decltype(m_structure_change_mutex)> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					auto retval = base_class::insert(_Where, _Ilist);
					/*m_debug_size = size();*/
					return retval;
				}
				else {
					msev_int di = std::distance(base_class::cbegin(), _Where);
					msev_size_t d = msev_size_t(di);
					if ((0 > di) || ((*this).size() < msev_size_t(di))) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

					auto _M = _Ilist.size();
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					auto retval = base_class::insert(_Where, _Ilist);
					/*m_debug_size = size();*/

					assert(msev_size_t(original_size + _M) == msev_size_t((*this).size()));
					assert(di == std::distance(base_class::begin(), retval));
					m_mmitset.shift_inclusive_range(d, original_size, msev_int(_M));
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
					return retval;
				}
			}
	#endif /*defined(GPP4P8_COMPATIBLE)*/

			//size_t m_debug_size;

			iterator begin() _NOEXCEPT {	// return iterator for beginning of mutable sequence
				return base_class::contained_vector().begin();
			}
			const_iterator begin() const _NOEXCEPT {	// return iterator for beginning of nonmutable sequence
				return base_class::contained_vector().begin();
			}
			iterator end() _NOEXCEPT {	// return iterator for end of mutable sequence
				return base_class::contained_vector().end();
			}
			const_iterator end() const _NOEXCEPT {	// return iterator for beginning of nonmutable sequence
				return base_class::contained_vector().end();
			}
			reverse_iterator rbegin() _NOEXCEPT {	// return iterator for beginning of reversed mutable sequence
				return base_class::contained_vector().rbegin();
			}
			const_reverse_iterator rbegin() const _NOEXCEPT {	// return iterator for beginning of reversed nonmutable sequence
				return base_class::contained_vector().rbegin();
			}
			reverse_iterator rend() _NOEXCEPT {	// return iterator for end of reversed mutable sequence
				return base_class::contained_vector().rend();
			}
			const_reverse_iterator rend() const _NOEXCEPT {	// return iterator for end of reversed nonmutable sequence
				return base_class::contained_vector().rend();
			}
			const_iterator cbegin() const _NOEXCEPT {	// return iterator for beginning of nonmutable sequence
				return base_class::contained_vector().cbegin();
			}
			const_iterator cend() const _NOEXCEPT {	// return iterator for end of nonmutable sequence
				return base_class::contained_vector().cend();
			}
			const_reverse_iterator crbegin() const _NOEXCEPT {	// return iterator for beginning of reversed nonmutable sequence
				return base_class::contained_vector().crbegin();
			}
			const_reverse_iterator crend() const _NOEXCEPT {	// return iterator for end of reversed nonmutable sequence
				return base_class::contained_vector().crend();
			}

			//class random_access_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
			//class random_access_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};
			class random_access_const_iterator_base {
			public:
				using iterator_category = std::random_access_iterator_tag;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(_Myt);
			};
			class random_access_iterator_base {
			public:
				using iterator_category = std::random_access_iterator_tag;
				MSE_INHERITED_RANDOM_ACCESS_MEMBER_TYPE_DECLARATIONS(_Myt);
			};

			class mm_iterator_type;

			/* mm_const_iterator_type acts much like a list iterator. */
			class mm_const_iterator_type : public random_access_const_iterator_base {
			public:
				typedef typename base_class::const_iterator::iterator_category iterator_category;
				typedef typename base_class::const_iterator::value_type value_type;
				//typedef typename base_class::const_iterator::difference_type difference_type;
				typedef msev_int difference_type;
				typedef difference_type distance_type;	// retained
				typedef typename base_class::const_iterator::pointer pointer;
				typedef typename base_class::const_iterator::pointer const_pointer;
				typedef typename base_class::const_iterator::reference reference;
				typedef typename base_class::const_reference const_reference;

				mm_const_iterator_type(const mm_iterator_type& src) : m_points_to_an_item(src.m_points_to_an_item)
					, m_index(src.m_index), m_owner_cptr(src.m_owner_cptr) {}
				void reset() { set_to_end_marker(); }
				bool points_to_an_item() const {
					if (m_points_to_an_item) { assert((1 <= m_owner_cptr->size()) && (m_index < m_owner_cptr->size())); return true; }
					else { assert(!((1 <= m_owner_cptr->size()) && (m_index < m_owner_cptr->size()))); return false; }
				}
				bool points_to_end_marker() const {
					if (false == points_to_an_item()) { assert(m_index == m_owner_cptr->size()); return true; }
					else { return false; }
				}
				bool points_to_beginning() const {
					if (0 == m_index) { return true; }
					else { return false; }
				}
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return points_to_an_item(); } //his is
																						 /* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return has_next_item_or_end_marker(); }
				bool has_previous() const { return (!points_to_beginning()); }
				void set_to_beginning() {
					m_index = 0;
					if (1 <= m_owner_cptr->size()) {
						m_points_to_an_item = true;
					}
					else { assert(false == m_points_to_an_item); }
				}
				void set_to_end_marker() {
					m_index = m_owner_cptr->size();
					m_points_to_an_item = false;
				}
				void set_to_next() {
					if (points_to_an_item()) {
						m_index += 1;
						if (m_owner_cptr->size() <= m_index) {
							(*this).m_points_to_an_item = false;
							if (m_owner_cptr->size() < m_index) { assert(false); reset(); }
						}
					}
					else {
						MSE_THROW(msevector_range_error("attempt to use invalid const_item_pointer - void set_to_next() - mm_const_iterator_type - msevector"));
					}
				}
				void set_to_previous() {
					if (has_previous()) {
						m_index -= 1;
						(*this).m_points_to_an_item = true;
					}
					else {
						MSE_THROW(msevector_range_error("attempt to use invalid const_item_pointer - void set_to_previous() - mm_const_iterator_type - msevector"));
					}
				}
				mm_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
				mm_const_iterator_type operator++(int) { mm_const_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
				mm_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
				mm_const_iterator_type operator--(int) { mm_const_iterator_type _Tmp = *this; --*this; return (_Tmp); }
				void advance(difference_type n) {
					auto new_index = msev_int(m_index) + n;
					if ((0 > new_index) || (m_owner_cptr->size() < msev_size_t(new_index))) {
						MSE_THROW(msevector_range_error("index out of range - void advance(difference_type n) - mm_const_iterator_type - msevector"));
					}
					else {
						m_index = msev_size_t(new_index);
						if (m_owner_cptr->size() <= m_index) {
							(*this).m_points_to_an_item = false;
						}
						else {
							(*this).m_points_to_an_item = true;
						}
					}
				}
				void regress(difference_type n) { advance(-n); }
				mm_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
				mm_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
				mm_const_iterator_type operator+(difference_type n) const {
					mm_const_iterator_type retval(*this);
					retval = (*this);
					retval += (n);
					return retval;
				}
				mm_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const mm_const_iterator_type &rhs) const {
					if ((rhs.m_owner_cptr) != ((*this).m_owner_cptr)) { MSE_THROW(msevector_range_error("invalid argument - difference_type operator-(const mm_const_iterator_type &rhs) const - msevector::mm_const_iterator_type")); }
					auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
					assert(difference_type(m_owner_cptr->size()) >= retval);
					return retval;
				}
				MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_GIVEN_SUBTRACTION(mm_const_iterator_type)

				const_reference operator*() const {
					return m_owner_cptr->at(msev_as_a_size_t(m_index));
				}
				const_reference item() const { return operator*(); }
				const_reference previous_item() const {
					return m_owner_cptr->at(msev_as_a_size_t(m_index - 1));
				}
				const_pointer operator->() const {
					return std::addressof(m_owner_cptr->at(msev_as_a_size_t(m_index)));
				}
				const_reference operator[](difference_type _Off) const { return (*m_owner_cptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }
				mm_const_iterator_type& operator=(const mm_const_iterator_type& _Right_cref)
				{
					if (((*this).m_owner_cptr) == (_Right_cref.m_owner_cptr)) {
						assert((*this).m_owner_cptr->size() >= _Right_cref.m_index);
						(*this).m_points_to_an_item = _Right_cref.m_points_to_an_item;
						(*this).m_index = _Right_cref.m_index;
					}
					else {
						MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
					}
					return (*this);
				}
				void set_to_const_item_pointer(const mm_const_iterator_type& _Right_cref) {
					(*this) = _Right_cref;
				}
				void invalidate_inclusive_range(msev_size_t index_of_first, msev_size_t index_of_last) {
					if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
						(*this).reset();
					}
				}
				void shift_inclusive_range(msev_size_t index_of_first, msev_size_t index_of_last, msev_int shift) {
					if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
						auto new_index = (*this).m_index + shift;
						if ((0 > new_index) || (m_owner_cptr->size() < new_index)) {
							MSE_THROW(msevector_range_error("void shift_inclusive_range() - mm_const_iterator_type - msevector"));
						}
						else {
							(*this).m_index = msev_size_t(new_index);
							(*this).sync_const_iterator_to_index();
						}
					}
				}
				msev_size_t position() const {
					return m_index;
				}
				operator typename base_class::const_iterator() const {
					typename base_class::const_iterator retval = (*m_owner_cptr).cbegin();
					retval += msev_as_a_size_t(m_index);
					return retval;
				}
				void async_not_shareable_and_not_passable_tag() const {}

				/* We actually want to make this constructor private, but doing so seems to break std::make_shared<mm_const_iterator_type>.  */
				mm_const_iterator_type(const _Myt& owner_cref) : m_owner_cptr(&owner_cref) { set_to_beginning(); }
			private:
				mm_const_iterator_type(const mm_const_iterator_type& src_cref) : m_owner_cptr(src_cref.m_owner_cptr) { (*this) = src_cref; }
				void sync_const_iterator_to_index() {
					assert(m_owner_cptr->size() >= (*this).m_index);
				}
				msev_bool m_points_to_an_item = false;
				msev_size_t m_index = 0;
				const _Myt* m_owner_cptr = nullptr;
				friend class mm_iterator_set_type;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class mm_iterator_type;
			};
			/* mm_iterator_type acts much like a list iterator. */
			class mm_iterator_type : random_access_iterator_base {
			public:
				typedef typename base_class::iterator::iterator_category iterator_category;
				typedef typename base_class::iterator::value_type value_type;
				//typedef typename base_class::iterator::difference_type difference_type;
				typedef msev_int difference_type;
				typedef difference_type distance_type;	// retained
				typedef typename base_class::iterator::pointer pointer;
				typedef typename base_class::iterator::reference reference;

				void reset() { set_to_end_marker(); }
				bool points_to_an_item() const {
					if (m_points_to_an_item) { assert((1 <= m_owner_ptr->size()) && (m_index < m_owner_ptr->size())); return true; }
					else { assert(!((1 <= m_owner_ptr->size()) && (m_index < m_owner_ptr->size())));  return false; }
				}
				bool points_to_end_marker() const {
					if (false == points_to_an_item()) { assert(m_index == m_owner_ptr->size()); return true; }
					else { return false; }
				}
				bool points_to_beginning() const {
					if (0 == m_index) { return true; }
					else { return false; }
				}
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return points_to_an_item(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return has_next_item_or_end_marker(); }
				bool has_previous() const { return (!points_to_beginning()); }
				void set_to_beginning() {
					m_index = 0;
					if (1 <= m_owner_ptr->size()) {
						m_points_to_an_item = true;
					}
					else { assert(false == m_points_to_an_item); }
				}
				void set_to_end_marker() {
					m_index = msev_size_t(m_owner_ptr->size());
					m_points_to_an_item = false;
				}
				void set_to_next() {
					if (points_to_an_item()) {
						m_index += 1;
						if (m_owner_ptr->size() <= m_index) {
							(*this).m_points_to_an_item = false;
							if (m_owner_ptr->size() < m_index) { assert(false); reset(); }
						}
					}
					else {
						MSE_THROW(msevector_range_error("attempt to use invalid item_pointer - void set_to_next() - mm_const_iterator_type - msevector"));
					}
				}
				void set_to_previous() {
					if (has_previous()) {
						m_index -= 1;
						(*this).m_points_to_an_item = true;
					}
					else {
						MSE_THROW(msevector_range_error("attempt to use invalid item_pointer - void set_to_previous() - mm_iterator_type - msevector"));
					}
				}
				reference operator*() const {
					return m_owner_ptr->at(msev_as_a_size_t(m_index));
				}
				reference item() const { return operator*(); }
				reference previous_item() const {
					return m_owner_ptr->at(msev_as_a_size_t(m_index - 1));
				}
				pointer operator->() const {
					return std::addressof(m_owner_ptr->at(msev_as_a_size_t(m_index)));
				}
				reference operator[](difference_type _Off) const { return (*m_owner_ptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }
				mm_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
				mm_iterator_type operator++(int) { mm_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
				mm_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
				mm_iterator_type operator--(int) { mm_iterator_type _Tmp = *this; --*this; return (_Tmp); }
				void advance(difference_type n) {
					auto new_index = msev_int(m_index) + n;
					if ((0 > new_index) || (m_owner_ptr->size() < msev_size_t(new_index))) {
						MSE_THROW(msevector_range_error("index out of range - void advance(difference_type n) - mm_iterator_type - msevector"));
					}
					else {
						m_index = msev_size_t(new_index);
						if (m_owner_ptr->size() <= m_index) {
							(*this).m_points_to_an_item = false;
						}
						else {
							(*this).m_points_to_an_item = true;
						}
					}
				}
				void regress(int n) { advance(-n); }
				mm_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
				mm_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
				mm_iterator_type operator+(difference_type n) const {
					mm_iterator_type retval(*this);
					retval = (*this);
					retval += (n);
					return retval;
				}
				mm_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const mm_const_iterator_type& _Right_cref) const {
					return (mm_const_iterator_type(*this) - _Right_cref);
				}
				MSE_IMPL_ORDERED_TYPE_OPERATOR_DELEGATING_DECLARATIONS(mm_iterator_type, mm_const_iterator_type)

				mm_iterator_type& operator=(const mm_iterator_type& _Right_cref)
				{
					if (((*this).m_owner_ptr) == (_Right_cref.m_owner_ptr)) {
						assert((*this).m_owner_ptr->size() >= _Right_cref.m_index);
						(*this).m_points_to_an_item = _Right_cref.m_points_to_an_item;
						(*this).m_index = _Right_cref.m_index;
					}
					else {
						MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
					}
					return (*this);
				}

				void set_to_item_pointer(const mm_iterator_type& _Right_cref) {
					(*this) = _Right_cref;
				}
				void invalidate_inclusive_range(msev_size_t index_of_first, msev_size_t index_of_last) {
					if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
						(*this).reset();
					}
				}
				void shift_inclusive_range(msev_size_t index_of_first, msev_size_t index_of_last, msev_int shift) {
					if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
						auto new_index = (*this).m_index + shift;
						if ((0 > new_index) || (m_owner_ptr->size() < new_index)) {
							MSE_THROW(msevector_range_error("void shift_inclusive_range() - mm_iterator_type - msevector"));
						}
						else {
							(*this).m_index = msev_size_t(new_index);
							(*this).sync_iterator_to_index();
						}
					}
				}
				msev_size_t position() const {
					return m_index;
				}
				/*
				operator mm_const_iterator_type() const {
					assert(m_owner_ptr);
					mm_const_iterator_type retval(*m_owner_ptr);
					retval.set_to_beginning();
					retval += (msev_int(m_index));
					return retval;
				}
				*/
				void async_not_shareable_and_not_passable_tag() const {}

				/* We actually want to make this constructor private, but doing so seems to break std::make_shared<mm_iterator_type>.  */
				mm_iterator_type(_Myt& owner_ref) : m_owner_ptr(&owner_ref) { set_to_beginning(); }
			private:
				mm_iterator_type(const mm_iterator_type& src_cref) : m_owner_ptr(src_cref.m_owner_ptr) { (*this) = src_cref; }
				void sync_iterator_to_index() {
					assert(m_owner_ptr->size() >= (*this).m_index);
				}
				msev_bool m_points_to_an_item = false;
				msev_size_t m_index = 0;
				_Myt* m_owner_ptr = nullptr;
				friend class mm_const_iterator_type;
				friend class mm_iterator_set_type;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};

		private:
			typedef std::size_t CHashKey1;
			class mm_const_iterator_handle_type {
			public:
				mm_const_iterator_handle_type(const CHashKey1& key_cref, const std::shared_ptr<mm_const_iterator_type>& shptr_cref) : m_shptr(shptr_cref), m_key(key_cref) {}
			private:
				std::shared_ptr<mm_const_iterator_type> m_shptr;
				CHashKey1 m_key;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class mm_iterator_set_type;
			};
			class mm_iterator_handle_type {
			public:
				mm_iterator_handle_type(const CHashKey1& key_cref, const std::shared_ptr<mm_iterator_type>& shptr_ref) : m_shptr(shptr_ref), m_key(key_cref) {}
			private:
				std::shared_ptr<mm_iterator_type> m_shptr;
				CHashKey1 m_key;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class mm_iterator_set_type;
			};

			class mm_iterator_set_type {
			public:
				class CMMConstIterators : public std::unordered_map<CHashKey1, std::shared_ptr<mm_const_iterator_type>> {};
				class CMMIterators : public std::unordered_map<CHashKey1, std::shared_ptr<mm_iterator_type>> {};

				class assignable_CMMConstIterators_value_type : public std::pair<CHashKey1, std::shared_ptr<mm_const_iterator_type>> {
				public:
					assignable_CMMConstIterators_value_type() {}
					assignable_CMMConstIterators_value_type(const typename CMMConstIterators::value_type& src) : std::pair<CHashKey1, std::shared_ptr<mm_iterator_type>>(src.first, src.second) {}
					assignable_CMMConstIterators_value_type& operator=(const typename CMMConstIterators::value_type& rhs) { (*this).first = rhs.first; (*this).second = rhs.second; return (*this); }
					operator typename CMMConstIterators::value_type() const { return typename CMMConstIterators::value_type((*this).first, (*this).second); }
				};
				class assignable_CMMIterators_value_type : public std::pair<CHashKey1, std::shared_ptr<mm_iterator_type>> {
				public:
					assignable_CMMIterators_value_type() {}
					assignable_CMMIterators_value_type(const typename CMMIterators::value_type& src) : std::pair<CHashKey1, std::shared_ptr<mm_iterator_type>>(src.first, src.second) {}
					assignable_CMMIterators_value_type& operator=(const typename CMMIterators::value_type& rhs) { (*this).first = rhs.first; (*this).second = rhs.second; return (*this); }
					operator typename CMMIterators::value_type() const { return typename CMMIterators::value_type((*this).first, (*this).second); }
				};

				~mm_iterator_set_type() {
					if (!mm_const_fast_mode1()) {
						delete m_aux_mm_const_iterator_shptrs_ptr;
					}
					if (!mm_fast_mode1()) {
						delete m_aux_mm_iterator_shptrs_ptr;
					}
				}

				template<typename TFn>
				void T_apply_to_all_mm_const_iterator_shptrs(const TFn& func_obj_ref) {
					if (!mm_const_fast_mode1()) {
						for (auto it = (*m_aux_mm_const_iterator_shptrs_ptr).begin(); (*m_aux_mm_const_iterator_shptrs_ptr).end() != it; it++) {
							func_obj_ref((*it).second);
						}
					}
					else {
						for (int i = 0; i < m_fm1_num_mm_const_iterators; i += 1) {
							func_obj_ref(m_fm1_key_mm_const_it_array[i].second);
						}
					}
				}
				template<typename TFn>
				void T_apply_to_all_mm_iterator_shptrs(const TFn& func_obj_ref) {
					if (!mm_fast_mode1()) {
						for (auto it = (*m_aux_mm_iterator_shptrs_ptr).begin(); (*m_aux_mm_iterator_shptrs_ptr).end() != it; it++) {
							func_obj_ref((*it).second);
						}
					}
					else {
						for (int i = 0; i < m_fm1_num_mm_iterators; i += 1) {
							func_obj_ref(m_fm1_key_mm_it_array[i].second);
						}
					}
				}
				mm_iterator_set_type(_Myt& owner_ref) : m_next_available_key(0), m_owner_ptr(&owner_ref) {}
				void reset() {
					/* We can use "static" here because the lambda function does not capture any parameters. */
					T_apply_to_all_mm_const_iterator_shptrs([](std::shared_ptr<mm_const_iterator_type>& a) { a->reset(); });
					T_apply_to_all_mm_iterator_shptrs([](std::shared_ptr<mm_iterator_type>& a) { a->reset(); });
				}
				void sync_iterators_to_index() {
					/* No longer used. Relic from when mm_iterator_type contained a "native" iterator. */
					/*
					T_apply_to_all_mm_const_iterator_shptrs([](std::shared_ptr<mm_const_iterator_type>& a) { a->sync_const_iterator_to_index(); });
					T_apply_to_all_mm_iterator_shptrs([](std::shared_ptr<mm_iterator_type>& a) { a->sync_iterator_to_index(); });
					*/
				}
				void invalidate_inclusive_range(msev_size_t start_index, msev_size_t end_index) {
					T_apply_to_all_mm_const_iterator_shptrs([start_index, end_index](std::shared_ptr<mm_const_iterator_type>& a) { a->invalidate_inclusive_range(start_index, end_index); });
					T_apply_to_all_mm_iterator_shptrs([start_index, end_index](std::shared_ptr<mm_iterator_type>& a) { a->invalidate_inclusive_range(start_index, end_index); });
				}
				void shift_inclusive_range(msev_size_t start_index, msev_size_t end_index, msev_int shift) {
					T_apply_to_all_mm_const_iterator_shptrs([start_index, end_index, shift](std::shared_ptr<mm_const_iterator_type>& a) { a->shift_inclusive_range(start_index, end_index, shift); });
					T_apply_to_all_mm_iterator_shptrs([start_index, end_index, shift](std::shared_ptr<mm_iterator_type>& a) { a->shift_inclusive_range(start_index, end_index, shift); });
				}
				bool is_empty() const {
					if (mm_const_fast_mode1()) {
						if (1 <= m_fm1_num_mm_const_iterators) {
							return false;
						}
					}
					else {
						if (1 <= m_aux_mm_const_iterator_shptrs_ptr->size()) {
							return false;
						}
					}
					if (mm_fast_mode1()) {
						if (1 <= m_fm1_num_mm_iterators) {
							return false;
						}
					}
					else {
						if (1 <= m_aux_mm_iterator_shptrs_ptr->size()) {
							return false;
						}
					}
					return true;
				}

				mm_const_iterator_handle_type allocate_new_const_item_pointer() {
					//auto shptr = std::shared_ptr<mm_const_iterator_type>(new mm_const_iterator_type(*m_owner_ptr));
					auto shptr = std::make_shared<mm_const_iterator_type>(*m_owner_ptr);
					auto key = m_next_available_key; m_next_available_key++;
					mm_const_iterator_handle_type retval(key, shptr);
					typename CMMConstIterators::value_type new_item(key, shptr);
					if (!mm_const_fast_mode1()) {
						(*m_aux_mm_const_iterator_shptrs_ptr).insert(new_item);
					}
					else {
						if (sc_fm1_max_mm_iterators == m_fm1_num_mm_const_iterators) {
							/* Too many items. Initiate and switch to slow mode. */
							/* Initialize slow storage. */
							m_aux_mm_const_iterator_shptrs_ptr = new CMMConstIterators();
							/* First copy the items from fast storage to slow storage. */
							for (int i = 0; i < sc_fm1_max_mm_iterators; i += 1) {
								(*m_aux_mm_const_iterator_shptrs_ptr).insert(m_fm1_key_mm_const_it_array[i]);
							}
							/* Add the new items to slow storage. */
							(*m_aux_mm_const_iterator_shptrs_ptr).insert(new_item);
						}
						else {
							m_fm1_key_mm_const_it_array[m_fm1_num_mm_const_iterators] = new_item;
							m_fm1_num_mm_const_iterators += 1;
						}
					}
					return retval;
				}
				void release_const_item_pointer(mm_const_iterator_handle_type handle) {
					if (!mm_const_fast_mode1()) {
						auto it = (*m_aux_mm_const_iterator_shptrs_ptr).find(handle.m_key);
						if ((*m_aux_mm_const_iterator_shptrs_ptr).end() != it) {
							(*m_aux_mm_const_iterator_shptrs_ptr).erase(it);
						}
						else {
							/* Do we need to throw here? */
							MSE_THROW(msevector_range_error("invalid handle - void release_aux_mm_const_iterator(mm_const_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
						}
					}
					else {
						int found_index = -1;
						for (int i = 0; i < m_fm1_num_mm_const_iterators; i += 1) {
							if (handle.m_key == m_fm1_key_mm_const_it_array[i].first) {
								found_index = i;
								break;
							}
						}
						if (0 <= found_index) {
							m_fm1_num_mm_const_iterators -= 1;
							assert(0 <= m_fm1_num_mm_const_iterators);
							for (int j = found_index; j < m_fm1_num_mm_const_iterators; j += 1) {
								m_fm1_key_mm_const_it_array[j] = m_fm1_key_mm_const_it_array[j + 1];
							}
						}
						else {
							/* Do we need to throw here? */
							MSE_THROW(msevector_range_error("invalid handle - void release_aux_mm_const_iterator(mm_const_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
						}
					}
				}

				mm_iterator_handle_type allocate_new_item_pointer() {
					//auto shptr = std::shared_ptr<mm_iterator_type>(new mm_iterator_type(*m_owner_ptr));
					auto shptr = std::make_shared<mm_iterator_type>(*m_owner_ptr);
					auto key = m_next_available_key; m_next_available_key++;
					mm_iterator_handle_type retval(key, shptr);
					typename CMMIterators::value_type new_item(key, shptr);
					if (!mm_fast_mode1()) {
						(*m_aux_mm_iterator_shptrs_ptr).insert(new_item);
					}
					else {
						if (sc_fm1_max_mm_iterators == m_fm1_num_mm_iterators) {
							/* Too many items. Initiate and switch to slow mode. */
							/* Initialize slow storage. */
							m_aux_mm_iterator_shptrs_ptr = new CMMIterators();
							/* First copy the items from fast storage to slow storage. */
							for (int i = 0; i < sc_fm1_max_mm_iterators; i += 1) {
								(*m_aux_mm_iterator_shptrs_ptr).insert(m_fm1_key_mm_it_array[i]);
							}
							/* Add the new items to slow storage. */
							(*m_aux_mm_iterator_shptrs_ptr).insert(new_item);
						}
						else {
							m_fm1_key_mm_it_array[m_fm1_num_mm_iterators] = new_item;
							m_fm1_num_mm_iterators += 1;
						}
					}
					return retval;
				}
				void release_item_pointer(mm_iterator_handle_type handle) {
					if (!mm_fast_mode1()) {
						auto it = (*m_aux_mm_iterator_shptrs_ptr).find(handle.m_key);
						if ((*m_aux_mm_iterator_shptrs_ptr).end() != it) {
							(*m_aux_mm_iterator_shptrs_ptr).erase(it);
						}
						else {
							/* Do we need to throw here? */
							MSE_THROW(msevector_range_error("invalid handle - void release_aux_mm_iterator(mm_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
						}
					}
					else {
						int found_index = -1;
						for (int i = 0; i < m_fm1_num_mm_iterators; i += 1) {
							if (handle.m_key == m_fm1_key_mm_it_array[i].first) {
								found_index = i;
								break;
							}
						}
						if (0 <= found_index) {
							m_fm1_num_mm_iterators -= 1;
							assert(0 <= m_fm1_num_mm_iterators);
							for (int j = found_index; j < m_fm1_num_mm_iterators; j += 1) {
								m_fm1_key_mm_it_array[j] = m_fm1_key_mm_it_array[j + 1];
							}
						}
						else {
							/* Do we need to throw here? */
							MSE_THROW(msevector_range_error("invalid handle - void release_aux_mm_iterator(mm_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
						}
					}
				}
				void release_all_item_pointers() {
					if (!mm_fast_mode1()) {
						(*m_aux_mm_iterator_shptrs_ptr).clear();
					}
					else {
						for (int i = 0; i < m_fm1_num_mm_iterators; i += 1) {
							m_fm1_key_mm_it_array[i] = assignable_CMMIterators_value_type();
						}
						m_fm1_num_mm_iterators = 0;
					}
				}
				mm_const_iterator_type &const_item_pointer(mm_const_iterator_handle_type handle) const {
					return (*(handle.m_shptr));
				}
				mm_iterator_type &item_pointer(mm_iterator_handle_type handle) {
					return (*(handle.m_shptr));
				}

			private:
				void release_all_const_item_pointers() {
					if (!mm_const_fast_mode1()) {
						(*m_aux_mm_const_iterator_shptrs_ptr).clear();
					}
					else {
						for (int i = 0; i < m_fm1_num_mm_const_iterators; i += 1) {
							m_fm1_key_mm_const_it_array[i] = assignable_CMMConstIterators_value_type();
						}
						m_fm1_num_mm_const_iterators = 0;
					}
				}

				mm_iterator_set_type& operator=(const mm_iterator_set_type& src_cref) {
					/* This is a special type of class. The state (i.e. member values) of an object of this class is specific to (and only
					valid for) the particular instance of the object (or the object of which it is a member). So the correct state of a new
					copy of this type of object is not a copy of the state, but rather the state of a new object (which is just the default
					initialization state). */
					(*this).reset();
					return (*this);
				}
				mm_iterator_set_type& operator=(mm_iterator_set_type&& src) { /* see above */ (*this).reset(); return (*this); }
				mm_iterator_set_type(const mm_iterator_set_type& src) { /* see above */ }
				mm_iterator_set_type(const mm_iterator_set_type&& src) { /* see above */ }

				CHashKey1 m_next_available_key = 0;

				static const int sc_fm1_max_mm_iterators = 6/*arbitrary*/;

				bool mm_const_fast_mode1() const { return (nullptr == m_aux_mm_const_iterator_shptrs_ptr); }
				int m_fm1_num_mm_const_iterators = 0;
				assignable_CMMConstIterators_value_type m_fm1_key_mm_const_it_array[sc_fm1_max_mm_iterators];
				CMMConstIterators* m_aux_mm_const_iterator_shptrs_ptr = nullptr;

				bool mm_fast_mode1() const { return (nullptr == m_aux_mm_iterator_shptrs_ptr); }
				int m_fm1_num_mm_iterators = 0;
				assignable_CMMIterators_value_type m_fm1_key_mm_it_array[sc_fm1_max_mm_iterators];
				CMMIterators* m_aux_mm_iterator_shptrs_ptr = nullptr;

				_Myt* m_owner_ptr = nullptr;

				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};
			mutable mm_iterator_set_type m_mmitset;

		public:
			mm_const_iterator_type &const_item_pointer(mm_const_iterator_handle_type handle) const {
				return m_mmitset.const_item_pointer(handle);
			}
			mm_iterator_type &item_pointer(mm_iterator_handle_type handle) {
				return m_mmitset.item_pointer(handle);
			}

		private:
			mm_const_iterator_handle_type allocate_new_const_item_pointer() const { return m_mmitset.allocate_new_const_item_pointer(); }
			void release_const_item_pointer(mm_const_iterator_handle_type handle) const { m_mmitset.release_const_item_pointer(handle); }
			void release_all_const_item_pointers() const { m_mmitset.release_all_const_item_pointers(); }
			mm_iterator_handle_type allocate_new_item_pointer() const { return m_mmitset.allocate_new_item_pointer(); }
			void release_item_pointer(mm_iterator_handle_type handle) const { m_mmitset.release_item_pointer(handle); }
			void release_all_item_pointers() const { m_mmitset.release_all_item_pointers(); }

		public:
			class cipointer : public random_access_const_iterator_base {
			public:
				typedef typename mm_const_iterator_type::iterator_category iterator_category;
				typedef typename mm_const_iterator_type::value_type value_type;
				typedef typename mm_const_iterator_type::difference_type difference_type;
				typedef difference_type distance_type;	// retained
				typedef typename mm_const_iterator_type::pointer pointer;
				typedef typename mm_const_iterator_type::const_pointer const_pointer;
				typedef typename mm_const_iterator_type::reference reference;
				typedef typename mm_const_iterator_type::const_reference const_reference;

				cipointer(const _Myt& owner_cref) : m_owner_cptr(&owner_cref) {
					mm_const_iterator_handle_type handle = m_owner_cptr->allocate_new_const_item_pointer();
					m_handle_shptr = std::make_shared<mm_const_iterator_handle_type>(handle);
				}
				cipointer(const cipointer& src_cref) : m_owner_cptr(src_cref.m_owner_cptr) {
					mm_const_iterator_handle_type handle = m_owner_cptr->allocate_new_const_item_pointer();
					m_handle_shptr = std::make_shared<mm_const_iterator_handle_type>(handle);
					const_item_pointer() = src_cref.const_item_pointer();
				}
				~cipointer() {
					m_owner_cptr->release_const_item_pointer(*m_handle_shptr);
				}
				mm_const_iterator_type& const_item_pointer() const { return m_owner_cptr->const_item_pointer(*m_handle_shptr); }
				mm_const_iterator_type& cip() const { return const_item_pointer(); }
				//const mm_const_iterator_handle_type& handle() const { return (*m_handle_shptr); }

				void reset() { const_item_pointer().reset(); }
				bool points_to_an_item() const { return const_item_pointer().points_to_an_item(); }
				bool points_to_end_marker() const { return const_item_pointer().points_to_end_marker(); }
				bool points_to_beginning() const { return const_item_pointer().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return const_item_pointer().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return const_item_pointer().has_next(); }
				bool has_previous() const { return const_item_pointer().has_previous(); }
				void set_to_beginning() { const_item_pointer().set_to_beginning(); }
				void set_to_end_marker() { const_item_pointer().set_to_end_marker(); }
				void set_to_next() { const_item_pointer().set_to_next(); }
				void set_to_previous() { const_item_pointer().set_to_previous(); }
				cipointer& operator ++() { const_item_pointer().operator ++(); return (*this); }
				cipointer operator++(int) { cipointer _Tmp = *this; ++*this; return (_Tmp); }
				cipointer& operator --() { const_item_pointer().operator --(); return (*this); }
				cipointer operator--(int) { cipointer _Tmp = *this; --*this; return (_Tmp); }
				void advance(difference_type n) { const_item_pointer().advance(n); }
				void regress(difference_type n) { const_item_pointer().regress(n); }
				cipointer& operator +=(difference_type n) { const_item_pointer().operator +=(n); return (*this); }
				cipointer& operator -=(difference_type n) { const_item_pointer().operator -=(n); return (*this); }
				cipointer operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				cipointer operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const cipointer& _Right_cref) const { return const_item_pointer() - (_Right_cref.const_item_pointer()); }
				MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_GIVEN_SUBTRACTION(cipointer)

				const_reference operator*() const { return const_item_pointer().operator*(); }
				const_reference item() const { return operator*(); }
				const_reference previous_item() const { return const_item_pointer().previous_item(); }
				const_pointer operator->() const { return const_item_pointer().operator->(); }
				const_reference operator[](difference_type _Off) const { return const_item_pointer()[_Off]; }
				cipointer& operator=(const cipointer& _Right_cref) { const_item_pointer().operator=(_Right_cref.const_item_pointer()); return (*this); }
				void set_to_const_item_pointer(const cipointer& _Right_cref) { const_item_pointer().set_to_const_item_pointer(_Right_cref.const_item_pointer()); }
				msev_size_t position() const { return const_item_pointer().position(); }
				auto target_container_ptr() const {
					return m_owner_cptr;
				}
				void async_not_shareable_and_not_passable_tag() const {}
			private:
				const _Myt* m_owner_cptr = nullptr;
				std::shared_ptr<mm_const_iterator_handle_type> m_handle_shptr;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};
			class ipointer : public random_access_iterator_base {
			public:
				typedef typename mm_iterator_type::iterator_category iterator_category;
				typedef typename mm_iterator_type::value_type value_type;
				typedef typename mm_iterator_type::difference_type difference_type;
				typedef difference_type distance_type;	// retained
				typedef typename mm_iterator_type::pointer pointer;
				typedef typename mm_iterator_type::reference reference;

				ipointer(_Myt& owner_ref) : m_owner_ptr(&owner_ref) {
					mm_iterator_handle_type handle = m_owner_ptr->allocate_new_item_pointer();
					m_handle_shptr = std::make_shared<mm_iterator_handle_type>(handle);
				}
				ipointer(const ipointer& src_cref) : m_owner_ptr(src_cref.m_owner_ptr) {
					mm_iterator_handle_type handle = m_owner_ptr->allocate_new_item_pointer();
					m_handle_shptr = std::make_shared<mm_iterator_handle_type>(handle);
					item_pointer() = src_cref.item_pointer();
				}
				~ipointer() {
					m_owner_ptr->release_item_pointer(*m_handle_shptr);
				}
				mm_iterator_type& item_pointer() const { return m_owner_ptr->item_pointer(*m_handle_shptr); }
				mm_iterator_type& ip() const { return item_pointer(); }
				//const mm_iterator_handle_type& handle() const { return (*m_handle_shptr); }
				operator cipointer() const {
					assert(m_owner_ptr);
					cipointer retval(*m_owner_ptr);
					retval.const_item_pointer().set_to_beginning();
					retval.const_item_pointer().advance(msev_int(item_pointer().position()));
					return retval;
				}

				reference operator*() const { return item_pointer().operator*(); }
				reference item() const { return operator*(); }
				reference previous_item() const { return item_pointer().previous_item(); }
				pointer operator->() const { return item_pointer().operator->(); }
				reference operator[](difference_type _Off) const { return item_pointer()[_Off]; }

				void reset() { item_pointer().reset(); }
				bool points_to_an_item() const { return item_pointer().points_to_an_item(); }
				bool points_to_end_marker() const { return item_pointer().points_to_end_marker(); }
				bool points_to_beginning() const { return item_pointer().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return item_pointer().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return item_pointer().has_next(); }
				bool has_previous() const { return item_pointer().has_previous(); }
				void set_to_beginning() { item_pointer().set_to_beginning(); }
				void set_to_end_marker() { item_pointer().set_to_end_marker(); }
				void set_to_next() { item_pointer().set_to_next(); }
				void set_to_previous() { item_pointer().set_to_previous(); }
				ipointer& operator ++() { item_pointer().operator ++(); return (*this); }
				ipointer operator++(int) { ipointer _Tmp = *this; ++*this; return (_Tmp); }
				ipointer& operator --() { item_pointer().operator --(); return (*this); }
				ipointer operator--(int) { ipointer _Tmp = *this; --*this; return (_Tmp); }
				void advance(difference_type n) { item_pointer().advance(n); }
				void regress(difference_type n) { item_pointer().regress(n); }
				ipointer& operator +=(difference_type n) { item_pointer().operator +=(n); return (*this); }
				ipointer& operator -=(difference_type n) { item_pointer().operator -=(n); return (*this); }
				ipointer operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				ipointer operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const cipointer& _Right_cref) const {
					return (cipointer(*this) - _Right_cref);
				}
				MSE_IMPL_ORDERED_TYPE_OPERATOR_DELEGATING_DECLARATIONS(ipointer, cipointer)

				ipointer& operator=(const ipointer& _Right_cref) { item_pointer().operator=(_Right_cref.item_pointer()); return (*this); }
				void set_to_item_pointer(const ipointer& _Right_cref) { item_pointer().set_to_item_pointer(_Right_cref.item_pointer()); }
				msev_size_t position() const { return item_pointer().position(); }
				auto target_container_ptr() const {
					return m_owner_ptr;
				}
				void async_not_shareable_and_not_passable_tag() const {}
			private:
				_Myt* m_owner_ptr = nullptr;
				std::shared_ptr<mm_iterator_handle_type> m_handle_shptr;
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};

			ipointer ibegin() {	// return ipointer for beginning of mutable sequence
				ipointer retval(*this);
				retval.set_to_beginning();
				return retval;
			}
			cipointer ibegin() const {	// return ipointer for beginning of nonmutable sequence
				cipointer retval(*this);
				retval.set_to_beginning();
				return retval;
			}
			ipointer iend() {	// return ipointer for end of mutable sequence
				ipointer retval(*this);
				retval.set_to_end_marker();
				return retval;
			}
			cipointer iend() const {	// return ipointer for end of nonmutable sequence
				cipointer retval(*this);
				retval.set_to_end_marker();
				return retval;
			}
			cipointer cibegin() const {	// return ipointer for beginning of nonmutable sequence
				cipointer retval(*this);
				retval.set_to_beginning();
				return retval;
			}
			cipointer ciend() const {	// return ipointer for end of nonmutable sequence
				cipointer retval(*this);
				retval.set_to_end_marker();
				return retval;
			}

			msevector(const cipointer &start, const cipointer &end, const _A& _Al = _A())
				: base_class(_Al), m_mmitset(*this) {
				/*m_debug_size = size();*/
				assign(start, end);
			}
			void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
				if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
				if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
				typename base_class::const_iterator _F = start;
				typename base_class::const_iterator _L = end;
				(*this).assign(_F, _L);
			}
			void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
				auto end = last;
				end++; // this should include some checks
				(*this).assign(first, end);
			}
			void assign(const cipointer &start, const cipointer &end) {
				assign(start.const_item_pointer(), end.const_item_pointer());
			}
			void assign_inclusive(const cipointer &first, const cipointer &last) {
				assign_inclusive(first.const_item_pointer(), last.const_item_pointer());
			}
			void insert_before(const mm_const_iterator_type &pos, size_type _M, const _Ty& _X) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).insert(_P, _M, _X);
			}
			void insert_before(const mm_const_iterator_type &pos, _Ty&& _X) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).insert(_P, 1, MSE_FWD(_X));
			}
			void insert_before(const mm_const_iterator_type &pos, const _Ty& _X = _Ty()) { (*this).insert(pos, 1, _X); }
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				void insert_before(const mm_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const mm_const_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).insert(_P, start, end);
			}
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				void insert_before_inclusive(const mm_const_iterator_type &pos, const _Iter &first, const _Iter &last) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				if (first.m_owner_cptr != last.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before_inclusive(const mm_const_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
				if (!(last.points_to_item())) { MSE_THROW(msevector_range_error("invalid argument - void insert_before_inclusive(const mm_const_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
				typename base_class::const_iterator _P = pos;
				auto _L = last;
				_L++;
				(*this).insert(_P, first, _L);
			}
			void insert_before(const mm_const_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).insert(_P, _Ilist);
			}
			ipointer insert_before(const cipointer &pos, size_type _M, const _Ty& _X) {
				msev_size_t original_pos = pos.position();
				insert_before(pos.const_item_pointer(), _M, _X);
				ipointer retval(*this); retval += (msev_int(original_pos));
				return retval;
			}
			ipointer insert_before(const cipointer &pos, _Ty&& _X) {
				msev_size_t original_pos = pos.position();
				insert_before(pos.const_item_pointer(), MSE_FWD(_X));
				ipointer retval(*this); retval += (msev_int(original_pos));
				return retval;
			}
			ipointer insert_before(const cipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				ipointer insert_before(const cipointer &pos, const _Iter &start, const _Iter &end) {
				msev_size_t original_pos = pos.position();
				insert_before(pos.const_item_pointer(), start, end);
				ipointer retval(*this); retval += (msev_int(original_pos));
				return retval;
			}
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				ipointer insert_before_inclusive(const cipointer &pos, const _Iter &first, const _Iter &last) {
				auto end = last; end++;
				return insert_before(pos, first, end);
			}
			ipointer insert_before(const cipointer &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				msev_size_t original_pos = pos.position();
				(*this).insert_before(pos.const_item_pointer(), _Ilist);
				ipointer retval(*this); retval += (msev_int(original_pos));
				return retval;
			}
			void insert_before(msev_size_t pos, _Ty&& _X) {
				typename base_class::const_iterator _P = (*this).begin() + msev_as_a_size_t(pos);
				(*this).insert(_P, MSE_FWD(_X));
			}
			void insert_before(msev_size_t pos, const _Ty& _X = _Ty()) {
				typename base_class::const_iterator _P = (*this).begin() + msev_as_a_size_t(pos);
				(*this).insert(_P, _X);
			}
			void insert_before(msev_size_t pos, size_t _M, const _Ty& _X) {
				typename base_class::const_iterator _P = (*this).begin() + msev_as_a_size_t(pos);
				(*this).insert(_P, _M, _X);
			}
			void insert_before(msev_size_t pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				typename base_class::const_iterator _P = (*this).begin() + msev_as_a_size_t(pos);
				(*this).insert(_P, _Ilist);
			}
			/* These insert() functions are just aliases for their corresponding insert_before() functions. */
			ipointer insert(const cipointer &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
			ipointer insert(const cipointer &pos, _Ty&& _X) { return insert_before(pos, MSE_FWD(_X)); }
			ipointer insert(const cipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
				ipointer insert(const cipointer &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
			ipointer insert(const cipointer &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) { return insert_before(pos, _Ilist); }
			template<class ..._Valty>
	#if !(defined(GPP4P8_COMPATIBLE))
			void emplace(const mm_const_iterator_type &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
	#else /*!(defined(GPP4P8_COMPATIBLE))*/
			void emplace(const mm_iterator_type &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void emplace() - msevector")); }
				typename base_class::const_iterator _P = pos;
				auto retval = (*this).emplace(_P, std::forward<_Valty>(_Val)...);
			}
			template<class ..._Valty>
	#if !(defined(GPP4P8_COMPATIBLE))
			ipointer emplace(const cipointer &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
	#else /*!(defined(GPP4P8_COMPATIBLE))*/
			ipointer emplace(const ipointer &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/
				msev_size_t original_pos = pos.position();
				(*this).emplace(pos.const_item_pointer(), std::forward<_Valty>(_Val)...);
				ipointer retval(*this); retval += (msev_int(original_pos));
				return retval;
			}
			void erase(const mm_const_iterator_type &pos) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).erase(_P);
			}
			void erase(const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
				if (start.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				if (end.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				typename base_class::const_iterator _F = start;
				typename base_class::const_iterator _L = end;
				(*this).erase(_F, _L);
			}
			void erase_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
				if (first.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase_inclusive() - msevector")); }
				if (last.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase_inclusive() - msevector")); }
				if (!(last.points_to_item())) { MSE_THROW(msevector_range_error("invalid argument - void erase_inclusive() - msevector")); }
				typename base_class::const_iterator _F = first;
				typename base_class::const_iterator _L = last;
				_L++;
				(*this).erase(_F, _L);
			}
			ipointer erase(const cipointer &pos) {
				auto retval_pos = pos;
				retval_pos.set_to_next();
				(*this).erase(pos.const_item_pointer());
				ipointer retval = (*this).ibegin();
				retval += (msev_int(retval_pos.position()));
				return retval;
			}
			ipointer erase(const cipointer &start, const cipointer &end) {
				auto retval_pos = end;
				retval_pos.set_to_next();
				(*this).erase(start.const_item_pointer(), end.const_item_pointer());
				ipointer retval = (*this).ibegin();
				retval += (msev_int(retval_pos.position()));
				return retval;
			}
			ipointer erase_inclusive(const cipointer &first, const cipointer &last) {
				auto end = last; end.set_to_next();
				return (*this).erase(first, end);
			}
			void erase_previous_item(const mm_const_iterator_type &pos) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
				if (!(pos.has_previous())) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
				typename base_class::const_iterator _P = pos;
				_P--;
				(*this).erase(_P);
			}
			ipointer erase_previous_item(const cipointer &pos) {
				erase_previous_item(pos.const_item_pointer());
				ipointer retval = (*this).ibegin();
				retval += (pos.position());
				return retval;
			}


			typedef typename base_class::ss_iterator_type ss_iterator_type;
			typedef typename base_class::ss_const_iterator_type ss_const_iterator_type;
			typedef typename base_class::ss_reverse_iterator_type ss_reverse_iterator_type;
			typedef typename base_class::ss_const_reverse_iterator_type ss_const_reverse_iterator_type;

			ss_iterator_type ss_begin() {	// return std_vector::iterator for beginning of mutable sequence
				return base_class::ss_begin();
			}
			ss_const_iterator_type ss_begin() const {	// return std_vector::iterator for beginning of nonmutable sequence
				return base_class::ss_begin();
			}
			ss_iterator_type ss_end() {	// return std_vector::iterator for end of mutable sequence
				return base_class::ss_end();
			}
			ss_const_iterator_type ss_end() const {	// return std_vector::iterator for end of nonmutable sequence
				return base_class::ss_end();
			}
			ss_const_iterator_type ss_cbegin() const {	// return std_vector::iterator for beginning of nonmutable sequence
				return base_class::ss_cbegin();
			}
			ss_const_iterator_type ss_cend() const {	// return std_vector::iterator for end of nonmutable sequence
				return base_class::ss_cend();
			}

			ss_const_reverse_iterator_type ss_crbegin() const {	// return std_vector::iterator for beginning of reversed nonmutable sequence
				return base_class::ss_crbegin();
			}
			ss_const_reverse_iterator_type ss_crend() const {	// return std_vector::iterator for end of reversed nonmutable sequence
				return base_class::ss_crend();
			}
			ss_reverse_iterator_type ss_rbegin() {	// return std_vector::iterator for beginning of reversed mutable sequence
				return base_class::ss_rbegin();
			}
			ss_const_reverse_iterator_type ss_rbegin() const {	// return std_vector::iterator for beginning of reversed nonmutable sequence
				return base_class::ss_rbegin();
			}
			ss_reverse_iterator_type ss_rend() {	// return std_vector::iterator for end of reversed mutable sequence
				return base_class::ss_rend();
			}
			ss_const_reverse_iterator_type ss_rend() const {	// return std_vector::iterator for end of reversed nonmutable sequence
				return base_class::ss_rend();
			}

			typename base_class::const_iterator const_iterator_from_ss_const_iterator_type(const ss_const_iterator_type& ss_citer) const {
				assert(ss_citer.target_container_ptr() == this);
				typename base_class::const_iterator retval = (*this).cbegin();
				retval += mse::msev_as_a_size_t(ss_citer.position());
				return retval;
			}

			msevector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
				: base_class(_Al), m_mmitset(*this) {
				/*m_debug_size = size();*/
				assign(start, end);
			}
			void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
				if (start.target_container_ptr() != end.target_container_ptr()) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				typename base_class::const_iterator _F = const_iterator_from_ss_const_iterator_type(start);
				typename base_class::const_iterator _L = const_iterator_from_ss_const_iterator_type(end);
				(*this).assign(_F, _L);
			}
			void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
				auto end = last;
				end++; // this should include some checks
				(*this).assign(first, end);
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid argument - void insert_before() - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, _M, _X);
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid argument - void insert_before() - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, MSE_FWD(_X));
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid argument - ss_iterator_type insert_before() - msevector")); }
				//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, start, end);
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type& start, const ss_const_iterator_type &end) {
				if (start.target_container_ptr() != end.target_container_ptr()) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				end.assert_valid_index();
				if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				typename base_class::const_iterator _S = start;
				typename base_class::const_iterator _E = end;
				return (*this).insert_before(pos, _S, _E);
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* end) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - ss_iterator_type insert_before() - msevector")); }
				//if (start.target_container_ptr() != end.target_container_ptr()) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - ss_iterator_type insert_before() - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, start, end);
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before_inclusive(const ss_iterator_type &pos, const _Iter &first, const _Iter &last) {
				auto end = last;
				end++; // this may include some checks
				return (*this).insert_before(pos, first, end);
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, _Ilist);
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			/* These insert() functions are just aliases for their corresponding insert_before() functions. */
			ss_iterator_type insert(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, _Ty&& _X) { return insert_before(pos, MSE_FWD(_X)); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
			template<class _Iter
				//>mse::impl::enable_if_t<mse::impl::_mse_Is_iterator<_Iter>::value, typename base_class::iterator>
				, class = mse::impl::_mse_RequireInputIter<_Iter> >
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) { return insert_before(pos, _Ilist); }
			template<class ..._Valty>
	#if !(defined(GPP4P8_COMPATIBLE))
			ss_iterator_type emplace(const ss_const_iterator_type &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void emplace() - msevector")); }
	#else /*!(defined(GPP4P8_COMPATIBLE))*/
			ipointer emplace(const ipointer &pos, _Valty&& ..._Val)
			{	// insert by moving _Val at pos
				if (pos.m_owner_ptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void emplace() - msevector")); }
	#endif /*!(defined(GPP4P8_COMPATIBLE))*/
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).emplace(_P, std::forward<_Valty>(_Val)...);
				ss_iterator_type retval = ss_begin();
				retval += (msev_int(original_pos));
				return retval;
			}
			ss_iterator_type erase(const ss_const_iterator_type &pos) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				if (!pos.points_to_an_item()) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				auto pos_index = pos.position();

				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).erase(_P);

				ss_iterator_type retval = (*this).ss_begin();
				retval += (typename ss_const_iterator_type::difference_type(pos_index));
				return retval;
			}
			ss_iterator_type erase(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
				if (start.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				if (end.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				if (start.position() > end.position()) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				auto pos_index = start.position();

				typename base_class::const_iterator _F = const_iterator_from_ss_const_iterator_type(start);
				typename base_class::const_iterator _L = const_iterator_from_ss_const_iterator_type(end);
				(*this).erase(_F, _L);

				ss_iterator_type retval = (*this).ss_begin();
				retval += (typename ss_const_iterator_type::difference_type(pos_index));
				return retval;
			}
			ss_iterator_type erase_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
				auto end = last; end.set_to_next();
				return erase(first, end);
			}
			void erase_previous_item(const ss_const_iterator_type &pos) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
				if (!(pos.has_previous())) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				_P--;
				(*this).erase(_P);
			}

			template<typename _TMseArrayPointer> using Tss_iterator_type = typename base_class::template Tss_iterator_type<_TMseArrayPointer>;
			template<typename _TMseArrayPointer> using Tss_const_iterator_type = typename base_class::template Tss_const_iterator_type<_TMseArrayPointer>;
			template<typename _TMseArrayPointer> using Tss_reverse_iterator_type = typename base_class::template Tss_reverse_iterator_type<_TMseArrayPointer>;
			template<typename _TMseArrayPointer> using Tss_const_reverse_iterator_type = typename base_class::template Tss_const_reverse_iterator_type<_TMseArrayPointer>;

			//template <class X> using ss_begin = typename base_class::template ss_begin<X>;
			template<typename _TMseArrayPointer>
			static Tss_iterator_type<_TMseArrayPointer> ss_begin(const _TMseArrayPointer& owner_ptr) {	// return iterator for beginning of mutable sequence
				return base_class::template ss_begin<_TMseArrayPointer>(owner_ptr);
			}
			template<typename _TMseArrayPointer>
			static Tss_iterator_type<_TMseArrayPointer> ss_end(_TMseArrayPointer owner_ptr) {	// return iterator for end of mutable sequence
				return base_class::template ss_end<_TMseArrayPointer>(owner_ptr);
			}
			template<typename _TMseArrayPointer>
			static Tss_const_iterator_type<_TMseArrayPointer> ss_cbegin(_TMseArrayPointer owner_ptr) {	// return iterator for beginning of nonmutable sequence
				return base_class::template ss_cbegin<_TMseArrayPointer>(owner_ptr);
			}
			template<typename _TMseArrayPointer>
			static Tss_const_iterator_type<_TMseArrayPointer> ss_cend(_TMseArrayPointer owner_ptr) {	// return iterator for end of nonmutable sequence
				return base_class::template ss_cend<_TMseArrayPointer>(owner_ptr);
			}
			template<typename _TMseArrayPointer>
			static Tss_reverse_iterator_type<_TMseArrayPointer> ss_rbegin(_TMseArrayPointer owner_ptr) {	// return iterator for beginning of reversed mutable sequence
				return (Tss_reverse_iterator_type<_TMseArrayPointer>(ss_end<_TMseArrayPointer>(owner_ptr)));
			}
			template<typename _TMseArrayPointer>
			static Tss_reverse_iterator_type<_TMseArrayPointer> ss_rend(_TMseArrayPointer owner_ptr) {	// return iterator for end of reversed mutable sequence
				return (Tss_reverse_iterator_type<_TMseArrayPointer>(ss_cbegin<_TMseArrayPointer>(owner_ptr)));
			}
			template<typename _TMseArrayPointer>
			static Tss_const_reverse_iterator_type<_TMseArrayPointer> ss_crbegin(_TMseArrayPointer owner_ptr) {	// return iterator for beginning of reversed nonmutable sequence
				return (Tss_const_reverse_iterator_type<_TMseArrayPointer>(ss_end<_TMseArrayPointer>(owner_ptr)));
			}

			typedef typename base_class::xscope_ss_const_iterator_type xscope_ss_const_iterator_type;
			typedef typename base_class::xscope_ss_iterator_type xscope_ss_iterator_type;

			typedef typename base_class::xscope_const_iterator xscope_const_iterator;
			typedef typename base_class::xscope_iterator xscope_iterator;


			class xscope_ipointer;

			class xscope_cipointer : public cipointer, public mse::us::impl::XScopeTagBase {
			public:
				xscope_cipointer(const mse::TXScopeObjFixedConstPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
				xscope_cipointer(const mse::TXScopeObjFixedPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_cipointer(const mse::TXScopeFixedConstPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
				xscope_cipointer(const mse::TXScopeFixedPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				xscope_cipointer(const xscope_cipointer& src_cref) : cipointer(src_cref) {}
				xscope_cipointer(const xscope_ipointer& src_cref) : cipointer(src_cref) {}
				~xscope_cipointer() {}
				const cipointer& msevector_cipointer() const {
					return (*this);
				}
				cipointer& msevector_cipointer() {
					return (*this);
				}
				const cipointer& mvssci() const { return msevector_cipointer(); }
				cipointer& mvssci() { return msevector_cipointer(); }

				void reset() { cipointer::reset(); }
				bool points_to_an_item() const { return cipointer::points_to_an_item(); }
				bool points_to_end_marker() const { return cipointer::points_to_end_marker(); }
				bool points_to_beginning() const { return cipointer::points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return cipointer::has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return cipointer::has_next(); }
				bool has_previous() const { return cipointer::has_previous(); }
				void set_to_beginning() { cipointer::set_to_beginning(); }
				void set_to_end_marker() { cipointer::set_to_end_marker(); }
				void set_to_next() { cipointer::set_to_next(); }
				void set_to_previous() { cipointer::set_to_previous(); }
				xscope_cipointer& operator ++() { cipointer::operator ++(); return (*this); }
				xscope_cipointer operator++(int) { xscope_cipointer _Tmp = *this; cipointer::operator++(); return (_Tmp); }
				xscope_cipointer& operator --() { cipointer::operator --(); return (*this); }
				xscope_cipointer operator--(int) { xscope_cipointer _Tmp = *this; cipointer::operator--(); return (_Tmp); }
				void advance(difference_type n) { cipointer::advance(n); }
				void regress(difference_type n) { cipointer::regress(n); }
				xscope_cipointer& operator +=(difference_type n) { cipointer::operator +=(n); return (*this); }
				xscope_cipointer& operator -=(difference_type n) { cipointer::operator -=(n); return (*this); }
				xscope_cipointer operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_cipointer operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const xscope_cipointer& _Right_cref) const { return cipointer::operator-(_Right_cref); }
				MSE_IMPL_ORDERED_TYPE_IMPLIED_OPERATOR_DECLARATIONS_GIVEN_SUBTRACTION(xscope_cipointer)

				const_reference operator*() const { return cipointer::operator*(); }
				const_reference item() const { return operator*(); }
				const_reference previous_item() const { return cipointer::previous_item(); }
				const_pointer operator->() const { return cipointer::operator->(); }
				const_reference operator[](difference_type _Off) const { return cipointer::operator[](_Off); }
				xscope_cipointer& operator=(const cipointer& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_cipointer& operator=(const xscope_cipointer& _Right_cref) - msevector::xscope_cipointer")); }
					cipointer::operator=(_Right_cref);
					return (*this);
				}
				xscope_cipointer& operator=(const ipointer& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_cipointer& operator=(const ipointer& _Right_cref) - msevector::xscope_cipointer")); }
					return operator=(cipointer(_Right_cref));
				}
				void set_to_const_item_pointer(const xscope_cipointer& _Right_cref) { cipointer::set_to_item_pointer(_Right_cref); }
				msev_size_t position() const { return cipointer::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_const_pointer_to(*(cipointer::target_container_ptr()));
				}
				void xscope_ipointer_tag() const {}
				void async_not_shareable_and_not_passable_tag() const {}
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename cipointer (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class xscope_ipointer;
			};
			class xscope_ipointer : public ipointer, public mse::us::impl::XScopeTagBase {
			public:
				xscope_ipointer(const mse::TXScopeObjFixedPointer<msevector>& owner_ptr) : ipointer((*owner_ptr).ibegin()) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_ipointer(const mse::TXScopeFixedPointer<msevector>& owner_ptr) : ipointer((*owner_ptr).ibegin()) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				xscope_ipointer(const xscope_ipointer& src_cref) : ipointer(src_cref) {}
				~xscope_ipointer() {}
				const ipointer& msevector_ipointer() const {
					return (*this);
				}
				ipointer& msevector_ipointer() {
					return (*this);
				}
				const ipointer& mvssi() const { return msevector_ipointer(); }
				ipointer& mvssi() { return msevector_ipointer(); }
				operator xscope_cipointer() const {
					assert((*this).target_container_ptr());
					auto xs_ptr = mse::us::unsafe_make_xscope_pointer_to(*((*this).target_container_ptr()));
					xscope_cipointer retval(xs_ptr);
					retval.set_to_beginning();
					retval += (msev_int(position()));
					return retval;
				}

				reference operator*() const { return ipointer::operator*(); }
				reference item() const { return operator*(); }
				reference previous_item() const { return ipointer::previous_item(); }
				pointer operator->() const { return ipointer::operator->(); }
				reference operator[](difference_type _Off) const { return ipointer::operator[](_Off); }

				void reset() { ipointer::reset(); }
				bool points_to_an_item() const { return ipointer::points_to_an_item(); }
				bool points_to_end_marker() const { return ipointer::points_to_end_marker(); }
				bool points_to_beginning() const { return ipointer::points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return ipointer::has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return ipointer::has_next(); }
				bool has_previous() const { return ipointer::has_previous(); }
				void set_to_beginning() { ipointer::set_to_beginning(); }
				void set_to_end_marker() { ipointer::set_to_end_marker(); }
				void set_to_next() { ipointer::set_to_next(); }
				void set_to_previous() { ipointer::set_to_previous(); }
				xscope_ipointer& operator ++() { ipointer::operator ++(); return (*this); }
				xscope_ipointer operator++(int) { xscope_ipointer _Tmp = *this; ipointer::operator++(); return (_Tmp); }
				xscope_ipointer& operator --() { ipointer::operator --(); return (*this); }
				xscope_ipointer operator--(int) { xscope_ipointer _Tmp = *this; ipointer::operator--(); return (_Tmp); }
				void advance(difference_type n) { ipointer::advance(n); }
				void regress(difference_type n) { ipointer::regress(n); }
				xscope_ipointer& operator +=(difference_type n) { ipointer::operator +=(n); return (*this); }
				xscope_ipointer& operator -=(difference_type n) { ipointer::operator -=(n); return (*this); }
				xscope_ipointer operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_ipointer operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const xscope_cipointer& _Right_cref) const {
					return (xscope_cipointer(*this) - _Right_cref);
				}
				MSE_IMPL_ORDERED_TYPE_OPERATOR_DELEGATING_DECLARATIONS(xscope_ipointer, xscope_cipointer)

				xscope_ipointer& operator=(const ipointer& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_ipointer& operator=(const xscope_ipointer& _Right_cref) - msevector::xscope_ipointer")); }
					ipointer::operator=(_Right_cref);
					return (*this);
				}
				void set_to_item_pointer(const xscope_ipointer& _Right_cref) { ipointer::set_to_item_pointer(_Right_cref); }
				msev_size_t position() const { return ipointer::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_pointer_to(*(ipointer::target_container_ptr()));
				}
				void xscope_ipointer_tag() const {}
				void async_not_shareable_and_not_passable_tag() const {}
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename ipointer (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};

			void async_not_shareable_and_not_passable_tag() const {}

		private:

			template<class _Mutex>
			class structure_change_guard {
			public:
				structure_change_guard(_Mutex& _Mtx) MSE_FUNCTION_TRY : m_lock_guard(_Mtx) {} MSE_FUNCTION_CATCH_ANY {
					MSE_THROW(mse::structure_lock_violation_error("structure lock violation - Attempting to modify \
							the structure (size/capacity) of a container while a reference (iterator) to one of its elements \
							still exists?"));
				}
			private:
				std::lock_guard<_Mutex> m_lock_guard;
			};

			/* These are a couple of functions that are basically just here for the convenience of the mstd::vector<>
			implementation, which uses this class. */
#ifdef MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE
			void note_parent_destruction() { m_parent_destroyed = true; }
			void assert_parent_not_destroyed() const {
				/* This assert can fail if, for example, you dereference an mstd::vector<> iterator after the vector has
				been destroyed. You can supress this assert by defining MSE_SUPPRESS_MSTD_VECTOR_CHECK_USE_AFTER_FREE. */
				assert(!m_parent_destroyed);
			}
			bool m_parent_destroyed = false;
#else // MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE
			void note_parent_destruction() {}
			void assert_parent_not_destroyed() const {}
#endif // MSE_MSTD_VECTOR_CHECK_USE_AFTER_FREE

			mutable _TStateMutex m_structure_change_mutex;

			auto contained_vector() const -> decltype(base_class::contained_vector()) { return base_class::contained_vector(); }
			auto contained_vector() -> decltype(base_class::contained_vector()) { return base_class::contained_vector(); }

			friend class mse::us::ns_msevector::xscope_shared_structure_lock_guard<_Myt>;
			friend class mse::us::ns_msevector::xscope_shared_const_structure_lock_guard<_Myt>;
			friend class mse::us::impl::Txscope_shared_structure_lock_guard<_Myt>;
			friend class mse::us::impl::Txscope_shared_const_structure_lock_guard<_Myt>;
#ifndef MSE_MSTDVECTOR_DISABLED
			template<class _Ty2, class _A2/* = std::allocator<_Ty> */>
			friend class mse::mstd::vector;
#endif /*!MSE_MSTDVECTOR_DISABLED*/
		};

#ifdef MSE_HAS_CXX17
		/* deduction guides */
		template<class _Iter, class _Alloc = std::allocator<typename std::iterator_traits<_Iter>::value_type> >
		msevector(_Iter, _Iter, _Alloc = _Alloc())
			->msevector<typename std::iterator_traits<_Iter>::value_type, _Alloc>;
#endif /* MSE_HAS_CXX17 */

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex> inline bool operator!=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test for vector inequality
			return (!(_Left == _Right));
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex> inline bool operator>(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left > _Right for vectors
			return (_Right < _Left);
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex> inline bool operator<=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left <= _Right for vectors
			return (!(_Right < _Left));
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex> inline bool operator>=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left >= _Right for vectors
			return (!(_Left < _Right));
		}

		namespace ns_msevector {

			/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			template<class _TContainer>
			class xscope_shared_structure_lock_guard : public mse::us::impl::Txscope_shared_structure_lock_guard<_TContainer> {
			public:
				typedef mse::us::impl::Txscope_shared_structure_lock_guard<_TContainer> base_class;
				typedef _TContainer MV;

				xscope_shared_structure_lock_guard(const xscope_shared_structure_lock_guard&) = default;
				xscope_shared_structure_lock_guard(xscope_shared_structure_lock_guard&&) = default;

				xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<MV>& owner_ptr) : base_class(owner_ptr)
					, m_base_xscope_shared_structure_lock_guard(owner_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<MV>& owner_ptr) : base_class(owner_ptr)
					, m_base_xscope_shared_structure_lock_guard(owner_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(typename MV::size_type _P) const {
					return base_class::xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const typename MV::xscope_const_iterator& ss_iter) const {
					return base_class::xscope_ptr_to_element(ss_iter);
				}
				auto xscope_ptr_to_element(const typename MV::xscope_cipointer& ciptr) const {
					assert(std::addressof(*(ciptr.target_container_ptr())) == std::addressof(*(*this)));
					return xscope_ptr_to_element(ciptr.position());
				}

				operator mse::TXScopeFixedPointer<MV>() const {
					return static_cast<const base_class&>(*this);
				}
				/*
				template<class MV2 = MV, MSE_IMPL_EIP mse::impl::enable_if_t<!std::is_same<mse::TXScopeFixedConstPointer<MV2>, mse::TXScopeFixedPointer<MV> >::value> MSE_IMPL_EIS >
				explicit operator mse::TXScopeFixedConstPointer<MV2>() const {
					return static_cast<const base_class&>(*this);
				}
				*/
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::impl::ns_stnii_vector::xscope_shared_structure_lock_guard<typename _TContainer::base_class> m_base_xscope_shared_structure_lock_guard;

				friend class xscope_shared_const_structure_lock_guard<_TContainer>;
			};
			template<class _TContainer>
			class xscope_shared_const_structure_lock_guard : public mse::us::impl::Txscope_shared_const_structure_lock_guard<_TContainer> {
			public:
				typedef mse::us::impl::Txscope_shared_const_structure_lock_guard<_TContainer> base_class;
				typedef _TContainer MV;

				xscope_shared_const_structure_lock_guard(const xscope_shared_const_structure_lock_guard&) = default;
				xscope_shared_const_structure_lock_guard(xscope_shared_const_structure_lock_guard&&) = default;

				xscope_shared_const_structure_lock_guard(const xscope_shared_structure_lock_guard<_TContainer>& src) : base_class(src), m_base_xscope_shared_structure_lock_guard(src.m_base_xscope_shared_structure_lock_guard) {}
				xscope_shared_const_structure_lock_guard(xscope_shared_structure_lock_guard<_TContainer>&& src) : base_class(MSE_FWD(src)), m_base_xscope_shared_structure_lock_guard(MSE_FWD(src).m_base_xscope_shared_structure_lock_guard) {}

				xscope_shared_const_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<MV>& owner_ptr) : base_class(owner_ptr)
					, m_base_xscope_shared_structure_lock_guard(owner_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_shared_const_structure_lock_guard(const mse::TXScopeFixedConstPointer<MV>& owner_ptr) : base_class(owner_ptr)
					, m_base_xscope_shared_structure_lock_guard(owner_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(typename MV::size_type _P) const {
					return base_class::xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const typename MV::xscope_const_iterator& ss_iter) const {
					return base_class::xscope_ptr_to_element(ss_iter);
				}
				auto xscope_ptr_to_element(const typename MV::xscope_cipointer& ciptr) const {
					assert(std::addressof(*(ciptr.target_container_ptr())) == std::addressof(*(*this)));
					return xscope_ptr_to_element(ciptr.position());
				}

				operator mse::TXScopeFixedConstPointer<MV>() const {
					return static_cast<const base_class&>(*this);
				}
				MSE_DEFAULT_OPERATOR_DELETE_DECLARATION

			private:
				MSE_DEFAULT_OPERATOR_NEW_AND_AMPERSAND_DECLARATION;

				mse::impl::ns_stnii_vector::xscope_shared_const_structure_lock_guard<typename _TContainer::base_class> m_base_xscope_shared_structure_lock_guard;
			};
		}

		/* While an instance of xscope_shared_structure_lock_guard exists it ensures that direct (scope) pointers to
		individual elements in the vector do not become invalid by preventing any operation that might resize the vector
		or increase its capacity. Any attempt to execute such an operation would result in an exception. */
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
		auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return ns_msevector::xscope_shared_structure_lock_guard<msevector<_Ty, _A, _TStateMutex> >(owner_ptr);
		}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
		auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return ns_msevector::xscope_shared_structure_lock_guard<msevector<_Ty, _A, _TStateMutex> >(owner_ptr);
		}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
		auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return ns_msevector::xscope_shared_const_structure_lock_guard<msevector<_Ty, _A, _TStateMutex> >(owner_ptr);
		}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
		auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return ns_msevector::xscope_shared_const_structure_lock_guard<msevector<_Ty, _A, _TStateMutex> >(owner_ptr);
		}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	}

	namespace impl {
		template<class _Ty, class _A>
		struct can_be_structure_locked_as_const<mse::us::msevector<_Ty, _A> > : std::true_type {};
	}

	/* Using the mse::msevector<> alias of mse::us::msevector<> is deprecated. */
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
	using msevector MSE_DEPRECATED = us::msevector< _Ty, _A, _TStateMutex>;

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedPointer<us::msevector<_Ty, _A, _TStateMutex> > & owner_ptr) {
		return us::make_xscope_shared_structure_lock_guard(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedPointer<us::msevector<_Ty, _A, _TStateMutex> > & owner_ptr) {
		return us::make_xscope_shared_structure_lock_guard(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeObjFixedConstPointer<us::msevector<_Ty, _A, _TStateMutex> > & owner_ptr) {
		return us::make_xscope_shared_structure_lock_guard(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::non_thread_safe_shared_mutex>
	auto make_xscope_shared_structure_lock_guard(const mse::TXScopeFixedConstPointer<us::msevector<_Ty, _A, _TStateMutex> > & owner_ptr) {
		return us::make_xscope_shared_structure_lock_guard(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeObjFixedConstPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeObjFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeFixedConstPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _TVector>
	auto make_xscope_ipointer(const mse::TXScopeObjFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_ipointer(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TVector>
	auto make_xscope_ipointer(const mse::TXScopeFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_ipointer(owner_ptr);
	}
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)
	/* Overloads for rsv::TReturnableFParam<>. */
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_cipointer)
	MSE_OVERLOAD_FOR_RETURNABLE_FPARAM_DECLARATION(make_xscope_ipointer)


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class msevector_test2 {
	public:
		void test1() {
#ifdef MSE_SELF_TESTS
			{
				int i1 = 3;
				int i2 = 5;
				int i3 = 7;

				/* The lifetime (lower bound) associated with the rsv::xslta_array<>, and each of its
				contained elements, is the lower bound of all of the lifetimes of the elements in the initializer
				list. */
				auto vec2 = mse::rsv::xslta_vector<mse::rsv::TXSLTAPointer<int> >{ &i1, &i2 };

				/* Note that although the initializer list used in the declaration of vec3 is different than the
				initializer list used for vec2, the lower bound of the lifetimes of both initializer lists is
				the same. */
				auto vec3 = mse::rsv::xslta_vector<mse::rsv::TXSLTAPointer<int> >{ &i2, &i2 };
				std::swap(vec2, vec3);
				vec2.swap(vec3);

				auto vec4 = mse::rsv::xslta_vector<mse::rsv::TXSLTAPointer<int> >{ &i3, &i1 };
				/* Since the (lower bound) lifetime values of arr2 and arr4 are not the same, their values
				cannot be safely swapped.*/
				//std::swap(vec2, vec4);    // scpptool would complain
				//vec2.swap(vec4);    // scpptool would complain

				/* Even when you want to construct an empty rsv::xslta_vector<>, if the element type has an annotated 
				lifetime, you would still need to provide (a reference to) an initialization element object from which 
				a lower bound lifetime can be inferred. You could just initialize the vector with a (non-empty) 
				initializer list, then clear() the vector. Alternatively, you can pass mse::nullopt as the first 
				constructor parameter, in which case the lower bound lifetime will be inferred from the second 
				(otherwise unused) parameter. */
				auto vec5 = mse::rsv::xslta_vector<mse::rsv::TXSLTAPointer<int> >(mse::nullopt, &i2);
				assert(0 == vec5.size());
				//auto vec6 = mse::rsv::xslta_vector<mse::rsv::TXSLTAPointer<int> >{};    // scpptool would complain
				auto vec7 = mse::rsv::xslta_vector<int>{};    // fine, the element type does not have an annotated lifetime

				{
					/* The preferred way of accessing the contents of an rsv::xslta_vector<> is via an associated 
					rsv::xslta_borrowing_fixed_vector<> (which, while it exists, "borrows" exclusive access to the
					contents of the given vector and (efficiently) prevents any of the elements from being 
					removed or relocated). */
					auto bf_vec2a = mse::rsv::make_xslta_borrowing_fixed_vector(&vec2);
					// or
					//auto bf_vec2a = mse::rsv::xslta_borrowing_fixed_vector(&vec2);

					auto& elem_ref1 = bf_vec2a[0];
					elem_ref1 = &i1;
					//elem_ref1 = &i3;    // scpptool would complain (because i3 does not live long enough)

					/* rsv::xslta_borrowing_fixed_vector<> has an interface largely similar to rsv::xslta_fixed_vector<>,
					which is essentially similar to that of std::vector<>, but without any of the methods or operators
					that could resize (or relocate the contents of) the container. */
				}

				/* While not the preferred method, rsv::xslta_vector<> does (currently) have limited support for accessing 
				its elements (pseudo-)directly. */

				/* non-const mse::rsv::xslta_vector<> element access operators and methods (like front()) do not return a
				raw reference. They return a "proxy reference" object that (while it exists, prevents the vector from 
				being resized, etc. and) behaves like a (raw) reference in some situations. For example, like a reference, 
				it can be cast to the element type. */
				typename decltype(vec2)::value_type ilaptr3 = vec2.front();
				ilaptr3 = &i1;
				//ilaptr3 = &i3; // scpptool would complain (because i3 does not live long enough)

				/* The returned "proxy reference" object also has limited support for assignment operations. */
				vec2.front() = &i1;
				//vec2.front() = &i3;    // scpptool would complain (because i3 does not live long enough)

				/* Note that these returned "proxy reference" objects are designed to be used as temporary (rvalue) objects, 
				not as (lvalue) declared variables or stored objects. */

				/* Note again that we've been using a non-const rsv::xslta_vector<>. Perhaps unintuitively, the contents of
				an rsv::xslta_vector<> cannot be safely accessed via const reference to the vector. */
				auto const& vec2_cref1 = vec2;
				//typename decltype(vec2)::value_type ilaptr3b = vec2_cref1.front();    // scpptool would complain

				{
					/* Like the (non-const) element access operators and methods, dereferencing operations of rsv::xslta_vector<>'s 
					(non-const) iterators return "proxy reference" objects rather than raw references. */
					auto xslta_iter1 = std::begin(vec2);
					auto xslta_iter2 = mse::rsv::make_xslta_end_iterator(&vec2);
					*xslta_iter1 = &i1;

					/* rsv::xslta_vector<>'s iterators can be used to specify an insertion or erasure position in the standard way. */
					vec2.insert(xslta_iter1, &i1);
					vec2.erase(xslta_iter1);
				}
				{
					/* rsv::xslta_fixed_vector<> is a (lifetime annotated) vector that doesn't support any operations that 
					could resize the vector or move its contents (subsequent to initialization). It can be initialized from 
					an rsv::xslta_vector<>. */
					auto f_vec1 = mse::rsv::xslta_fixed_vector<typename decltype(vec2)::value_type>(vec2);
				}
			}
#endif // MSE_SELF_TESTS
		}
	};

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
#pragma pop_macro("_NOEXCEPT")
#pragma pop_macro("_NOEXCEPT_OP")
#endif // !MSE_PUSH_MACRO_NOT_SUPPORTED

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif /*ndef MSEMSEVECTOR_H*/
