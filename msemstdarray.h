
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSTDARRAY_H
#define MSEMSTDARRAY_H

#include <memory>
#include <type_traits>
#include <stdexcept>
#include "msemsearray.h"
#ifndef MSE_MSTDARRAY_DISABLED
#include "mseregistered.h"
#endif /*MSE_MSTDARRAY_DISABLED*/

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDARRAY_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	namespace mstd {

#ifdef MSE_MSTDARRAY_DISABLED
		template<class _Ty, size_t _Size > using array = std::array<_Ty, _Size>;

		template<class _TArray> using xscope_const_iterator = typename _TArray::const_iterator;
		template<class _TArray>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeFixedConstPointer<_TArray>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}
		template<class _TArray>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>();
		}

		template<class _TArray> using xscope_iterator = typename _TArray::iterator;
		template<class _TArray>
		xscope_iterator<_TArray> make_xscope_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
			return xscope_iterator<_TArray>();
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_iterator<_TArray> make_xscope_iterator(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_iterator<_TArray>();
		}

#else /*MSE_MSTDARRAY_DISABLED*/

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

#ifndef _STD
#define _STD std::
#endif /*_STD*/

#ifndef _CONST_FUN
#define _CONST_FUN  constexpr
#endif /*_CONST_FUN*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

		class mstdarray_range_error : public std::range_error { public:
			using std::range_error::range_error;
		};

		template<class _Ty, size_t _Size>
		class array_helper_type {
		public:
			static typename mse::msearray<_Ty, _Size> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				/* Template specializations of this function construct mse::msearrays of non-default constructible
				elements. This (non-specialized) implementation here should cause a compile error when invoked. */
				if (0 < _Ilist.size()) { MSE_THROW(mstdarray_range_error("sorry, arrays of this size are not supported with when the elements are non-default constructible - mse::mstd::array")); }
				typename mse::msearray<_Ty, _Size> retval {};
				return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 1> { public:
			static typename mse::msearray<_Ty, 1> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				/* This template specialization constructs an mse::msearray of size 1 and supports non-default
				constructible elements. */
				if (1 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 1> retval { *(_Ilist.begin()) }; return retval;
			}
		};
		/* Template specializations that construct mse::msearrays of different sizes are located later in the file. */

		template<class _Ty, size_t _Size >
		class array {
		public:
			typedef mse::mstd::array<_Ty, _Size> _Myt;
			typedef mse::msearray<_Ty, _Size> _MA;

			typedef typename _MA::value_type value_type;
			typedef typename _MA::size_type size_type;
			typedef typename _MA::difference_type difference_type;
			typedef typename _MA::pointer pointer;
			typedef typename _MA::const_pointer const_pointer;
			typedef typename _MA::reference reference;
			typedef typename _MA::const_reference const_reference;

			const _MA& msearray() const { return m_msearray; }
			_MA& msearray() { return m_msearray; }
			operator const _MA() const { return msearray(); }
			operator _MA() { return msearray(); }

			array() {}
			array(_MA&& _X) : m_msearray(std::move(_X)) {}
			array(const _MA& _X) : m_msearray(_X) {}
			array(_Myt&& _X) : m_msearray(std::move(_X.msearray())) {}
			array(const _Myt& _X) : m_msearray(_X.msearray()) {}
			//array(_XSTD initializer_list<typename _MA::base_class::value_type> _Ilist) : m_msearray(_Ilist) {}
			static mse::msearray<_Ty, _Size> msearray_initial_value(std::true_type, _XSTD initializer_list<_Ty> _Ilist) {
				/* _Ty is default constructible. */
				mse::msearray<_Ty, _Size> retval;
				assert(_Size >= _Ilist.size());
				auto stop_size = _Size;
				if (_Size > _Ilist.size()) {
					stop_size = _Ilist.size();
					/* just to make sure that all the retval elements are initialized as if by aggregate initialization. */
					retval = mse::msearray<_Ty, _Size>{};
				}
				msear_size_t count = 0;
				auto Il_it = _Ilist.begin();
				auto target_it = retval.begin();
				for (; (count < stop_size); Il_it++, count += 1, target_it++) {
					(*target_it) = (*Il_it);
				}
				return retval;
			}
			static mse::msearray<_Ty, _Size> msearray_initial_value(std::false_type, _XSTD initializer_list<_Ty> _Ilist) {
				/* _Ty is not default constructible. */
				return array_helper_type<_Ty, _Size>::msearray_initial_value2(_Ilist);
			}
			array(_XSTD initializer_list<_Ty> _Ilist) : m_msearray(msearray_initial_value(std::is_default_constructible<_Ty>(), _Ilist)) {
				/* std::array<> is an "aggregate type" (basically a POD struct with no base class, constructors or private
				data members (details here: http://en.cppreference.com/w/cpp/language/aggregate_initialization)). As such,
				support for construction from initializer list is automatically generated by the compiler. Specifically,
				aggregate types support "aggregate initialization". But since mstd::array has a member with an explicitly
				defined constructor (or at least I think that's why), it is not an aggregate type and therefore doesn't
				qualify to have support for "aggregate initialization" automatically generated by the compiler. It doesn't
				seem possible to emulate full aggregate initialization compatibility, so we'll just have to do the best we
				can. */
			}

			_Myt& operator=(_MA&& _X) { m_msearray.operator=(std::move(_X)); return (*this); }
			_Myt& operator=(const _MA& _X) { m_msearray.operator=(_X); return (*this); }
			_Myt& operator=(_Myt&& _X) { m_msearray.operator=(std::move(_X.msearray())); return (*this); }
			_Myt& operator=(const _Myt& _X) { m_msearray.operator=(_X.msearray()); return (*this); }
			_CONST_FUN typename _MA::const_reference operator[](size_type _P) const { return m_msearray.operator[](_P); }
			typename _MA::reference operator[](size_type _P) { return m_msearray.operator[](_P); }
			void fill(const _Ty& _Value) { m_msearray.fill(_Value); }
			void swap(_MA& _X) { m_msearray.swap(_X); }
			void swap(_Myt& _X) { m_msearray.swap(_X.msearray()); }

			_CONST_FUN size_type size() const _NOEXCEPT { return m_msearray.size(); }
			_CONST_FUN size_type max_size() const _NOEXCEPT { return m_msearray.max_size(); }
			_CONST_FUN bool empty() const _NOEXCEPT { return m_msearray.empty(); }
			_CONST_FUN typename _MA::const_reference at(size_type _Pos) const { return m_msearray.at(_Pos); }
			typename _MA::reference at(size_type _Pos) { return m_msearray.at(_Pos); }
			typename _MA::reference front() { return m_msearray.front(); }
			_CONST_FUN typename _MA::const_reference front() const { return m_msearray.front(); }
			typename _MA::reference back() { return m_msearray.back(); }
			_CONST_FUN typename _MA::const_reference back() const { return m_msearray.back(); }

			/* Try to avoid using these whenever possible. */
			value_type *data() _NOEXCEPT {	// return pointer to mutable data array
				return m_msearray.data();
			}
			const value_type *data() const _NOEXCEPT {	// return pointer to nonmutable data array
				return m_msearray.data();
			}

			array(std::array<_Ty, _Size>&& _X) {
				m_msearray.m_array = std::move(_X);
			}
			array(const std::array<_Ty, _Size>& _X) {
				m_msearray.m_array = _X;
			}
			operator const std::array<_Ty, _Size>() const { return msearray().m_array; }
			operator std::array<_Ty, _Size>() { return msearray().m_array; }


			class xscope_const_iterator;
			class xscope_iterator;

			class const_iterator : public _MA::random_access_const_iterator_base {
			public:
				typedef typename _MA::ss_const_iterator_type::iterator_category iterator_category;
				typedef typename _MA::ss_const_iterator_type::value_type value_type;
				typedef typename _MA::ss_const_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::ss_const_iterator_type::pointer pointer;
				typedef typename _MA::ss_const_iterator_type::reference reference;

				const_iterator() {}
				const_iterator(const const_iterator& src_cref) : m_msearray_regcptr(src_cref.m_msearray_regcptr) {
					(*this) = src_cref;
				}
				~const_iterator() {}
				const typename _MA::ss_const_iterator_type& msearray_ss_const_iterator_type() const {
					if (!m_msearray_regcptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::const_iterator")); }
					return m_ss_const_iterator;
				}
				typename _MA::ss_const_iterator_type& msearray_ss_const_iterator_type() {
					if (!m_msearray_regcptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::const_iterator")); }
					return m_ss_const_iterator;
				}
				const typename _MA::ss_const_iterator_type& mvssci() const { return msearray_ss_const_iterator_type(); }
				typename _MA::ss_const_iterator_type& mvssci() { return msearray_ss_const_iterator_type(); }

				void reset() { msearray_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msearray_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msearray_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msearray_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msearray_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msearray_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msearray_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msearray_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msearray_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msearray_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msearray_ss_const_iterator_type().set_to_previous(); }
				const_iterator& operator ++() { msearray_ss_const_iterator_type().operator ++(); return (*this); }
				const_iterator operator++(int) { const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				const_iterator& operator --() { msearray_ss_const_iterator_type().operator --(); return (*this); }
				const_iterator operator--(int) { const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { msearray_ss_const_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { msearray_ss_const_iterator_type().regress(n); }
				const_iterator& operator +=(difference_type n) { msearray_ss_const_iterator_type().operator +=(n); return (*this); }
				const_iterator& operator -=(difference_type n) { msearray_ss_const_iterator_type().operator -=(n); return (*this); }
				const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const const_iterator& _Right_cref) const { return msearray_ss_const_iterator_type() - (_Right_cref.msearray_ss_const_iterator_type()); }
				typename _MA::const_reference operator*() const { return msearray_ss_const_iterator_type().operator*(); }
				typename _MA::const_reference item() const { return operator*(); }
				typename _MA::const_reference previous_item() const { return msearray_ss_const_iterator_type().previous_item(); }
				typename _MA::const_pointer operator->() const { return msearray_ss_const_iterator_type().operator->(); }
				typename _MA::const_reference operator[](typename _MA::difference_type _Off) const { return msearray_ss_const_iterator_type()[_Off]; }
				bool operator==(const const_iterator& _Right_cref) const { return msearray_ss_const_iterator_type().operator==(_Right_cref.msearray_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() < _Right.msearray_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() <= _Right.msearray_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() > _Right.msearray_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() >= _Right.msearray_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { msearray_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msearray_ss_const_iterator_type()); }
				msear_size_t position() const { return msearray_ss_const_iterator_type().position(); }
			private:
				const_iterator(mse::TRegisteredConstPointer<_MA> msearray_regcptr) : m_msearray_regcptr(msearray_regcptr) {
					m_ss_const_iterator = msearray_regcptr->ss_cbegin();
				}
				mse::TRegisteredConstPointer<_MA> m_msearray_regcptr = nullptr;
				/* m_ss_const_iterator needs to be declared after m_msearray_regcptr so that it's destructor will be called first. */
				typename _MA::ss_const_iterator_type m_ss_const_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
				friend class iterator;
				friend class xscope_const_iterator;
			};
			class iterator : public _MA::random_access_iterator_base {
			public:
				typedef typename _MA::ss_iterator_type::iterator_category iterator_category;
				typedef typename _MA::ss_iterator_type::value_type value_type;
				typedef typename _MA::ss_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::ss_iterator_type::pointer pointer;
				typedef typename _MA::ss_iterator_type::reference reference;

				iterator() {}
				iterator(const iterator& src_cref) : m_msearray_regptr(src_cref.m_msearray_regptr) {
					(*this) = src_cref;
				}
				~iterator() {}
				const typename _MA::ss_iterator_type& msearray_ss_iterator_type() const {
					if (!m_msearray_regptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::iterator")); }
					return m_ss_iterator;
				}
				typename _MA::ss_iterator_type& msearray_ss_iterator_type() {
					if (!m_msearray_regptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::iterator")); }
					return m_ss_iterator;
				}
				const typename _MA::ss_iterator_type& mvssi() const { return msearray_ss_iterator_type(); }
				typename _MA::ss_iterator_type& mvssi() { return msearray_ss_iterator_type(); }
				operator const_iterator() const {
					const_iterator retval(m_msearray_regptr);
					if (m_msearray_regptr) {
						retval.msearray_ss_const_iterator_type().set_to_beginning();
						retval.msearray_ss_const_iterator_type().advance(msear_int(msearray_ss_iterator_type().position()));
					}
					return retval;
				}

				void reset() { msearray_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msearray_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msearray_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msearray_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msearray_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msearray_ss_iterator_type().has_next(); }
				bool has_previous() const { return msearray_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msearray_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msearray_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msearray_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msearray_ss_iterator_type().set_to_previous(); }
				iterator& operator ++() { msearray_ss_iterator_type().operator ++(); return (*this); }
				iterator operator++(int) { iterator _Tmp = *this; ++*this; return (_Tmp); }
				iterator& operator --() { msearray_ss_iterator_type().operator --(); return (*this); }
				iterator operator--(int) { iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { msearray_ss_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { msearray_ss_iterator_type().regress(n); }
				iterator& operator +=(difference_type n) { msearray_ss_iterator_type().operator +=(n); return (*this); }
				iterator& operator -=(difference_type n) { msearray_ss_iterator_type().operator -=(n); return (*this); }
				iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const iterator& _Right_cref) const { return msearray_ss_iterator_type() - (_Right_cref.msearray_ss_iterator_type()); }
				typename _MA::reference operator*() const { return msearray_ss_iterator_type().operator*(); }
				typename _MA::reference item() const { return operator*(); }
				typename _MA::reference previous_item() const { return msearray_ss_iterator_type().previous_item(); }
				typename _MA::pointer operator->() const { return msearray_ss_iterator_type().operator->(); }
				typename _MA::reference operator[](typename _MA::difference_type _Off) const { return msearray_ss_iterator_type()[_Off]; }
				bool operator==(const iterator& _Right_cref) const { return msearray_ss_iterator_type().operator==(_Right_cref.msearray_ss_iterator_type()); }
				bool operator!=(const iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const iterator& _Right) const { return (msearray_ss_iterator_type() < _Right.msearray_ss_iterator_type()); }
				bool operator<=(const iterator& _Right) const { return (msearray_ss_iterator_type() <= _Right.msearray_ss_iterator_type()); }
				bool operator>(const iterator& _Right) const { return (msearray_ss_iterator_type() > _Right.msearray_ss_iterator_type()); }
				bool operator>=(const iterator& _Right) const { return (msearray_ss_iterator_type() >= _Right.msearray_ss_iterator_type()); }
				void set_to_item_pointer(const iterator& _Right_cref) { msearray_ss_iterator_type().set_to_item_pointer(_Right_cref.msearray_ss_iterator_type()); }
				msear_size_t position() const { return msearray_ss_iterator_type().position(); }
			private:
				mse::TRegisteredPointer<_MA> m_msearray_regptr = nullptr;
				/* m_ss_iterator needs to be declared after m_msearray_regptr so that it's destructor will be called first. */
				typename _MA::ss_iterator_type m_ss_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
				friend class xscope_const_iterator;
				friend class xscope_iterator;
			};

			iterator begin()
			{	// return iterator for beginning of mutable sequence
				iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_iterator) = m_msearray.ss_begin();
				return retval;
			}

			const_iterator begin() const
			{	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msearray_regcptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_begin();
				return retval;
			}

			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_iterator) = m_msearray.ss_end();
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msearray_regcptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_end();
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msearray_regcptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_cbegin();
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msearray_regcptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_cend();
				return retval;
			}


			bool operator==(const _Myt& _Right) const {	// test for array equality
				return (_Right.m_msearray == m_msearray);
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for arrays
				return (m_msearray < _Right.m_msearray);
			}

			/* These static functions are just used to obtain a (base class) reference to an
			object of a (possibly) derived class. */
			static _MA& _MA_ref(_MA& obj) { return obj; }
			static const _MA& _MA_cref(const _MA& obj) { return obj; }

			class xscope_const_iterator {
			public:
				typedef typename _MA::xscope_ss_const_iterator_type::iterator_category iterator_category;
				typedef typename _MA::xscope_ss_const_iterator_type::value_type value_type;
				typedef typename _MA::xscope_ss_const_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::xscope_ss_const_iterator_type::pointer pointer;
				typedef typename _MA::xscope_ss_const_iterator_type::reference reference;

				xscope_const_iterator(const mse::TXScopeFixedConstPointer<array>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr)) {}
				xscope_const_iterator(const mse::TXScopeFixedPointer<array>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr)) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<const array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedPointer<const array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_msearray), owner_ptr.lease_pointer())) {}

				xscope_const_iterator(const xscope_const_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_const_iterator) {}
				xscope_const_iterator(const xscope_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_const_iterator() {}
				const typename _MA::xscope_ss_const_iterator_type& msearray_xscope_ss_const_iterator_type() const {
					return m_xscope_ss_const_iterator;
				}
				typename _MA::xscope_ss_const_iterator_type& msearray_xscope_ss_const_iterator_type() {
					return m_xscope_ss_const_iterator;
				}
				const typename _MA::xscope_ss_const_iterator_type& mvssci() const { return msearray_xscope_ss_const_iterator_type(); }
				typename _MA::xscope_ss_const_iterator_type& mvssci() { return msearray_xscope_ss_const_iterator_type(); }

				void reset() { msearray_xscope_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msearray_xscope_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msearray_xscope_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msearray_xscope_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msearray_xscope_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msearray_xscope_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msearray_xscope_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msearray_xscope_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msearray_xscope_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msearray_xscope_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msearray_xscope_ss_const_iterator_type().set_to_previous(); }
				xscope_const_iterator& operator ++() { msearray_xscope_ss_const_iterator_type().operator ++(); return (*this); }
				xscope_const_iterator operator++(int) { xscope_const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_const_iterator& operator --() { msearray_xscope_ss_const_iterator_type().operator --(); return (*this); }
				xscope_const_iterator operator--(int) { xscope_const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { msearray_xscope_ss_const_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { msearray_xscope_ss_const_iterator_type().regress(n); }
				xscope_const_iterator& operator +=(difference_type n) { msearray_xscope_ss_const_iterator_type().operator +=(n); return (*this); }
				xscope_const_iterator& operator -=(difference_type n) { msearray_xscope_ss_const_iterator_type().operator -=(n); return (*this); }
				xscope_const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const xscope_const_iterator& _Right_cref) const { return msearray_xscope_ss_const_iterator_type() - (_Right_cref.msearray_xscope_ss_const_iterator_type()); }
				typename _MA::const_reference operator*() const { return msearray_xscope_ss_const_iterator_type().operator*(); }
				typename _MA::const_reference item() const { return operator*(); }
				typename _MA::const_reference previous_item() const { return msearray_xscope_ss_const_iterator_type().previous_item(); }
				typename _MA::const_pointer operator->() const { return msearray_xscope_ss_const_iterator_type().operator->(); }
				typename _MA::const_reference operator[](typename _MA::difference_type _Off) const { return msearray_xscope_ss_const_iterator_type()[_Off]; }
				xscope_const_iterator& operator=(const xscope_const_iterator& _Right_cref) {
					msearray_xscope_ss_const_iterator_type().operator=(_Right_cref.msearray_xscope_ss_const_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const xscope_iterator& _Right_cref) {
					msearray_xscope_ss_const_iterator_type().operator=(_Right_cref.msearray_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const const_iterator& _Right_cref) {
					msearray_xscope_ss_const_iterator_type().operator=(_Right_cref.msearray_ss_const_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const iterator& _Right_cref) {
					msearray_xscope_ss_const_iterator_type().operator=(_Right_cref.msearray_ss_iterator_type());
					return (*this);
				}
				bool operator==(const xscope_const_iterator& _Right_cref) const { return msearray_xscope_ss_const_iterator_type().operator==(_Right_cref.msearray_xscope_ss_const_iterator_type()); }
				bool operator!=(const xscope_const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_const_iterator& _Right) const { return (msearray_xscope_ss_const_iterator_type() < _Right.msearray_xscope_ss_const_iterator_type()); }
				bool operator<=(const xscope_const_iterator& _Right) const { return (msearray_xscope_ss_const_iterator_type() <= _Right.msearray_xscope_ss_const_iterator_type()); }
				bool operator>(const xscope_const_iterator& _Right) const { return (msearray_xscope_ss_const_iterator_type() > _Right.msearray_xscope_ss_const_iterator_type()); }
				bool operator>=(const xscope_const_iterator& _Right) const { return (msearray_xscope_ss_const_iterator_type() >= _Right.msearray_xscope_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const xscope_const_iterator& _Right_cref) { msearray_xscope_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msearray_xscope_ss_const_iterator_type()); }
				msear_size_t position() const { return msearray_xscope_ss_const_iterator_type().position(); }
				void xscope_iterator_tag() const {}
			private:
				typename _MA::xscope_ss_const_iterator_type m_xscope_ss_const_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
			};
			class xscope_iterator {
			public:
				typedef typename _MA::xscope_ss_iterator_type::iterator_category iterator_category;
				typedef typename _MA::xscope_ss_iterator_type::value_type value_type;
				typedef typename _MA::xscope_ss_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::xscope_ss_iterator_type::pointer pointer;
				typedef typename _MA::xscope_ss_iterator_type::reference reference;

				xscope_iterator(const mse::TXScopeFixedPointer<array>& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscopeweak(_MA_ref((*owner_ptr).m_msearray), owner_ptr)) {}
				template <class _TLeasePointerType>
				xscope_iterator(const mse::TXScopeWeakFixedPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscopeweak(_MA_ref((*owner_ptr).m_msearray), owner_ptr.lease_pointer())) {}

				xscope_iterator(const xscope_iterator& src_cref) : m_xscope_ss_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_iterator() {}
				const typename _MA::xscope_ss_iterator_type& msearray_xscope_ss_iterator_type() const {
					return m_xscope_ss_iterator;
				}
				typename _MA::xscope_ss_iterator_type& msearray_xscope_ss_iterator_type() {
					return m_xscope_ss_iterator;
				}
				const typename _MA::xscope_ss_iterator_type& mvssi() const { return msearray_xscope_ss_iterator_type(); }
				typename _MA::xscope_ss_iterator_type& mvssi() { return msearray_xscope_ss_iterator_type(); }

				void reset() { msearray_xscope_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msearray_xscope_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msearray_xscope_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msearray_xscope_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msearray_xscope_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msearray_xscope_ss_iterator_type().has_next(); }
				bool has_previous() const { return msearray_xscope_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msearray_xscope_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msearray_xscope_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msearray_xscope_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msearray_xscope_ss_iterator_type().set_to_previous(); }
				xscope_iterator& operator ++() { msearray_xscope_ss_iterator_type().operator ++(); return (*this); }
				xscope_iterator operator++(int) { xscope_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_iterator& operator --() { msearray_xscope_ss_iterator_type().operator --(); return (*this); }
				xscope_iterator operator--(int) { xscope_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { msearray_xscope_ss_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { msearray_xscope_ss_iterator_type().regress(n); }
				xscope_iterator& operator +=(difference_type n) { msearray_xscope_ss_iterator_type().operator +=(n); return (*this); }
				xscope_iterator& operator -=(difference_type n) { msearray_xscope_ss_iterator_type().operator -=(n); return (*this); }
				xscope_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const xscope_iterator& _Right_cref) const { return msearray_xscope_ss_iterator_type() - (_Right_cref.msearray_xscope_ss_iterator_type()); }
				typename _MA::reference operator*() const { return msearray_xscope_ss_iterator_type().operator*(); }
				typename _MA::reference item() const { return operator*(); }
				typename _MA::reference previous_item() const { return msearray_xscope_ss_iterator_type().previous_item(); }
				typename _MA::pointer operator->() const { return msearray_xscope_ss_iterator_type().operator->(); }
				typename _MA::reference operator[](typename _MA::difference_type _Off) const { return msearray_xscope_ss_iterator_type()[_Off]; }
				xscope_iterator& operator=(const xscope_iterator& _Right_cref) {
					msearray_xscope_ss_iterator_type().operator=(_Right_cref.msearray_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_iterator& operator=(const iterator& _Right_cref) {
					msearray_xscope_ss_iterator_type().operator=(_Right_cref.msearray_ss_iterator_type());
					return (*this);
				}
				bool operator==(const xscope_iterator& _Right_cref) const { return msearray_xscope_ss_iterator_type().operator==(_Right_cref.msearray_xscope_ss_iterator_type()); }
				bool operator!=(const xscope_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_iterator& _Right) const { return (msearray_xscope_ss_iterator_type() < _Right.msearray_xscope_ss_iterator_type()); }
				bool operator<=(const xscope_iterator& _Right) const { return (msearray_xscope_ss_iterator_type() <= _Right.msearray_xscope_ss_iterator_type()); }
				bool operator>(const xscope_iterator& _Right) const { return (msearray_xscope_ss_iterator_type() > _Right.msearray_xscope_ss_iterator_type()); }
				bool operator>=(const xscope_iterator& _Right) const { return (msearray_xscope_ss_iterator_type() >= _Right.msearray_xscope_ss_iterator_type()); }
				void set_to_item_pointer(const xscope_iterator& _Right_cref) { msearray_xscope_ss_iterator_type().set_to_item_pointer(_Right_cref.msearray_xscope_ss_iterator_type()); }
				msear_size_t position() const { return msearray_xscope_ss_iterator_type().position(); }
				void xscope_iterator_tag() const {}
			private:
				typename _MA::xscope_ss_iterator_type m_xscope_ss_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
				friend class xscope_const_iterator;
			};

		private:
			mse::TRegisteredObj<_MA> m_msearray;

			template<size_t _Idx, class _Tz, size_t _Size2>
			friend _CONST_FUN _Tz& std::get(mse::mstd::array<_Tz, _Size2>& _Arr) _NOEXCEPT;
			template<size_t _Idx, class _Tz, size_t _Size2>
			friend _CONST_FUN const _Tz& std::get(const mse::mstd::array<_Tz, _Size2>& _Arr) _NOEXCEPT;
			template<size_t _Idx, class _Tz, size_t _Size2>
			friend _CONST_FUN _Tz&& std::get(mse::mstd::array<_Tz, _Size2>&& _Arr) _NOEXCEPT;
		};

		template<class _Ty, size_t _Size> inline bool operator!=(const array<_Ty, _Size>& _Left,
			const array<_Ty, _Size>& _Right) {	// test for array inequality
			return (!(_Left == _Right));
		}

		template<class _Ty, size_t _Size> inline bool operator>(const array<_Ty, _Size>& _Left,
			const array<_Ty, _Size>& _Right) {	// test if _Left > _Right for arrays
			return (_Right < _Left);
		}

		template<class _Ty, size_t _Size> inline bool operator<=(const array<_Ty, _Size>& _Left,
			const array<_Ty, _Size>& _Right) {	// test if _Left <= _Right for arrays
			return (!(_Right < _Left));
		}

		template<class _Ty, size_t _Size> inline bool operator>=(const array<_Ty, _Size>& _Left,
			const array<_Ty, _Size>& _Right) {	// test if _Left >= _Right for arrays
			return (!(_Left < _Right));
		}


		template<class _TArray> using xscope_const_iterator = typename _TArray::xscope_const_iterator;
		template<class _TArray>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeFixedConstPointer<_TArray>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_const_iterator<_TArray> make_xscope_const_iterator(const mse::TXScopeWeakFixedPointer<const _TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_const_iterator<_TArray>(owner_ptr);
		}

		template<class _TArray> using xscope_iterator = typename _TArray::xscope_iterator;
		template<class _TArray>
		xscope_iterator<_TArray> make_xscope_iterator(const mse::TXScopeFixedPointer<_TArray>& owner_ptr) {
			return xscope_iterator<_TArray>(owner_ptr);
		}
		template<class _TArray, class _TLeasePointerType>
		xscope_iterator<_TArray> make_xscope_iterator(const mse::TXScopeWeakFixedPointer<_TArray, _TLeasePointerType>& owner_ptr) {
			return xscope_iterator<_TArray>(owner_ptr);
		}


		/* These are specializations of the array_helper_type template class defined near the beginning of this file. */
		template<class _Ty> class array_helper_type<_Ty, 2> { public:
			static typename mse::msearray<_Ty, 2> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (2 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 2> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 3> { public:
			static typename mse::msearray<_Ty, 3> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (3 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 3> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 4> { public:
			static typename mse::msearray<_Ty, 4> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (4 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 4> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 5> { public:
			static typename mse::msearray<_Ty, 5> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (5 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 5> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 6> { public:
			static typename mse::msearray<_Ty, 6> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (6 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 6> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 7> { public:
			static typename mse::msearray<_Ty, 7> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (7 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 7> retval { *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 8> {
		public:
			static typename mse::msearray<_Ty, 8> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (8 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 8> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 9> {
		public:
			static typename mse::msearray<_Ty, 9> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (9 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 9> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 10> {
		public:
			static typename mse::msearray<_Ty, 10> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (10 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 10> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 11> {
		public:
			static typename mse::msearray<_Ty, 11> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (11 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 11> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 12> {
		public:
			static typename mse::msearray<_Ty, 12> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (12 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 12> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 13> {
		public:
			static typename mse::msearray<_Ty, 13> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (13 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 13> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 14> {
		public:
			static typename mse::msearray<_Ty, 14> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (14 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 14> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 15> {
		public:
			static typename mse::msearray<_Ty, 15> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (15 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 15> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 16> {
		public:
			static typename mse::msearray<_Ty, 16> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (16 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 16> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14), *(_Ilist.begin() + 15) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 17> {
		public:
			static typename mse::msearray<_Ty, 17> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (17 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 17> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14), *(_Ilist.begin() + 15), *(_Ilist.begin() + 16) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 18> {
		public:
			static typename mse::msearray<_Ty, 18> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (18 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 18> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14), *(_Ilist.begin() + 15), *(_Ilist.begin() + 16), *(_Ilist.begin() + 17) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 19> {
		public:
			static typename mse::msearray<_Ty, 19> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (19 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 19> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14), *(_Ilist.begin() + 15), *(_Ilist.begin() + 16), *(_Ilist.begin() + 17), *(_Ilist.begin() + 18) }; return retval;
			}
		};
		template<class _Ty> class array_helper_type<_Ty, 20> {
		public:
			static typename mse::msearray<_Ty, 20> msearray_initial_value2(_XSTD initializer_list<_Ty> _Ilist) {
				if (20 != _Ilist.size()) { MSE_THROW(mstdarray_range_error("the size of the initializer list does not match the size of the array - mse::mstd::array")); }
				typename mse::msearray<_Ty, 20> retval{ *(_Ilist.begin()), *(_Ilist.begin() + 1), *(_Ilist.begin() + 2), *(_Ilist.begin() + 3), *(_Ilist.begin() + 4), *(_Ilist.begin() + 5), *(_Ilist.begin() + 6), *(_Ilist.begin() + 7), *(_Ilist.begin() + 8), *(_Ilist.begin() + 9), *(_Ilist.begin() + 10), *(_Ilist.begin() + 11), *(_Ilist.begin() + 12), *(_Ilist.begin() + 13), *(_Ilist.begin() + 14), *(_Ilist.begin() + 15), *(_Ilist.begin() + 16), *(_Ilist.begin() + 17), *(_Ilist.begin() + 18), *(_Ilist.begin() + 19) }; return retval;
			}
		};
#endif /*MSE_MSTDARRAY_DISABLED*/
	}
}

#ifndef MSE_MSTDARRAY_DISABLED
namespace std {

	template<class _Ty, size_t _Size>
	struct tuple_size<mse::mstd::array<_Ty, _Size> >
		: integral_constant<size_t, _Size>
	{	// struct to determine number of elements in array
	};

	template<size_t _Idx, class _Ty, size_t _Size>
	struct tuple_element<_Idx, mse::mstd::array<_Ty, _Size> >
	{	// struct to determine type of element _Idx in array
		static_assert(_Idx < _Size, "array index out of bounds");

		typedef _Ty type;
	};

	// TUPLE INTERFACE TO array
	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty& get(mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_msearray));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
		_CONST_FUN const _Ty& get(const mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_msearray));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty&& get(mse::mstd::array<_Ty, _Size>&& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (_STD move(std::get<_Idx>(_Arr.m_msearray)));
	}
}
#endif // !MSE_MSTDARRAY_DISABLED

namespace mse {
	namespace mstd {

		class array_test {
		public:
			void test1() {
#ifdef MSE_SELF_TESTS
				// construction uses aggregate initialization
				mse::mstd::array<int, 3> a1{ { 1, 2, 3 } }; // double-braces required in C++11 (not in C++14)
				mse::mstd::array<int, 3> a2 = { 11, 12, 13 };  // never required after =
				mse::mstd::array<std::string, 2> a3 = { std::string("a"), "b" };

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

				auto l_tuple_size = std::tuple_size<mse::mstd::array<int, 3>>::value;
				std::tuple_element<1, mse::mstd::array<int, 3>>::type b1 = 5;

				a1 = a2;

				{
					mse::mstd::array<int, 5> a = { 10, 20, 30, 40, 50 };
					mse::mstd::array<int, 5> b = { 10, 20, 30, 40, 50 };
					mse::mstd::array<int, 5> c = { 50, 40, 30, 20, 10 };

					if (a == b) std::cout << "a and b are equal\n";
					if (b != c) std::cout << "b and c are not equal\n";
					if (b<c) std::cout << "b is less than c\n";
					if (c>b) std::cout << "c is greater than b\n";
					if (a <= b) std::cout << "a is less than or equal to b\n";
					if (a >= b) std::cout << "a is greater than or equal to b\n";
				}
				{
					mse::mstd::array<int, 6> myarray;

					myarray.fill(5);

					std::cout << "myarray contains:";
					for (int& x : myarray) { std::cout << ' ' << x; }

					std::cout << '\n';
				}
				{
					/* arrays of non-default constructible elements */
					class A {
					public:
						A(int i) : m_i(i) {}
						int m_i;
					};
					int z1 = 7;
					std::array<A, 3> sa1{z1, A(8), 9};
					mse::mstd::array<A, 3> mstda1{ z1, A(8), 9 };
				}
#endif // MSE_SELF_TESTS
			}
		};
	}
}

#undef MSE_THROW

#endif /*ndef MSEMSTDARRAY_H*/
