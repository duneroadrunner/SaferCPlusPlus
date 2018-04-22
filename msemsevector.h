
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
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include <climits>       // ULONG_MAX
#include <stdexcept>

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

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

	class nii_vector_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};
	class nii_vector_null_dereference_error : public std::logic_error {
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
		template<class _Ty2, class = typename std::enable_if<
			std::is_same<_Ty2, _Ty>::value || ((!std::is_const<_Ty2>::value) && std::is_same<const _Ty2, _Ty>::value)
			, void>::type>
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
		operator bool() const { return (m_ptr != nullptr); }

		explicit operator _Ty*() const { return m_ptr; }

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

		_Ty* m_ptr;
	};

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

	template<class _Ty, _Ty _Val>
	struct integral_constant
	{	// convenient template for integral constant types
	static const _Ty value = _Val;

	typedef _Ty value_type;
	typedef integral_constant<_Ty, _Val> type;

	operator value_type() const
		{	// return stored value
		return (value);
		}
	};

	template<class _Iter>
	struct _mse_Is_iterator
	: public integral_constant<bool, !std::is_integral<_Iter>::value>
	{	// tests for reasonable iterator candidate
	};
	template<typename _InIter>
	using _mse_RequireInputIter = typename std::enable_if<
		std::is_convertible<typename std::iterator_traits<_InIter>::iterator_category, std::input_iterator_tag>::value
		//_mse_Is_iterator<_InIter>::value
	>::type;

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
		typename std::remove_reference<T>::type, typename std::remove_reference<EqualTo>::type>::type {};


	namespace us {
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
		class msevector;
	}

	/* nii_vector<> is essentially a memory-safe vector that does not expose (unprotected) non-static member functions
	like begin() or end() which return (memory) unsafe iterators. It does provide static member function templates
	like ss_begin<>(...) and ss_end<>(...) which take a pointer parameter and return a (bounds-checked) iterator that
	inherits the safety of the given pointer. nii_vector<> also supports "scope" iterators which are safe without any
	run-time overhead. nii_vector<> is a data type that is eligible to be shared between asynchronous threads. */
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	class nii_vector {
	public:
		typedef std::vector<_Ty, _A> std_vector;
		typedef std_vector _MV;
		typedef nii_vector _Myt;
		typedef std_vector base_class;

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

		explicit nii_vector(const _A& _Al = _A())
			: m_vector(_Al) {
			/*m_debug_size = size();*/
		}
		explicit nii_vector(size_type _N)
			: m_vector(msev_as_a_size_t(_N)) {
			/*m_debug_size = size();*/
		}
		explicit nii_vector(size_type _N, const _Ty& _V, const _A& _Al = _A())
			: m_vector(msev_as_a_size_t(_N), _V, _Al) {
			/*m_debug_size = size();*/
		}
		nii_vector(std_vector&& _X) : m_vector(std::forward<decltype(_X)>(_X)) { /*m_debug_size = size();*/ }
		nii_vector(const std_vector& _X) : m_vector(_X) { /*m_debug_size = size();*/ }
		nii_vector(_Myt&& _X) : m_vector(std::forward<decltype(_X.contained_vector())>(_X.contained_vector())) { /*m_debug_size = size();*/ }
		nii_vector(const _Myt& _X) : m_vector(_X.contained_vector()) { /*m_debug_size = size();*/ }
		typedef typename std_vector::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		nii_vector(_It _F, _It _L, const _A& _Al = _A()) : m_vector(_F, _L, _Al) { /*m_debug_size = size();*/ }
		nii_vector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : m_vector(_F, _L, _Al) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
			nii_vector(const _Iter& _First, const _Iter& _Last) : m_vector(_First, _Last) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
			//nii_vector(const _Iter& _First, const _Iter& _Last, const typename std_vector::_Alloc& _Al) : m_vector(_First, _Last, _Al) { /*m_debug_size = size();*/ }
			nii_vector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : m_vector(_First, _Last, _Al) { /*m_debug_size = size();*/ }
		_Myt& operator=(const std_vector& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.operator =(_X);
			/*m_debug_size = size();*/
			return (*this);
		}
		_Myt& operator=(_Myt&& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.operator=(std::forward<std_vector>(_X.contained_vector()));
			return (*this);
		}
		_Myt& operator=(const _Myt& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.operator=(_X.contained_vector());
			return (*this);
		}

		~nii_vector() {
			mse::destructor_lock_guard1<_TStateMutex> lock1(m_mutex1);

			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_not_an_xscope_type();
		}

		operator _MV() const { return this->contained_vector(); }

		void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.reserve(msev_as_a_size_t(_Count));
		}
		size_type capacity() const _NOEXCEPT
		{	// return current length of allocated storage
			return m_vector.capacity();
		}
		void shrink_to_fit() {	// reduce capacity
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.shrink_to_fit();
		}
		void resize(size_type _N, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.resize(msev_as_a_size_t(_N), _X);
		}
		typename std_vector::const_reference operator[](msev_size_t _P) const {
			return (*this).at(msev_as_a_size_t(_P));
		}
		typename std_vector::reference operator[](msev_size_t _P) {
			return (*this).at(msev_as_a_size_t(_P));
		}
		typename std_vector::reference front() {	// return first element of mutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_vector_range_error("front() on empty - typename std_vector::reference front() - nii_vector")); }
			return m_vector.front();
		}
		typename std_vector::const_reference front() const {	// return first element of nonmutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_vector_range_error("front() on empty - typename std_vector::const_reference front() - nii_vector")); }
			return m_vector.front();
		}
		typename std_vector::reference back() {	// return last element of mutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_vector_range_error("back() on empty - typename std_vector::reference back() - nii_vector")); }
			return m_vector.back();
		}
		typename std_vector::const_reference back() const {	// return last element of nonmutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_vector_range_error("back() on empty - typename std_vector::const_reference back() - nii_vector")); }
			return m_vector.back();
		}
		void push_back(_Ty&& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.push_back(std::forward<decltype(_X)>(_X));
		}
		void push_back(const _Ty& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.push_back(_X);
		}
		void pop_back() {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.pop_back();
		}
		void assign(_It _F, _It _L) {
			smoke_check_source_iterators(_F, _L);
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.assign(_F, _L);
			/*m_debug_size = size();*/
		}
		template<class _Iter>
		void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
			smoke_check_source_iterators(_First, _Last);
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.assign(_First, _Last);
			/*m_debug_size = size();*/
		}
		void assign(size_type _N, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.assign(msev_as_a_size_t(_N), _X);
			/*m_debug_size = size();*/
		}

		template<class _Iter>
		void smoke_check_source_iterators_helper(std::true_type, const _Iter& _First, const _Iter& _Last) {
			if (_Last < _First)/*comparison operations should also verify that safe iterators point to the same container*/ {
				MSE_THROW(nii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - nii_vector"));
			} else if ((!(*this).empty()) && (_First < _Last)) {
#ifdef MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ALIAS_CHECK
				/* check for overlap between source and target sequences */
				auto start_of_this_ptr = std::addressof(*begin());
				auto end_of_this_ptr = std::addressof(*(end() - 1)) + 1;
				auto _First_ptr = std::addressof(*_First);
				auto _Last_ptr = std::addressof(*(_Last - 1)) + 1;
				if ((end_of_this_ptr > _First_ptr) && (start_of_this_ptr < _Last_ptr)) {
					MSE_THROW(nii_vector_range_error("invalid arguments - void smoke_check_source_iterators() const - nii_vector"));
				}
#endif // !MSE_NII_VECTOR_ENABLE_SOURCE_ITER_ALIAS_CHECK
			}
		}
		template<class _Iter>
		void smoke_check_source_iterators_helper(std::false_type, const _Iter&, const _Iter&) {}
		template<class _Iter>
		void smoke_check_source_iterators(const _Iter& _First, const _Iter& _Last) {
			smoke_check_source_iterators_helper(typename HasOrInheritsLessThanOperator_msemsevector<_Iter>::type(), _First, _Last);
		}

		template<class ..._Valty>
		void emplace_back(_Valty&& ..._Val)
		{	// insert by moving into element at end
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.emplace_back(std::forward<_Valty>(_Val)...);
			/*m_debug_size = size();*/
		}
		void clear() {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.clear();
			/*m_debug_size = size();*/
		}

		void swap(_Myt& _Other) {	// swap contents with _Other
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.swap(_Other.m_vector);
		}

		void swap(_MV& _Other) {	// swap contents with _Other
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_vector.swap(_Other);
		}

		size_type size() const _NOEXCEPT
		{	// return length of sequence
			return m_vector.size();
		}

		size_type max_size() const _NOEXCEPT
		{	// return maximum possible length of sequence
			return m_vector.max_size();
		}

		bool empty() const _NOEXCEPT
		{	// test if sequence is empty
			return m_vector.empty();
		}
		_A get_allocator() const _NOEXCEPT
		{	// return allocator object for values
			return m_vector.get_allocator();
		}

		reference at(msev_size_t _Pos)
		{	// subscript mutable sequence with checking
			return m_vector.at(msev_as_a_size_t(_Pos));
		}

		const_reference at(msev_size_t _Pos) const
		{	// subscript nonmutable sequence with checking
			return m_vector.at(msev_as_a_size_t(_Pos));
		}

		nii_vector(_XSTD initializer_list<typename std_vector::value_type> _Ilist, const _A& _Al = _A())
			: m_vector(_Ilist, _Al) {	// construct from initializer_list
			/*m_debug_size = size();*/
		}
		_Myt& operator=(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
			m_vector.operator=(static_cast<std_vector>(_Ilist));
			return (*this);
		}
		void assign(_XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// assign initializer_list
			m_vector.assign(_Ilist);
			/*m_debug_size = size();*/
		}

		value_type *data() _NOEXCEPT
		{	// return pointer to mutable data vector
			return m_vector.data();
		}

		const value_type *data() const _NOEXCEPT
		{	// return pointer to nonmutable data vector
			return m_vector.data();
		}

		//class random_access_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		//class random_access_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};
		class random_access_const_iterator_base {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = typename _Myt::value_type;
			using difference_type = typename _Myt::difference_type;
			using pointer = typename _Myt::const_pointer;
			using reference = typename _Myt::const_reference;
		};
		class random_access_iterator_base {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = typename _Myt::value_type;
			using difference_type = typename _Myt::difference_type;
			using pointer = typename _Myt::pointer;
			using reference = typename _Myt::reference;
		};

		class xscope_ss_const_iterator_type;
		class xscope_ss_iterator_type;

		/* The reason we specify the default parameter in the definition instead of this forward declaration is that there seems to be a
		bug in clang (3.8.0) such that if we don't specify the default parameter in the definition it seems to subsequently behave as if
		one were never specified. g++ and msvc don't seem to have the same issue. */
		template<typename _TVectorPointer, class/* = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type*/>
		class Tss_iterator_type;

		/* Tss_const_iterator_type is a bounds checked const_iterator. */
		template<typename _TVectorConstPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorConstPointer>::value), void>::type>
		class Tss_const_iterator_type : public random_access_const_iterator_base {
		public:
			typedef random_access_const_iterator_base base_class;
			typedef typename base_class::iterator_category iterator_category;
			typedef typename base_class::value_type value_type;
			typedef typename base_class::difference_type difference_type;
			typedef typename base_class::pointer pointer;
			typedef typename base_class::reference reference;
			typedef const pointer const_pointer;
			typedef const reference const_reference;

			//template<class = typename std::enable_if<std::is_default_constructible<_TVectorConstPointer>::value, void>::type>
			template<class _TVectorConstPointer2 = _TVectorConstPointer, class = typename std::enable_if<(std::is_same<_TVectorConstPointer2, _TVectorConstPointer>::value) && (std::is_default_constructible<_TVectorConstPointer>::value), void>::type>
			Tss_const_iterator_type() {}

			Tss_const_iterator_type(const _TVectorConstPointer& owner_cptr) : m_owner_cptr(owner_cptr) {}
			Tss_const_iterator_type(_TVectorConstPointer&& owner_cptr) : m_owner_cptr(std::forward<decltype(owner_cptr)>(owner_cptr)) {}

			Tss_const_iterator_type(Tss_const_iterator_type&& src) = default;
			Tss_const_iterator_type(const Tss_const_iterator_type& src) = default;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorConstPointer>::value, void>::type>
			Tss_const_iterator_type(const Tss_const_iterator_type<_Ty2>& src) : m_owner_cptr(src.target_container_ptr()), m_index(src.position()) {}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorConstPointer>::value, void>::type>
			Tss_const_iterator_type(const Tss_iterator_type<_Ty2, void>& src) : m_owner_cptr(src.target_container_ptr()), m_index(src.position()) {}

			void assert_valid_index() const {
				if (m_owner_cptr->size() < m_index) { MSE_THROW(nii_vector_range_error("invalid index - void assert_valid_index() const - ss_const_iterator_type - nii_vector")); }
			}
			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_owner_cptr->size() > m_index) { return true; }
				else {
					assert(m_index == m_owner_cptr->size());
					return false;
				}
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) {
					assert(m_index == m_owner_cptr->size());
					return true;
				}
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
			bool has_previous() const {
				return ((1 <= m_owner_cptr->size()) && (!points_to_beginning()));
			}
			void set_to_beginning() {
				m_index = 0;
			}
			void set_to_end_marker() {
				m_index = m_owner_cptr->size();
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					assert(m_owner_cptr->size() >= m_index);
				}
				else {
					MSE_THROW(nii_vector_range_error("attempt to use invalid const_item_pointer - void set_to_next() - Tss_const_iterator_type - nii_vector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(nii_vector_range_error("attempt to use invalid const_item_pointer - void set_to_previous() - Tss_const_iterator_type - nii_vector"));
				}
			}
			Tss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			Tss_const_iterator_type operator++(int) { Tss_const_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			Tss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			Tss_const_iterator_type operator--(int) { Tss_const_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_cptr->size() < msev_size_t(new_index))) {
					MSE_THROW(nii_vector_range_error("index out of range - void advance(difference_type n) - Tss_const_iterator_type - nii_vector"));
				}
				else {
					m_index = msev_size_t(new_index);
				}
			}
			void regress(difference_type n) { advance(-n); }
			Tss_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			Tss_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			Tss_const_iterator_type operator+(difference_type n) const {
				Tss_const_iterator_type retval(*this);
				retval.advance(n);
				return retval;
			}
			Tss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const Tss_const_iterator_type &rhs) const {
				if (rhs.m_owner_cptr != (*this).m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid argument - difference_type operator-(const Tss_const_iterator_type &rhs) const - nii_vector::Tss_const_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert(difference_type((*m_owner_cptr).size()) >= retval);
				return retval;
			}
			const_reference operator*() const {
				return (*m_owner_cptr).at(msev_as_a_size_t((*this).m_index));
			}
			const_reference item() const { return operator*(); }
			const_reference previous_item() const {
				return (*m_owner_cptr).at(msev_as_a_size_t((*this).m_index - 1));
			}
			const_pointer operator->() const {
				return std::addressof((*m_owner_cptr).at(msev_as_a_size_t((*this).m_index)));
			}
			const_reference operator[](difference_type _Off) const { return (*m_owner_cptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }

			bool operator==(const Tss_const_iterator_type& _Right_cref) const {
				if (this->m_owner_cptr != _Right_cref.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid argument - Tss_const_iterator_type& operator==(const Tss_const_iterator_type& _Right) - Tss_const_iterator_type - nii_vector")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const Tss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const Tss_const_iterator_type& _Right) const {
				if (this->m_owner_cptr != _Right.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid argument - Tss_const_iterator_type& operator<(const Tss_const_iterator_type& _Right) - Tss_const_iterator_type - nii_vector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const Tss_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const Tss_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const Tss_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const Tss_const_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}

			template<class _Ty2 = _TVectorConstPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TVectorConstPointer>::value)
				&& (mse::HasOrInheritsAssignmentOperator_msemsearray<_Ty2>::value), void>::type>
				void assignment_helper1(std::true_type, const Tss_const_iterator_type& _Right_cref) {
				((*this).m_owner_cptr) = _Right_cref.m_owner_cptr;
				(*this).m_index = _Right_cref.m_index;
			}
			void assignment_helper1(std::false_type, const Tss_const_iterator_type& _Right_cref) {
				if (std::addressof(*((*this).m_owner_cptr)) != std::addressof(*(_Right_cref.m_owner_cptr))
					|| (!std::is_same<typename std::remove_const<decltype(*((*this).m_owner_cptr))>::type, typename std::remove_const<decltype(*(_Right_cref.m_owner_cptr))>::type>::value)) {
					/* In cases where the container pointer type stored by this iterator doesn't support assignment (as with, for
					example, mse::TRegisteredFixedPointer<>), this iterator may only be assigned the value of another iterator
					pointing to the same container. */
					MSE_THROW(nii_vector_range_error("invalid argument - Tss_const_iterator_type& operator=(const Tss_const_iterator_type& _Right) - Tss_const_iterator_type - nii_vector"));
				}
				(*this).m_index = _Right_cref.m_index;
			}
			Tss_const_iterator_type& operator=(const Tss_const_iterator_type& _Right_cref) {
				assignment_helper1(typename mse::HasOrInheritsAssignmentOperator_msemsearray<_TVectorConstPointer>::type(), _Right_cref);
				return (*this);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorConstPointer>::value, void>::type>
			Tss_const_iterator_type& operator=(const Tss_const_iterator_type<_Ty2>& _Right_cref) {
				return (*this) = Tss_const_iterator_type(_Right_cref);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorConstPointer>::value, void>::type>
			Tss_const_iterator_type& operator=(const Tss_iterator_type<_Ty2, void>& _Right_cref) {
				return (*this) = Tss_const_iterator_type(_Right_cref);
			}

			msev_size_t position() const {
				return m_index;
			}
			_TVectorConstPointer target_container_ptr() const {
				return m_owner_cptr;
			}

			/* This iterator is safely "async shareable" if the owner pointer it contains is also "async shareable". */
			template<class _Ty2 = _TVectorConstPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TVectorConstPointer>::value)
				&& ((std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>())), void>::type>
				void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

		private:
			_TVectorConstPointer m_owner_cptr;
			msev_size_t m_index = 0;

			friend class /*_Myt*/nii_vector<_Ty>;
		};
		/* Tss_iterator_type is a bounds checked iterator. */
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		class Tss_iterator_type : public random_access_iterator_base {
		public:
			typedef random_access_iterator_base base_class;
			typedef typename base_class::iterator_category iterator_category;
			typedef typename base_class::value_type value_type;
			typedef typename base_class::difference_type difference_type;
			typedef typename base_class::pointer pointer;
			typedef typename base_class::reference reference;
			typedef const pointer const_pointer;
			typedef const reference const_reference;

			//template<class = typename std::enable_if<std::is_default_constructible<_TVectorPointer>::value, void>::type>
			template<class _TVectorPointer2 = _TVectorPointer, class = typename std::enable_if<(std::is_same<_TVectorPointer2, _TVectorPointer>::value) && (std::is_default_constructible<_TVectorPointer>::value), void>::type>
			Tss_iterator_type() {}

			Tss_iterator_type(const _TVectorPointer& owner_ptr) : m_owner_ptr(owner_ptr) {}
			Tss_iterator_type(_TVectorPointer&& owner_ptr) : m_owner_ptr(std::forward<decltype(owner_ptr)>(owner_ptr)) {}

			Tss_iterator_type(Tss_iterator_type&& src) = default;
			Tss_iterator_type(const Tss_iterator_type& src) = default;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorPointer>::value, void>::type>
			Tss_iterator_type(const Tss_iterator_type<_Ty2>& src) : m_owner_ptr(src.target_container_ptr()), m_index(src.position()) {}

			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_owner_ptr->size() > m_index) { return true; }
				else {
					assert(m_index == m_owner_ptr->size());
					return false;
				}
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) {
					assert(m_index == m_owner_ptr->size());
					return true;
				}
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
			bool has_previous() const {
				return ((1 <= m_owner_ptr->size()) && (!points_to_beginning()));
			}
			void set_to_beginning() {
				m_index = 0;
			}
			void set_to_end_marker() {
				m_index = m_owner_ptr->size();
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					assert(m_owner_ptr->size() >= m_index);
				}
				else {
					MSE_THROW(nii_vector_range_error("attempt to use invalid item_pointer - void set_to_next() - Tss_const_iterator_type - nii_vector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(nii_vector_range_error("attempt to use invalid item_pointer - void set_to_previous() - Tss_iterator_type - nii_vector"));
				}
			}
			Tss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			Tss_iterator_type operator++(int) { Tss_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			Tss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			Tss_iterator_type operator--(int) { Tss_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_ptr->size() < msev_size_t(new_index))) {
					MSE_THROW(nii_vector_range_error("index out of range - void advance(difference_type n) - Tss_iterator_type - nii_vector"));
				}
				else {
					m_index = msev_size_t(new_index);
				}
			}
			void regress(difference_type n) { advance(-n); }
			Tss_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			Tss_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			Tss_iterator_type operator+(difference_type n) const {
				Tss_iterator_type retval(*this);
				retval.advance(n);
				return retval;
			}
			Tss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const Tss_iterator_type& rhs) const {
				if (rhs.m_owner_ptr != (*this).m_owner_ptr) { MSE_THROW(nii_vector_range_error("invalid argument - difference_type operator-(const Tss_iterator_type& rhs) const - nii_vector::Tss_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert(difference_type((*m_owner_ptr).size()) >= retval);
				return retval;
			}
			reference operator*() const {
				return (*m_owner_ptr).at(msev_as_a_size_t((*this).m_index));
			}
			reference item() const { return operator*(); }
			reference previous_item() const {
				if ((*this).has_previous()) {
					return (*m_owner_ptr)[m_index - 1];
				}
				else {
					MSE_THROW(nii_vector_range_error("attempt to use invalid item_pointer - reference previous_item() - Tss_const_iterator_type - nii_vector"));
				}
			}
			pointer operator->() const {
				return std::addressof((*m_owner_ptr).at(msev_as_a_size_t((*this).m_index)));
			}
			reference operator[](difference_type _Off) const { return (*m_owner_ptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }

			bool operator==(const Tss_iterator_type& _Right_cref) const {
				if (this->m_owner_ptr != _Right_cref.m_owner_ptr) { MSE_THROW(nii_vector_range_error("invalid argument - Tss_iterator_type& operator==(const Tss_iterator_type& _Right) - Tss_iterator_type - nii_vector")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const Tss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const Tss_iterator_type& _Right) const {
				if (this->m_owner_ptr != _Right.m_owner_ptr) { MSE_THROW(nii_vector_range_error("invalid argument - Tss_iterator_type& operator<(const Tss_iterator_type& _Right) - Tss_iterator_type - nii_vector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const Tss_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const Tss_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const Tss_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const Tss_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}

			template<class _Ty2 = _TVectorPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TVectorPointer>::value)
				&& (mse::HasOrInheritsAssignmentOperator_msemsearray<_Ty2>::value), void>::type>
				void assignment_helper1(std::true_type, const Tss_iterator_type& _Right_cref) {
				((*this).m_owner_ptr) = _Right_cref.m_owner_ptr;
				(*this).m_index = _Right_cref.m_index;
			}
			void assignment_helper1(std::false_type, const Tss_iterator_type& _Right_cref) {
				if (std::addressof(*((*this).m_owner_ptr)) != std::addressof(*(_Right_cref.m_owner_ptr))
					|| (!std::is_same<typename std::remove_const<decltype(*((*this).m_owner_ptr))>::type, typename std::remove_const<decltype(*(_Right_cref.m_owner_ptr))>::type>::value)) {
					/* In cases where the container pointer type stored by this iterator doesn't support assignment (as with, for
					example, mse::TRegisteredFixedPointer<>), this iterator may only be assigned the value of another iterator
					pointing to the same container. */
					MSE_THROW(nii_vector_range_error("invalid argument - Tss_iterator_type& operator=(const Tss_iterator_type& _Right) - Tss_iterator_type - nii_vector"));
				}
				(*this).m_index = _Right_cref.m_index;
			}
			Tss_iterator_type& operator=(const Tss_iterator_type& _Right_cref) {
				assignment_helper1(typename mse::HasOrInheritsAssignmentOperator_msemsearray<_TVectorPointer>::type(), _Right_cref);
				return (*this);
			}
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorPointer>::value, void>::type>
			Tss_iterator_type& operator=(const Tss_iterator_type<_Ty2>& _Right_cref) {
				return (*this) = Tss_iterator_type(_Right_cref);
			}

			msev_size_t position() const {
				return m_index;
			}
			_TVectorPointer target_container_ptr() const {
				return m_owner_ptr;
			}
			/*
			operator Tss_const_iterator_type<_TVectorPointer>() const {
			Tss_const_iterator_type<_TVectorPointer> retval;
			if (nullptr != m_owner_ptr) {
			retval = m_owner_ptr->ss_cbegin<_TVectorPointer>(m_owner_ptr);
			retval.advance(msev_int(m_index));
			}
			return retval;
			}
			*/

			/* This iterator is safely "async shareable" if the owner pointer it contains is also "async shareable". */
			template<class _Ty2 = _TVectorPointer, class = typename std::enable_if<(std::is_same<_Ty2, _TVectorPointer>::value)
				&& ((std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>())), void>::type>
				void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

		private:
			//msev_pointer<_Myt> m_owner_ptr = nullptr;
			_TVectorPointer m_owner_ptr;
			msev_size_t m_index = 0;

			friend class /*_Myt*/nii_vector<_Ty>;
			template<typename _TVectorConstPointer, class/* = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorConstPointer>::value), void>::type*/>
			friend class Tss_const_iterator_type;
		};

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		using Tss_reverse_iterator_type = std::reverse_iterator<Tss_iterator_type<_TVectorPointer>>;
		template<typename _TVectorConstPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorConstPointer>::value), void>::type>
		using Tss_const_reverse_iterator_type = std::reverse_iterator<Tss_const_iterator_type<_TVectorConstPointer>>;

		typedef Tss_iterator_type<msev_pointer<_Myt>> ss_iterator_type;
		typedef Tss_const_iterator_type<msev_pointer<const _Myt>> ss_const_iterator_type;
		typedef Tss_reverse_iterator_type<msev_pointer<_Myt>> ss_reverse_iterator_type;
		typedef Tss_const_reverse_iterator_type<msev_pointer<const _Myt>> ss_const_reverse_iterator_type;

	private:
		/* ss_iterator_type is bounds checked, but not safe against "use-after-free", so the member functions that
		involve ss_iterator_type are made publicly inaccessible. They are used by friend type us::msevector<>,
		which is in turn used by mstd::vector<>. */

		ss_iterator_type ss_begin() {	// return std_vector::iterator for beginning of mutable sequence
			ss_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_const_iterator_type ss_begin() const {	// return std_vector::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_iterator_type ss_end() {	// return std_vector::iterator for end of mutable sequence
			ss_iterator_type retval(this);
			retval.set_to_end_marker();
			return retval;
		}
		ss_const_iterator_type ss_end() const {	// return std_vector::iterator for end of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_end_marker();
			return retval;
		}
		ss_const_iterator_type ss_cbegin() const {	// return std_vector::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_const_iterator_type ss_cend() const {	// return std_vector::iterator for end of nonmutable sequence
			ss_const_iterator_type retval(this);
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

		nii_vector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
			: m_vector(_Al) {
			/*m_debug_size = size();*/
			assign(start, end);
		}
		void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
			if (start > end) { MSE_THROW(nii_vector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
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
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid argument - void insert_before() - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).insert(_P, _M, _X);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid argument - void insert_before() - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).insert(_P, std::forward<decltype(_X)>(_X));
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid argument - ss_iterator_type insert_before() - nii_vector")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type& start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
			end.assert_valid_index();
			if (start > end) { MSE_THROW(nii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
			typename std_vector::const_iterator _S = start;
			typename std_vector::const_iterator _E = end;
			return (*this).insert_before(pos, _S, _E);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - ss_iterator_type insert_before() - nii_vector")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_vector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_vector")); }
			if (start > end) { MSE_THROW(nii_vector_range_error("invalid arguments - ss_iterator_type insert_before() - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before_inclusive(const ss_iterator_type &pos, const _Iter &first, const _Iter &last) {
			auto end = last;
			end++; // this may include some checks
			return (*this).insert_before(pos, first, end);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void insert_before() - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).insert(_P, _Ilist);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		/*
		ss_iterator_type insert(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, _Ty&& _X) { return insert_before(pos, std::forward<decltype(_X)>(_X)); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) { return insert_before(pos, _Ilist); }
		*/
		/*
		template<class ..._Valty>
		ss_iterator_type emplace(const ss_const_iterator_type &pos, _Valty&& ..._Val)
		{	// insert by moving _Val at pos
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void emplace() - nii_vector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).emplace(_P, std::forward<_Valty>(_Val)...);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &pos) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			if (!pos.points_to_an_item()) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			auto pos_index = pos.position();

			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			(*this).erase(_P);

			ss_iterator_type retval = (*this).ss_begin();
			retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
			return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			if (end.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			if (start.position() > end.position()) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			auto pos_index = start.position();

			typename std_vector::const_iterator _F = start;
			typename std_vector::const_iterator _L = end;
			(*this).erase(_F, _L);

			ss_iterator_type retval = (*this).ss_begin();
			retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
			return retval;
		}
		*/
		ss_iterator_type erase_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			auto end = last; end.set_to_next();
			return erase(first, end);
		}
		void erase_previous_item(const ss_const_iterator_type &pos) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase_previous_item() - nii_vector")); }
			if (!(pos.has_previous())) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase_previous_item() - nii_vector")); }
			typename std_vector::const_iterator _P = pos.target_container_ptr()->m_vector.cbegin() + pos.position();
			_P--;
			(*this).erase(_P);
		}

	public:
		/* Here we provide static versions of the member functions that return iterators. As static member functions do
		not have access to a "this" pointer, these functions require a pointer to the container to be passed as the first
		argument. Any returned iterator will contain a copy of the supplied pointer and inherit its safety properties. */

		template<typename _TVectorPointer>
		static auto ss_begin(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			typedef typename std::conditional<std::is_const<typename std::remove_reference<decltype(*owner_ptr)>::type>::value
				, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> >::type return_type;
			return_type retval(owner_ptr);
			retval.set_to_beginning();
			return retval;
		}

		template<typename _TVectorPointer>
		static auto ss_end(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			typedef typename std::conditional<std::is_const<typename std::remove_reference<decltype(*owner_ptr)>::type>::value
				, Tss_const_iterator_type<_TVectorPointer>, Tss_iterator_type<_TVectorPointer> >::type return_type;
			return_type retval(owner_ptr);
			retval.set_to_end_marker();
			return retval;
		}

		template<typename _TVectorPointer>
		static auto ss_cbegin(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_beginning();
			return retval;
		}

		template<typename _TVectorPointer>
		static auto ss_cend(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_end_marker();
			return retval;
		}

		template<typename _TVectorPointer>
		static auto ss_rbegin(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			typedef typename std::conditional<std::is_const<typename std::remove_reference<decltype(*owner_ptr)>::type>::value
				, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> >::type return_type;
			return return_type(ss_end<_TVectorPointer>(owner_ptr));
		}

		template<typename _TVectorPointer>
		static auto ss_rend(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			typedef typename std::conditional<std::is_const<typename std::remove_reference<decltype(*owner_ptr)>::type>::value
				, Tss_const_reverse_iterator_type<_TVectorPointer>, Tss_reverse_iterator_type<_TVectorPointer> >::type return_type;
			return return_type(ss_begin<_TVectorPointer>(owner_ptr));
		}

		template<typename _TVectorPointer>
		static auto ss_crbegin(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_cend<_TVectorPointer>(owner_ptr)));
		}

		template<typename _TVectorPointer>
		static auto ss_crend(const _TVectorPointer& owner_ptr) {
			mse::T_valid_if_not_an_xscope_type<_TVectorPointer>();
			return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_crbegin<_TVectorPointer>(owner_ptr)));
		}

		template<typename _TVectorPointer1>
		static void s_assert_valid_index(const _TVectorPointer1& this_ptr, size_type index) {
			if ((*this_ptr).size() < index) { MSE_THROW(nii_vector_range_error("invalid index - void assert_valid_index() const - nii_vector")); }
		}

		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, size_type pos, _Ty&& _X) {
			return (emplace(this_ptr, pos, std::forward<decltype(_X)>(_X)));
		}
		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, size_type pos, const _Ty& _X = _Ty()) {
			s_assert_valid_index(this_ptr, pos);
			msev_size_t original_pos = pos;
			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).insert(_P, _X);
			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, size_type pos, size_type _M, const _Ty& _X) {
			s_assert_valid_index(this_ptr, pos);
			msev_size_t original_pos = pos;
			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).insert(_P, _M, _X);
			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer1, class _Iter, class = _mse_RequireInputIter<_Iter> >
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, size_type pos, const _Iter& _First, const _Iter& _Last) {
			s_assert_valid_index(this_ptr, pos);
			msev_size_t original_pos = pos;
			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).insert(_P, _First, _Last);
			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, size_type pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
			s_assert_valid_index(this_ptr, pos);
			msev_size_t original_pos = pos;
			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).insert(_P, _Ilist);
			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer1, class ..._Valty>
		static Tss_iterator_type<_TVectorPointer1> emplace(_TVectorPointer1 this_ptr, size_type pos, _Valty&& ..._Val)
		{	// insert by moving _Val at _Where
			s_assert_valid_index(this_ptr, pos);
			msev_size_t original_pos = pos;
			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).emplace(_P, std::forward<_Valty>(_Val)...);
			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, size_type pos) {
			s_assert_valid_index(this_ptr, pos);
			auto pos_index = pos;

			typename std_vector::const_iterator _P = (*this_ptr).m_vector.cbegin() + difference_type(pos);
			(*this_ptr).erase(_P);

			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(typename Tss_iterator_type<_TVectorPointer1>::difference_type(pos_index));
			return retval;
		}
		template<typename _TVectorPointer1>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, size_type start, size_type end) {
			if (start > end) { MSE_THROW(nii_vector_range_error("invalid arguments - void erase() - nii_vector")); }
			auto pos_index = start;

			typename std_vector::const_iterator _F = (*this_ptr).m_vector.cbegin() + difference_type(start);
			typename std_vector::const_iterator _L = (*this_ptr).m_vector.cbegin() + difference_type(end);
			(*this_ptr).erase(_F, _L);

			Tss_iterator_type<_TVectorPointer1> retval = ss_begin(this_ptr);
			retval.advance(typename Tss_iterator_type<_TVectorPointer1>::difference_type(pos_index));
			return retval;
		}

		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _Ty&& _X) {
			return insert(this_ptr, pos.position(), std::forward<decltype(_X)>(_X));
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, const _Ty& _X = _Ty()) {
			return insert(this_ptr, pos.position(), _X);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, size_type _M, const _Ty& _X) {
			return insert(this_ptr, pos.position(), _M, _X);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type
			, class _Iter, class = _mse_RequireInputIter<_Iter> >
			static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, const _Iter& _First, const _Iter& _Last) {
			return insert(this_ptr, pos.position(), _First, _Last);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
			return insert(this_ptr, pos.position(), _Ilist);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class ..._Valty, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> emplace(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos, _Valty&& ..._Val) {
			return emplace(this_ptr, pos.position(), std::forward<_Valty>(_Val)...);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& pos) {
			return erase(this_ptr, pos.position());
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, const Tss_const_iterator_type<_TVectorPointer2>& start, const Tss_const_iterator_type<_TVectorPointer2>& end) {
			return erase(this_ptr, start.position(), end.position());
		}

		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _Ty&& _X) {
			return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), std::forward<decltype(_X)>(_X));
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, const _Ty& _X = _Ty()) {
			return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _X);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, size_type _M, const _Ty& _X) {
			return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _M, _X);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type
			, class _Iter, class = _mse_RequireInputIter<_Iter> >
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, const _Iter& _First, const _Iter& _Last) {
			return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _First, _Last);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> insert(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {
			return insert(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), _Ilist);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class ..._Valty, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> emplace(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos, _Valty&& ..._Val) {
			return emplace(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos), std::forward<_Valty>(_Val)...);
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& pos) {
			return erase(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(pos));
		}
		template<typename _TVectorPointer1, typename _TVectorPointer2, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer2>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer1> erase(_TVectorPointer1 this_ptr, const Tss_iterator_type<_TVectorPointer2>& start, const Tss_iterator_type<_TVectorPointer2>& end) {
			return erase(this_ptr, Tss_const_iterator_type<_TVectorPointer2>(start), Tss_const_iterator_type<_TVectorPointer2>(end));
		}

		/* While ss_iterator_type is not, in general, safe against "use-after-free", a "scope" version of it would be.
		So here we provide such an iterator type. */
		class xscope_ss_const_iterator_type : public ss_const_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
		public:
			template <typename _TXScopePointer, class = typename std::enable_if<
				std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedConstPointer<nii_vector> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<nii_vector> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedConstPointer<nii_vector> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<nii_vector> >::value
				, void>::type>
			xscope_ss_const_iterator_type(const _TXScopePointer& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}

			xscope_ss_const_iterator_type(const xscope_ss_const_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			xscope_ss_const_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			~xscope_ss_const_iterator_type() {}
			const ss_const_iterator_type& nii_vector_ss_const_iterator_type() const {
				return (*this);
			}
			ss_const_iterator_type& nii_vector_ss_const_iterator_type() {
				return (*this);
			}
			const ss_const_iterator_type& mvssci() const { return nii_vector_ss_const_iterator_type(); }
			ss_const_iterator_type& mvssci() { return nii_vector_ss_const_iterator_type(); }

			void reset() { ss_const_iterator_type::reset(); }
			bool points_to_an_item() const { return ss_const_iterator_type::points_to_an_item(); }
			bool points_to_end_marker() const { return ss_const_iterator_type::points_to_end_marker(); }
			bool points_to_beginning() const { return ss_const_iterator_type::points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return ss_const_iterator_type::has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return ss_const_iterator_type::has_next(); }
			bool has_previous() const { return ss_const_iterator_type::has_previous(); }
			void set_to_beginning() { ss_const_iterator_type::set_to_beginning(); }
			void set_to_end_marker() { ss_const_iterator_type::set_to_end_marker(); }
			void set_to_next() { ss_const_iterator_type::set_to_next(); }
			void set_to_previous() { ss_const_iterator_type::set_to_previous(); }
			xscope_ss_const_iterator_type& operator ++() { ss_const_iterator_type::operator ++(); return (*this); }
			xscope_ss_const_iterator_type operator++(int) { xscope_ss_const_iterator_type _Tmp = *this; ss_const_iterator_type::operator++(); return (_Tmp); }
			xscope_ss_const_iterator_type& operator --() { ss_const_iterator_type::operator --(); return (*this); }
			xscope_ss_const_iterator_type operator--(int) { xscope_ss_const_iterator_type _Tmp = *this; ss_const_iterator_type::operator--(); return (_Tmp); }
			void advance(difference_type n) { ss_const_iterator_type::advance(n); }
			void regress(difference_type n) { ss_const_iterator_type::regress(n); }
			xscope_ss_const_iterator_type& operator +=(difference_type n) { ss_const_iterator_type::operator +=(n); return (*this); }
			xscope_ss_const_iterator_type& operator -=(difference_type n) { ss_const_iterator_type::operator -=(n); return (*this); }
			xscope_ss_const_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
			xscope_ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const xscope_ss_const_iterator_type& _Right_cref) const { return ss_const_iterator_type::operator-(_Right_cref); }
			const_reference operator*() const { return ss_const_iterator_type::operator*(); }
			const_reference item() const { return operator*(); }
			const_reference previous_item() const { return ss_const_iterator_type::previous_item(); }
			const_pointer operator->() const { return ss_const_iterator_type::operator->(); }
			const_reference operator[](difference_type _Off) const { return ss_const_iterator_type::operator[](_Off); }
			xscope_ss_const_iterator_type& operator=(const ss_const_iterator_type& _Right_cref) {
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_vector_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const xscope_ss_const_iterator_type& _Right_cref) - nii_vector::xscope_ss_const_iterator_type")); }
				ss_const_iterator_type::operator=(_Right_cref);
				return (*this);
			}
			xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_vector_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) - nii_vector::xscope_ss_const_iterator_type")); }
				return operator=(ss_const_iterator_type(_Right_cref));
			}
			bool operator==(const xscope_ss_const_iterator_type& _Right_cref) const { return ss_const_iterator_type::operator==(_Right_cref); }
			bool operator!=(const xscope_ss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator<(_Right); }
			bool operator<=(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator<=(_Right); }
			bool operator>(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator>(_Right); }
			bool operator>=(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator>=(_Right); }
			void set_to_const_item_pointer(const xscope_ss_const_iterator_type& _Right_cref) { ss_const_iterator_type::set_to_item_pointer(_Right_cref); }
			msev_size_t position() const { return ss_const_iterator_type::position(); }
			auto target_container_ptr() const {
				return mse::us::unsafe_make_xscope_const_pointer_to(*(ss_const_iterator_type::target_container_ptr()));
			}
			void xscope_ss_iterator_type_tag() const {}
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		private:
			void* operator new(size_t size) { return ::operator new(size); }

			//typename ss_const_iterator_type (*this);
			friend class /*_Myt*/nii_vector<_Ty>;
			friend class xscope_ss_iterator_type;
		};
		class xscope_ss_iterator_type : public ss_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
		public:
			template <typename _TXScopePointer, class = typename std::enable_if<
				std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<nii_vector> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<nii_vector> >::value
				, void>::type>
			xscope_ss_iterator_type(const _TXScopePointer& owner_ptr) : ss_iterator_type((*owner_ptr).ss_begin()) {}

			xscope_ss_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_iterator_type(src_cref) {}
			~xscope_ss_iterator_type() {}
			const ss_iterator_type& nii_vector_ss_iterator_type() const {
				return (*this);
			}
			ss_iterator_type& nii_vector_ss_iterator_type() {
				return (*this);
			}
			const ss_iterator_type& mvssi() const { return nii_vector_ss_iterator_type(); }
			ss_iterator_type& mvssi() { return nii_vector_ss_iterator_type(); }

			void reset() { ss_iterator_type::reset(); }
			bool points_to_an_item() const { return ss_iterator_type::points_to_an_item(); }
			bool points_to_end_marker() const { return ss_iterator_type::points_to_end_marker(); }
			bool points_to_beginning() const { return ss_iterator_type::points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return ss_iterator_type::has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return ss_iterator_type::has_next(); }
			bool has_previous() const { return ss_iterator_type::has_previous(); }
			void set_to_beginning() { ss_iterator_type::set_to_beginning(); }
			void set_to_end_marker() { ss_iterator_type::set_to_end_marker(); }
			void set_to_next() { ss_iterator_type::set_to_next(); }
			void set_to_previous() { ss_iterator_type::set_to_previous(); }
			xscope_ss_iterator_type& operator ++() { ss_iterator_type::operator ++(); return (*this); }
			xscope_ss_iterator_type operator++(int) { xscope_ss_iterator_type _Tmp = *this; ss_iterator_type::operator++(); return (_Tmp); }
			xscope_ss_iterator_type& operator --() { ss_iterator_type::operator --(); return (*this); }
			xscope_ss_iterator_type operator--(int) { xscope_ss_iterator_type _Tmp = *this; ss_iterator_type::operator--(); return (_Tmp); }
			void advance(difference_type n) { ss_iterator_type::advance(n); }
			void regress(difference_type n) { ss_iterator_type::regress(n); }
			xscope_ss_iterator_type& operator +=(difference_type n) { ss_iterator_type::operator +=(n); return (*this); }
			xscope_ss_iterator_type& operator -=(difference_type n) { ss_iterator_type::operator -=(n); return (*this); }
			xscope_ss_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
			xscope_ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const xscope_ss_iterator_type& _Right_cref) const { return ss_iterator_type::operator-(_Right_cref); }
			reference operator*() const { return ss_iterator_type::operator*(); }
			reference item() const { return operator*(); }
			reference previous_item() const { return ss_iterator_type::previous_item(); }
			pointer operator->() const { return ss_iterator_type::operator->(); }
			reference operator[](difference_type _Off) const { return ss_iterator_type::operator[](_Off); }
			xscope_ss_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_vector_range_error("invalid argument - xscope_ss_iterator_type& operator=(const xscope_ss_iterator_type& _Right_cref) - nii_vector::xscope_ss_iterator_type")); }
				ss_iterator_type::operator=(_Right_cref);
				return (*this);
			}
			bool operator==(const xscope_ss_iterator_type& _Right_cref) const { return ss_iterator_type::operator==(_Right_cref); }
			bool operator!=(const xscope_ss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator<(_Right); }
			bool operator<=(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator<=(_Right); }
			bool operator>(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator>(_Right); }
			bool operator>=(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator>=(_Right); }
			void set_to_item_pointer(const xscope_ss_iterator_type& _Right_cref) { ss_iterator_type::set_to_item_pointer(_Right_cref); }
			msev_size_t position() const { return ss_iterator_type::position(); }
			auto target_container_ptr() const {
				return mse::us::unsafe_make_xscope_pointer_to(*(ss_iterator_type::target_container_ptr()));
			}
			void xscope_ss_iterator_type_tag() const {}
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		private:
			void* operator new(size_t size) { return ::operator new(size); }

			//typename ss_iterator_type (*this);
			friend class /*_Myt*/nii_vector<_Ty>;
		};

		typedef xscope_ss_const_iterator_type xscope_const_iterator;
		typedef xscope_ss_iterator_type xscope_iterator;

		bool operator==(const _Myt& _Right) const {	// test for vector equality
			return (_Right.m_vector == m_vector);
		}
		bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for vectors
			return (m_vector < _Right.m_vector);
		}

		template<class _Ty2, class _Traits2>
		std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr, size_type byte_count, const size_type byte_start_offset = 0) const {
			const auto array_size_in_bytes = mse::msear_as_a_size_t(sizeof(_Ty) * (*this).size());
			auto byte_ptr = reinterpret_cast<const char *>((*this).contained_vector().data());
			if ((array_size_in_bytes <= byte_start_offset) || (0 >= byte_count)) {
				return _Ostr;
			}
			else {
				byte_ptr += mse::msev_as_a_size_t(byte_start_offset);
				return _Ostr.write(byte_ptr, std::min(mse::msear_as_a_size_t(array_size_in_bytes - byte_start_offset), mse::msear_as_a_size_t(byte_count)));
			}
		}
		template<class _Ty2, class _Traits2>
		std::basic_ostream<_Ty2, _Traits2>& write_bytes(std::basic_ostream<_Ty2, _Traits2>& _Ostr) const {
			return write_bytes(_Ostr, mse::msear_as_a_size_t(sizeof(_Ty) * (*this).size()));
		}

		/* This vector is safely "async shareable" if the elements it contains are also "async shareable". */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (
			(std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>()) || (std::is_arithmetic<_Ty2>::value)
			), void>::type>
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_an_xscope_type() const {}

		typename std_vector::iterator begin() {	// return iterator for beginning of mutable sequence
			return m_vector.begin();
		}
		typename std_vector::iterator end() {	// return iterator for end of mutable sequence
			return m_vector.end();
		}
		typename std_vector::const_iterator cbegin() {	// return iterator for beginning of nonmutable sequence
			return m_vector.cbegin();
		}
		typename std_vector::const_iterator cend() {	// return iterator for end of nonmutable sequence
			return m_vector.cend();
		}
		typename std_vector::reverse_iterator rbegin() {	// return iterator for beginning of reversed mutable sequence
			return m_vector.rbegin();
		}
		typename std_vector::reverse_iterator rend() {	// return iterator for end of reversed mutable sequence
			return m_vector.rend();
		}
		typename std_vector::const_reverse_iterator crbegin() {	// return iterator for beginning of reversed nonmutable sequence
			return m_vector.crbegin();
		}
		typename std_vector::const_reverse_iterator crend() {	// return iterator for end of reversed nonmutable sequence
			return m_vector.crend();
		}


		typename std_vector::iterator insert(typename std_vector::const_iterator _P, _Ty&& _X) {
			return (emplace(_P, std::forward<decltype(_X)>(_X)));
		}
		typename std_vector::iterator insert(typename std_vector::const_iterator _P, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_vector::iterator retval = m_vector.insert(_P, _X);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_vector::iterator insert(typename std_vector::const_iterator _P, size_type _M, const _Ty& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_vector::iterator retval = m_vector.insert(_P, msev_as_a_size_t(_M), _X);
			/*m_debug_size = size();*/
			return retval;
		}
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_vector::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			typename std_vector::iterator insert(typename std_vector::const_iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			auto retval = m_vector.insert(_Where, _First, _Last);
			/*m_debug_size = size();*/
			return retval;
		}
		template<class ..._Valty>
		typename std_vector::iterator emplace(typename std_vector::const_iterator _Where, _Valty&& ..._Val)
		{	// insert by moving _Val at _Where
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			auto retval = m_vector.emplace(_Where, std::forward<_Valty>(_Val)...);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_vector::iterator erase(typename std_vector::const_iterator _P) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_vector::iterator retval = m_vector.erase(_P);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_vector::iterator erase(typename std_vector::const_iterator _F, typename std_vector::const_iterator _L) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_vector::iterator retval = m_vector.erase(_F, _L);
			/*m_debug_size = size();*/
			return retval;
		}

		typename std_vector::iterator insert(typename std_vector::const_iterator _Where, _XSTD initializer_list<typename std_vector::value_type> _Ilist) {	// insert initializer_list
			auto retval = m_vector.insert(_Where, _Ilist);
			/*m_debug_size = size();*/
			return retval;
		}


		const _MV& contained_vector() const { return m_vector; }
		_MV& contained_vector() { return m_vector; }

		std_vector m_vector;
		_TStateMutex m_mutex1;

		friend class xscope_ss_const_iterator_type;
		friend class xscope_ss_iterator_type;
		friend class us::msevector<_Ty, _A, _TStateMutex>;
	};

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator!=(const nii_vector<_Ty, _A, _TStateMutex>& _Left,
		const nii_vector<_Ty, _A, _TStateMutex>& _Right) {	// test for vector inequality
		return (!(_Left == _Right));
	}

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator>(const nii_vector<_Ty, _A, _TStateMutex>& _Left,
		const nii_vector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left > _Right for vectors
		return (_Right < _Left);
	}

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator<=(const nii_vector<_Ty, _A, _TStateMutex>& _Left,
		const nii_vector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left <= _Right for vectors
		return (!(_Right < _Left));
	}

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator>=(const nii_vector<_Ty, _A, _TStateMutex>& _Left,
		const nii_vector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left >= _Right for vectors
		return (!(_Left < _Right));
	}
}

