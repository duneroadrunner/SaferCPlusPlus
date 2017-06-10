
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
#if SIZE_MAX <= ULONG_MAX
#define MSE_MSEVECTOR_BASE_INTEGER_TYPE long int
#else // SIZE_MAX <= ULONG_MAX
#define MSE_MSEVECTOR_BASE_INTEGER_TYPE long long int
#endif // SIZE_MAX <= ULONG_MAX

	typedef size_t msev_size_t;
	typedef MSE_MSEVECTOR_BASE_INTEGER_TYPE msev_int;
	typedef bool msev_bool;
	typedef size_t msev_as_a_size_t;
#endif // MSE_MSEVECTOR_USE_MSE_PRIMITIVES


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
		msev_pointer(const msev_pointer<_Ty>& src) : m_ptr(src.m_ptr) {}

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

	/* Note that, at the moment, msevector inherits publicly from std::vector. This is not intended to be a permanent
		characteristic of msevector and any reference to, or interpretation of, an msevector as an std::vector is (and has
		always been) depricated. msevector endeavors to support the subset of the std::vector interface that is compatible
		with the security/safety goals of msevector. (The remaining part of the std::vector interface may be supported, as a
		user option, for compatibility.)
		In particular, keep in mind that std::vector does not have a virtual destructor, so deallocating an msevector as an
		std::vector would result in memory leaks. */
	template<class _Ty, class _A = std::allocator<_Ty> >
	class msevector : public std::vector<_Ty, _A> {
	public:
		typedef std::vector<_Ty, _A> base_class;
		typedef msevector<_Ty, _A> _Myt;

		typedef typename base_class::value_type value_type;
		//typedef typename base_class::size_type size_type;
		typedef msev_size_t size_type;
		//typedef typename base_class::difference_type difference_type;
		typedef msev_int difference_type;
		typedef typename base_class::pointer pointer;
		typedef typename base_class::const_pointer const_pointer;
		typedef typename base_class::reference reference;
		typedef typename base_class::const_reference const_reference;

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
		msevector(base_class&& _X) : base_class(std::move(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
		msevector(const base_class& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
		msevector(_Myt&& _X) : base_class(std::move(_X)), m_mmitset(*this) { /*m_debug_size = size();*/ }
		msevector(const _Myt& _X) : base_class(_X), m_mmitset(*this) { /*m_debug_size = size();*/ }
		typedef typename base_class::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		msevector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
		msevector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
		msevector(_Iter _First, _Iter _Last) : base_class(_First, _Last), m_mmitset(*this) { /*m_debug_size = size();*/ }
		template<class _Iter
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> >
		//msevector(_Iter _First, _Iter _Last, const typename base_class::_Alloc& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
		msevector(_Iter _First, _Iter _Last, const _A& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { /*m_debug_size = size();*/ }
		_Myt& operator=(const base_class& _X) {
			base_class::operator =(_X);
			/*m_debug_size = size();*/
			m_mmitset.reset();
			return (*this);
		}
		_Myt& operator=(_Myt&& _X) {
			operator=(std::move(static_cast<base_class&>(_X)));
			m_mmitset.reset();
			return (*this);
		}
		_Myt& operator=(const _Myt& _X) {
			operator=((base_class)_X);
			m_mmitset.reset();
			return (*this);
		}
		void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
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
			if (m_mmitset.is_empty()) {
				base_class::push_back(std::move(_X));
			}
			else {
				auto original_size = msev_size_t((*this).size());
				auto original_capacity = msev_size_t((*this).capacity());

				base_class::push_back(std::move(_X));
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
			base_class::assign(_F, _L);
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
		template<class _Iter>
		void assign(_Iter _First, _Iter _Last) {	// assign [_First, _Last)
			base_class::assign(_First, _Last);
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
		void assign(size_type _N, const _Ty& _X = _Ty()) {
			base_class::assign(msev_as_a_size_t(_N), _X);
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
		typename base_class::iterator insert(typename base_class::const_iterator _P, _Ty&& _X) {
			return (emplace(_P, std::move(_X)));
		}
		typename base_class::iterator insert(typename base_class::const_iterator _P, const _Ty& _X = _Ty()) {
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
		typename base_class::iterator insert(typename base_class::const_iterator _Where, _Iter _First, _Iter _Last) {	// insert [_First, _Last) at _Where
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
		insert(typename base_class::/*const_*/iterator _Where, _Iter _First, _Iter _Last) {	// insert [_First, _Last) at _Where
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
				msev_size_t d2 = msev_size_t(di2);
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
			base_class::clear();
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
		void swap(base_class& _X) {
			base_class::swap(_X);
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
		void swap(_Myt& _X) {
			swap(static_cast<base_class&>(_X));
			m_mmitset.reset();
		}

		msevector(_XSTD initializer_list<typename base_class::value_type> _Ilist,
			const _A& _Al = _A())
			: base_class(_Ilist, _Al), m_mmitset(*this) {	// construct from initializer_list
			/*m_debug_size = size();*/
		}
		_Myt& operator=(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
			operator=((base_class)_Ilist);
			m_mmitset.reset();
			return (*this);
		}
		void assign(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
			base_class::assign(_Ilist);
			/*m_debug_size = size();*/
			m_mmitset.reset();
		}
#if defined(GPP4P8_COMPATIBLE)
		/* g++4.8 seems to be (incorrectly) using the c++98 version of this insert function instead of the c++11 version. */
		/*typename base_class::iterator*/void insert(typename base_class::/*const_*/iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
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

		class random_access_const_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {};
		class random_access_iterator_base : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference> {};

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
				return &(m_owner_cptr->at(msev_as_a_size_t(m_index)));
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
				return &(m_owner_ptr->at(msev_as_a_size_t(m_index)));
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
			friend class /*_Myt*/msevector<_Ty, _A>;
		};

	private:
		typedef std::size_t CHashKey1;
		class mm_const_iterator_handle_type {
		public:
			mm_const_iterator_handle_type(const CHashKey1& key_cref, const std::shared_ptr<mm_const_iterator_type>& shptr_cref) : m_shptr(shptr_cref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_const_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class /*_Myt*/msevector<_Ty, _A>;
			friend class mm_iterator_set_type;
		};
		class mm_iterator_handle_type {
		public:
			mm_iterator_handle_type(const CHashKey1& key_cref, const std::shared_ptr<mm_iterator_type>& shptr_ref) : m_shptr(shptr_ref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class /*_Myt*/msevector<_Ty, _A>;
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
				} else {
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

			friend class /*_Myt*/msevector<_Ty, _A>;
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
		private:
			const _Myt* m_owner_cptr = nullptr;
			std::shared_ptr<mm_const_iterator_handle_type> m_handle_shptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
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
		private:
			_Myt* m_owner_ptr = nullptr;
			std::shared_ptr<mm_iterator_handle_type> m_handle_shptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
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
			(*this).insert(_P, 1, std::move(_X));
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
			insert_before(pos.const_item_pointer(), std::move(_X));
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
			(*this).insert(_P, std::move(_X));
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
		ipointer insert(const cipointer &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
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
			auto retval = base_class::emplace(_P, std::forward<_Valty>(_Val)...);
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
			erase(pos.const_item_pointer());
			ipointer retval = (*this).ibegin();
			retval.advance(msev_int(retval_pos.position()));
			return retval;
		}
		ipointer erase(const cipointer &start, const cipointer &end) {
			auto retval_pos = end;
			retval_pos.set_to_next();
			erase(start.const_item_pointer(), end.const_item_pointer());
			ipointer retval = (*this).ibegin();
			retval.advance(msev_int(retval_pos.position()));
			return retval;
		}
		ipointer erase_inclusive(const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return erase(first, end);
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


		/* ss_const_iterator_type is a bounds checked iterator. */
		class ss_const_iterator_type : public random_access_const_iterator_base {
		public:
			typedef typename base_class::const_iterator::iterator_category iterator_category;
			typedef typename base_class::const_iterator::value_type value_type;
			//typedef typename base_class::const_iterator::difference_type difference_type;
			typedef typename _Myt::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::const_iterator::pointer pointer;
			typedef typename base_class::const_pointer const_pointer;
			typedef typename base_class::const_iterator::reference reference;
			typedef typename base_class::const_reference const_reference;

			ss_const_iterator_type() {}
			void assert_valid_index() const {
				if (m_owner_cptr->size() < m_index) { MSE_THROW(msevector_range_error("invalid index - void assert_valid_index() const - ss_const_iterator_type - msevector")); }
			}
			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_owner_cptr->size() > m_index) { return true; }
				else {
					if (m_index == m_owner_cptr->size()) { return false; }
					else { MSE_THROW(msevector_range_error("attempt to use invalid ss_const_iterator_type - bool points_to_an_item() const - ss_const_iterator_type - msevector")); }
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
			bool has_next_item_or_end_marker() const { return points_to_an_item(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return has_next_item_or_end_marker(); }
			bool has_previous() const {
				if (m_owner_cptr->size() < m_index) {
					MSE_THROW(msevector_range_error("attempt to use invalid ss_const_iterator_type - bool has_previous() const - ss_const_iterator_type - msevector"));
				}
				else if (1 <= m_index) {
					return true;
				}
				else {
					return false;
				}
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
				}
				else {
					MSE_THROW(msevector_range_error("attempt to use invalid const_item_pointer - void set_to_next() - ss_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(msevector_range_error("attempt to use invalid const_item_pointer - void set_to_previous() - ss_const_iterator_type - msevector"));
				}
			}
			ss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_const_iterator_type operator++(int) { ss_const_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			ss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_const_iterator_type operator--(int) { ss_const_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_cptr->size() < msev_size_t(new_index))) {
					MSE_THROW(msevector_range_error("index out of range - void advance(difference_type n) - ss_const_iterator_type - msevector"));
				}
				else {
					m_index = msev_size_t(new_index);
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
				if (rhs.m_owner_cptr != (*this).m_owner_cptr) { MSE_THROW(msevector_range_error("invalid argument - difference_type operator-(const ss_const_iterator_type &rhs) const - msevector::ss_const_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert(difference_type((*m_owner_cptr).size()) >= retval);
				return retval;
			}
			const_reference operator*() const {
				return (*m_owner_cptr).at(msev_as_a_size_t((*this).m_index));
			}
			const_reference item() const { return operator*(); }
			const_reference previous_item() const {
				return m_owner_cptr->at(msev_as_a_size_t(m_index - 1));
			}
			const_pointer operator->() const {
				return &((*m_owner_cptr).at(msev_as_a_size_t((*this).m_index)));
			}
			const_reference operator[](difference_type _Off) const { return (*m_owner_cptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }
			/*
			ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref)
			{
			msev_int d = std::distance<typename base_class::iterator>(m_owner_cptr->cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cptr->size() >= d)) {
			if (m_owner_cptr->size() == d) {
			assert(m_owner_cptr->cend() == _Right_cref);
			}
			m_index = msev_size_t(d);
			base_class::const_iterator::operator=(_Right_cref);
			}
			else {
			MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref) - ss_const_iterator_type - msevector"));
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
				if (this->m_owner_cptr != _Right_cref.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid argument - ss_const_iterator_type& operator==(const ss_const_iterator_type& _Right) - ss_const_iterator_type - msevector")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const ss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_const_iterator_type& _Right) const {
				if (this->m_owner_cptr != _Right.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid argument - ss_const_iterator_type& operator<(const ss_const_iterator_type& _Right) - ss_const_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const ss_const_iterator_type& _Right_cref) {
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
						MSE_THROW(msevector_range_error("void shift_inclusive_range() - ss_const_iterator_type - msevector"));
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
		private:
			void sync_const_iterator_to_index() {
				assert(m_owner_cptr->size() >= (*this).m_index);
				//base_class::const_iterator::operator=(m_owner_cptr->cbegin());
				//base_class::const_iterator::operator+=(msev_as_a_size_t(m_index));
			}
			msev_size_t m_index = 0;
			msev_pointer<const _Myt> m_owner_cptr = nullptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};
		/* ss_iterator_type is a bounds checked iterator. */
		class ss_iterator_type : public random_access_iterator_base {
		public:
			typedef typename base_class::iterator::iterator_category iterator_category;
			typedef typename base_class::iterator::value_type value_type;
			//typedef typename base_class::iterator::difference_type difference_type;
			typedef typename _Myt::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::iterator::pointer pointer;
			typedef typename base_class::iterator::reference reference;

			ss_iterator_type() {}
			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_owner_ptr->size() > m_index) { return true; }
				else {
					if (m_index == m_owner_ptr->size()) { return false; }
					else { MSE_THROW(msevector_range_error("attempt to use invalid ss_iterator_type - bool points_to_an_item() const - ss_iterator_type - msevector")); }
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
				if (m_owner_ptr->size() < m_index) {
					MSE_THROW(msevector_range_error("attempt to use invalid ss_iterator_type - bool has_previous() const - ss_iterator_type - msevector"));
				} else if (1 <= m_index) {
					return true;
				}
				else {
					return false;
				}
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
				}
				else {
					MSE_THROW(msevector_range_error("attempt to use invalid item_pointer - void set_to_next() - ss_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
				}
				else {
					MSE_THROW(msevector_range_error("attempt to use invalid item_pointer - void set_to_previous() - ss_iterator_type - msevector"));
				}
			}
			ss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_iterator_type operator++(int) { ss_iterator_type _Tmp = *this; (*this).set_to_next(); return (_Tmp); }
			ss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_iterator_type operator--(int) { ss_iterator_type _Tmp = *this; (*this).set_to_previous(); return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = msev_int(m_index) + n;
				if ((0 > new_index) || (m_owner_ptr->size() < msev_size_t(new_index))) {
					MSE_THROW(msevector_range_error("index out of range - void advance(difference_type n) - ss_iterator_type - msevector"));
				}
				else {
					m_index = msev_size_t(new_index);
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
				if (rhs.m_owner_ptr != (*this).m_owner_ptr) { MSE_THROW(msevector_range_error("invalid argument - difference_type operator-(const ss_iterator_type& rhs) const - msevector::ss_iterator_type")); }
				auto retval = difference_type((*this).m_index) - difference_type(rhs.m_index);
				assert((int)((*m_owner_ptr).size()) >= retval);
				return retval;
			}
			reference operator*() const {
				return (*m_owner_ptr).at(msev_as_a_size_t((*this).m_index));
			}
			reference item() const { return operator*(); }
			reference previous_item() const {
				return m_owner_ptr->at(msev_as_a_size_t(m_index - 1));
			}
			pointer operator->() const {
				return &((*m_owner_ptr).at(msev_as_a_size_t((*this).m_index)));
			}
			reference operator[](difference_type _Off) const { return (*m_owner_ptr).at(msev_as_a_size_t(difference_type(m_index) + _Off)); }
			/*
			ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref)
			{
			msev_int d = std::distance<typename base_class::iterator>(m_owner_ptr->begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ptr->size() >= d)) {
			if (m_owner_ptr->size() == d) {
			assert(m_owner_ptr->end() == _Right_cref);
			}
			m_index = msev_size_t(d);
			base_class::iterator::operator=(_Right_cref);
			}
			else {
			MSE_THROW(msevector_range_error("doesn't seem to be a valid assignment value - ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - ss_const_iterator_type - msevector"));
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
				if (this->m_owner_ptr != _Right_cref.m_owner_ptr) { MSE_THROW(msevector_range_error("invalid argument - ss_iterator_type& operator==(const ss_iterator_type& _Right) - ss_iterator_type - msevector")); }
				return (_Right_cref.m_index == m_index);
			}
			bool operator!=(const ss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_iterator_type& _Right) const {
				if (this->m_owner_ptr != _Right.m_owner_ptr) { MSE_THROW(msevector_range_error("invalid argument - ss_iterator_type& operator<(const ss_iterator_type& _Right) - ss_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const ss_iterator_type& _Right_cref) {
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
						MSE_THROW(msevector_range_error("void shift_inclusive_range() - ss_iterator_type - msevector"));
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
			operator ss_const_iterator_type() const {
				ss_const_iterator_type retval;
				if (nullptr != m_owner_ptr) {
					retval = m_owner_ptr->ss_cbegin();
					retval.advance(msev_int(m_index));
				}
				return retval;
			}
			operator typename base_class::iterator() const {
				typename base_class::iterator retval = (*m_owner_ptr).begin();
				retval += msev_as_a_size_t(m_index);
				return retval;
			}

		private:
			void sync_iterator_to_index() {
				assert(m_owner_ptr->size() >= (*this).m_index);
				//base_class::iterator::operator=(m_owner_ptr->begin());
				//base_class::iterator::operator+=(msev_as_a_size_t(m_index));
			}
			msev_size_t m_index = 0;
			msev_pointer<_Myt> m_owner_ptr = nullptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
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

		msevector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
			: base_class(_Al), m_mmitset(*this) {
			/*m_debug_size = size();*/
			assign(start, end);
		}
		void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			auto end = last;
			end++; // this should include some checks
			(*this).assign(first, end);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) {
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid argument - void insert_before() - msevector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
			(*this).insert(_P, _M, _X);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid argument - void insert_before() - msevector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
			(*this).insert(_P, std::move(_X));
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid argument - ss_iterator_type insert_before() - msevector")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type& start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			end.assert_valid_index();
			if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			typename base_class::const_iterator _S = start;
			typename base_class::const_iterator _E = end;
			return (*this).insert_before(pos, _S, _E);
		}
		ss_iterator_type insert_before(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* end) {
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - ss_iterator_type insert_before() - msevector")); }
			//if (start.m_owner_cptr != end.m_owner_cptr) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			if (start > end) { MSE_THROW(msevector_range_error("invalid arguments - ss_iterator_type insert_before() - msevector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
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
			if (pos.m_owner_ptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void insert_before() - msevector")); }
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
			(*this).insert(_P, _Ilist);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		ss_iterator_type insert(const ss_const_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
			, class = _mse_RequireInputIter<_Iter> >
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(const ss_const_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) { return insert_before(pos, _Ilist); }
		template<class ..._Valty>
#if !(defined(GPP4P8_COMPATIBLE))
		ss_iterator_type emplace(const ss_const_iterator_type &pos, _Valty&& ..._Val)
		{	// insert by moving _Val at pos
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void emplace() - msevector")); }
#else /*!(defined(GPP4P8_COMPATIBLE))*/
		ipointer emplace(const ipointer &pos, _Valty&& ..._Val)
		{	// insert by moving _Val at pos
			if (pos.m_owner_ptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void emplace() - msevector")); }
#endif /*!(defined(GPP4P8_COMPATIBLE))*/
			pos.assert_valid_index();
			msev_size_t original_pos = pos.position();
			typename base_class::const_iterator _P = pos;
			(*this).emplace(_P,  std::forward<_Valty>(_Val)...);
			ss_iterator_type retval = ss_begin();
			retval.advance(msev_int(original_pos));
			return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &pos) {
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
			if (!pos.points_to_an_item()) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
			auto pos_index = pos.position();

			typename base_class::const_iterator _P = pos;
			(*this).erase(_P);

			ss_iterator_type retval = (*this).ss_begin();
			retval.advance(typename ss_const_iterator_type::difference_type(pos_index));
			return retval;
		}
		ss_iterator_type erase(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
			if (end.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
			if (start.position() > end.position()) { MSE_THROW(msevector_range_error("invalid arguments - void erase() - msevector")); }
			auto pos_index = start.position();

			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
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
			if (pos.m_owner_cptr != this) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
			if (!(pos.has_previous())) { MSE_THROW(msevector_range_error("invalid arguments - void erase_previous_item() - msevector")); }
			typename base_class::const_iterator _P = pos;
			_P--;
			(*this).erase(_P);
		}
	};

}

#undef MSE_THROW

#endif /*ndef MSEMSEVECTOR_H*/
