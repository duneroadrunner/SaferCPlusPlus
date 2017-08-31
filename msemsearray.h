
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSEARRAY_H
#define MSEMSEARRAY_H

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

//define MSE_MSEARRAY_USE_MSE_PRIMITIVES 1
#ifdef MSE_MSEARRAY_USE_MSE_PRIMITIVES
#include "mseprimitives.h"
#endif // MSE_MSEARRAY_USE_MSE_PRIMITIVES

#include "msescope.h"
#include <array>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <climits>       // ULONG_MAX
#include <stdexcept>
#ifdef MSE_SELF_TESTS
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#endif // MSE_SELF_TESTS

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

#ifdef MSE_MSEARRAY_USE_MSE_PRIMITIVES
	typedef mse::CSize_t msear_size_t;
	typedef mse::CInt msear_int;
	typedef bool msear_bool; // no added safety benefit to using mse::CBool in this case
	#define msear_as_a_size_t as_a_size_t
#else // MSE_MSEARRAY_USE_MSE_PRIMITIVES
#if SIZE_MAX <= ULONG_MAX
#define MSE_MSEARRAY_BASE_INTEGER_TYPE long int
#else // SIZE_MAX <= ULONG_MAX
#define MSE_MSEARRAY_BASE_INTEGER_TYPE long long int
#endif // SIZE_MAX <= ULONG_MAX

	typedef size_t msear_size_t;
	typedef MSE_MSEARRAY_BASE_INTEGER_TYPE msear_int;
	typedef bool msear_bool;
	typedef size_t msear_as_a_size_t;