namespace std {

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_vector<_Ty, _A, _TStateMutex>& _Left, mse::nii_vector<_Ty, _A, _TStateMutex>& _Right) _NOEXCEPT
	{
		_Left.swap(_Right);
	}
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(vector<_Ty, _A>& _Left, mse::nii_vector<_Ty, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap vectors
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_vector<_Ty, _A, _TStateMutex>& _Left, vector<_Ty, _A>& _Right) _NOEXCEPT
	{
		_Left.swap(_Right);
	}
}

namespace mse {

	namespace us {
		/* msevector<> is an unsafe extension of nii_vector<> that provides the traditional begin() and end() (non-static)
		member functions that return unsafe iterators. It also provides ss_begin() and ss_end() (non-static) member
		functions which return bounds-checked, but still technically unsafe iterators. */
		template<class _Ty, class _A, class _TStateMutex>
		class msevector : public nii_vector<_Ty, _A, _TStateMutex> {
		public:
			typedef nii_vector<_Ty, _A, _TStateMutex> base_class;
			typedef std::vector<_Ty, _A> std_vector;
			typedef msevector _Myt;

			typedef typename base_class::allocator_type allocator_type;
			typedef typename base_class::value_type value_type;
			typedef typename base_class::size_type size_type;
			//typedef msear_size_t size_type;
			typedef typename base_class::difference_type difference_type;
			//typedef msear_int difference_type;
			typedef typename base_class::pointer pointer;
			typedef typename base_class::const_pointer const_pointer;
			typedef typename base_class::reference reference;
			typedef typename base_class::const_reference const_reference;

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
			msevector(std_vector&& _X) : base_class(std::forward<decltype(_X)>(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const std_vector& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(base_class&& _X) : base_class(std::forward<decltype(_X)>(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const base_class& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(_Myt&& _X) : base_class(std::forward<decltype(_X)>(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const _Myt& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
			typedef typename base_class::const_iterator _It;
			/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
			msevector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			msevector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
				, class = _mse_RequireInputIter<_Iter> >
				msevector(const _Iter& _First, const _Iter& _Last) : base_class(_First, _Last), m_mmitset(*this) { /*m_debug_size = size();*/ }
			template<class _Iter
				//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
				, class = _mse_RequireInputIter<_Iter> >
				//msevector(const _Iter& _First, const _Iter& _Last, const typename base_class::_Alloc& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
				msevector(const _Iter& _First, const _Iter& _Last, const _A& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
			_Myt& operator=(const base_class& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::operator =(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
				return (*this);
			}
			_Myt& operator=(_Myt&& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::operator=(std::forward<decltype(_X)>(_X));
				m_mmitset.reset();
				return (*this);
			}
			_Myt& operator=(const _Myt& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::operator=(_X);
				m_mmitset.reset();
				return (*this);
			}
			void reserve(size_type _Count)
			{	// determine new minimum length of allocated storage
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::push_back(std::forward<decltype(_X)>(_X));
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::push_back(std::forward<decltype(_X)>(_X));
					/*m_debug_size = size();*/

					assert((original_size + 1) == msev_size_t((*this).size()));
					m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			void push_back(const _Ty& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::push_back(_X);
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::push_back(_X);
					/*m_debug_size = size();*/

					assert((original_size + 1) == msev_size_t((*this).size()));
					m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
					auto new_capacity = msev_size_t((*this).capacity());
					bool realloc_occured = (new_capacity != original_capacity);
					if (realloc_occured) {
						m_mmitset.sync_iterators_to_index();
					}
				}
			}
			void pop_back() {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::pop_back();
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					if (0 == original_size) { MSE_THROW(msevector_range_error("pop_back() on empty - void pop_back() - msevector")); }
					base_class::pop_back();
					/*m_debug_size = size();*/

					assert((original_size - 1) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::assign(_F, _L);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			template<class _Iter>
			void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::assign(_First, _Last);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void assign(size_type _N, const _Ty& _X = _Ty()) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::assign(msev_as_a_size_t(_N), _X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			typename base_class::iterator insert(typename base_class::const_iterator _P, _Ty&& _X) {
				return (emplace(_P, std::forward<decltype(_X)>(_X)));
			}
			typename base_class::iterator insert(typename base_class::const_iterator _P, const _Ty& _X = _Ty()) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size + 1) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size + _M) == msev_size_t((*this).size()));
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
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				typename base_class::iterator insert(typename base_class::const_iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size + _M) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				msev_int di = std::distance(base_class::/*c*/begin(), _P);
				msev_size_t d = msev_size_t(di);
				if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				/*typename base_class::iterator retval =*/
				base_class::insert(_P, _M, _X);
				/*m_debug_size = size();*/

				assert((original_size + _M) == msev_size_t((*this).size()));
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
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
				, class = _mse_RequireInputIter<_Iter> > void
				insert(typename base_class::/*const_*/iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

				assert((original_size + _M) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				if (m_mmitset.is_empty()) {
					base_class::emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/
				}
				else {
					auto original_size = msev_size_t((*this).size());
					auto original_capacity = msev_size_t((*this).capacity());

					base_class::emplace_back(std::forward<_Valty>(_Val)...);
					/*m_debug_size = size();*/

					assert((original_size + 1) == msev_size_t((*this).size()));
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

				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size + 1) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size - 1) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size - _M) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::clear();
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(std::vector<_Ty, _A>& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::swap(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(base_class& _X) {
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::swap(_X);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
			void swap(_Myt& _X) {
				(*this).swap(static_cast<base_class&>(_X));
				m_mmitset.reset();
			}

			msevector(_XSTD initializer_list<typename base_class::value_type> _Ilist,
				const _A& _Al = _A())
				: base_class(_Ilist, _Al), m_mmitset(*this) {	// construct from initializer_list
																/*m_debug_size = size();*/
			}
			_Myt& operator=(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::operator=(_Ilist);
				m_mmitset.reset();
				return (*this);
			}
			void assign(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				base_class::assign(_Ilist);
				/*m_debug_size = size();*/
				m_mmitset.reset();
			}
	#if defined(GPP4P8_COMPATIBLE)
			/* g++4.8 seems to be (incorrectly) using the c++98 version of this insert function instead of the c++11 version. */
			/*typename base_class::iterator*/void insert(typename base_class::/*const_*/iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
				msev_int di = std::distance(base_class::/*c*/begin(), _Where);
				msev_size_t d = msev_size_t(di);
				if ((0 > di) || (msev_size_t((*this).size()) < di)) { MSE_THROW(msevector_range_error("index out of range - typename base_class::iterator insert() - msevector")); }

				auto _M = _Ilist.size();
				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				/*auto retval = */base_class::insert(_Where, _Ilist);
				/*m_debug_size = size();*/

				assert((original_size + _M) == msev_size_t((*this).size()));
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
				std::lock_guard<mse::non_thread_safe_mutex> lock2(m_structure_change_mutex);
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

					assert((original_size + _M) == msev_size_t((*this).size()));
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
				using value_type = typename _Myt::_Myt::value_type;
				using difference_type = typename _Myt::_Myt::difference_type;
				using pointer = typename _Myt::_Myt::const_pointer;
				using reference = typename _Myt::_Myt::const_reference;
			};
			class random_access_iterator_base {
			public:
				using iterator_category = std::random_access_iterator_tag;
				using value_type = typename _Myt::_Myt::value_type;
				using difference_type = typename _Myt::_Myt::difference_type;
				using pointer = typename _Myt::_Myt::pointer;
				using reference = typename _Myt::_Myt::reference;
			};

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
					retval.advance(n);
					return retval;
				}
				mm_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const mm_const_iterator_type &rhs) const {
					if ((rhs.m_owner_cptr) != ((*this).m_owner_cptr)) { MSE_THROW(msevector_range_error("invalid argument - difference_type operator-(const mm_const_iterator_type &rhs) const - msevector::mm_const_iterator_type")); }
					auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
					assert(difference_type(m_owner_cptr->size()) >= retval);
					return retval;
				}
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
				/*
				mm_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref)
				{
				msev_int d = std::distance<typename base_class::iterator>(m_owner_cptr->cbegin(), _Right_cref);
				if ((0 <= d) && (m_owner_cptr->size() >= d)) {
				if (m_owner_cptr->size() == d) {
				assert(m_owner_cptr->cend() == _Right_cref);
				m_points_to_an_item = false;
				} else {
				m_points_to_an_item = true;
				}
				m_index = msev_size_t(d);
				base_class::const_iterator::operator=(_Right_cref);
				}
				else {
				MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref) - mm_const_iterator_type - msevector"));
				}
				return (*this);
				}
				*/
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
				bool operator==(const mm_const_iterator_type& _Right_cref) const {
					if (((*this).m_owner_cptr) != (_Right_cref.m_owner_cptr)) { MSE_THROW(msevector_range_error("invalid argument - mm_const_iterator_type& operator==(const mm_const_iterator_type& _Right) - mm_const_iterator_type - msevector")); }
					return (_Right_cref.m_index == m_index);
				}
				bool operator!=(const mm_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const mm_const_iterator_type& _Right) const {
					if (((*this).m_owner_cptr) != (_Right.m_owner_cptr)) { MSE_THROW(msevector_range_error("invalid argument - mm_const_iterator_type& operator<(const mm_const_iterator_type& _Right) - mm_const_iterator_type - msevector")); }
					return (m_index < _Right.m_index);
				}
				bool operator<=(const mm_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
				bool operator>(const mm_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
				bool operator>=(const mm_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
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
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

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
				friend class /*_Myt*/msevector<_Ty, _A>;
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
					m_index = m_owner_ptr->size();
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
					retval.advance(n);
					return retval;
				}
				mm_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const mm_iterator_type& rhs) const {
					if ((rhs.m_owner_ptr) != ((*this).m_owner_ptr)) { MSE_THROW(msevector_range_error("invalid argument - difference_type operator-(const mm_iterator_type& rhs) const - msevector::mm_iterator_type")); }
					auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
					assert(difference_type(m_owner_ptr->size()) >= retval);
					return retval;
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
				/*
				mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref)
				{
				msev_int d = std::distance<typename base_class::iterator>(m_owner_ptr->begin(), _Right_cref);
				if ((0 <= d) && (m_owner_ptr->size() >= d)) {
				if (m_owner_ptr->size() == d) {
				assert(m_owner_ptr->end() == _Right_cref);
				m_points_to_an_item = false;
				} else {
				m_points_to_an_item = true;
				}
				m_index = msev_size_t(d);
				base_class::iterator::operator=(_Right_cref);
				}
				else {
				MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
				}
				return (*this);
				}
				*/
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
				bool operator==(const mm_iterator_type& _Right_cref) const {
					if (((*this).m_owner_ptr) != (_Right_cref.m_owner_ptr)) { MSE_THROW(msevector_range_error("invalid argument - mm_iterator_type& operator==(const typename base_class::iterator& _Right) - mm_iterator_type - msevector")); }
					return (_Right_cref.m_index == m_index);
				}
				bool operator!=(const mm_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const mm_iterator_type& _Right) const {
					if (((*this).m_owner_ptr) != (_Right.m_owner_ptr)) { MSE_THROW(msevector_range_error("invalid argument - mm_iterator_type& operator<(const typename base_class::iterator& _Right) - mm_iterator_type - msevector")); }
					return (m_index < _Right.m_index);
				}
				bool operator<=(const mm_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
				bool operator>(const mm_iterator_type& _Right) const { return (!((*this) <= _Right)); }
				bool operator>=(const mm_iterator_type& _Right) const { return (!((*this) < _Right)); }
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
				operator mm_const_iterator_type() const {
					mm_const_iterator_type retval(*m_owner_ptr);
					retval.set_to_beginning();
					retval.advance(msev_int(m_index));
					return retval;
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

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
					operator typename CMMConstIterators::value_type() const { return CMMConstIterators::value_type((*this).first, (*this).second); }
				};
				class assignable_CMMIterators_value_type : public std::pair<CHashKey1, std::shared_ptr<mm_iterator_type>> {
				public:
					assignable_CMMIterators_value_type() {}
					assignable_CMMIterators_value_type(const typename CMMIterators::value_type& src) : std::pair<CHashKey1, std::shared_ptr<mm_iterator_type>>(src.first, src.second) {}
					assignable_CMMIterators_value_type& operator=(const typename CMMIterators::value_type& rhs) { (*this).first = rhs.first; (*this).second = rhs.second; return (*this); }
					operator typename CMMIterators::value_type() const { return CMMIterators::value_type((*this).first, (*this).second); }
				};

				~mm_iterator_set_type() {
					if (!mm_const_fast_mode1()) {
						delete m_aux_mm_const_iterator_shptrs_ptr;
					}
					if (!mm_fast_mode1()) {
						delete m_aux_mm_iterator_shptrs_ptr;
					}
				}

				void apply_to_all_mm_const_iterator_shptrs(const std::function<void(std::shared_ptr<mm_const_iterator_type>&)>& func_obj_ref) {
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
				void apply_to_all_mm_iterator_shptrs(const std::function<void(std::shared_ptr<mm_iterator_type>&)>& func_obj_ref) {
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
					static const std::function<void(std::shared_ptr<mm_const_iterator_type>&)> cit_func_obj = [](std::shared_ptr<mm_const_iterator_type>& a) { a->reset(); };
					apply_to_all_mm_const_iterator_shptrs(cit_func_obj);
					static const std::function<void(std::shared_ptr<mm_iterator_type>&)> it_func_obj = [](std::shared_ptr<mm_iterator_type>& a) { a->reset(); };
					apply_to_all_mm_iterator_shptrs(it_func_obj);
				}
				void sync_iterators_to_index() {
					/* No longer used. Relic from when mm_iterator_type contained a "native" iterator. */
					/* We can use "static" here because the lambda function does not capture any parameters. */
					/*
					static const std::function<void(std::shared_ptr<mm_const_iterator_type>&)> cit_func_obj = [](std::shared_ptr<mm_const_iterator_type>& a) { a->sync_const_iterator_to_index(); };
					apply_to_all_mm_const_iterator_shptrs(cit_func_obj);
					static const std::function<void(std::shared_ptr<mm_iterator_type>&)> it_func_obj = [](std::shared_ptr<mm_iterator_type>& a) { a->sync_iterator_to_index(); };
					apply_to_all_mm_iterator_shptrs(it_func_obj);
					*/
				}
				void invalidate_inclusive_range(msev_size_t start_index, msev_size_t end_index) {
					const std::function<void(std::shared_ptr<mm_const_iterator_type>&)> cit_func_obj = [start_index, end_index](std::shared_ptr<mm_const_iterator_type>& a) { a->invalidate_inclusive_range(start_index, end_index); };
					apply_to_all_mm_const_iterator_shptrs(cit_func_obj);
					const std::function<void(std::shared_ptr<mm_iterator_type>&)> it_func_obj = [start_index, end_index](std::shared_ptr<mm_iterator_type>& a) { a->invalidate_inclusive_range(start_index, end_index); };
					apply_to_all_mm_iterator_shptrs(it_func_obj);
				}
				void shift_inclusive_range(msev_size_t start_index, msev_size_t end_index, msev_int shift) {
					const std::function<void(std::shared_ptr<mm_const_iterator_type>&)> cit_func_obj = [start_index, end_index, shift](std::shared_ptr<mm_const_iterator_type>& a) { a->shift_inclusive_range(start_index, end_index, shift); };
					apply_to_all_mm_const_iterator_shptrs(cit_func_obj);
					const std::function<void(std::shared_ptr<mm_iterator_type>&)> it_func_obj = [start_index, end_index, shift](std::shared_ptr<mm_iterator_type>& a) { a->shift_inclusive_range(start_index, end_index, shift); };
					apply_to_all_mm_iterator_shptrs(it_func_obj);
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
				const_reference operator*() const { return const_item_pointer().operator*(); }
				const_reference item() const { return operator*(); }
				const_reference previous_item() const { return const_item_pointer().previous_item(); }
				const_pointer operator->() const { return const_item_pointer().operator->(); }
				const_reference operator[](difference_type _Off) const { return const_item_pointer()[_Off]; }
				cipointer& operator=(const cipointer& _Right_cref) { const_item_pointer().operator=(_Right_cref.const_item_pointer()); return (*this); }
				bool operator==(const cipointer& _Right_cref) const { return const_item_pointer().operator==(_Right_cref.const_item_pointer()); }
				bool operator!=(const cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const cipointer& _Right) const { return (const_item_pointer() < _Right.const_item_pointer()); }
				bool operator<=(const cipointer& _Right) const { return (const_item_pointer() <= _Right.const_item_pointer()); }
				bool operator>(const cipointer& _Right) const { return (const_item_pointer() > _Right.const_item_pointer()); }
				bool operator>=(const cipointer& _Right) const { return (const_item_pointer() >= _Right.const_item_pointer()); }
				void set_to_const_item_pointer(const cipointer& _Right_cref) { const_item_pointer().set_to_const_item_pointer(_Right_cref.const_item_pointer()); }
				msev_size_t position() const { return const_item_pointer().position(); }
				auto target_container_ptr() const {
					return m_owner_cptr;
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
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
					cipointer retval(*m_owner_ptr);
					retval.const_item_pointer().set_to_beginning();
					retval.const_item_pointer().advance(msev_int(item_pointer().position()));
					return retval;
				}

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
				difference_type operator-(const ipointer& _Right_cref) const { return item_pointer() - (_Right_cref.item_pointer()); }
				reference operator*() const { return item_pointer().operator*(); }
				reference item() const { return operator*(); }
				reference previous_item() const { return item_pointer().previous_item(); }
				pointer operator->() const { return item_pointer().operator->(); }
				reference operator[](difference_type _Off) const { return item_pointer()[_Off]; }
				ipointer& operator=(const ipointer& _Right_cref) { item_pointer().operator=(_Right_cref.item_pointer()); return (*this); }
				bool operator==(const ipointer& _Right_cref) const { return item_pointer().operator==(_Right_cref.item_pointer()); }
				bool operator!=(const ipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const ipointer& _Right) const { return (item_pointer() < _Right.item_pointer()); }
				bool operator<=(const ipointer& _Right) const { return (item_pointer() <= _Right.item_pointer()); }
				bool operator>(const ipointer& _Right) const { return (item_pointer() > _Right.item_pointer()); }
				bool operator>=(const ipointer& _Right) const { return (item_pointer() >= _Right.item_pointer()); }
				void set_to_item_pointer(const ipointer& _Right_cref) { item_pointer().set_to_item_pointer(_Right_cref.item_pointer()); }
				msev_size_t position() const { return item_pointer().position(); }
				auto target_container_ptr() const {
					return m_owner_ptr;
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
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
				(*this).insert(_P, 1, std::forward<decltype(_X)>(_X));
			}
			void insert_before(const mm_const_iterator_type &pos, const _Ty& _X = _Ty()) { (*this).insert(pos, 1, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				void insert_before(const mm_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
				if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
				//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const mm_const_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
				typename base_class::const_iterator _P = pos;
				(*this).insert(_P, start, end);
			}
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
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
				ipointer retval(*this); retval.advance(msev_int(original_pos));
				return retval;
			}
			ipointer insert_before(const cipointer &pos, _Ty&& _X) {
				msev_size_t original_pos = pos.position();
				insert_before(pos.const_item_pointer(), std::forward<decltype(_X)>(_X));
				ipointer retval(*this); retval.advance(msev_int(original_pos));
				return retval;
			}
			ipointer insert_before(const cipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				ipointer insert_before(const cipointer &pos, const _Iter &start, const _Iter &end) {
				msev_size_t original_pos = pos.position();
				insert_before(pos.const_item_pointer(), start, end);
				ipointer retval(*this); retval.advance(msev_int(original_pos));
				return retval;
			}
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				ipointer insert_before_inclusive(const cipointer &pos, const _Iter &first, const _Iter &last) {
				auto end = last; end++;
				return insert_before(pos, first, end);
			}
			ipointer insert_before(const cipointer &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
				msev_size_t original_pos = pos.position();
				(*this).insert_before(pos.const_item_pointer(), _Ilist);
				ipointer retval(*this); retval.advance(msev_int(original_pos));
				return retval;
			}
			void insert_before(msev_size_t pos, _Ty&& _X) {
				typename base_class::const_iterator _P = (*this).begin() + msev_as_a_size_t(pos);
				(*this).insert(_P, std::forward<decltype(_X)>(_X));
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
			ipointer insert(const cipointer &pos, _Ty&& _X) { return insert_before(pos, std::forward<decltype(_X)>(_X)); }
			ipointer insert(const cipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
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
				ipointer retval(*this); retval.advance(msev_int(original_pos));
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
				retval.advance(msev_int(retval_pos.position()));
				return retval;
			}
			ipointer erase(const cipointer &start, const cipointer &end) {
				auto retval_pos = end;
				retval_pos.set_to_next();
				(*this).erase(start.const_item_pointer(), end.const_item_pointer());
				ipointer retval = (*this).ibegin();
				retval.advance(msev_int(retval_pos.position()));
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
				retval.advance(pos.position());
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
				retval.advance(msev_int(original_pos));
				return retval;
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid argument - void insert_before() - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, std::forward<decltype(_X)>(_X));
				ss_iterator_type retval = ss_begin();
				retval.advance(msev_int(original_pos));
				return retval;
			}
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid argument - ss_iterator_type insert_before() - msevector")); }
				//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
				pos.assert_valid_index();
				msev_size_t original_pos = pos.position();
				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).insert(_P, start, end);
				ss_iterator_type retval = ss_begin();
				retval.advance(msev_int(original_pos));
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
				retval.advance(msev_int(original_pos));
				return retval;
			}
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
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
				retval.advance(msev_int(original_pos));
				return retval;
			}
			/* These insert() functions are just aliases for their corresponding insert_before() functions. */
			ss_iterator_type insert(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, _Ty&& _X) { return insert_before(pos, std::forward<decltype(_X)>(_X)); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
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
				retval.advance(msev_int(original_pos));
				return retval;
			}
			ss_iterator_type erase(const ss_const_iterator_type &pos) {
				if (pos.target_container_ptr() != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				if (!pos.points_to_an_item()) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
				auto pos_index = pos.position();

				typename base_class::const_iterator _P = const_iterator_from_ss_const_iterator_type(pos);
				(*this).erase(_P);

				ss_iterator_type retval = (*this).ss_begin();
				retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
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
				retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
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

			class xscope_ss_iterator_type;

			class xscope_ss_const_iterator_type : public ss_const_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedConstPointer<msevector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<msevector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedConstPointer<msevector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<msevector> >::value
					, void>::type>
				xscope_ss_const_iterator_type(const _TXScopePointer& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}

				xscope_ss_const_iterator_type(const xscope_ss_const_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
				xscope_ss_const_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
				~xscope_ss_const_iterator_type() {}
				const ss_const_iterator_type& msevector_ss_const_iterator_type() const {
					return (*this);
				}
				ss_const_iterator_type& msevector_ss_const_iterator_type() {
					return (*this);
				}
				const ss_const_iterator_type& mvssci() const { return msevector_ss_const_iterator_type(); }
				ss_const_iterator_type& mvssci() { return msevector_ss_const_iterator_type(); }

				void reset() { ss_const_iterator_type::reset(); }
				bool points_to_an_item() const { return ss_const_iterator_type::points_to_an_item(); }
				bool points_to_end_marker() const { return ss_const_iterator_type::points_to_end_marker(); }
				bool points_to_beginning() const { return ss_const_iterator_type::points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return ss_const_iterator_type::has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return ss_const_iterator_type::has_next(); }
				bool has_previous() const { return ss_const_iterator_type::has_previous(); }
				void set_to_beginning() { ss_const_iterator_type::set_to_beginning(); }
				void set_to_end_marker() { ss_const_iterator_type::set_to_end_marker(); }
				void set_to_next() { ss_const_iterator_type::set_to_next(); }
				void set_to_previous() { ss_const_iterator_type::set_to_previous(); }
				xscope_ss_const_iterator_type& operator ++() { ss_const_iterator_type::operator ++(); return (*this); }
				xscope_ss_const_iterator_type operator++(int) { xscope_ss_const_iterator_type _Tmp = *this; ss_const_iterator_type::operator++(); return (_Tmp); }
				xscope_ss_const_iterator_type& operator --() { ss_const_iterator_type::operator --(); return (*this); }
				xscope_ss_const_iterator_type operator--(int) { xscope_ss_const_iterator_type _Tmp = *this; ss_const_iterator_type::operator--(); return (_Tmp); }
				void advance(difference_type n) { ss_const_iterator_type::advance(n); }
				void regress(difference_type n) { ss_const_iterator_type::regress(n); }
				xscope_ss_const_iterator_type& operator +=(difference_type n) { ss_const_iterator_type::operator +=(n); return (*this); }
				xscope_ss_const_iterator_type& operator -=(difference_type n) { ss_const_iterator_type::operator -=(n); return (*this); }
				xscope_ss_const_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const xscope_ss_const_iterator_type& _Right_cref) const { return ss_const_iterator_type::operator-(_Right_cref); }
				const_reference operator*() const { return ss_const_iterator_type::operator*(); }
				const_reference item() const { return operator*(); }
				const_reference previous_item() const { return ss_const_iterator_type::previous_item(); }
				const_pointer operator->() const { return ss_const_iterator_type::operator->(); }
				const_reference operator[](difference_type _Off) const { return ss_const_iterator_type::operator[](_Off); }
				xscope_ss_const_iterator_type& operator=(const ss_const_iterator_type& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const xscope_ss_const_iterator_type& _Right_cref) - msevector::xscope_ss_const_iterator_type")); }
					ss_const_iterator_type::operator=(_Right_cref);
					return (*this);
				}
				xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) - msevector::xscope_ss_const_iterator_type")); }
					return operator=(ss_const_iterator_type(_Right_cref));
				}
				bool operator==(const xscope_ss_const_iterator_type& _Right_cref) const { return ss_const_iterator_type::operator==(_Right_cref); }
				bool operator!=(const xscope_ss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator<(_Right); }
				bool operator<=(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator<=(_Right); }
				bool operator>(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator>(_Right); }
				bool operator>=(const xscope_ss_const_iterator_type& _Right) const { return ss_const_iterator_type::operator>=(_Right); }
				void set_to_const_item_pointer(const xscope_ss_const_iterator_type& _Right_cref) { ss_const_iterator_type::set_to_item_pointer(_Right_cref); }
				msear_size_t position() const { return ss_const_iterator_type::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_const_pointer_to(*(ss_const_iterator_type::target_container_ptr()));
				}
				void xscope_ss_iterator_type_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename ss_const_iterator_type (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class xscope_ss_iterator_type;
			};
			class xscope_ss_iterator_type : public ss_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<msevector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<msevector> >::value
					, void>::type>
				xscope_ss_iterator_type(const _TXScopePointer& owner_ptr) : ss_iterator_type((*owner_ptr).ss_begin()) {}

				xscope_ss_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_iterator_type(src_cref) {}
				~xscope_ss_iterator_type() {}
				const ss_iterator_type& msevector_ss_iterator_type() const {
					return (*this);
				}
				ss_iterator_type& msevector_ss_iterator_type() {
					return (*this);
				}
				const ss_iterator_type& mvssi() const { return msevector_ss_iterator_type(); }
				ss_iterator_type& mvssi() { return msevector_ss_iterator_type(); }

				void reset() { ss_iterator_type::reset(); }
				bool points_to_an_item() const { return ss_iterator_type::points_to_an_item(); }
				bool points_to_end_marker() const { return ss_iterator_type::points_to_end_marker(); }
				bool points_to_beginning() const { return ss_iterator_type::points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return ss_iterator_type::has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return ss_iterator_type::has_next(); }
				bool has_previous() const { return ss_iterator_type::has_previous(); }
				void set_to_beginning() { ss_iterator_type::set_to_beginning(); }
				void set_to_end_marker() { ss_iterator_type::set_to_end_marker(); }
				void set_to_next() { ss_iterator_type::set_to_next(); }
				void set_to_previous() { ss_iterator_type::set_to_previous(); }
				xscope_ss_iterator_type& operator ++() { ss_iterator_type::operator ++(); return (*this); }
				xscope_ss_iterator_type operator++(int) { xscope_ss_iterator_type _Tmp = *this; ss_iterator_type::operator++(); return (_Tmp); }
				xscope_ss_iterator_type& operator --() { ss_iterator_type::operator --(); return (*this); }
				xscope_ss_iterator_type operator--(int) { xscope_ss_iterator_type _Tmp = *this; ss_iterator_type::operator--(); return (_Tmp); }
				void advance(difference_type n) { ss_iterator_type::advance(n); }
				void regress(difference_type n) { ss_iterator_type::regress(n); }
				xscope_ss_iterator_type& operator +=(difference_type n) { ss_iterator_type::operator +=(n); return (*this); }
				xscope_ss_iterator_type& operator -=(difference_type n) { ss_iterator_type::operator -=(n); return (*this); }
				xscope_ss_iterator_type operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
				difference_type operator-(const xscope_ss_iterator_type& _Right_cref) const { return ss_iterator_type::operator-(_Right_cref); }
				reference operator*() const { return ss_iterator_type::operator*(); }
				reference item() const { return operator*(); }
				reference previous_item() const { return ss_iterator_type::previous_item(); }
				pointer operator->() const { return ss_iterator_type::operator->(); }
				reference operator[](difference_type _Off) const { return ss_iterator_type::operator[](_Off); }
				xscope_ss_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_ss_iterator_type& operator=(const xscope_ss_iterator_type& _Right_cref) - msevector::xscope_ss_iterator_type")); }
					ss_iterator_type::operator=(_Right_cref);
					return (*this);
				}
				bool operator==(const xscope_ss_iterator_type& _Right_cref) const { return ss_iterator_type::operator==(_Right_cref); }
				bool operator!=(const xscope_ss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator<(_Right); }
				bool operator<=(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator<=(_Right); }
				bool operator>(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator>(_Right); }
				bool operator>=(const xscope_ss_iterator_type& _Right) const { return ss_iterator_type::operator>=(_Right); }
				void set_to_item_pointer(const xscope_ss_iterator_type& _Right_cref) { ss_iterator_type::set_to_item_pointer(_Right_cref); }
				msear_size_t position() const { return ss_iterator_type::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_pointer_to(*(ss_iterator_type::target_container_ptr()));
				}
				void xscope_ss_iterator_type_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename ss_iterator_type (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};

			typedef xscope_ss_const_iterator_type xscope_const_iterator;
			typedef xscope_ss_iterator_type xscope_iterator;


			class xscope_ipointer;

			class xscope_cipointer : public cipointer, public XScopeTagBase {
			public:
				xscope_cipointer(const mse::TXScopeFixedConstPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
				xscope_cipointer(const mse::TXScopeFixedPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_cipointer(const mse::TXScopeItemFixedConstPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
				xscope_cipointer(const mse::TXScopeItemFixedPointer<msevector>& owner_ptr) : cipointer((*owner_ptr).cibegin()) {}
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
				bool operator==(const xscope_cipointer& _Right_cref) const { return cipointer::operator==(_Right_cref); }
				bool operator!=(const xscope_cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_cipointer& _Right) const { return cipointer::operator<(_Right); }
				bool operator<=(const xscope_cipointer& _Right) const { return cipointer::operator<=(_Right); }
				bool operator>(const xscope_cipointer& _Right) const { return cipointer::operator>(_Right); }
				bool operator>=(const xscope_cipointer& _Right) const { return cipointer::operator>=(_Right); }
				void set_to_const_item_pointer(const xscope_cipointer& _Right_cref) { cipointer::set_to_item_pointer(_Right_cref); }
				msear_size_t position() const { return cipointer::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_const_pointer_to(*(cipointer::target_container_ptr()));
				}
				void xscope_ipointer_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename cipointer (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
				friend class xscope_ipointer;
			};
			class xscope_ipointer : public ipointer, public XScopeTagBase {
			public:
				xscope_ipointer(const mse::TXScopeFixedPointer<msevector>& owner_ptr) : ipointer((*owner_ptr).ibegin()) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_ipointer(const mse::TXScopeItemFixedPointer<msevector>& owner_ptr) : ipointer((*owner_ptr).ibegin()) {}
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
				difference_type operator-(const xscope_ipointer& _Right_cref) const { return ipointer::operator-(_Right_cref); }
				reference operator*() const { return ipointer::operator*(); }
				reference item() const { return operator*(); }
				reference previous_item() const { return ipointer::previous_item(); }
				pointer operator->() const { return ipointer::operator->(); }
				reference operator[](difference_type _Off) const { return ipointer::operator[](_Off); }
				xscope_ipointer& operator=(const ipointer& _Right_cref) {
					if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(msevector_range_error("invalid argument - xscope_ipointer& operator=(const xscope_ipointer& _Right_cref) - msevector::xscope_ipointer")); }
					ipointer::operator=(_Right_cref);
					return (*this);
				}
				bool operator==(const xscope_ipointer& _Right_cref) const { return ipointer::operator==(_Right_cref); }
				bool operator!=(const xscope_ipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_ipointer& _Right) const { return ipointer::operator<(_Right); }
				bool operator<=(const xscope_ipointer& _Right) const { return ipointer::operator<=(_Right); }
				bool operator>(const xscope_ipointer& _Right) const { return ipointer::operator>(_Right); }
				bool operator>=(const xscope_ipointer& _Right) const { return ipointer::operator>=(_Right); }
				void set_to_item_pointer(const xscope_ipointer& _Right_cref) { ipointer::set_to_item_pointer(_Right_cref); }
				msear_size_t position() const { return ipointer::position(); }
				auto target_container_ptr() const {
					return mse::us::unsafe_make_xscope_pointer_to(*(ipointer::target_container_ptr()));
				}
				void xscope_ipointer_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				void* operator new(size_t size) { return ::operator new(size); }

				//typename ipointer (*this);
				friend class /*_Myt*/msevector<_Ty, _A, _TStateMutex>;
			};


			/* For each (scope) vector instance, only one instance of xscope_structure_change_lock_guard may exist at any one
			time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			class xscope_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_structure_change_lock_guard(const mse::TXScopeFixedPointer<msevector>& owner_ptr) : m_unique_lock((*owner_ptr).m_structure_change_mutex), m_stored_ptr(owner_ptr) {}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_structure_change_lock_guard(const mse::TXScopeItemFixedPointer<msevector>& owner_ptr) : m_unique_lock((*owner_ptr).m_structure_change_mutex), m_stored_ptr(owner_ptr) {}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return mse::us::unsafe_make_xscope_pointer_to((*m_stored_ptr)[_P]);
				}
				auto xscope_ptr_to_element(const xscope_ss_iterator_type& ss_iter) const {
					assert(std::addressof(*(ss_iter.target_container_ptr())) == std::addressof(*m_stored_ptr));
					return xscope_ptr_to_element(ss_iter.position());
				}
				auto xscope_ptr_to_element(const xscope_ipointer& iptr) const {
					assert(std::addressof(*(iptr.target_container_ptr())) == std::addressof(*m_stored_ptr));
					return xscope_ptr_to_element(iptr.position());
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

			private:
				std::unique_lock<mse::non_thread_safe_mutex> m_unique_lock;
				mse::TXScopeItemFixedPointer<msevector> m_stored_ptr;
			};
			class xscope_const_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_const_structure_change_lock_guard(const mse::TXScopeFixedConstPointer<msevector>& owner_ptr) : m_unique_lock((*owner_ptr).m_structure_change_mutex), m_stored_ptr(owner_ptr) {}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_const_structure_change_lock_guard(const mse::TXScopeItemFixedConstPointer<msevector>& owner_ptr) : m_unique_lock((*owner_ptr).m_structure_change_mutex), m_stored_ptr(owner_ptr) {}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return mse::us::unsafe_make_xscope_const_pointer_to((*m_stored_ptr)[_P]);
				}
				auto xscope_ptr_to_element(const xscope_cipointer& ciptr) const {
					assert(std::addressof(*(ciptr.target_container_ptr())) == std::addressof(*m_stored_ptr));
					return xscope_ptr_to_element(ciptr.position());
				}
				auto target_container_ptr() const {
					return m_stored_ptr;
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

			private:
				std::unique_lock<mse::non_thread_safe_mutex> m_unique_lock;
				mse::TXScopeItemFixedConstPointer<msevector> m_stored_ptr;
			};

			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

		private:
			mutable mse::non_thread_safe_mutex m_structure_change_mutex;

			auto contained_vector() const -> decltype(base_class::contained_vector()) { return base_class::contained_vector(); }
			auto contained_vector() -> decltype(base_class::contained_vector()) { return base_class::contained_vector(); }
		};

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator!=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test for vector inequality
			return (!(_Left == _Right));
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator>(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left > _Right for vectors
			return (_Right < _Left);
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator<=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left <= _Right for vectors
			return (!(_Right < _Left));
		}

		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex> inline bool operator>=(const msevector<_Ty, _A, _TStateMutex>& _Left,
			const msevector<_Ty, _A, _TStateMutex>& _Right) {	// test if _Left >= _Right for vectors
			return (!(_Left < _Right));
		}

		/* For each (scope) vector instance, only one instance of xscope_structure_change_lock_guard may exist at any one
		time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
		individual elements in the vector do not become invalid by preventing any operation that might resize the vector
		or increase its capacity. Any attempt to execute such an operation would result in an exception. */
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return typename msevector<_Ty, _A, _TStateMutex>::xscope_structure_change_lock_guard(owner_ptr);
		}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return typename msevector<_Ty, _A, _TStateMutex>::xscope_structure_change_lock_guard(owner_ptr);
		}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return msevector<_Ty, _A, _TStateMutex>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
	#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
			return msevector<_Ty, _A, _TStateMutex>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
	#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	}

	/* Using the mse::msevector<> alias of mse::us::msevector<> is deprecated. */
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	using msevector = us::msevector< _Ty, _A, _TStateMutex>;

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
		return us::make_xscope_vector_size_change_lock_guard(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
		return us::make_xscope_vector_size_change_lock_guard(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
		return us::make_xscope_vector_size_change_lock_guard(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedConstPointer<msevector<_Ty, _A, _TStateMutex> >& owner_ptr) {
		return us::make_xscope_vector_size_change_lock_guard(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)


	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeFixedConstPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeItemFixedConstPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
	template<class _TVector>
	auto make_xscope_cipointer(const mse::TXScopeItemFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_cipointer(owner_ptr);
	}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	template<class _TVector>
	auto make_xscope_ipointer(const mse::TXScopeFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_ipointer(owner_ptr);
	}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
	template<class _TVector>
	auto make_xscope_ipointer(const mse::TXScopeItemFixedPointer<_TVector>& owner_ptr) {
		return typename _TVector::xscope_ipointer(owner_ptr);
	}
#endif // !defined(MSE_REGISTEREDPOINTER_DISABLED)


	/* Some iterators are prone to having their target container prematurely deallocated out from under them. In cases where the
	target container is owned by reference counting pointers or "lock" pointers, you can use TStrongFixedIterator<> to weld an
	owning pointer (aka "lease") to the iterator to prevent the target container from being deallocated prematurely. */
	template <class _TIterator, class _TLeaseType>
	class TStrongFixedIterator : public _TIterator
		/* add StrongPointerNotAsyncShareableTagBase as a base class iff it is not already a base class */
		, public std::conditional<std::is_base_of<StrongPointerNotAsyncShareableTagBase, _TIterator>::value, TPlaceHolder_msepointerbasics<TStrongFixedIterator<_TIterator, _TLeaseType> >, StrongPointerNotAsyncShareableTagBase>::type
	{
	public:
		typedef _TIterator base_class;
		TStrongFixedIterator(const TStrongFixedIterator&) = default;
		template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
		TStrongFixedIterator(const TStrongFixedIterator<_TIterator, _TLeaseType2>&src) : base_class(src), m_lease(src.lease()) {}
		_TLeaseType lease() const { return (*this).m_lease; }

		template <class _TIterator2, class _TLeaseType2>
		static TStrongFixedIterator make(const _TIterator2& src_iterator, const _TLeaseType2& lease) {
			return TStrongFixedIterator(src_iterator, lease);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TStrongFixedIterator(const _TIterator& src_iterator, const _TLeaseType& lease/* often a reference counting pointer */)
			: base_class(src_iterator), m_lease(lease) {}
	private:
		TStrongFixedIterator& operator=(const TStrongFixedIterator& _Right_cref) = delete;

		_TLeaseType m_lease;

		//friend class TStrongFixedConstIterator<_TIterator, _TLeaseType>;
	};

	template <class _Ty, class _TLeaseType>
	class TStrongFixedIterator<_Ty*, _TLeaseType> : public TSaferPtrForLegacy<_Ty>, public StrongPointerTagBase {
	public:
		typedef TSaferPtrForLegacy<_Ty> _TIterator;
		typedef _TIterator base_class;
		TStrongFixedIterator(const TStrongFixedIterator&) = default;
		template<class _TLeaseType2, class = typename std::enable_if<std::is_convertible<_TLeaseType2, _TLeaseType>::value, void>::type>
		TStrongFixedIterator(const TStrongFixedIterator<_TIterator, _TLeaseType2>&src) : base_class(src), m_lease(src.lease()) {}
		_TLeaseType lease() const { return (*this).m_lease; }

		template <class _TIterator2, class _TLeaseType2>
		static TStrongFixedIterator make(const _TIterator2& src_iterator, const _TLeaseType2& lease) {
			return TStrongFixedIterator(src_iterator, lease);
		}

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

	protected:
		TStrongFixedIterator(const _TIterator& src_iterator, const _TLeaseType& lease/* often a reference counting pointer */)
			: base_class(src_iterator), m_lease(lease) {}
		TStrongFixedIterator(const _Ty* & src_iterator, const _TLeaseType& lease/* often a reference counting pointer */)
			: base_class(TSaferPtrForLegacy<_Ty>(src_iterator)), m_lease(lease) {}
	private:
		TStrongFixedIterator& operator=(const TStrongFixedIterator& _Right_cref) = delete;

		_TLeaseType m_lease;

		//friend class TStrongFixedConstIterator<_TIterator, _TLeaseType>;
	};

	template <class _TIterator, class _TLeaseType>
	TStrongFixedIterator<_TIterator, _TLeaseType> make_strong_iterator(const _TIterator& src_iterator, const _TLeaseType& lease) {
		return TStrongFixedIterator<_TIterator, _TLeaseType>::make(src_iterator, lease);
	}

}

#undef MSE_THROW

#endif /*ndef MSEMSEVECTOR_H*/
