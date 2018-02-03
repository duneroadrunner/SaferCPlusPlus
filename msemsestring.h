
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSESTRING_H
#define MSEMSESTRING_H

#include <string>
#include "msemsevector.h"

/* for string_view */
//include <__config>
//include <__string>
#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>
//include <__debug>

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

#ifndef _STD
#define _STD std::
#endif /*_STD*/

namespace mse {

	/* This macro roughly simulates constructor inheritance.*/
#define MSE_MSESTRING_USING(Derived, Base) \
    template<typename ...Args, typename = typename std::enable_if<std::is_constructible<Base, Args...>::value>::type> \
    Derived(Args &&...args) : Base(std::forward<Args>(args)...) {}

#if 0
	/* These do not have safe implementations yet. Presumably, safe implementations will resemble those of the vectors. */

	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class nii_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		typedef std::basic_string<_Elem, _Traits, _Alloc> base_class;
		MSE_MSESTRING_USING(nii_basic_string, base_class);

		/* This placeholder implementation is actually not safe to share asynchronously (due to its unsafe iterators), but
		the eventual implementation will be.*/
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};
#endif

	class nii_basic_string_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};
	class nii_basic_string_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};
	class msebasic_string_range_error : public std::range_error {
	public:
		using std::range_error::range_error;
	};
	class msebasic_string_null_dereference_error : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	/* nii_basic_string<> is essentially a memory-safe basic_string that does not expose (unprotected) non-static member functions
	like begin() or end() which return (memory) unsafe iterators. It does provide static member function templates
	like ss_begin<>(...) and ss_end<>(...) which take a pointer parameter and return a (bounds-checked) iterator that
	inherits the safety of the given pointer. nii_basic_string<> also supports "scope" iterators which are safe without any
	run-time overhead. nii_basic_string<> is a data type that is eligible to be shared between asynchronous threads. */
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	class nii_basic_string {
	public:
		typedef std::basic_string<_Ty, _Traits, _A> std_basic_string;
		typedef std_basic_string _MV;
		typedef nii_basic_string _Myt;
		typedef std_basic_string base_class;

		typedef typename std_basic_string::allocator_type allocator_type;
		typedef typename std_basic_string::value_type value_type;
		//typedef typename std_basic_string::size_type size_type;
		typedef msev_size_t size_type;
		//typedef typename std_basic_string::difference_type difference_type;
		typedef msev_int difference_type;
		typedef typename std_basic_string::pointer pointer;
		typedef typename std_basic_string::const_pointer const_pointer;
		typedef typename std_basic_string::reference reference;
		typedef typename std_basic_string::const_reference const_reference;
		static const size_type npos = size_type(-1);

		typedef typename std_basic_string::iterator iterator;
		typedef typename std_basic_string::const_iterator const_iterator;

		typedef typename std_basic_string::reverse_iterator reverse_iterator;
		typedef typename std_basic_string::const_reverse_iterator const_reverse_iterator;

		explicit nii_basic_string(const _A& _Al = _A())
			: m_basic_string(_Al) {
			/*m_debug_size = size();*/
		}
		explicit nii_basic_string(size_type _N)
			: m_basic_string(msev_as_a_size_t(_N)) {
			/*m_debug_size = size();*/
		}
		explicit nii_basic_string(size_type _N, const _Ty& _V, const _A& _Al = _A())
			: m_basic_string(msev_as_a_size_t(_N), _V, _Al) {
			/*m_debug_size = size();*/
		}
		nii_basic_string(std_basic_string&& _X) : m_basic_string(std::forward<decltype(_X)>(_X)) { /*m_debug_size = size();*/ }
		nii_basic_string(const std_basic_string& _X) : m_basic_string(_X) { /*m_debug_size = size();*/ }
		nii_basic_string(_Myt&& _X) : m_basic_string(std::forward<decltype(_X.contained_basic_string())>(_X.contained_basic_string())) { /*m_debug_size = size();*/ }
		nii_basic_string(const _Myt& _X) : m_basic_string(_X.contained_basic_string()) { /*m_debug_size = size();*/ }
		typedef typename std_basic_string::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		nii_basic_string(_It _F, _It _L, const _A& _Al = _A()) : m_basic_string(_F, _L, _Al) { /*m_debug_size = size();*/ }
		nii_basic_string(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : m_basic_string(_F, _L, _Al) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
			nii_basic_string(const _Iter& _First, const _Iter& _Last) : m_basic_string(_First, _Last) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
			//nii_basic_string(const _Iter& _First, const _Iter& _Last, const typename std_basic_string::_Alloc& _Al) : m_basic_string(_First, _Last, _Al) { /*m_debug_size = size();*/ }
			nii_basic_string(const _Iter& _First, const _Iter& _Last, const _A& _Al) : m_basic_string(_First, _Last, _Al) { /*m_debug_size = size();*/ }
		_Myt& operator=(const std_basic_string& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.operator =(_X);
			/*m_debug_size = size();*/
			return (*this);
		}
		_Myt& operator=(_Myt&& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.operator=(std::forward<std_basic_string>(_X));
			return (*this);
		}
		_Myt& operator=(const _Myt& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.operator=(static_cast<const std_basic_string&>(_X));
			return (*this);
		}

		~nii_basic_string() {
			mse::destructor_lock_guard1<_TStateMutex> lock1(m_mutex1);

			/* This is just a no-op function that will cause a compile error when _Ty is not an eligible type. */
			valid_if_Ty_is_not_an_xscope_type();
		}

		operator const _MV() const { return contained_basic_string(); }
		operator _MV() { return contained_basic_string(); }

		void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.reserve(msev_as_a_size_t(_Count));
		}
		size_type capacity() const _NOEXCEPT
		{	// return current length of allocated storage
			return m_basic_string.capacity();
		}
		void shrink_to_fit() {	// reduce capacity
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.shrink_to_fit();
		}
		void resize(size_type _N, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.resize(msev_as_a_size_t(_N), _X);
		}
		typename std_basic_string::const_reference operator[](msev_size_t _P) const {
			return (*this).at(msev_as_a_size_t(_P));
		}
		typename std_basic_string::reference operator[](msev_size_t _P) {
			return (*this).at(msev_as_a_size_t(_P));
		}
		typename std_basic_string::reference front() {	// return first element of mutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_basic_string_range_error("front() on empty - typename std_basic_string::reference front() - nii_basic_string")); }
			return m_basic_string.front();
		}
		typename std_basic_string::const_reference front() const {	// return first element of nonmutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_basic_string_range_error("front() on empty - typename std_basic_string::const_reference front() - nii_basic_string")); }
			return m_basic_string.front();
		}
		typename std_basic_string::reference back() {	// return last element of mutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_basic_string_range_error("back() on empty - typename std_basic_string::reference back() - nii_basic_string")); }
			return m_basic_string.back();
		}
		typename std_basic_string::const_reference back() const {	// return last element of nonmutable sequence
			if (0 == (*this).size()) { MSE_THROW(nii_basic_string_range_error("back() on empty - typename std_basic_string::const_reference back() - nii_basic_string")); }
			return m_basic_string.back();
		}
		void push_back(_Ty&& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.push_back(std::forward<decltype(_X)>(_X));
		}
		void push_back(const _Ty& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.push_back(_X);
		}
		void pop_back() {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.pop_back();
		}
		void assign(_It _F, _It _L) {
			smoke_check_source_iterators(_F, _L);
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.assign(_F, _L);
			/*m_debug_size = size();*/
		}
		template<class _Iter>
		void assign(const _Iter& _First, const _Iter& _Last) {	// assign [_First, _Last)
			smoke_check_source_iterators(_First, _Last);
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.assign(_First, _Last);
			/*m_debug_size = size();*/
		}
		void assign(size_type _N, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.assign(msev_as_a_size_t(_N), _X);
			/*m_debug_size = size();*/
		}
		template<class _Iter>
		void smoke_check_source_iterators(const _Iter& _First, const _Iter& _Last) {
			if (_Last < _First)/*comparison operations should also verify that safe iterators point to the same container*/ {
				MSE_THROW(nii_basic_string_range_error("invalid arguments - void smoke_check_source_iterators() const - nii_basic_string"));
			}
			else if ((!(*this).empty()) && (_First < _Last)) {
#ifndef MSE_NII_VECTOR_SUPPRESS_SOURCE_ITER_ALIAS_CHECK
				/* check for overlap between source and target sequences */
				auto start_of_this_ptr = std::addressof(*begin());
				auto end_of_this_ptr = std::addressof(*(end() - 1)) + 1;
				auto _First_ptr = std::addressof(*_First);
				auto _Last_ptr = std::addressof(*(_Last - 1)) + 1;
				if ((end_of_this_ptr > _First_ptr) && (start_of_this_ptr < _Last_ptr)) {
					MSE_THROW(nii_basic_string_range_error("invalid arguments - void smoke_check_source_iterators() const - nii_basic_string"));
				}
#endif // !MSE_NII_VECTOR_SUPPRESS_SOURCE_ITER_ALIAS_CHECK
			}
		}

		template<class ..._Valty>
		void emplace_back(_Valty&& ..._Val)
		{	// insert by moving into element at end
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.emplace_back(std::forward<_Valty>(_Val)...);
			/*m_debug_size = size();*/
		}
		void clear() {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.clear();
			/*m_debug_size = size();*/
		}

		void swap(_Myt& _Other) {	// swap contents with _Other
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.swap(_Other.m_basic_string);
		}

		void swap(_MV& _Other) {	// swap contents with _Other
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			m_basic_string.swap(_Other);
		}

		size_type size() const _NOEXCEPT
		{	// return length of sequence
			return m_basic_string.size();
		}

		size_type max_size() const _NOEXCEPT
		{	// return maximum possible length of sequence
			return m_basic_string.max_size();
		}

		bool empty() const _NOEXCEPT
		{	// test if sequence is empty
			return m_basic_string.empty();
		}
		_A get_allocator() const _NOEXCEPT
		{	// return allocator object for values
			return m_basic_string.get_allocator();
		}

		reference at(msev_size_t _Pos)
		{	// subscript mutable sequence with checking
			return m_basic_string.at(msev_as_a_size_t(_Pos));
		}

		const_reference at(msev_size_t _Pos) const
		{	// subscript nonmutable sequence with checking
			return m_basic_string.at(msev_as_a_size_t(_Pos));
		}

		nii_basic_string(_XSTD initializer_list<typename std_basic_string::value_type> _Ilist, const _A& _Al = _A())
			: m_basic_string(_Ilist, _Al) {	// construct from initializer_list
										/*m_debug_size = size();*/
		}
		_Myt& operator=(_XSTD initializer_list<typename std_basic_string::value_type> _Ilist) {	// assign initializer_list
			m_basic_string.operator=(static_cast<std_basic_string>(_Ilist));
			return (*this);
		}
		void assign(_XSTD initializer_list<typename std_basic_string::value_type> _Ilist) {	// assign initializer_list
			m_basic_string.assign(_Ilist);
			/*m_debug_size = size();*/
		}

		value_type *data() _NOEXCEPT
		{	// return pointer to mutable data basic_string
			return m_basic_string.data();
		}

		const value_type *data() const _NOEXCEPT
		{	// return pointer to nonmutable data basic_string
			return m_basic_string.data();
		}

		class random_access_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		class random_access_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

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
			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::iterator_category iterator_category;
			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::value_type value_type;
			//typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::difference_type difference_type;
			typedef msev_int difference_type;
			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::pointer const_pointer;
			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::reference const_reference;

			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::pointer pointer;
			typedef typename std::iterator_traits<typename std_basic_string::const_iterator>::reference reference;

			//template<class = typename std::enable_if<std::is_default_constructible<_TVectorConstPointer>::value, void>::type>
			template<class _TVectorConstPointer2 = _TVectorConstPointer, class = typename std::enable_if<(std::is_same<_TVectorConstPointer2, _TVectorConstPointer>::value) && (std::is_default_constructible<_TVectorConstPointer>::value), void>::type>
			Tss_const_iterator_type() {}

			Tss_const_iterator_type(const _TVectorConstPointer& owner_cptr) : m_owner_cptr(owner_cptr) {}

			Tss_const_iterator_type(const Tss_const_iterator_type& src) = default;
			template<class _Ty2, class = typename std::enable_if<std::is_convertible<_Ty2, _TVectorConstPointer>::value, void>::type>
			Tss_const_iterator_type(const Tss_iterator_type<_Ty2, void>& src) : m_owner_cptr(src.target_container_ptr()), m_index(src.position()) {}

			void assert_valid_index() const {
				if (m_owner_cptr->size() < m_index) { MSE_THROW(nii_basic_string_range_error("invalid index - void assert_valid_index() const - ss_const_iterator_type - nii_basic_string")); }
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
					MSE_THROW(nii_basic_string_range_error("attempt to use invalid const_item_pointer - void set_to_next() - Tss_const_iterator_type - nii_basic_string"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(nii_basic_string_range_error("attempt to use invalid const_item_pointer - void set_to_previous() - Tss_const_iterator_type - nii_basic_string"));
				}
			}
			Tss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			Tss_const_iterator_type operator++(int) { Tss_const_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			Tss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			Tss_const_iterator_type operator--(int) { Tss_const_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_cptr->size() < msev_size_t(new_index))) {
					MSE_THROW(nii_basic_string_range_error("index out of range - void advance(difference_type n) - Tss_const_iterator_type - nii_basic_string"));
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
				if (rhs.m_owner_cptr != (*this).m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - difference_type operator-(const Tss_const_iterator_type &rhs) const - nii_basic_string::Tss_const_iterator_type")); }
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
			/*
			Tss_const_iterator_type& operator=(const typename std_basic_string::const_iterator& _Right_cref)
			{
			msev_int d = std::distance<typename std_basic_string::iterator>(m_owner_cptr->cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cptr->size() >= d)) {
			if (m_owner_cptr->size() == d) {
			assert(m_owner_cptr->cend() == _Right_cref);
			}
			m_index = msev_size_t(d);
			std_basic_string::const_iterator::operator=(_Right_cref);
			}
			else {
			MSE_THROW(nii_basic_string_range_error("doesn't seem to be a valid assignment value - Tss_const_iterator_type& operator=(const typename std_basic_string::const_iterator& _Right_cref) - Tss_const_iterator_type - nii_basic_string"));
			}
			return (*this);
			}
			*/
			/*
			Tss_const_iterator_type& operator=(const Tss_const_iterator_type& _Right_cref) {
			((*this).m_owner_cptr) = _Right_cref.m_owner_cptr;
			(*this).m_index = _Right_cref.m_index;
			return (*this);
			}
			*/
			bool operator==(const Tss_const_iterator_type& _Right_cref) const {
				if (this->m_owner_cptr != _Right_cref.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - Tss_const_iterator_type& operator==(const Tss_const_iterator_type& _Right) - Tss_const_iterator_type - nii_basic_string")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const Tss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const Tss_const_iterator_type& _Right) const {
				if (this->m_owner_cptr != _Right.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - Tss_const_iterator_type& operator<(const Tss_const_iterator_type& _Right) - Tss_const_iterator_type - nii_basic_string")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const Tss_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const Tss_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const Tss_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const Tss_const_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			msev_size_t position() const {
				return m_index;
			}
			_TVectorConstPointer target_container_ptr() const {
				return m_owner_cptr;
			}

		private:
			_TVectorConstPointer m_owner_cptr;
			msev_size_t m_index = 0;

			friend class /*_Myt*/nii_basic_string<_Ty>;
		};
		/* Tss_iterator_type is a bounds checked iterator. */
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		class Tss_iterator_type : public random_access_iterator_base {
		public:
			typedef typename std::iterator_traits<typename std_basic_string::iterator>::iterator_category iterator_category;
			typedef typename std::iterator_traits<typename std_basic_string::iterator>::value_type value_type;
			//typedef typename std::iterator_traits<typename std_basic_string::iterator>::difference_type difference_type;
			typedef msev_int difference_type;
			typedef typename std::iterator_traits<typename std_basic_string::iterator>::pointer pointer;
			typedef typename std::iterator_traits<typename std_basic_string::iterator>::reference reference;
			typedef difference_type distance_type;	// retained

													//template<class = typename std::enable_if<std::is_default_constructible<_TVectorPointer>::value, void>::type>
			template<class _TVectorPointer2 = _TVectorPointer, class = typename std::enable_if<(std::is_same<_TVectorPointer2, _TVectorPointer>::value) && (std::is_default_constructible<_TVectorPointer>::value), void>::type>
			Tss_iterator_type() {}

			Tss_iterator_type(const _TVectorPointer& owner_ptr) : m_owner_ptr(owner_ptr) {}

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
					MSE_THROW(nii_basic_string_range_error("attempt to use invalid item_pointer - void set_to_next() - Tss_const_iterator_type - nii_basic_string"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(nii_basic_string_range_error("attempt to use invalid item_pointer - void set_to_previous() - Tss_iterator_type - nii_basic_string"));
				}
			}
			Tss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			Tss_iterator_type operator++(int) { Tss_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			Tss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			Tss_iterator_type operator--(int) { Tss_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_ptr->size() < msev_size_t(new_index))) {
					MSE_THROW(nii_basic_string_range_error("index out of range - void advance(difference_type n) - Tss_iterator_type - nii_basic_string"));
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
				if (rhs.m_owner_ptr != (*this).m_owner_ptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - difference_type operator-(const Tss_iterator_type& rhs) const - nii_basic_string::Tss_iterator_type")); }
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
					MSE_THROW(nii_basic_string_range_error("attempt to use invalid item_pointer - reference previous_item() - Tss_const_iterator_type - nii_basic_string"));
				}
			}
			pointer operator->() const {
				return std::addressof((*m_owner_ptr).at(msev_as_a_size_t((*this).m_index)));
			}
			reference operator[](difference_type _Off) const { return (*m_owner_ptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }
			/*
			Tss_iterator_type& operator=(const typename std_basic_string::iterator& _Right_cref)
			{
			msev_int d = std::distance<typename std_basic_string::iterator>(m_owner_ptr->begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ptr->size() >= d)) {
			if (m_owner_ptr->size() == d) {
			assert(m_owner_ptr->end() == _Right_cref);
			}
			m_index = msev_size_t(d);
			(*this).m_base_iterator.operator=(_Right_cref);
			}
			else {
			MSE_THROW(nii_basic_string_range_error("doesn't seem to be a valid assignment value - Tss_iterator_type& operator=(const typename std_basic_string::iterator& _Right_cref) - Tss_const_iterator_type - nii_basic_string"));
			}
			return (*this);
			}
			*/
			/*
			Tss_iterator_type& operator=(const Tss_iterator_type& _Right_cref) {
			((*this).m_owner_ptr) = _Right_cref.m_owner_ptr;
			(*this).m_index = _Right_cref.m_index;
			return (*this);
			}
			*/
			bool operator==(const Tss_iterator_type& _Right_cref) const {
				if (this->m_owner_ptr != _Right_cref.m_owner_ptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - Tss_iterator_type& operator==(const Tss_iterator_type& _Right) - Tss_iterator_type - nii_basic_string")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const Tss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const Tss_iterator_type& _Right) const {
				if (this->m_owner_ptr != _Right.m_owner_ptr) { MSE_THROW(nii_basic_string_range_error("invalid argument - Tss_iterator_type& operator<(const Tss_iterator_type& _Right) - Tss_iterator_type - nii_basic_string")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const Tss_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const Tss_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const Tss_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const Tss_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
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
		private:
			//msev_pointer<_Myt> m_owner_ptr = nullptr;
			_TVectorPointer m_owner_ptr;
			msev_size_t m_index = 0;

			friend class /*_Myt*/nii_basic_string<_Ty>;
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
		ss_iterator_type ss_begin() {	// return std_basic_string::iterator for beginning of mutable sequence
			ss_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_const_iterator_type ss_begin() const {	// return std_basic_string::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_iterator_type ss_end() {	// return std_basic_string::iterator for end of mutable sequence
			ss_iterator_type retval(this);
			retval.set_to_end_marker();
			return retval;
		}
		ss_const_iterator_type ss_end() const {	// return std_basic_string::iterator for end of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_end_marker();
			return retval;
		}
		ss_const_iterator_type ss_cbegin() const {	// return std_basic_string::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_beginning();
			return retval;
		}
		ss_const_iterator_type ss_cend() const {	// return std_basic_string::iterator for end of nonmutable sequence
			ss_const_iterator_type retval(this);
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_reverse_iterator_type ss_crbegin() const {	// return std_basic_string::iterator for beginning of reversed nonmutable sequence
			return (ss_rbegin());
		}
		ss_const_reverse_iterator_type ss_crend() const {	// return std_basic_string::iterator for end of reversed nonmutable sequence
			return (ss_rend());
		}
		ss_reverse_iterator_type ss_rbegin() {	// return std_basic_string::iterator for beginning of reversed mutable sequence
			return (reverse_iterator(ss_end()));
		}
		ss_const_reverse_iterator_type ss_rbegin() const {	// return std_basic_string::iterator for beginning of reversed nonmutable sequence
			return (const_reverse_iterator(ss_end()));
		}
		ss_reverse_iterator_type ss_rend() {	// return std_basic_string::iterator for end of reversed mutable sequence
			return (reverse_iterator(ss_begin()));
		}
		ss_const_reverse_iterator_type ss_rend() const {	// return std_basic_string::iterator for end of reversed nonmutable sequence
			return (const_reverse_iterator(ss_begin()));
		}

		nii_basic_string(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
			: m_basic_string(_Al) {
			/*m_debug_size = size();*/
			assign(start, end);
		}
		void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			if (start > end) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			typename std_basic_string::const_iterator _F = start;
			typename std_basic_string::const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			auto end = last;
			end++; // this should include some checks
			(*this).assign(first, end);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid argument - void insert_before() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*this).insert(_P, _M, _X);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid argument - void insert_before() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*this).insert(_P, std::forward<decltype(_X)>(_X));
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_basic_string::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid argument - ss_iterator_type insert_before() - nii_basic_string")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type& start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			end.assert_valid_index();
			if (start > end) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			typename std_basic_string::const_iterator _S = start;
			typename std_basic_string::const_iterator _E = end;
			return (*this).insert_before(pos, _S, _E);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - ss_iterator_type insert_before() - nii_basic_string")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - nii_basic_string")); }
			if (start > end) { MSE_THROW(nii_basic_string_range_error("invalid arguments - ss_iterator_type insert_before() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_basic_string::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			ss_iterator_type insert_before_inclusive(const ss_iterator_type &pos, const _Iter &first, const _Iter &last) {
			auto end = last;
			end++; // this may include some checks
			return (*this).insert_before(pos, first, end);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_basic_string::value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert_before() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
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
		//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_basic_string::iterator>::type
		, class = _mse_RequireInputIter<_Iter> >
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, _XSTD initializer_list<typename std_basic_string::value_type> _Ilist) { return insert_before(pos, _Ilist); }
		*/
		/*
		template<class ..._Valty>
		ss_iterator_type emplace(const ss_const_iterator_type &pos, _Valty&& ..._Val)
		{	// insert by moving _Val at pos
		if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void emplace() - nii_basic_string")); }
		pos.assert_valid_index();
		msev_size_t original_pos = pos.position();
		typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
		(*this).emplace(_P, std::forward<_Valty>(_Val)...);
		ss_iterator_type retval = ss_begin();
		retval.advance(msev_int(original_pos));
		return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &pos) {
		if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
		if (!pos.points_to_an_item()) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
		auto pos_index = pos.position();

		typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
		(*this).erase(_P);

		ss_iterator_type retval = (*this).ss_begin();
		retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
		return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
		if (start.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
		if (end.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
		if (start.position() > end.position()) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
		auto pos_index = start.position();

		typename std_basic_string::const_iterator _F = start;
		typename std_basic_string::const_iterator _L = end;
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
			if (pos.m_owner_cptr != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase_previous_item() - nii_basic_string")); }
			if (!(pos.has_previous())) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase_previous_item() - nii_basic_string")); }
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			_P--;
			(*this).erase(_P);
		}

	public:
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> ss_begin(const _TVectorPointer& owner_ptr)
		{	// return iterator for beginning of mutable sequence
			Tss_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_beginning();
			return retval;
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> ss_end(const _TVectorPointer& owner_ptr)
		{	// return iterator for end of mutable sequence
			Tss_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_end_marker();
			return retval;
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_const_iterator_type<_TVectorPointer> ss_cbegin(const _TVectorPointer& owner_ptr)
		{	// return iterator for beginning of nonmutable sequence
			Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_beginning();
			return retval;
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_const_iterator_type<_TVectorPointer> ss_cend(const _TVectorPointer& owner_ptr)
		{	// return iterator for end of nonmutable sequence
			Tss_const_iterator_type<_TVectorPointer> retval(owner_ptr);
			retval.set_to_end_marker();
			return retval;
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_reverse_iterator_type<_TVectorPointer> ss_rbegin(const _TVectorPointer& owner_ptr)
		{	// return iterator for beginning of reversed mutable sequence
			return (Tss_reverse_iterator_type<_TVectorPointer>(ss_end<_TVectorPointer>(owner_ptr)));
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_reverse_iterator_type<_TVectorPointer> ss_rend(const _TVectorPointer& owner_ptr)
		{	// return iterator for end of reversed mutable sequence
			return (Tss_reverse_iterator_type<_TVectorPointer>(ss_cbegin<_TVectorPointer>(owner_ptr)));
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_const_reverse_iterator_type<_TVectorPointer> ss_crbegin(const _TVectorPointer& owner_ptr)
		{	// return iterator for beginning of reversed nonmutable sequence
			return (Tss_const_reverse_iterator_type<_TVectorPointer>(ss_end<_TVectorPointer>(owner_ptr)));
		}

		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> insert(const Tss_const_iterator_type<_TVectorPointer>& _P, _Ty&& _X) {
			return (emplace(_P, std::forward<decltype(_X)>(_X)));
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> insert(const Tss_const_iterator_type<_TVectorPointer>& pos, const _Ty& _X = _Ty()) {
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).insert(_P, _X);
			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> insert(const Tss_const_iterator_type<_TVectorPointer>& pos, size_type _M, const _Ty& _X) {
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).insert(_P, _M, _X);
			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type
			, class _Iter, class = _mse_RequireInputIter<_Iter> >
			static Tss_iterator_type<_TVectorPointer> insert(const Tss_const_iterator_type<_TVectorPointer>& pos, const _Iter& _First, const _Iter& _Last) {
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).insert(_P, _First, _Last);
			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> insert(const Tss_const_iterator_type<_TVectorPointer>& pos, _XSTD initializer_list<typename std_basic_string::value_type> _Ilist) {
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void insert() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).insert(_P, _Ilist);
			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer, class ..._Valty, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> emplace(const Tss_const_iterator_type<_TVectorPointer>& pos, _Valty&& ..._Val)
		{	// insert by moving _Val at _Where
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void emplace() - nii_basic_string")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).emplace(_P, std::forward<_Valty>(_Val)...);
			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(msev_int(original_pos));
			return retval;
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> erase(const Tss_const_iterator_type<_TVectorPointer>& pos) {
			//if (std::addressof(*(pos.target_container_ptr())) != this) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
			if (!pos.points_to_an_item()) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
			auto pos_index = pos.position();

			typename std_basic_string::const_iterator _P = pos.target_container_ptr()->m_basic_string.cbegin() + pos.position();
			(*(pos.target_container_ptr())).erase(_P);

			Tss_iterator_type<_TVectorPointer> retval = ss_begin(pos.target_container_ptr());
			retval.advance(typename Tss_iterator_type<_TVectorPointer>::difference_type(pos_index));
			return retval;
		}
		template<typename _TVectorPointer, class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TVectorPointer>::value), void>::type>
		static Tss_iterator_type<_TVectorPointer> erase(const Tss_const_iterator_type<_TVectorPointer>& start, Tss_const_iterator_type<_TVectorPointer> end) {
			if (std::addressof(*(start.target_container_ptr())) != std::addressof(*(end.target_container_ptr()))) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
			if (start.position() > end.position()) { MSE_THROW(nii_basic_string_range_error("invalid arguments - void erase() - nii_basic_string")); }
			auto pos_index = start.position();

			typename std_basic_string::const_iterator _F = start.target_container_ptr()->m_basic_string.cbegin() + start.position();
			typename std_basic_string::const_iterator _L = end.target_container_ptr()->m_basic_string.cbegin() + end.position();
			(*(start.target_container_ptr())).erase(_F, _L);

			Tss_iterator_type<_TVectorPointer> retval = ss_begin(start.target_container_ptr());
			retval.advance(typename Tss_iterator_type<_TVectorPointer>::difference_type(pos_index));
			return retval;
		}

		class xscope_ss_const_iterator_type : public ss_const_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
		public:
			template <typename _TXScopePointer, class = typename std::enable_if<
				std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedConstPointer<nii_basic_string> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<nii_basic_string> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedConstPointer<nii_basic_string> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<nii_basic_string> >::value
				, void>::type>
			xscope_ss_const_iterator_type(const _TXScopePointer& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}

			xscope_ss_const_iterator_type(const xscope_ss_const_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			xscope_ss_const_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			~xscope_ss_const_iterator_type() {}
			const ss_const_iterator_type& nii_basic_string_ss_const_iterator_type() const {
				return (*this);
			}
			ss_const_iterator_type& nii_basic_string_ss_const_iterator_type() {
				return (*this);
			}
			const ss_const_iterator_type& mvssci() const { return nii_basic_string_ss_const_iterator_type(); }
			ss_const_iterator_type& mvssci() { return nii_basic_string_ss_const_iterator_type(); }

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
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_basic_string_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const xscope_ss_const_iterator_type& _Right_cref) - nii_basic_string::xscope_ss_const_iterator_type")); }
				ss_const_iterator_type::operator=(_Right_cref);
				return (*this);
			}
			xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_basic_string_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) - nii_basic_string::xscope_ss_const_iterator_type")); }
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
			friend class /*_Myt*/nii_basic_string<_Ty>;
			friend class xscope_ss_iterator_type;
		};
		class xscope_ss_iterator_type : public ss_iterator_type, public XScopeContainsNonOwningScopeReferenceTagBase {
		public:
			template <typename _TXScopePointer, class = typename std::enable_if<
				std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<nii_basic_string> >::value
				|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<nii_basic_string> >::value
				, void>::type>
			xscope_ss_iterator_type(const _TXScopePointer& owner_ptr) : ss_iterator_type((*owner_ptr).ss_begin()) {}

			xscope_ss_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_iterator_type(src_cref) {}
			~xscope_ss_iterator_type() {}
			const ss_iterator_type& nii_basic_string_ss_iterator_type() const {
				return (*this);
			}
			ss_iterator_type& nii_basic_string_ss_iterator_type() {
				return (*this);
			}
			const ss_iterator_type& mvssi() const { return nii_basic_string_ss_iterator_type(); }
			ss_iterator_type& mvssi() { return nii_basic_string_ss_iterator_type(); }

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
				if ((&(*_Right_cref.target_container_ptr())) != (&(*(*this).target_container_ptr()))) { MSE_THROW(nii_basic_string_range_error("invalid argument - xscope_ss_iterator_type& operator=(const xscope_ss_iterator_type& _Right_cref) - nii_basic_string::xscope_ss_iterator_type")); }
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
			friend class /*_Myt*/nii_basic_string<_Ty>;
		};

		typedef xscope_ss_const_iterator_type xscope_const_iterator;
		typedef xscope_ss_iterator_type xscope_iterator;

		bool operator==(const _Myt& _Right) const {	// test for basic_string equality
			return (_Right.m_basic_string == m_basic_string);
		}
		bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for basic_strings
			return (m_basic_string < _Right.m_basic_string);
		}

		nii_basic_string& append(const std::initializer_list<_Ty>& _Ilist) {
			m_basic_string.append(_Ilist);
			return (*this);
		}
		nii_basic_string& append(const nii_basic_string& _Right) {
			m_basic_string.append(_Right);
			return (*this);
		}
		nii_basic_string& append(const nii_basic_string& _Right, const size_type _Roff, size_type _Count = npos) {
			m_basic_string.append(_Right, _Roff, _Count);
			return (*this);
		}
#if 0//_HAS_CXX17
		nii_basic_string& append(const nii_basic_string_view<_Ty, _Traits> _Right)
		{	// append _Right
			return (append(_Right.data(), _Convert_size<size_type>(_Right.size())));
		}
		template<class _StringViewIsh,
			class = _Is_string_view_ish<_StringViewIsh>>
			nii_basic_string& append(const _StringViewIsh& _Right, const size_type _Roff, const size_type _Count = npos)
		{	// append _Right [_Roff, _Roff + _Count)
			nii_basic_string_view<_Ty, _Traits> _As_view = _Right;
			return (append(_As_view.substr(_Roff, _Count)));
		}
#endif /* _HAS_CXX17 */
		nii_basic_string& append(const _Ty * const _Ptr, const size_type _Count) {
			m_basic_string.append(_Ptr, _Count);
			return (*this);
		}
		nii_basic_string& append(const _Ty * const _Ptr) {
			m_basic_string.append(_Ptr);
			return (*this);
		}
		nii_basic_string& append(const size_type _Count, const _Ty _Ch) {
			m_basic_string.append(_Count, _Ch);
			return (*this);
		}
		template<class _Iter, class = typename std::enable_if<mse::_mse_Is_iterator<_Iter>::value, void>::type>
		nii_basic_string& append(const _Iter _First, const _Iter _Last) {
			m_basic_string.append(_First, _Last);
			return (*this);
		}

		nii_basic_string& operator+=(const std::initializer_list<_Ty>& _Ilist) {
			m_basic_string.append(_Ilist);
			return (*this);
		}
		nii_basic_string& operator+=(const nii_basic_string& _Right) {
			m_basic_string.append(_Right);
			return (*this);
		}
#if 0//_HAS_CXX17
		nii_basic_string& operator+=(const nii_basic_string_view<_Ty, _Traits> _Right)
		{	// append _Right
			return (append(_Right.data(), _Convert_size<size_type>(_Right.size())));
		}
#endif /* _HAS_CXX17 */
		/*
		nii_basic_string& operator+=(_In_z_ const _Ty * const _Ptr) {	// append [_Ptr, <null>)
		return (append(_Ptr, _Convert_size<size_type>(_Traits::length(_Ptr))));
		}
		*/
		nii_basic_string& operator+=(const _Ty _Ch) {
			m_basic_string += _Ch;
			return (*this);
		}

		nii_basic_string& replace(const size_type _Off, const size_type _N0, const nii_basic_string& _Right) {
			m_basic_string.replace(_Off, _N0, _Right);
			return (*this);
		}

		nii_basic_string& replace(const size_type _Off, size_type _N0,
			const nii_basic_string& _Right, const size_type _Roff, size_type _Count = npos) {
			m_basic_string.replace(_Right, _Roff, _Count);
			return (*this);
		}

#if 0//_HAS_CXX17
		nii_basic_string& replace(const size_type _Off, const size_type _N0, const nii_basic_string_view<_Ty, _Traits> _Right)
		{	// replace [_Off, _Off + _N0) with _Right
			m_basic_string.replace(_Off, _N0, _Right);
			return (*this);
		}

		template<class _StringViewIsh,
			class = _Is_string_view_ish<_StringViewIsh>>
			nii_basic_string& replace(const size_type _Off, const size_type _N0,
				const _StringViewIsh& _Right, const size_type _Roff, const size_type _Count = npos)
		{	// replace [_Off, _Off + _N0) with _Right [_Roff, _Roff + _Count)
			nii_basic_string_view<_Ty, _Traits> _As_view = _Right;
			return (replace(_Off, _N0, _As_view.substr(_Roff, _Count)));
		}
#endif /* _HAS_CXX17 */

		nii_basic_string& replace(const size_type _Off, size_type _N0, const _Ty * const _Ptr, const size_type _Count) {
			m_basic_string.replace(_Off, _N0, _Ptr, _Count);
			return (*this);
		}

		nii_basic_string& replace(const size_type _Off, const size_type _N0, const _Ty * const _Ptr) {
			return (replace(_Off, _N0, _Ptr, size_type(_Traits::length(_Ptr))));
		}

		nii_basic_string& replace(const size_type _Off, size_type _N0, const size_type _Count, const _Ty _Ch) {
			m_basic_string.replace(_Off, _N0, _Count, _Ch);
			return (*this);
		}

		nii_basic_string& replace(const const_iterator _First, const const_iterator _Last, const nii_basic_string& _Right) {
			return (replace(_First - begin(), _Last - _First, _Right));
		}

#if 0//_HAS_CXX17
		nii_basic_string& replace(const const_iterator _First, const const_iterator _Last,
			const nii_basic_string_view<_Ty, _Traits> _Right) {
			return (replace(_First - begin(), _Last - _First, _Right));
		}
#endif /* _HAS_CXX17 */

		nii_basic_string& replace(const const_iterator _First, const const_iterator _Last,
			const _Ty * const _Ptr, const size_type _Count) {
			return (replace(_First - begin(), _Last - _First, _Ptr, _Count));
		}

		nii_basic_string& replace(const const_iterator _First, const const_iterator _Last, const _Ty * const _Ptr) {
			return (replace(_First - begin(), _Last - _First, _Ptr));
		}

		nii_basic_string& replace(const const_iterator _First, const const_iterator _Last, const size_type _Count, const _Ty _Ch) {
			return (replace(_First - begin(), _Last - _First, _Count, _Ch));
		}

		template<class _Iter, class = typename std::enable_if<mse::_mse_Is_iterator<_Iter>::value>::type>
			nii_basic_string& replace(const const_iterator _First, const const_iterator _Last,
				const _Iter _First2, const _Iter _Last2) {
			const nii_basic_string _Right(_First2, _Last2, get_allocator());
			replace(_First, _Last, _Right);
			return (*this);
		}

#if 0//_HAS_CXX17
		int compare(const nii_basic_string_view<_Ty, _Traits> _Right) const _NOEXCEPT {
			auto& _My_data = this->_Get_data();
			return (_Traits_compare<_Traits>(_My_data._Myptr(), _My_data._Mysize,
				_Right.data(), _Right.size()));
		}

		int compare(const size_type _Off, const size_type _N0,
			const nii_basic_string_view<_Ty, _Traits> _Right) const
		{	// compare [_Off, _Off + _N0) with _Right
			auto& _My_data = this->_Get_data();
			_My_data._Check_offset(_Off);
			return (_Traits_compare<_Traits>(_My_data._Myptr() + _Off, _My_data._Clamp_suffix_size(_Off, _N0),
				_Right.data(), _Right.size()));
		}

		template<class _StringViewIsh,
			class = _Is_string_view_ish<_StringViewIsh>>
			int compare(const size_type _Off, const size_type _N0,
				const _StringViewIsh& _Right, const size_type _Roff, const size_type _Count = npos) const
		{	// compare [_Off, _Off + _N0) with _Right [_Roff, _Roff + _Count)
			nii_basic_string_view<_Ty, _Traits> _As_view = _Right;
			auto& _My_data = this->_Get_data();
			_My_data._Check_offset(_Off);
			const auto _With_substr = _As_view.substr(_Roff, _Count);
			return (_Traits_compare<_Traits>(_My_data._Myptr() + _Off, _My_data._Clamp_suffix_size(_Off, _N0),
				_With_substr.data(), _With_substr.size()));
		}
#endif /* _HAS_CXX17 */

		int compare(const nii_basic_string& _Right) const _NOEXCEPT {
			return m_basic_string.compare(_Right);
		}

		int compare(size_type _Off, size_type _N0, const nii_basic_string& _Right) const {
			return m_basic_string.compare(_Off, _N0, _Right);
		}

		int compare(const size_type _Off, const size_type _N0, const nii_basic_string& _Right,
			const size_type _Roff, const size_type _Count = npos) const {
			return m_basic_string.compare(_Off, _N0, _Right, _Roff, _Count);
		}

		int compare(const _Ty * const _Ptr) const _NOEXCEPT {
			return m_basic_string.compare(_Ptr);
		}

		int compare(const size_type _Off, const size_type _N0, const _Ty * const _Ptr) const {
			return m_basic_string.compare(_Off, _N0, _Ptr);
		}

		int compare(const size_type _Off, const size_type _N0, const _Ty * const _Ptr,
			const size_type _Count) const {
			return m_basic_string.compare(_Off, _N0, _Ptr, _Count);
		}


#if 0//_HAS_CXX17
		size_type find(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const _NOEXCEPT
		{	// look for _Right beginning at or after _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(
				_Traits_find<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off, _Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type find(const nii_basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find(_Right, _Off);
		}

		size_type find(const _Ty * const _Ptr, const size_type _Off, const size_type _Count) const _NOEXCEPT {
			return m_basic_string.find(_Ptr, _Off, _Count);
		}

		size_type find(const _Ty * const _Ptr, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find(_Ptr, _Off);
		}

		size_type find(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find(_Ch, _Off);
		}

#if 0//_HAS_CXX17
		size_type rfind(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const _NOEXCEPT
		{	// look for _Right beginning before _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(
				_Traits_rfind<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off, _Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type rfind(const nii_basic_string& _Right, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.rfind(_Right, _Off);
		}

		size_type rfind(const _Ty * const _Ptr, const size_type _Off, const size_type _Count) const _NOEXCEPT {
			return m_basic_string.rfind(_Ptr, _Off, _Count);
		}

		size_type rfind(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.rfind(_Ptr, _Off);
		}

		size_type rfind(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.rfind(_Ch, _Off);
		}

#if 0//_HAS_CXX17
		size_type find_first_of(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const _NOEXCEPT
		{	// look for one of _Right at or after _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(_Traits_find_first_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
				_Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type find_first_of(const nii_basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_of(_Right, _Off);
		}

		size_type find_first_of(const _Ty * const _Ptr, const size_type _Off,
			const size_type _Count) const _NOEXCEPT {
			return m_basic_string.find_first_of(_Ptr, _Off, _Count);
		}

		size_type find_first_of(const _Ty * const _Ptr, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_of(_Ptr, _Off);
		}

		size_type find_first_of(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_of(_Ch, _Off);
		}

#if 0//_HAS_CXX17
		size_type find_last_of(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const _NOEXCEPT
		{	// look for one of _Right before _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(_Traits_find_last_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
				_Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type find_last_of(const nii_basic_string& _Right, size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_of(_Right, _Off);
		}

		size_type find_last_of(const _Ty * const _Ptr, const size_type _Off,
			const size_type _Count) const _NOEXCEPT {
			return m_basic_string.find_last_of(_Ptr, _Off, _Count);
		}

		size_type find_last_of(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_of(_Ptr, _Off);
		}

		size_type find_last_of(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_of(_Ch, _Off);
		}

#if 0//_HAS_CXX17
		size_type find_first_not_of(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const
			_NOEXCEPT
		{	// look for none of _Right at or after _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(_Traits_find_first_not_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
				_Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type find_first_not_of(const nii_basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_not_of(_Right, _Off);
		}

		size_type find_first_not_of(const _Ty * const _Ptr, const size_type _Off,
			const size_type _Count) const _NOEXCEPT {
			return m_basic_string.find_first_not_of(_Ptr, _Off, _Count);
		}

		size_type find_first_not_of(const _Ty * const _Ptr, size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_not_of(_Ptr, _Off);
		}

		size_type find_first_not_of(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
			return m_basic_string.find_first_not_of(_Ch, _Off);
		}

#if 0//_HAS_CXX17
		size_type find_last_not_of(const nii_basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const
			_NOEXCEPT
		{	// look for none of _Right before _Off
			auto& _My_data = this->_Get_data();
			return (static_cast<size_type>(_Traits_find_last_not_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
				_Right.data(), _Right.size())));
		}
#endif /* _HAS_CXX17 */

		size_type find_last_not_of(const nii_basic_string& _Right, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_not_of(_Right, _Off);
		}

		size_type find_last_not_of(const _Ty * const _Ptr, const size_type _Off,
			const size_type _Count) const _NOEXCEPT {
			return m_basic_string.find_last_not_of(_Ptr, _Off, _Count);
		}

		size_type find_last_not_of(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_not_of(_Ptr, _Off);
		}

		size_type find_last_not_of(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
			return m_basic_string.find_last_not_of(_Ch, _Off);
		}

		template <typename _TRAIterator2>
		size_type copy(_TRAIterator2 target_iter, size_type n, size_type pos = 0) const {
			if (pos + n > (*this).size()) {
				if (pos >= (*this).size()) {
					return 0;
				}
				else {
					n = (*this).size() - pos;
				}
			}
			for (size_type i = 0; i < n; i += 1) {
				(*target_iter) = m_basic_string[i];
				++target_iter;
			}
			return n;
		}

		nii_basic_string substr(const size_type _Off = 0, const size_type _Count = npos) const {
			//return (nii_basic_string(*this, _Off, _Count, get_allocator()));
			return (nii_basic_string(m_basic_string.substr(_Off, _Count)));
		}

		/* This basic_string is safely "async shareable" if the elements it contains are also "async shareable". */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (
			(std::integral_constant<bool, HasAsyncShareableTagMethod_msemsearray<_Ty2>::Has>()) || (std::is_arithmetic<_Ty2>::value)
			), void>::type>
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */

	private:
		/* If _Ty is an xscope type, then the following member function will not instantiate, causing an
		(intended) compile error. */
		template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value) && (!std::is_base_of<XScopeTagBase, _Ty2>::value), void>::type>
		void valid_if_Ty_is_not_an_xscope_type() const {}

		typename std_basic_string::iterator begin() {	// return iterator for beginning of mutable sequence
			return m_basic_string.begin();
		}
		typename std_basic_string::iterator end() {	// return iterator for end of mutable sequence
			return m_basic_string.end();
		}
		typename std_basic_string::const_iterator cbegin() {	// return iterator for beginning of nonmutable sequence
			return m_basic_string.cbegin();
		}
		typename std_basic_string::const_iterator cend() {	// return iterator for end of nonmutable sequence
			return m_basic_string.cend();
		}
		typename std_basic_string::reverse_iterator rbegin() {	// return iterator for beginning of reversed mutable sequence
			return m_basic_string.rbegin();
		}
		typename std_basic_string::reverse_iterator rend() {	// return iterator for end of reversed mutable sequence
			return m_basic_string.rend();
		}
		typename std_basic_string::const_reverse_iterator crbegin() {	// return iterator for beginning of reversed nonmutable sequence
			return m_basic_string.crbegin();
		}
		typename std_basic_string::const_reverse_iterator crend() {	// return iterator for end of reversed nonmutable sequence
			return m_basic_string.crend();
		}


		typename std_basic_string::iterator insert(typename std_basic_string::const_iterator _P, _Ty&& _X) {
			return (emplace(_P, std::forward<decltype(_X)>(_X)));
		}
		typename std_basic_string::iterator insert(typename std_basic_string::const_iterator _P, const _Ty& _X = _Ty()) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_basic_string::iterator retval = m_basic_string.insert(_P, _X);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_basic_string::iterator insert(typename std_basic_string::const_iterator _P, size_type _M, const _Ty& _X) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_basic_string::iterator retval = m_basic_string.insert(_P, msev_as_a_size_t(_M), _X);
			/*m_debug_size = size();*/
			return retval;
		}
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename std_basic_string::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
			typename std_basic_string::iterator insert(typename std_basic_string::const_iterator _Where, const _Iter& _First, const _Iter& _Last) {	// insert [_First, _Last) at _Where
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			auto retval = m_basic_string.insert(_Where, _First, _Last);
			/*m_debug_size = size();*/
			return retval;
		}
		template<class ..._Valty>
		typename std_basic_string::iterator emplace(typename std_basic_string::const_iterator _Where, _Valty&& ..._Val)
		{	// insert by moving _Val at _Where
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			auto retval = m_basic_string.emplace(_Where, std::forward<_Valty>(_Val)...);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_basic_string::iterator erase(typename std_basic_string::const_iterator _P) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_basic_string::iterator retval = m_basic_string.erase(_P);
			/*m_debug_size = size();*/
			return retval;
		}
		typename std_basic_string::iterator erase(typename std_basic_string::const_iterator _F, typename std_basic_string::const_iterator _L) {
			std::lock_guard<_TStateMutex> lock1(m_mutex1);
			typename std_basic_string::iterator retval = m_basic_string.erase(_F, _L);
			/*m_debug_size = size();*/
			return retval;
		}

		typename std_basic_string::iterator insert(typename std_basic_string::const_iterator _Where, _XSTD initializer_list<typename std_basic_string::value_type> _Ilist) {	// insert initializer_list
			auto retval = m_basic_string.insert(_Where, _Ilist);
			/*m_debug_size = size();*/
			return retval;
		}


		const _MV& contained_basic_string() const { return m_basic_string; }
		_MV& contained_basic_string() { return m_basic_string; }

		std_basic_string m_basic_string;
		_TStateMutex m_mutex1;

		friend class xscope_ss_const_iterator_type;
		friend class xscope_ss_iterator_type;
		//friend class us::msebasic_string<_Ty, _Traits, _A, _TStateMutex>;
	};

	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	inline bool operator!=(const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left,
		const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) {	// test for basic_string inequality
		return (!(_Left == _Right));
	}

	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	inline bool operator>(const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left,
		const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) {	// test if _Left > _Right for basic_strings
		return (_Right < _Left);
	}

	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	inline bool operator<=(const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left,
		const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) {	// test if _Left <= _Right for basic_strings
		return (!(_Right < _Left));
	}

	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = default_state_mutex>
	inline bool operator>=(const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left,
		const nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) {	// test if _Left >= _Right for basic_strings
		return (!(_Left < _Right));
	}


	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const nii_basic_string<_Elem, _Traits, _Alloc>& _Left,
			const nii_basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return string + string
		nii_basic_string<_Elem, _Traits, _Alloc> _Ans;
		_Ans.reserve(_Left.size() + _Right.size());
		_Ans += _Left;
		_Ans += _Right;
		return (_Ans);
	}

	/*
	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(_In_z_ const _Elem * const _Left,
			const nii_basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return NTCTS + string
		using _String_type = nii_basic_string<_Elem, _Traits, _Alloc>;
		using _Size_type = typename _String_type::size_type;
		_String_type _Ans;
		_Ans.reserve(_Convert_size<_Size_type>(_Traits::length(_Left) + _Right.size()));
		_Ans += _Left;
		_Ans += _Right;
		return (_Ans);
	}
	*/

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem _Left,
			const nii_basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return character + string
		nii_basic_string<_Elem, _Traits, _Alloc> _Ans;
		_Ans.reserve(1 + _Right.size());
		_Ans += _Left;
		_Ans += _Right;
		return (_Ans);
	}

	/*
	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const nii_basic_string<_Elem, _Traits, _Alloc>& _Left,
			_In_z_ const _Elem * const _Right) {	// return string + NTCTS
		using _String_type = nii_basic_string<_Elem, _Traits, _Alloc>;
		using _Size_type = typename _String_type::size_type;
		_String_type _Ans;
		_Ans.reserve(_Convert_size<_Size_type>(_Left.size() + _Traits::length(_Right)));
		_Ans += _Left;
		_Ans += _Right;
		return (_Ans);
	}
	*/

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const nii_basic_string<_Elem, _Traits, _Alloc>& _Left,
			const _Elem _Right) {	// return string + character
		nii_basic_string<_Elem, _Traits, _Alloc> _Ans;
		_Ans.reserve(_Left.size() + 1);
		_Ans += _Left;
		_Ans += _Right;
		return (_Ans);
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const nii_basic_string<_Elem, _Traits, _Alloc>& _Left,
			nii_basic_string<_Elem, _Traits, _Alloc>&& _Right) {	// return string + string
		return (_STD move(_Right.insert(0, _Left)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(nii_basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const nii_basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return string + string
		return (_STD move(_Left.append(_Right)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(nii_basic_string<_Elem, _Traits, _Alloc>&& _Left,
			nii_basic_string<_Elem, _Traits, _Alloc>&& _Right) {	// return string + string
		if (_Right.size() <= _Left.capacity() - _Left.size()
			|| _Right.capacity() - _Right.size() < _Left.size())
			return (_STD move(_Left.append(_Right)));
		else
			return (_STD move(_Right.insert(0, _Left)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem * const _Left,
			nii_basic_string<_Elem, _Traits, _Alloc>&& _Right) {
		return (_STD move(_Right.insert(0, _Left)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem _Left,
			nii_basic_string<_Elem, _Traits, _Alloc>&& _Right) {
		using size_type = typename std::allocator_traits<_Alloc>::size_type;
		return (_STD move(_Right.insert(static_cast<size_type>(0), static_cast<size_type>(1), _Left)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(nii_basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const _Elem * const _Right) {
		return (_STD move(_Left.append(_Right)));
	}

	template<class _Elem, class _Traits, class _Alloc>
	inline nii_basic_string<_Elem, _Traits, _Alloc> operator+(nii_basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const _Elem _Right) {	// return string + character
		_Left.push_back(_Right);
		return (_STD move(_Left));
	}
}

namespace std {

	template<class _Elem, class _Traits, class _Alloc>
	struct hash<mse::nii_basic_string<_Elem, _Traits, _Alloc> > {	// hash functor for mse::nii_basic_string
		using argument_type = mse::nii_basic_string<_Elem, _Traits, _Alloc>;
		using result_type = size_t;

		size_t operator()(const mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Keyval) const _NOEXCEPT {
			std::hash<basic_string<_Elem, _Traits, _Alloc> > basic_string_hash1;
			basic_string<_Elem, _Traits, _Alloc> basic_string1;
			_Keyval.swap(basic_string1);
			auto retval = basic_string_hash1(basic_string1);
			_Keyval.swap(basic_string1);
			return retval;
		}
	};

	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& operator>>(basic_istream<_Elem, _Traits>&& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str) {	// extract a string
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = _Istr >> basic_string1;
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>&& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim) {	// get characters into string, discard delimiter
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = getline(_Istr, basic_string1, _Delim);
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>&& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str) {	// get characters into string, discard newline
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = getline(_Istr, basic_string1);
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& operator>>(basic_istream<_Elem, _Traits>& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str) {	// extract a string
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = _Istr >> basic_string1;
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim) {	// get characters into string, discard delimiter
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = getline(_Istr, basic_string1, _Delim);
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>& _Istr,
			mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str) {	// get characters into string, discard newline
		std::basic_string<_Elem, _Traits, _Alloc> basic_string1; _Str.swap(basic_string1);
		auto& retval = getline(_Istr, basic_string1);
		_Str.swap(basic_string1);
		return retval;
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_ostream<_Elem, _Traits>& operator<<(basic_ostream<_Elem, _Traits>& _Ostr,
			const mse::nii_basic_string<_Elem, _Traits, _Alloc>& _Str) {
		return _Ostr << _Str.operator const std::basic_string<_Elem, _Traits, _Alloc>();
	}

	template<class _Ty, class _Traits, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left, mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) _NOEXCEPT
	{
		_Left.swap(_Right);
	}
	template<class _Ty, class _Traits, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(basic_string<_Ty, _A>& _Left, mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap basic_strings
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _Traits, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Size == 0 || _Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left, basic_string<_Ty, _A>& _Right) _NOEXCEPT
	{
		_Left.swap(_Right);
	}
}

namespace mse {

	class nii_string : public nii_basic_string<char> {
	public:
		MSE_MSESTRING_USING(nii_string, nii_basic_string<char>);
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};
	class nii_wstring : public nii_basic_string<wchar_t> {
	public:
		MSE_MSESTRING_USING(nii_wstring, nii_basic_string<wchar_t>);
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};
	class nii_u16string : public nii_basic_string<char16_t> {
	public:
		MSE_MSESTRING_USING(nii_u16string, nii_basic_string<char16_t>);
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};
	class nii_u32string : public nii_basic_string<char32_t> {
	public:
		MSE_MSESTRING_USING(nii_u32string, nii_basic_string<char32_t>);
		void async_shareable_tag() const {} /* Indication that this type is eligible to be shared between threads. */
	};


	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem> >
	class mse_basic_string : public std::basic_string<_Elem, _Traits, _Alloc> {
	public:
		typedef std::basic_string<_Elem, _Traits, _Alloc> base_class;
		MSE_MSESTRING_USING(mse_basic_string, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};

	class msestring : public mse_basic_string<char> {
	public:
		typedef mse_basic_string<char> base_class;
		MSE_MSESTRING_USING(msestring, base_class);

		void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
	};


	//===------------------------ string_view ---------------------------------===//
	//
	//                     The LLVM Compiler Infrastructure
	//
	// This file is distributed under the University of Illinois Open Source
	// License.
	//
	//===----------------------------------------------------------------------===//

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif //_NOEXCEPT

#define _LIBCPP_TEMPLATE_VIS
#define _LIBCPP_INLINE_VISIBILITY
#define _LIBCPP_CONSTEXPR
#define _LIBCPP_CONSTEXPR_AFTER_CXX11
#define _LIBCPP_CONSTEXPR_AFTER_CXX14
#define _LIBCPP_CONSTEXPR_AFTER_CXX17
#define _LIBCPP_NODISCARD
#define _LIBCPP_NODISCARD_AFTER_CXX11
#define _LIBCPP_NODISCARD_AFTER_CXX14
#define _LIBCPP_NODISCARD_AFTER_CXX17
#define _LIBCPP_ASSERT(x, y) assert(x)
#define _VSTD std
#define __throw_out_of_range(x) throw(x)

	namespace us {
		namespace impl {
			typedef decltype(std::declval<int*>() - std::declval<int*>()) ptrdiff_t;

			// __str_find
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos >= __sz)
					return __npos;
				const _CharT* __r = _Traits::find(__p + __pos, __sz - __pos, __c);
				if (__r == 0)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}
			template <class _CharT, class _Traits>
			inline _LIBCPP_CONSTEXPR_AFTER_CXX11 const _CharT *
				__search_substring(const _CharT *__first1, const _CharT *__last1,
					const _CharT *__first2, const _CharT *__last2) {
				// Take advantage of knowing source and pattern lengths.
				// Stop short when source is smaller than pattern.
				const ptrdiff_t __len2 = __last2 - __first2;
				if (__len2 == 0)
					return __first1;

				ptrdiff_t __len1 = __last1 - __first1;
				if (__len1 < __len2)
					return __last1;

				// First element of __first2 is loop invariant.
				_CharT __f2 = *__first2;
				while (true) {
					__len1 = __last1 - __first1;
					// Check whether __first1 still has at least __len2 bytes.
					if (__len1 < __len2)
						return __last1;

					// Find __f2 the first byte matching in __first1.
					__first1 = _Traits::find(__first1, __len1 - __len2 + 1, __f2);
					if (__first1 == 0)
						return __last1;

					// It is faster to compare from the first byte of __first1 even if we
					// already know that it matches the first byte of __first2: this is because
					// __first2 is most likely aligned, as it is user's "pattern" string, and
					// __first1 + 1 is most likely not aligned, as the match is in the middle of
					// the string.
					if (_Traits::compare(__first1, __first2, __len2) == 0)
						return __first1;

					++__first1;
				}
			}

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos > __sz)
					return __npos;

				if (__n == 0) // There is nothing to search, just return __pos.
					return __pos;

				const _CharT *__r = __search_substring<_CharT, _Traits>(
					__p + __pos, __p + __sz, __s, __s + __n);

				if (__r == __p + __sz)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}


			// __str_rfind

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_rfind(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__sz < 1)
					return __npos;
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
				{
					if (_Traits::eq(*--__ps, __c))
						return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}

			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_rfind(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				__pos = _VSTD::min(__pos, __sz);
				if (__n < __sz - __pos)
					__pos += __n;
				else
					__pos = __sz;
				const _CharT* __r = _VSTD::__find_end(
					__p, __p + __pos, __s, __s + __n, _Traits::eq,
					std::random_access_iterator_tag(), std::random_access_iterator_tag());
				if (__n > 0 && __r == __p + __pos)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}

			template <class _ForwardIterator1, class _ForwardIterator2, class _BinaryPredicate>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _ForwardIterator1
				__find_first_of_ce(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
					_ForwardIterator2 __first2, _ForwardIterator2 __last2, _BinaryPredicate __pred)
			{
				for (; __first1 != __last1; ++__first1)
					for (_ForwardIterator2 __j = __first2; __j != __last2; ++__j)
						if (__pred(*__first1, *__j))
							return __first1;
				return __last1;
			}

			// __str_find_first_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos >= __sz || __n == 0)
					return __npos;
				const _CharT* __r = /*_VSTD::*/__find_first_of_ce
				(__p + __pos, __p + __sz, __s, __s + __n, _Traits::eq);
				if (__r == __p + __sz)
					return __npos;
				return static_cast<_SizeT>(__r - __p);
			}


			// __str_find_last_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__n != 0)
				{
					if (__pos < __sz)
						++__pos;
					else
						__pos = __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __p;)
					{
						const _CharT* __r = _Traits::find(__s, __n, *--__ps);
						if (__r)
							return static_cast<_SizeT>(__ps - __p);
					}
				}
				return __npos;
			}


			// __str_find_first_not_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos < __sz)
				{
					const _CharT* __pe = __p + __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
						if (_Traits::find(__s, __n, *__ps) == 0)
							return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}


			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_first_not_of(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos < __sz)
				{
					const _CharT* __pe = __p + __sz;
					for (const _CharT* __ps = __p + __pos; __ps != __pe; ++__ps)
						if (!_Traits::eq(*__ps, __c))
							return static_cast<_SizeT>(__ps - __p);
				}
				return __npos;
			}


			// __str_find_last_not_of
			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
					const _CharT* __s, _SizeT __pos, _SizeT __n) _NOEXCEPT
			{
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
					if (_Traits::find(__s, __n, *--__ps) == 0)
						return static_cast<_SizeT>(__ps - __p);
				return __npos;
			}


			template<class _CharT, class _SizeT, class _Traits, _SizeT __npos>
			inline _SizeT _LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				__str_find_last_not_of(const _CharT *__p, _SizeT __sz,
					_CharT __c, _SizeT __pos) _NOEXCEPT
			{
				if (__pos < __sz)
					++__pos;
				else
					__pos = __sz;
				for (const _CharT* __ps = __p + __pos; __ps != __p;)
					if (!_Traits::eq(*--__ps, __c))
						return static_cast<_SizeT>(__ps - __p);
				return __npos;
			}

			template<class _Ptr>
			inline _LIBCPP_INLINE_VISIBILITY
				size_t __do_string_hash(_Ptr __p, _Ptr __e)
			{
				typedef typename std::iterator_traits<_Ptr>::value_type value_type;
				//return __murmur2_or_cityhash<size_t>()(__p, (__e - __p) * sizeof(value_type));

				/* Rather than include a hash implementation, we'll just use the existing one for std::string. */
				/* todo: check if C++17 is available, and if so, use std::basic_string_view's hash instead. */
				typedef decltype(sizeof(value_type)) l_size_type;
				l_size_type num_bytes = (__e - __p) * sizeof(value_type);
				std::string string1;
				string1.resize(num_bytes);
				char * bytes__p = reinterpret_cast<char *>(__p);
				for (l_size_type i = 0; i < num_bytes; i += 1) {
					string1[i] = *bytes__p;
					++bytes__p;
				}
				std::hash<std::string> str_hash;
				return str_hash(string1);
			}

			template<class _CharT, class _Traits = std::char_traits<_CharT> >
			class _LIBCPP_TEMPLATE_VIS basic_string_view {
			public:
				// types
				typedef _Traits                                    traits_type;
				typedef _CharT                                     value_type;
				typedef const _CharT*                              pointer;
				typedef const _CharT*                              const_pointer;
				typedef const _CharT&                              reference;
				typedef const _CharT&                              const_reference;
				typedef const_pointer                              const_iterator; // See [string.view.iterators]
				typedef const_iterator                             iterator;
				typedef _VSTD::reverse_iterator<const_iterator>    const_reverse_iterator;
				typedef const_reverse_iterator                     reverse_iterator;
				typedef size_t                                     size_type;
				typedef ptrdiff_t                                  difference_type;
				//static _LIBCPP_CONSTEXPR const size_type npos = -1; // size_type(-1);
				static _LIBCPP_CONSTEXPR const size_type npos = size_type(-1);

				static_assert(std::is_pod<value_type>::value, "Character type of basic_string_view must be a POD");
				static_assert((std::is_same<_CharT, typename traits_type::char_type>::value),
					"traits_type::char_type must be the same type as CharT");

				// [string.view.cons], construct/copy
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view() _NOEXCEPT : __data(nullptr), __size(0) {}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const basic_string_view&) _NOEXCEPT = default;

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					basic_string_view& operator=(const basic_string_view&) _NOEXCEPT = default;

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const _CharT* __s, size_type __len) _NOEXCEPT
					: __data(__s), __size(__len)
				{
					// #if _LIBCPP_STD_VER > 11
					//         _LIBCPP_ASSERT(__len == 0 || __s != nullptr, "string_view::string_view(_CharT *, size_t): received nullptr");
					// #endif
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view(const _CharT* __s)
					: __data(__s), __size(_Traits::length(__s)) {}

				// [string.view.iterators], iterators
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator begin()  const _NOEXCEPT { return cbegin(); }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator end()    const _NOEXCEPT { return cend(); }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator cbegin() const _NOEXCEPT { return __data; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_iterator cend()   const _NOEXCEPT { return __data + __size; }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator rbegin()   const _NOEXCEPT { return const_reverse_iterator(cend()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator rend()     const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator crbegin()  const _NOEXCEPT { return const_reverse_iterator(cend()); }

				_LIBCPP_CONSTEXPR_AFTER_CXX14 _LIBCPP_INLINE_VISIBILITY
					const_reverse_iterator crend()    const _NOEXCEPT { return const_reverse_iterator(cbegin()); }

				// [string.view.capacity], capacity
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type size()     const _NOEXCEPT { return __size; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type length()   const _NOEXCEPT { return __size; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					size_type max_size() const _NOEXCEPT { return std::numeric_limits<size_type>::max(); }

				_LIBCPP_NODISCARD_AFTER_CXX17 _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
					bool empty()         const _NOEXCEPT { return __size == 0; }

				// [string.view.access], element access
				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference operator[](size_type __pos) const _NOEXCEPT { return __data[__pos]; }

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference at(size_type __pos) const
				{
					return __pos >= size()
						? (__throw_out_of_range("string_view::at"), __data[0])
						: __data[__pos];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference front() const
				{
					return _LIBCPP_ASSERT(!empty(), "string_view::front(): string is empty"), __data[0];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_reference back() const
				{
					return _LIBCPP_ASSERT(!empty(), "string_view::back(): string is empty"), __data[__size - 1];
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					const_pointer data() const _NOEXCEPT { return __data; }

				// [string.view.modifiers], modifiers:
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void remove_prefix(size_type __n) _NOEXCEPT
				{
					_LIBCPP_ASSERT(__n <= size(), "remove_prefix() can't remove more than size()");
					__data += __n;
					__size -= __n;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void remove_suffix(size_type __n) _NOEXCEPT
				{
					_LIBCPP_ASSERT(__n <= size(), "remove_suffix() can't remove more than size()");
					__size -= __n;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					void swap(basic_string_view& __other) _NOEXCEPT
				{
					const value_type *__p = __data;
					__data = __other.__data;
					__other.__data = __p;

					size_type __sz = __size;
					__size = __other.__size;
					__other.__size = __sz;
				}

				_LIBCPP_INLINE_VISIBILITY
					size_type copy(_CharT* __s, size_type __n, size_type __pos = 0) const
				{
					if (__pos > size())
						__throw_out_of_range("string_view::copy");
					size_type __rlen = _VSTD::min(__n, size() - __pos);
					_Traits::copy(__s, data() + __pos, __rlen);
					return __rlen;
				}

				_LIBCPP_CONSTEXPR _LIBCPP_INLINE_VISIBILITY
					basic_string_view substr(size_type __pos = 0, size_type __n = npos) const
				{
					return __pos > size()
						? (__throw_out_of_range("string_view::substr"), basic_string_view())
						: basic_string_view(data() + __pos, _VSTD::min(__n, size() - __pos));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 int compare(basic_string_view __sv) const _NOEXCEPT
				{
					size_type __rlen = _VSTD::min(size(), __sv.size());
					int __retval = _Traits::compare(data(), __sv.data(), __rlen);
					if (__retval == 0) // first __rlen chars matched
						__retval = size() == __sv.size() ? 0 : (size() < __sv.size() ? -1 : 1);
					return __retval;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, basic_string_view __sv) const
				{
					return substr(__pos1, __n1).compare(__sv);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1,
						basic_string_view __sv, size_type __pos2, size_type __n2) const
				{
					return substr(__pos1, __n1).compare(__sv.substr(__pos2, __n2));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(const _CharT* __s) const _NOEXCEPT
				{
					return compare(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, const _CharT* __s) const
				{
					return substr(__pos1, __n1).compare(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					int compare(size_type __pos1, size_type __n1, const _CharT* __s, size_type __n2) const
				{
					return substr(__pos1, __n1).compare(basic_string_view(__s, __n2));
				}

				// find
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find(): received nullptr");
					return __str_find<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// rfind
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::rfind(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type rfind(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::rfind(): received nullptr");
					return __str_rfind<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_first_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return find(__c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_of(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_first_of(): received nullptr");
					return __str_find_first_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_last_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return rfind(__c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_of(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_last_of(): received nullptr");
					return __str_find_last_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_first_not_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(basic_string_view __s, size_type __pos = 0) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(_CharT __c, size_type __pos = 0) const _NOEXCEPT
				{
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_first_not_of(const _CharT* __s, size_type __pos = 0) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_first_not_of(): received nullptr");
					return __str_find_first_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

				// find_last_not_of
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(basic_string_view __s, size_type __pos = npos) const _NOEXCEPT
				{
					_LIBCPP_ASSERT(__s.size() == 0 || __s.data() != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s.data(), __pos, __s.size());
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(_CharT __c, size_type __pos = npos) const _NOEXCEPT
				{
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __c, __pos);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(const _CharT* __s, size_type __pos, size_type __n) const
				{
					_LIBCPP_ASSERT(__n == 0 || __s != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, __n);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					size_type find_last_not_of(const _CharT* __s, size_type __pos = npos) const
				{
					_LIBCPP_ASSERT(__s != nullptr, "string_view::find_last_not_of(): received nullptr");
					return __str_find_last_not_of<value_type, size_type, traits_type, npos>
						(data(), size(), __s, __pos, traits_type::length(__s));
				}

#if 1//_LIBCPP_STD_VER > 17
				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(basic_string_view __s) const _NOEXCEPT
				{
					return size() >= __s.size() && compare(0, __s.size(), __s) == 0;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(value_type __c) const _NOEXCEPT
				{
					return !empty() && _Traits::eq(front(), __c);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool starts_with(const value_type* __s) const _NOEXCEPT
				{
					return starts_with(basic_string_view(__s));
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(basic_string_view __s) const _NOEXCEPT
				{
					return size() >= __s.size() && compare(size() - __s.size(), npos, __s) == 0;
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(value_type __c) const _NOEXCEPT
				{
					return !empty() && _Traits::eq(back(), __c);
				}

				_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
					bool ends_with(const value_type* __s) const _NOEXCEPT
				{
					return ends_with(basic_string_view(__s));
				}
#endif

			private:
				const   value_type* __data;
				size_type           __size;
			};


			// [string.view.comparison]
			// operator ==
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(basic_string_view<_CharT, _Traits> __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator==(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size()) return false;
				return __lhs.compare(__rhs) == 0;
			}


			// operator !=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator!=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				if (__lhs.size() != __rhs.size())
					return true;
				return __lhs.compare(__rhs) != 0;
			}


			// operator <
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) < 0;
			}


			// operator >
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator> (basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) > 0;
			}


			// operator <=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator<=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) <= 0;
			}


			// operator >=
			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(basic_string_view<_CharT, _Traits> __lhs, basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}


			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(basic_string_view<_CharT, _Traits> __lhs,
					typename std::common_type<basic_string_view<_CharT, _Traits> >::type __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}

			template<class _CharT, class _Traits>
			_LIBCPP_CONSTEXPR_AFTER_CXX11 _LIBCPP_INLINE_VISIBILITY
				bool operator>=(typename std::common_type<basic_string_view<_CharT, _Traits> >::type __lhs,
					basic_string_view<_CharT, _Traits> __rhs) _NOEXCEPT
			{
				return __lhs.compare(__rhs) >= 0;
			}

			typedef basic_string_view<char>     string_view;
			typedef basic_string_view<char16_t> u16string_view;
			typedef basic_string_view<char32_t> u32string_view;
			typedef basic_string_view<wchar_t>  wstring_view;

#if 0//_LIBCPP_STD_VER > 11 
			inline namespace literals
			{
				inline namespace string_view_literals
				{
					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char> operator "" sv(const char *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<wchar_t> operator "" sv(const wchar_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<wchar_t>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char16_t> operator "" sv(const char16_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char16_t>(__str, __len);
					}

					inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
						basic_string_view<char32_t> operator "" sv(const char32_t *__str, size_t __len) _NOEXCEPT
					{
						return basic_string_view<char32_t>(__str, __len);
					}
				}
			}
#endif
		}
	}
}

namespace std {
	// [string.view.hash]
	template<class _CharT, class _Traits>
	struct _LIBCPP_TEMPLATE_VIS hash<mse::us::impl::basic_string_view<_CharT, _Traits> >
		: public unary_function<mse::us::impl::basic_string_view<_CharT, _Traits>, size_t>
	{
		size_t operator()(const mse::us::impl::basic_string_view<_CharT, _Traits> __val) const _NOEXCEPT;
	};

	template<class _CharT, class _Traits>
	size_t
		hash<mse::us::impl::basic_string_view<_CharT, _Traits> >::operator()(
			const mse::us::impl::basic_string_view<_CharT, _Traits> __val) const _NOEXCEPT
	{
		return mse::us::impl::__do_string_hash(__val.data(), __val.data() + __val.size());
	}
}

namespace mse {
	template <typename _TRandomAccessSection>
	class TStringSectionFunctionaltyWrapper : public _TRandomAccessSection {
	public:
	};
}

#endif /*ndef MSEMSESTRING_H*/