#endif // MSE_MSEARRAY_USE_MSE_PRIMITIVES

	class msearray_range_error : public std::range_error { public:
		using std::range_error::range_error;
	};
	class msearray_null_dereference_error : public std::logic_error { public:
		using std::logic_error::logic_error;
	};

	/* msear_pointer behaves similar to native pointers. It's a bit safer in that it initializes to
	nullptr by default and checks for attempted dereference of null pointers. */
	template<typename _Ty>
	class msear_pointer {
	public:
		msear_pointer() : m_ptr(nullptr) {}
		msear_pointer(_Ty* ptr) : m_ptr(ptr) {}
		msear_pointer(const msear_pointer<_Ty>& src) : m_ptr(src.m_ptr) {}

		_Ty& operator*() const {
#ifndef MSE_DISABLE_MSEAR_POINTER_CHECKS
			if (nullptr == m_ptr) { MSE_THROW(msearray_null_dereference_error("attempt to dereference null pointer - mse::msear_pointer")); }
#endif /*MSE_DISABLE_MSEAR_POINTER_CHECKS*/
			return (*m_ptr);
		}
		_Ty* operator->() const {
#ifndef MSE_DISABLE_MSEAR_POINTER_CHECKS
			if (nullptr == m_ptr) { MSE_THROW(msearray_null_dereference_error("attempt to dereference null pointer - mse::msear_pointer")); }
#endif /*MSE_DISABLE_MSEAR_POINTER_CHECKS*/
			return m_ptr;
		}
		msear_pointer<_Ty>& operator=(_Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		bool operator==(const msear_pointer _Right_cref) const { return (_Right_cref.m_ptr == m_ptr); }
		bool operator!=(const msear_pointer _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { return (!((*this) == _Right_cref)); }

		bool operator!() const { return (!m_ptr); }
		operator bool() const { return (m_ptr != nullptr); }

		operator _Ty*() const { return m_ptr; }

		_Ty* m_ptr;
	};

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

#ifndef _STD
#define _STD std::
#endif /*_STD*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifndef _CONST_FUN
#define _CONST_FUN constexpr
#endif /*_CONST_FUN*/

	template<class _Ty, size_t _Size>
	class msearray {
	public:
		typedef std::array<_Ty, _Size> base_class;
		typedef msearray<_Ty, _Size> _Myt;

		std::array<_Ty, _Size> m_array;
		/* Note that msearray is now an "aggregate type" like std::array (basically a POD struct/class with
		no base class, constructors or private data members (details here:
		http://en.cppreference.com/w/cpp/language/aggregate_initialization)). As an aggregate type, msearray
		gets automatic support for "aggregate initialization". In some sense this increases msearray's ability
		to substitute for std::array as it doesn't seem possible to fully emulate aggregate initialization in
		non-aggregate types. But it comes at the cost of not being able to explicitly define any constructors
		or assignment operators. This means, for example, that msearray cannot be copy constructed from an
		std::array. So the choice to make msearray an aggregate type enables it to better substitute for
		std::array, but less able to interoperate with it. */

		_CONST_FUN typename base_class::const_reference operator[](msear_size_t _P) const {
			return (*this).at(msear_as_a_size_t(_P));
		}
		typename base_class::reference operator[](msear_size_t _P) {
			return (*this).at(msear_as_a_size_t(_P));
		}
		typename base_class::reference front() {	// return first element of mutable sequence
			//if (0 == (*this).size()) { MSE_THROW(msearray_range_error("front() on empty - typename base_class::reference front() - msearray")); }
			return m_array.front();
		}
		_CONST_FUN typename base_class::const_reference front() const {	// return first element of nonmutable sequence
			//if (0 == (*this).size()) { MSE_THROW(msearray_range_error("front() on empty - typename base_class::const_reference front() - msearray")); }
			return m_array.front();
		}
		typename base_class::reference back() {	// return last element of mutable sequence
			//if (0 == (*this).size()) { MSE_THROW(msearray_range_error("back() on empty - typename base_class::reference back() - msearray")); }
			return m_array.back();
		}
		_CONST_FUN typename base_class::const_reference back() const {	// return last element of nonmutable sequence
			//if (0 == (*this).size()) { MSE_THROW(msearray_range_error("back() on empty - typename base_class::const_reference back() - msearray")); }
			return m_array.back();
		}

		typedef typename base_class::value_type value_type;
		//typedef typename base_class::size_type size_type;
		typedef msear_size_t size_type;
		//typedef typename base_class::difference_type difference_type;
		typedef msear_int difference_type;
		typedef typename base_class::pointer pointer;
		typedef typename base_class::const_pointer const_pointer;
		typedef typename base_class::reference reference;
		typedef typename base_class::const_reference const_reference;

		typedef typename base_class::iterator iterator;
		typedef typename base_class::const_iterator const_iterator;

		typedef typename base_class::reverse_iterator reverse_iterator;
		typedef typename base_class::const_reverse_iterator const_reverse_iterator;

		void assign(const _Ty& _Value)
		{	// assign value to all elements
			m_array.assign(_Value);
		}

		void fill(const _Ty& _Value)
		{	// assign value to all elements
			m_array.fill(_Value);
		}

		void swap(_Myt& _Other) /*_NOEXCEPT_OP(_NOEXCEPT_OP(_Swap_adl(this->m_array[0], _Other.m_array[0])))*/
		{	// swap contents with _Other
			m_array.swap(_Other.m_array);
		}

		iterator begin() _NOEXCEPT
		{	// return iterator for beginning of mutable sequence
			return m_array.begin();
		}

		const_iterator begin() const _NOEXCEPT
		{	// return iterator for beginning of nonmutable sequence
			return m_array.begin();
		}

		iterator end() _NOEXCEPT
		{	// return iterator for end of mutable sequence
			return m_array.end();
		}

		const_iterator end() const _NOEXCEPT
		{	// return iterator for beginning of nonmutable sequence
			return m_array.end();
		}

		reverse_iterator rbegin() _NOEXCEPT
		{	// return iterator for beginning of reversed mutable sequence
			return m_array.rbegin();
		}

		const_reverse_iterator rbegin() const _NOEXCEPT
		{	// return iterator for beginning of reversed nonmutable sequence
			return m_array.rbegin();
		}

		reverse_iterator rend() _NOEXCEPT
		{	// return iterator for end of reversed mutable sequence
			return m_array.rend();
		}

		const_reverse_iterator rend() const _NOEXCEPT
		{	// return iterator for end of reversed nonmutable sequence
			return m_array.rend();
		}

		const_iterator cbegin() const _NOEXCEPT
		{	// return iterator for beginning of nonmutable sequence
			return m_array.cbegin();
		}

		const_iterator cend() const _NOEXCEPT
		{	// return iterator for end of nonmutable sequence
			return m_array.cend();
		}

		const_reverse_iterator crbegin() const _NOEXCEPT
		{	// return iterator for beginning of reversed nonmutable sequence
			return m_array.crbegin();
		}

		const_reverse_iterator crend() const _NOEXCEPT
		{	// return iterator for end of reversed nonmutable sequence
			return m_array.crend();
		}

		_CONST_FUN size_type size() const _NOEXCEPT
		{	// return length of sequence
			return m_array.size();
		}

		_CONST_FUN size_type max_size() const _NOEXCEPT
		{	// return maximum possible length of sequence
			return m_array.max_size();
		}

		_CONST_FUN bool empty() const _NOEXCEPT
		{	// test if sequence is empty
			return m_array.empty();
		}

		reference at(msear_size_t _Pos)
		{	// subscript mutable sequence with checking
			return m_array.at(msear_as_a_size_t(_Pos));
		}

		_CONST_FUN const_reference at(msear_size_t _Pos) const
		{	// subscript nonmutable sequence with checking
			return m_array.at(msear_as_a_size_t(_Pos));
		}

		value_type *data() _NOEXCEPT
		{	// return pointer to mutable data array
			return m_array.data();
		}

		const value_type *data() const _NOEXCEPT
		{	// return pointer to nonmutable data array
			return m_array.data();
		}

		class random_access_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		class random_access_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

		class xscope_ss_const_iterator_type;
		class xscope_ss_iterator_type;

		/* ss_const_iterator_type is a bounds checked const_iterator. */
		class ss_const_iterator_type : public random_access_const_iterator_base {
		public:
			typedef typename std::iterator_traits<typename base_class::const_iterator>::iterator_category iterator_category;
			typedef typename std::iterator_traits<typename base_class::const_iterator>::value_type value_type;
			//typedef typename std::iterator_traits<typename base_class::const_iterator>::difference_type difference_type;
			typedef msear_int difference_type;
			typedef typename std::iterator_traits<typename base_class::const_iterator>::pointer const_pointer;
			typedef typename std::iterator_traits<typename base_class::const_iterator>::reference const_reference;

			typedef typename std::iterator_traits<typename base_class::const_iterator>::pointer pointer;
			typedef typename std::iterator_traits<typename base_class::const_iterator>::reference reference;

			ss_const_iterator_type() {}
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
					MSE_THROW(msearray_range_error("attempt to use invalid const_item_pointer - void set_to_next() - ss_const_iterator_type - msearray"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(msearray_range_error("attempt to use invalid const_item_pointer - void set_to_previous() - ss_const_iterator_type - msearray"));
				}
			}
			ss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_const_iterator_type operator++(int) { ss_const_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			ss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_const_iterator_type operator--(int) { ss_const_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msear_int(m_index) + n;
				if ((0 > new_index) || (m_owner_cptr->size() < msear_size_t(new_index))) {
					MSE_THROW(msearray_range_error("index out of range - void advance(difference_type n) - ss_const_iterator_type - msearray"));
				}
				else {
					m_index = msear_size_t(new_index);
				}
			}
			void regress(difference_type n) { advance(-n); }
			ss_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			ss_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			ss_const_iterator_type operator+(difference_type n) const {
				ss_const_iterator_type retval; retval.m_owner_cptr = m_owner_cptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const ss_const_iterator_type &rhs) const {
				if (rhs.m_owner_cptr != (*this).m_owner_cptr) { MSE_THROW(msearray_range_error("invalid argument - difference_type operator-(const ss_const_iterator_type &rhs) const - msearray::ss_const_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert(difference_type((*m_owner_cptr).size()) >= retval);
				return retval;
			}
			const_reference operator*() const {
				return (*m_owner_cptr).at(msear_as_a_size_t((*this).m_index));
			}
			const_reference item() const { return operator*(); }
			const_reference previous_item() const {
				return (*m_owner_cptr).at(msear_as_a_size_t((*this).m_index - 1));
			}
			const_pointer operator->() const {
				return &((*m_owner_cptr).at(msear_as_a_size_t((*this).m_index)));
			}
			const_reference operator[](difference_type _Off) const { return (*m_owner_cptr).at(msear_as_a_size_t(difference_type(m_index) + _Off)); }
			/*
			ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref)
			{
			msear_int d = std::distance<typename base_class::iterator>(m_owner_cptr->cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cptr->size() >= d)) {
			if (m_owner_cptr->size() == d) {
			assert(m_owner_cptr->cend() == _Right_cref);
			}
			m_index = msear_size_t(d);
			base_class::const_iterator::operator=(_Right_cref);
			}
			else {
			MSE_THROW(msearray_range_error("doesn't seem to be a valid assignment value - ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref) - ss_const_iterator_type - msearray"));
			}
			return (*this);
			}
			*/
			ss_const_iterator_type& operator=(const ss_const_iterator_type& _Right_cref) {
				((*this).m_owner_cptr) = _Right_cref.m_owner_cptr;
				(*this).m_index = _Right_cref.m_index;
				return (*this);
			}
			bool operator==(const ss_const_iterator_type& _Right_cref) const {
				if (this->m_owner_cptr != _Right_cref.m_owner_cptr) { MSE_THROW(msearray_range_error("invalid argument - ss_const_iterator_type& operator==(const ss_const_iterator_type& _Right) - ss_const_iterator_type - msearray")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const ss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_const_iterator_type& _Right) const {
				if (this->m_owner_cptr != _Right.m_owner_cptr) { MSE_THROW(msearray_range_error("invalid argument - ss_const_iterator_type& operator<(const ss_const_iterator_type& _Right) - ss_const_iterator_type - msearray")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const ss_const_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			msear_size_t position() const {
				return m_index;
			}
		private:
			msear_size_t m_index = 0;
			msear_pointer<const _Myt> m_owner_cptr = nullptr;
			friend class /*_Myt*/msearray<_Ty, _Size>;
			friend class xscope_ss_const_iterator_type;
		};
		/* ss_iterator_type is a bounds checked iterator. */
		class ss_iterator_type : public random_access_iterator_base {
		public:
			typedef typename std::iterator_traits<typename base_class::iterator>::iterator_category iterator_category;
			typedef typename std::iterator_traits<typename base_class::iterator>::value_type value_type;
			//typedef typename std::iterator_traits<typename base_class::iterator>::difference_type difference_type;
			typedef msear_int difference_type;
			typedef typename std::iterator_traits<typename base_class::iterator>::pointer pointer;
			typedef typename std::iterator_traits<typename base_class::iterator>::reference reference;
			typedef difference_type distance_type;	// retained

			ss_iterator_type() {}
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
					MSE_THROW(msearray_range_error("attempt to use invalid item_pointer - void set_to_next() - ss_const_iterator_type - msearray"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(msearray_range_error("attempt to use invalid item_pointer - void set_to_previous() - ss_iterator_type - msearray"));
				}
			}
			ss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_iterator_type operator++(int) { ss_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			ss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_iterator_type operator--(int) { ss_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msear_int(m_index) + n;
				if ((0 > new_index) || (m_owner_ptr->size() < msear_size_t(new_index))) {
					MSE_THROW(msearray_range_error("index out of range - void advance(difference_type n) - ss_iterator_type - msearray"));
				}
				else {
					m_index = msear_size_t(new_index);
				}
			}
			void regress(difference_type n) { advance(-n); }
			ss_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			ss_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			ss_iterator_type operator+(difference_type n) const {
				ss_iterator_type retval; retval.m_owner_ptr = m_owner_ptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const ss_iterator_type& rhs) const {
				if (rhs.m_owner_ptr != (*this).m_owner_ptr) { MSE_THROW(msearray_range_error("invalid argument - difference_type operator-(const ss_iterator_type& rhs) const - msearray::ss_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert(difference_type((*m_owner_ptr).size()) >= retval);
				return retval;
			}
			reference operator*() const {
				return (*m_owner_ptr).at(msear_as_a_size_t((*this).m_index));
			}
			reference item() const { return operator*(); }
			reference previous_item() const {
				if ((*this).has_previous()) {
					return (*m_owner_ptr)[m_index - 1];
				}
				else {
					MSE_THROW(msearray_range_error("attempt to use invalid item_pointer - reference previous_item() - ss_const_iterator_type - msearray"));
				}
			}
			pointer operator->() const {
				return &((*m_owner_ptr).at(msear_as_a_size_t((*this).m_index)));
			}
			reference operator[](difference_type _Off) const { return (*m_owner_ptr).at(msear_as_a_size_t(difference_type(m_index) + _Off)); }
			/*
			ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref)
			{
			msear_int d = std::distance<typename base_class::iterator>(m_owner_ptr->begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ptr->size() >= d)) {
			if (m_owner_ptr->size() == d) {
			assert(m_owner_ptr->end() == _Right_cref);
			}
			m_index = msear_size_t(d);
			(*this).m_base_iterator.operator=(_Right_cref);
			}
			else {
			MSE_THROW(msearray_range_error("doesn't seem to be a valid assignment value - ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - ss_const_iterator_type - msearray"));
			}
			return (*this);
			}
			*/
			ss_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				((*this).m_owner_ptr) = _Right_cref.m_owner_ptr;
				(*this).m_index = _Right_cref.m_index;
				return (*this);
			}
			bool operator==(const ss_iterator_type& _Right_cref) const {
				if (this->m_owner_ptr != _Right_cref.m_owner_ptr) { MSE_THROW(msearray_range_error("invalid argument - ss_iterator_type& operator==(const ss_iterator_type& _Right) - ss_iterator_type - msearray")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const ss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_iterator_type& _Right) const {
				if (this->m_owner_ptr != _Right.m_owner_ptr) { MSE_THROW(msearray_range_error("invalid argument - ss_iterator_type& operator<(const ss_iterator_type& _Right) - ss_iterator_type - msearray")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const ss_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			msear_size_t position() const {
				return m_index;
			}
			operator ss_const_iterator_type() const {
				ss_const_iterator_type retval;
				if (nullptr != m_owner_ptr) {
					retval = m_owner_ptr->ss_cbegin();
					retval.advance(msear_int(m_index));
				}
				return retval;
			}
		private:
			msear_size_t m_index = 0;
			msear_pointer<_Myt> m_owner_ptr = nullptr;
			friend class /*_Myt*/msearray<_Ty, _Size>;
		};
		typedef std::reverse_iterator<ss_iterator_type> ss_reverse_iterator_type;
		typedef std::reverse_iterator<ss_const_iterator_type> ss_const_reverse_iterator_type;

		ss_iterator_type ss_begin()
		{	// return base_class::iterator for beginning of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_begin() const
		{	// return base_class::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_iterator_type ss_end()
		{	// return base_class::iterator for end of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_end() const
		{	// return base_class::iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_cbegin() const
		{	// return base_class::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_cend() const
		{	// return base_class::iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_reverse_iterator_type ss_crbegin() const
		{	// return base_class::iterator for beginning of reversed nonmutable sequence
			return (ss_rbegin());
		}

		ss_const_reverse_iterator_type ss_crend() const
		{	// return base_class::iterator for end of reversed nonmutable sequence
			return (ss_rend());
		}

		ss_reverse_iterator_type ss_rbegin()
		{	// return base_class::iterator for beginning of reversed mutable sequence
			return (reverse_iterator(ss_end()));
		}

		ss_const_reverse_iterator_type ss_rbegin() const
		{	// return base_class::iterator for beginning of reversed nonmutable sequence
			return (const_reverse_iterator(ss_end()));
		}

		ss_reverse_iterator_type ss_rend()
		{	// return base_class::iterator for end of reversed mutable sequence
			return (reverse_iterator(ss_begin()));
		}

		ss_const_reverse_iterator_type ss_rend() const
		{	// return base_class::iterator for end of reversed nonmutable sequence
			return (const_reverse_iterator(ss_begin()));
		}

		class xscope_ss_const_iterator_type : public ss_const_iterator_type, public XScopeTagBase {
		public:
			xscope_ss_const_iterator_type(const mse::TXScopeFixedConstPointer<msearray>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}
			xscope_ss_const_iterator_type(const mse::TXScopeFixedPointer<msearray>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}
			template <class _TLeasePointerType>
			xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedConstPointer<msearray, _TLeasePointerType>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}
			template <class _TLeasePointerType>
			xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedPointer<msearray, _TLeasePointerType>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}
			template <class _TLeasePointerType>
			xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedConstPointer<const msearray, _TLeasePointerType>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}
			template <class _TLeasePointerType>
			xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedPointer<const msearray, _TLeasePointerType>& owner_ptr) : ss_const_iterator_type((*owner_ptr).ss_cbegin()) {}

			xscope_ss_const_iterator_type(const xscope_ss_const_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			xscope_ss_const_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_const_iterator_type(src_cref) {}
			~xscope_ss_const_iterator_type() {}
			const ss_const_iterator_type& msearray_ss_const_iterator_type() const {
				return (*this);
			}
			ss_const_iterator_type& msearray_ss_const_iterator_type() {
				return (*this);
			}
			const ss_const_iterator_type& mvssci() const { return msearray_ss_const_iterator_type(); }
			ss_const_iterator_type& mvssci() { return msearray_ss_const_iterator_type(); }

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
				if (_Right_cref.m_owner_cptr != (*this).m_owner_cptr) { MSE_THROW(msearray_range_error("invalid argument - xscope_ss_const_iterator_type& operator=(const xscope_ss_const_iterator_type& _Right_cref) - msearray::xscope_ss_const_iterator_type")); }
				ss_const_iterator_type::operator=(_Right_cref);
				return (*this);
			}
			xscope_ss_const_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
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
			void xscope_ss_iterator_type_tag() const {}
		private:
			void* operator new(size_t size) { return ::operator new(size); }

			//typename ss_const_iterator_type (*this);
			friend class /*_Myt*/msearray<_Ty, _Size>;
			friend class xscope_ss_iterator_type;
		};
		class xscope_ss_iterator_type : public ss_iterator_type, public XScopeTagBase {
		public:
			xscope_ss_iterator_type(const mse::TXScopeFixedPointer<msearray>& owner_ptr) : ss_iterator_type((*owner_ptr).ss_begin()) {}
			template <class _TLeasePointerType>
			xscope_ss_iterator_type(const mse::TXScopeWeakFixedPointer<msearray, _TLeasePointerType>& owner_ptr) : ss_iterator_type((*owner_ptr).ss_begin()) {}

			xscope_ss_iterator_type(const xscope_ss_iterator_type& src_cref) : ss_iterator_type(src_cref) {}
			~xscope_ss_iterator_type() {}
			const ss_iterator_type& msearray_ss_iterator_type() const {
				return (*this);
			}
			ss_iterator_type& msearray_ss_iterator_type() {
				return (*this);
			}
			const ss_iterator_type& mvssi() const { return msearray_ss_iterator_type(); }
			ss_iterator_type& mvssi() { return msearray_ss_iterator_type(); }

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
				if (_Right_cref.m_owner_ptr != (*this).m_owner_ptr) { MSE_THROW(msearray_range_error("invalid argument - xscope_ss_iterator_type& operator=(const xscope_ss_iterator_type& _Right_cref) - msearray::xscope_ss_iterator_type")); }
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
			void xscope_ss_iterator_type_tag() const {}
		private:
			void* operator new(size_t size) { return ::operator new(size); }

			//typename ss_iterator_type (*this);
			friend class /*_Myt*/msearray<_Ty, _Size>;
		};


		bool operator==(const _Myt& _Right) const {	// test for array equality
			return (_Right.m_array == m_array);
		}
		bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for arrays
			return (m_array < _Right.m_array);
		}


		template<size_t _Idx, class _Tz, size_t _Size2>
		friend _CONST_FUN _Tz& std::get(mse::msearray<_Tz, _Size2>& _Arr) _NOEXCEPT;
		template<size_t _Idx, class _Tz, size_t _Size2>
		friend _CONST_FUN const _Tz& std::get(const mse::msearray<_Tz, _Size2>& _Arr) _NOEXCEPT;
		template<size_t _Idx, class _Tz, size_t _Size2>
		friend _CONST_FUN _Tz&& std::get(mse::msearray<_Tz, _Size2>&& _Arr) _NOEXCEPT;
	};

	template<class _Ty, size_t _Size> inline bool operator!=(const msearray<_Ty, _Size>& _Left,
		const msearray<_Ty, _Size>& _Right) {	// test for array inequality
		return (!(_Left == _Right));
	}

	template<class _Ty, size_t _Size> inline bool operator>(const msearray<_Ty, _Size>& _Left,
		const msearray<_Ty, _Size>& _Right) {	// test if _Left > _Right for arrays
		return (_Right < _Left);
	}

	template<class _Ty, size_t _Size> inline bool operator<=(const msearray<_Ty, _Size>& _Left,
		const msearray<_Ty, _Size>& _Right) {	// test if _Left <= _Right for arrays
		return (!(_Right < _Left));
	}

	template<class _Ty, size_t _Size> inline bool operator>=(const msearray<_Ty, _Size>& _Left,
		const msearray<_Ty, _Size>& _Right) {	// test if _Left >= _Right for arrays
		return (!(_Left < _Right));
	}


	template<class _TArray> using xscope_ss_const_iterator_type = typename _TArray::xscope_ss_const_iterator_type;
	template<class _TArray>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeFixedConstPointer<_TArray>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray, class _TLeasePointerType>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedConstPointer<_TArray, _TLeasePointerType>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray, class _TLeasePointerType>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray, class _TLeasePointerType>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedConstPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray, class _TLeasePointerType>
	xscope_ss_const_iterator_type<_TArray> make_xscope_ss_const_iterator_type(const mse::TXScopeWeakFixedPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
		return xscope_ss_const_iterator_type<_TArray>(owner_ptr);
	}

	template<class _TArray> using xscope_ss_iterator_type = typename _TArray::xscope_ss_iterator_type;
	template<class _TArray>
	xscope_ss_iterator_type<_TArray> make_xscope_ss_iterator_type(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
		return xscope_ss_iterator_type<_TArray>(owner_ptr);
	}
	template<class _TArray, class _TLeasePointerType>
	xscope_ss_iterator_type<_TArray> make_xscope_ss_iterator_type(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
		return xscope_ss_iterator_type<_TArray>(owner_ptr);
	}
}

namespace std {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmismatched-tags"
#endif /*__clang__*/

	template<class _Ty, size_t _Size>
	struct tuple_size<mse::msearray<_Ty, _Size> >
		: integral_constant<size_t, _Size>
	{	// struct to determine number of elements in array
	};

	template<size_t _Idx, class _Ty, size_t _Size>
	struct tuple_element<_Idx, mse::msearray<_Ty, _Size> >
	{	// struct to determine type of element _Idx in array
		static_assert(_Idx < _Size, "array index out of bounds");

		typedef _Ty type;
	};

#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

	// TUPLE INTERFACE TO array
	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty& get(mse::msearray<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_array));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN const _Ty& get(const mse::msearray<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_array));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty&& get(mse::msearray<_Ty, _Size>&& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (_STD move(std::get<_Idx>(_Arr.m_array)));
	}

	template<class _Ty, size_t _Size>
	void swap(mse::msearray<_Ty, _Size>& _Left, mse::msearray<_Ty, _Size>& _Right) _NOEXCEPT
	{
		_Left.swap(_Right);
	}
}

namespace mse {

	template <typename _TRAIterator> class TRASectionConstIteratorBase;

	template <typename _TRAIterator>
	class TRASectionIteratorBase {
	public:
		typedef typename mse::msearray<int, 0>::difference_type difference_t;
		typedef typename mse::msearray<int, 0>::size_type size_type;

	private:
		const _TRAIterator m_ra_iterator;
		const size_type m_count = 0;
		difference_t m_index = 0;

	public:
		TRASectionIteratorBase(const TRASectionIteratorBase& src)
			: m_ra_iterator(src.m_ra_iterator), m_count(src.m_count), m_index(src.m_index) {}
		TRASectionIteratorBase(_TRAIterator ra_iterator, size_type count, size_type index = 0)
			: m_ra_iterator(ra_iterator), m_count(count), m_index(difference_t(index)) {}

		void dereference_bounds_check() const {
			if ((0 > m_index) || (difference_t(m_count) <= m_index)) {
				MSE_THROW(msearray_range_error("out of bounds index - bool dereference_bounds_check() - TRASectionIteratorBase"));
			}
		}
		auto operator*() -> decltype(*m_ra_iterator) const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return (*tmp_ra_iterator);
		}
		auto operator->() -> decltype(std::addressof(*m_ra_iterator)) const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return std::addressof(*m_ra_iterator);
		}
		TRASectionIteratorBase& operator +=(difference_t x) {
			m_index += (x);
			return (*this);
		}
		TRASectionIteratorBase& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TRASectionIteratorBase& operator ++() { operator +=(1); return (*this); }
		TRASectionIteratorBase& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TRASectionIteratorBase& operator --() { operator -=(1); return (*this); }
		TRASectionIteratorBase& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TRASectionIteratorBase operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TRASectionIteratorBase operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionIteratorBase& _Right_cref) const {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - difference_t operator-() - TRASectionIteratorBase")); }
			return m_index - _Right_cref.m_index;
		}
		bool operator ==(const TRASectionIteratorBase& _Right_cref) const {
			return ((_Right_cref.m_index == m_index) && (_Right_cref.m_count == m_count) && (_Right_cref.m_ra_iterator == m_ra_iterator));
		}
		bool operator !=(const TRASectionIteratorBase& _Right_cref) const { return !((*this) == _Right_cref); }
		bool operator<(const TRASectionIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TRASectionIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TRASectionIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TRASectionIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TRASectionIteratorBase& operator=(const TRASectionIteratorBase& _Right_cref) {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - TRASectionIteratorBase& operator=() - TRASectionIteratorBase")); }
			m_index = _Right_cref.m_index;
			return (*this);
		}
		friend class TRASectionConstIteratorBase<_TRAIterator>;
	};

	template <typename _TRAIterator>
	class TXScopeRASectionIterator : public TRASectionIteratorBase<_TRAIterator>, public XScopeTagBase {
	public:
		typedef TRASectionIteratorBase<_TRAIterator> base_class;

		typedef typename base_class::difference_t difference_t;
		typedef typename base_class::size_type size_type;

		TXScopeRASectionIterator(const TRASectionIteratorBase<_TRAIterator>& src)
			: base_class(src) {}
		TXScopeRASectionIterator(_TRAIterator ra_iterator, size_type count, size_type index = 0)
			: base_class(ra_iterator, count, index) {}

		TXScopeRASectionIterator& operator +=(difference_t x) {
			base_class::operator +=(x);
			return (*this);
		}
		TXScopeRASectionIterator& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TXScopeRASectionIterator& operator ++() { operator +=(1); return (*this); }
		TXScopeRASectionIterator& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TXScopeRASectionIterator& operator --() { operator -=(1); return (*this); }
		TXScopeRASectionIterator& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TXScopeRASectionIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeRASectionIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionIteratorBase<_TRAIterator>& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TXScopeRASectionIterator& operator=(const TRASectionIteratorBase<_TRAIterator>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

	private:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRASectionIterator* operator&() { return this; }
		const TXScopeRASectionIterator* operator&() const { return this; }

	};

	template <typename _TRAIterator>
	class TRASectionIterator : public TRASectionIteratorBase<_TRAIterator> {
	public:
		typedef TRASectionIteratorBase<_TRAIterator> base_class;

		typedef typename base_class::difference_t difference_t;
		typedef typename base_class::size_type size_type;

		TRASectionIterator(const TRASectionIterator& src)
			: base_class(src) {}

		template <typename _TRAIterator1, class = typename std::enable_if<
			//(!std::is_convertible<_TRAIterator1, TPolyPointer>::value)
			//&& (!std::is_base_of<TPolyConstPointer<_Ty>, _TRAIterator1>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TRAIterator1>::Has>())
			/*&&*/ (!std::is_base_of<XScopeTagBase, _TRAIterator1>::value)
			, void>::type>
			TRASectionIterator(_TRAIterator1 ra_iterator, size_type count, size_type index = 0)
			: base_class(ra_iterator, count, index) {}

		TRASectionIterator& operator +=(difference_t x) {
			base_class::operator +=(x);
			return (*this);
		}
		TRASectionIterator& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TRASectionIterator& operator ++() { operator +=(1); return (*this); }
		TRASectionIterator& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TRASectionIterator& operator --() { operator -=(1); return (*this); }
		TRASectionIterator& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TRASectionIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TRASectionIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionIteratorBase<_TRAIterator>& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TRASectionIterator& operator=(const TRASectionIteratorBase<_TRAIterator>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
	};

	template <typename _TRAIterator>
	class TRASectionConstIteratorBase {
	public:
		typedef typename std::remove_reference<decltype(std::declval<_TRAIterator>()[0])>::type element_t;
		typedef decltype(std::declval<_TRAIterator>()[0]) reference_t;
		typedef typename std::add_lvalue_reference<typename std::add_const<element_t>::type>::type const_reference_t;
		typedef typename mse::msearray<int, 0>::size_type size_type;
		typedef typename mse::msearray<int, 0>::difference_type difference_t;

	private:
		const _TRAIterator m_ra_iterator;
		const size_type m_count = 0;
		difference_t m_index = 0;

	public:
		TRASectionConstIteratorBase(const TRASectionConstIteratorBase& src)
			: m_ra_iterator(src.m_ra_iterator), m_count(src.m_count), m_index(src.m_index) {}
		TRASectionConstIteratorBase(const TRASectionIteratorBase<_TRAIterator>& src)
			: m_ra_iterator(src.m_ra_iterator), m_count(src.m_count), m_index(src.m_index) {}
		TRASectionConstIteratorBase(_TRAIterator ra_iterator, size_type count, size_type index = 0)
			: m_ra_iterator(ra_iterator), m_count(count), m_index(difference_t(index)) {}

		void dereference_bounds_check() const {
			if ((0 > m_index) || (difference_t(m_count) <= m_index)) {
				MSE_THROW(msearray_range_error("out of bounds index - bool dereference_bounds_check() - TRASectionConstIteratorBase"));
			}
		}
		auto operator*() -> const_reference_t const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return (*tmp_ra_iterator);
		}
		auto operator->() -> typename std::add_pointer<typename std::add_const<element_t>::type>::type const {
			dereference_bounds_check();
			auto tmp_ra_iterator(m_ra_iterator);
			tmp_ra_iterator += m_index;
			return std::addressof(*m_ra_iterator);
		}
		TRASectionConstIteratorBase& operator +=(difference_t x) {
			m_index += (x);
			return (*this);
		}
		TRASectionConstIteratorBase& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TRASectionConstIteratorBase& operator ++() { operator +=(1); return (*this); }
		TRASectionConstIteratorBase& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TRASectionConstIteratorBase& operator --() { operator -=(1); return (*this); }
		TRASectionConstIteratorBase& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TRASectionConstIteratorBase operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TRASectionConstIteratorBase operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionConstIteratorBase& _Right_cref) const {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - difference_t operator-() - TRASectionConstIteratorBase")); }
			return m_index - _Right_cref.m_index;
		}
		bool operator ==(const TRASectionConstIteratorBase& _Right_cref) const {
			return ((_Right_cref.m_index == m_index) && (_Right_cref.m_count == m_count) && (_Right_cref.m_ra_iterator == m_ra_iterator));
		}
		bool operator !=(const TRASectionConstIteratorBase& _Right_cref) const { return !((*this) == _Right_cref); }
		bool operator<(const TRASectionConstIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator>(const TRASectionConstIteratorBase& _Right_cref) const { return (0 > operator-(_Right_cref)); }
		bool operator<=(const TRASectionConstIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		bool operator>=(const TRASectionConstIteratorBase& _Right_cref) const { return (0 >= operator-(_Right_cref)); }
		TRASectionConstIteratorBase& operator=(const TRASectionConstIteratorBase& _Right_cref) {
			if (!(_Right_cref.m_ra_iterator == m_ra_iterator)) { MSE_THROW(msearray_range_error("invalid argument - TRASectionConstIteratorBase& operator=() - TRASectionConstIteratorBase")); }
			m_index = _Right_cref.m_index;
			return (*this);
		}
	};

	template <typename _TRAIterator>
	class TXScopeRASectionConstIterator : public TRASectionConstIteratorBase<_TRAIterator>, public XScopeTagBase {
	public:
		typedef TRASectionConstIteratorBase<_TRAIterator> base_class;

		typedef typename base_class::difference_t difference_t;
		typedef typename base_class::size_type size_type;

		TXScopeRASectionConstIterator(const TRASectionConstIteratorBase<_TRAIterator>& src)
			: base_class(src) {}
		TXScopeRASectionConstIterator(_TRAIterator ra_iterator, size_type count, size_type index = 0)
			: base_class(ra_iterator, count, index) {}

		TXScopeRASectionConstIterator& operator +=(difference_t x) {
			base_class::operator +=(x);
			return (*this);
		}
		TXScopeRASectionConstIterator& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TXScopeRASectionConstIterator& operator ++() { operator +=(1); return (*this); }
		TXScopeRASectionConstIterator& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TXScopeRASectionConstIterator& operator --() { operator -=(1); return (*this); }
		TXScopeRASectionConstIterator& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TXScopeRASectionConstIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TXScopeRASectionConstIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionConstIteratorBase<_TRAIterator>& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TXScopeRASectionConstIterator& operator=(const TRASectionConstIteratorBase<_TRAIterator>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}

	private:
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRASectionConstIterator* operator&() { return this; }
		const TXScopeRASectionConstIterator* operator&() const { return this; }

	};

	template <typename _TRAIterator>
	class TRASectionConstIterator : public TRASectionConstIteratorBase<_TRAIterator> {
	public:
		typedef TRASectionConstIteratorBase<_TRAIterator> base_class;

		typedef typename base_class::difference_t difference_t;
		typedef typename base_class::size_type size_type;

		TRASectionConstIterator(const TRASectionConstIterator& src)
			: base_class(src) {}

		template <typename _TRAIterator1, class = typename std::enable_if<
			//(!std::is_convertible<_TRAIterator1, TPolyPointer>::value)
			//&& (!std::is_base_of<TPolyConstPointer<_Ty>, _TRAIterator1>::value)
			//&& (!std::integral_constant<bool, HasXScopeTagMethod_poly<_TRAIterator1>::Has>())
			/*&&*/ (!std::is_base_of<XScopeTagBase, _TRAIterator1>::value)
			, void>::type>
			TRASectionConstIterator(_TRAIterator1 ra_iterator, size_type count, size_type index = 0)
			: base_class(ra_iterator, count, index) {}

		TRASectionConstIterator& operator +=(difference_t x) {
			base_class::operator +=(x);
			return (*this);
		}
		TRASectionConstIterator& operator -=(difference_t x) { operator +=(-x); return (*this); }
		TRASectionConstIterator& operator ++() { operator +=(1); return (*this); }
		TRASectionConstIterator& operator ++(int) { auto _Tmp = *this; operator +=(1); return (_Tmp); }
		TRASectionConstIterator& operator --() { operator -=(1); return (*this); }
		TRASectionConstIterator& operator --(int) { auto _Tmp = *this; operator -=(1); return (_Tmp); }

		TRASectionConstIterator operator+(difference_t n) const { auto retval = (*this); retval += n; return retval; }
		TRASectionConstIterator operator-(difference_t n) const { return ((*this) + (-n)); }
		difference_t operator-(const TRASectionConstIteratorBase<_TRAIterator>& _Right_cref) const {
			return base_class::operator-(_Right_cref);
		}

		TRASectionConstIterator& operator=(const TRASectionConstIteratorBase<_TRAIterator>& _Right_cref) {
			base_class::operator=(_Right_cref);
			return (*this);
		}
	};

	template <typename _TRAIterator>
	class TRandomAccessConstSectionBase;
	template <typename _TRAIterator>
	class TXScopeRandomAccessSection;
	template <typename _TRAIterator>
	class TXScopeRandomAccessConstSection;
	template <typename _TRAIterator>
	class TRandomAccessSection;
	template <typename _TRAIterator>
	class TRandomAccessConstSection;

	template <typename _TRAIterator>
	class TRandomAccessSectionBase {
	public:
		typedef typename std::remove_reference<decltype(std::declval<_TRAIterator>()[0])>::type element_t;
		typedef decltype(std::declval<_TRAIterator>()[0]) reference_t;
		typedef typename std::add_lvalue_reference<typename std::add_const<element_t>::type>::type const_reference_t;
		typedef typename mse::msearray<element_t, 0>::size_type size_type;
		typedef decltype(std::declval<_TRAIterator>() - std::declval<_TRAIterator>()) difference_t;

		TRandomAccessSectionBase(const _TRAIterator& start_iter, size_type count) : m_start_iter(start_iter), m_count(count) {}
		//TRandomAccessSectionBase(const TRandomAccessSectionBase& src) = default;
		template <typename _TRAIterator1>
		TRandomAccessSectionBase(const TRandomAccessSectionBase<_TRAIterator1>& src) : m_start_iter(src.m_start_iter), m_count(src.m_count) {}

		reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - reference_t operator[](size_type _P) - TRandomAccessSectionBase")); }
			return m_start_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}

		typedef TRASectionIterator<_TRAIterator> iterator;
		typedef TRASectionConstIterator<_TRAIterator> const_iterator;
		iterator begin() const { return iterator(m_start_iter, m_count); }
		const_iterator cbegin() const { return const_iterator(m_start_iter, m_count); }
		iterator end() const {
			auto retval(iterator(m_start_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}
		const_iterator cend() const {
			auto retval(const_iterator(m_start_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		TRandomAccessSectionBase<_TRAIterator>* operator&() { return this; }
		const TRandomAccessSectionBase<_TRAIterator>* operator&() const { return this; }

		_TRAIterator m_start_iter;
		const size_type m_count = 0;

		friend class TXScopeRandomAccessSection<_TRAIterator>;
		template<typename _TRAIterator1> friend class TRandomAccessConstSectionBase;
		template<typename _TRAIterator1> friend class TRandomAccessSectionBase;
	};

	template <typename _TRAIterator>
	class TXScopeRandomAccessSection : public TRandomAccessSectionBase<_TRAIterator>, public XScopeTagBase {
	public:
		typedef TRandomAccessSectionBase<_TRAIterator> base_class;

		//TXScopeRandomAccessSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter,count) {}
		//TXScopeRandomAccessSection(const TXScopeRandomAccessSection& src) = default;
		//TXScopeRandomAccessSection(const TRandomAccessSectionBase<_TRAIterator>& src) : base_class(src) {}

		MSE_USING(TXScopeRandomAccessSection, base_class);

		typedef TXScopeRASectionIterator<_TRAIterator> iterator;
		typedef TXScopeRASectionConstIterator<_TRAIterator> const_iterator;
		iterator begin() const { return iterator((*this).m_start_iter, (*this).m_count); }
		const_iterator cbegin() const { return const_iterator((*this).m_start_iter, (*this).m_count); }
		iterator end() const {
			auto retval(iterator((*this).m_start_iter, (*this).m_count));
			retval += (*this).m_count;
			return retval;
		}
		const_iterator cend() const {
			auto retval(const_iterator((*this).m_start_iter, (*this).m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		//TXScopeRandomAccessSection<_TRAIterator>& operator=(const TXScopeRandomAccessSection<_TRAIterator>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRandomAccessSection<_TRAIterator>* operator&() { return this; }
		const TXScopeRandomAccessSection<_TRAIterator>* operator&() const { return this; }
	};

	template <typename _TRAIterator>
	class TRandomAccessSection : public TRandomAccessSectionBase<_TRAIterator> {
	public:
		typedef TRandomAccessSectionBase<_TRAIterator> base_class;
		typedef typename base_class::size_type size_type;

		template <class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TRAIterator>::value)>>
		TRandomAccessSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {}
		template <typename _TRAIterator1>
		TRandomAccessSection(const TRandomAccessSectionBase<_TRAIterator1>& src) : base_class(src) {}

		friend class TXScopeRandomAccessSection<_TRAIterator>;
		friend class TXScopeRandomAccessConstSection<_TRAIterator>;
		friend class TRandomAccessConstSection<_TRAIterator>;
	};

	template <typename _TRAIterator>
	class TRandomAccessConstSectionBase {
	public:
		typedef typename std::remove_reference<decltype(std::declval<_TRAIterator>()[0])>::type element_t;
		typedef decltype(std::declval<_TRAIterator>()[0]) reference_t;
		typedef typename std::add_lvalue_reference<typename std::add_const<element_t>::type>::type const_reference_t;
		typedef typename mse::msearray<element_t, 0>::size_type size_type;
		typedef decltype(std::declval<_TRAIterator>() - std::declval<_TRAIterator>()) difference_t;

		TRandomAccessConstSectionBase(const _TRAIterator& start_iter, size_type count) : m_start_iter(start_iter), m_count(count) {}
		//TRandomAccessConstSectionBase(const TRandomAccessConstSectionBase& src) = default;
		template <typename _TRAIterator1>
		TRandomAccessConstSectionBase(const TRandomAccessConstSectionBase<_TRAIterator1>& src) : m_start_iter(src.m_start_iter), m_count(src.m_count) {}
		//TRandomAccessConstSectionBase(const TRandomAccessSectionBase<_TRAIterator>& src) : m_start_iter(src.m_start_iter), m_count(src.m_count) {}
		template <typename _TRAIterator1>
		TRandomAccessConstSectionBase(const TRandomAccessSectionBase<_TRAIterator1>& src) : m_start_iter(src.m_start_iter), m_count(src.m_count) {}

		const_reference_t operator[](size_type _P) const {
			if (m_count <= _P) { MSE_THROW(msearray_range_error("out of bounds index - reference_t operator[](size_type _P) - TRandomAccessConstSectionBase")); }
			return m_start_iter[difference_t(_P)];
		}
		size_type size() const {
			return m_count;
		}

		typedef TRASectionIterator<_TRAIterator> iterator;
		typedef TRASectionConstIterator<_TRAIterator> const_iterator;
		const_iterator begin() const { return cbegin(); }
		const_iterator cbegin() const { return const_iterator(m_start_iter, m_count); }
		const_iterator end() const {
			return cend();
		}
		const_iterator cend() const {
			auto retval(const_iterator(m_start_iter, m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		TRandomAccessConstSectionBase<_TRAIterator>* operator&() { return this; }
		const TRandomAccessConstSectionBase<_TRAIterator>* operator&() const { return this; }

		_TRAIterator m_start_iter;
		const size_type m_count = 0;

		friend class TXScopeRandomAccessConstSection<_TRAIterator>;
		template<typename _TRAIterator1> friend class TRandomAccessConstSectionBase;
	};

	template <typename _TRAIterator>
	class TXScopeRandomAccessConstSection : public TRandomAccessConstSectionBase<_TRAIterator>, public XScopeTagBase {
	public:
		typedef TRandomAccessConstSectionBase<_TRAIterator> base_class;

		//TXScopeRandomAccessConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {}
		//TXScopeRandomAccessConstSection(const TXScopeRandomAccessConstSection& src) = default;
		//TXScopeRandomAccessConstSection(const TRandomAccessConstSectionBase<_TRAIterator>& src) : base_class(src) {}
		//TXScopeRandomAccessConstSection(const TRandomAccessSectionBase<_TRAIterator>& src) : base_class(src) {}

		MSE_USING(TXScopeRandomAccessConstSection, base_class);

		typedef TXScopeRASectionIterator<_TRAIterator> iterator;
		typedef TXScopeRASectionConstIterator<_TRAIterator> const_iterator;
		const_iterator begin() const { return cbegin(); }
		const_iterator cbegin() const { return const_iterator((*this).m_start_iter, (*this).m_count); }
		const_iterator end() const { return cend(); }
		const_iterator cend() const {
			auto retval(const_iterator((*this).m_start_iter, (*this).m_count));
			retval += (*this).m_count;
			return retval;
		}

	private:
		//TXScopeRandomAccessConstSection<_TRAIterator>& operator=(const TXScopeRandomAccessConstSection<_TRAIterator>& _Right_cref) = delete;
		void* operator new(size_t size) { return ::operator new(size); }

		TXScopeRandomAccessConstSection<_TRAIterator>* operator&() { return this; }
		const TXScopeRandomAccessConstSection<_TRAIterator>* operator&() const { return this; }
	};

	template <typename _TRAIterator>
	class TRandomAccessConstSection : public TRandomAccessConstSectionBase<_TRAIterator> {
	public:
		typedef TRandomAccessConstSectionBase<_TRAIterator> base_class;
		typedef typename base_class::size_type size_type;

		template <class = typename std::enable_if<(!std::is_base_of<XScopeTagBase, _TRAIterator>::value)>>
		TRandomAccessConstSection(const _TRAIterator& start_iter, size_type count) : base_class(start_iter, count) {}
		template <typename _TRAIterator1>
		TRandomAccessConstSection(const TRandomAccessConstSection<_TRAIterator1>& src) : base_class(src) {}
		template <typename _TRAIterator1>
		TRandomAccessConstSection(const TRandomAccessSectionBase<_TRAIterator1>& src) : base_class(src) {}

		friend class TXScopeRandomAccessConstSection<_TRAIterator>;
	};


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /*__GNUC__*/
#endif /*__clang__*/

	class msearray_test {
	public:
		void test1() {
#ifdef MSE_SELF_TESTS
			// construction uses aggregate initialization
			mse::msearray<int, 3> a1{ { 1, 2, 3 } }; // double-braces required in C++11 (not in C++14)
			mse::msearray<int, 3> a2 = { 11, 12, 13 };  // never required after =
			mse::msearray<std::string, 2> a3 = { std::string("a"), "b" };

			// container operations are supported
			std::sort(a1.begin(), a1.end());
			std::reverse_copy(a2.begin(), a2.end(),
				std::ostream_iterator<int>(std::cout, " "));

			std::cout << '\n';

			// ranged for loop is supported
			for (const auto& s : a3)
				std::cout << s << ' ';

			a1.swap(a2);
			assert(13 == a1[2]);
			assert(3 == a2[2]);

			std::swap(a1, a2);
			assert(3 == a1[2]);
			assert(13 == a2[2]);

			std::get<0>(a1) = 21;
			std::get<1>(a1) = 22;
			std::get<2>(a1) = 23;

			auto l_tuple_size = std::tuple_size<mse::msearray<int, 3>>::value;
			std::tuple_element<1, mse::msearray<int, 3>>::type b1 = 5;

			a1 = a2;

			{
				mse::msearray<int, 5> a = { 10, 20, 30, 40, 50 };
				mse::msearray<int, 5> b = { 10, 20, 30, 40, 50 };
				mse::msearray<int, 5> c = { 50, 40, 30, 20, 10 };

				if (a == b) std::cout << "a and b are equal\n";
				if (b != c) std::cout << "b and c are not equal\n";
				if (b<c) std::cout << "b is less than c\n";
				if (c>b) std::cout << "c is greater than b\n";
				if (a <= b) std::cout << "a is less than or equal to b\n";
				if (a >= b) std::cout << "a is greater than or equal to b\n";
			}
			{
				mse::msearray<int, 6> myarray;

				myarray.fill(5);

				std::cout << "myarray contains:";
				for (int& x : myarray) { std::cout << ' ' << x; }

				std::cout << '\n';
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

#undef MSE_THROW

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif /*ndef MSEMSEARRAY_H*/
