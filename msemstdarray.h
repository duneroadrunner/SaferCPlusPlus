
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

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 )
#endif /*_MSC_VER*/

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDARRAY_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

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

		template<class _Ty, size_t _Size >
		class array {
		public:
			typedef mse::mstd::array<_Ty, _Size> _Myt;
			typedef mse::nii_array<_Ty, _Size> _MA;

			typedef typename _MA::value_type value_type;
			typedef typename _MA::size_type size_type;
			typedef typename _MA::difference_type difference_type;
			typedef typename _MA::pointer pointer;
			typedef typename _MA::const_pointer const_pointer;
			typedef typename _MA::reference reference;
			typedef typename _MA::const_reference const_reference;

			const _MA& nii_array() const { return m_nii_array; }
			_MA& nii_array() { return m_nii_array; }
			operator const _MA() const { return nii_array(); }
			operator _MA() { return nii_array(); }

			array() {}
			array(_MA&& _X) : m_nii_array(std::forward<decltype(_X)>(_X)) {}
			array(const _MA& _X) : m_nii_array(_X) {}
			array(_Myt&& _X) : m_nii_array(std::forward<decltype(_X.nii_array())>(_X.nii_array())) {}
			array(const _Myt& _X) : m_nii_array(_X.nii_array()) {}
			//array(_XSTD initializer_list<typename _MA::base_class::value_type> _Ilist) : m_nii_array(_Ilist) {}
			static std::array<_Ty, _Size> std_array_initial_value(std::true_type, _XSTD initializer_list<_Ty> _Ilist) {
				/* _Ty is default constructible. */
				std::array<_Ty, _Size> retval;
				assert(_Size >= _Ilist.size());
				auto stop_size = _Size;
				if (_Size > _Ilist.size()) {
					stop_size = _Ilist.size();
					/* just to make sure that all the retval elements are initialized as if by aggregate initialization. */
					retval = std::array<_Ty, _Size>{};
				}
				msear_size_t count = 0;
				auto Il_it = _Ilist.begin();
				auto target_it = retval.begin();
				for (; (count < stop_size); Il_it++, count += 1, target_it++) {
					(*target_it) = (*Il_it);
				}
				return retval;
			}
			static std::array<_Ty, _Size> std_array_initial_value(std::false_type, _XSTD initializer_list<_Ty> _Ilist) {
				/* _Ty is not default constructible. */
				return array_helper_type<_Ty, _Size>::std_array_initial_value2(_Ilist);
			}
			array(_XSTD initializer_list<_Ty> _Ilist) : m_nii_array(_MA{ std_array_initial_value(std::is_default_constructible<_Ty>(), _Ilist) }) {
				/* std::array<> is an "aggregate type" (basically a POD struct with no base class, constructors or private
				data members (details here: http://en.cppreference.com/w/cpp/language/aggregate_initialization)). As such,
				support for construction from initializer list is automatically generated by the compiler. Specifically,
				aggregate types support "aggregate initialization". But since mstd::array has a member with an explicitly
				defined constructor (or at least I think that's why), it is not an aggregate type and therefore doesn't
				qualify to have support for "aggregate initialization" automatically generated by the compiler. It doesn't
				seem possible to emulate full aggregate initialization compatibility, so we'll just have to do the best we
				can. */
			}

			_Myt& operator=(_MA&& _X) { m_nii_array.operator=(std::forward<decltype(_X)>(_X)); return (*this); }
			_Myt& operator=(const _MA& _X) { m_nii_array.operator=(_X); return (*this); }
			_Myt& operator=(_Myt&& _X) { m_nii_array.operator=(std::forward<decltype(_X.nii_array())>(_X.nii_array())); return (*this); }
			_Myt& operator=(const _Myt& _X) { m_nii_array.operator=(_X.nii_array()); return (*this); }
			_CONST_FUN typename _MA::const_reference operator[](size_type _P) const { return m_nii_array.operator[](_P); }
			typename _MA::reference operator[](size_type _P) { return m_nii_array.operator[](_P); }
			void fill(const _Ty& _Value) { m_nii_array.fill(_Value); }
			void swap(_MA& _X) { m_nii_array.swap(_X); }
			void swap(_Myt& _X) { m_nii_array.swap(_X.nii_array()); }

			_CONST_FUN size_type size() const _NOEXCEPT { return m_nii_array.size(); }
			_CONST_FUN size_type max_size() const _NOEXCEPT { return m_nii_array.max_size(); }
			_CONST_FUN bool empty() const _NOEXCEPT { return m_nii_array.empty(); }
			_CONST_FUN typename _MA::const_reference at(size_type _Pos) const { return m_nii_array.at(_Pos); }
			typename _MA::reference at(size_type _Pos) { return m_nii_array.at(_Pos); }
			typename _MA::reference front() { return m_nii_array.front(); }
			_CONST_FUN typename _MA::const_reference front() const { return m_nii_array.front(); }
			typename _MA::reference back() { return m_nii_array.back(); }
			_CONST_FUN typename _MA::const_reference back() const { return m_nii_array.back(); }

			/* Try to avoid using these whenever possible. */
			value_type *data() _NOEXCEPT {	// return pointer to mutable data array
				return m_nii_array.data();
			}
			const value_type *data() const _NOEXCEPT {	// return pointer to nonmutable data array
				return m_nii_array.data();
			}

			array(std::array<_Ty, _Size>&& _X) {
				m_nii_array.m_array = std::forward<decltype(_X)>(_X);
			}
			array(const std::array<_Ty, _Size>& _X) {
				m_nii_array.m_array = _X;
			}
			operator const std::array<_Ty, _Size>() const { return nii_array(); }
			operator std::array<_Ty, _Size>() { return nii_array(); }


			class reg_ss_iterator_type : public _MA::template Tss_iterator_type<mse::TRegisteredPointer<_MA>> {
			public:
				typedef typename _MA::template Tss_iterator_type<mse::TRegisteredPointer<_MA>> base_class;
				MSE_USING(reg_ss_iterator_type, base_class);
				reg_ss_iterator_type(const base_class& src) : base_class(src) {}

				friend class reg_ss_const_iterator_type;
			};
			class reg_ss_const_iterator_type : public _MA::template Tss_const_iterator_type<mse::TRegisteredConstPointer<_MA>> {
			public:
				typedef typename _MA::template Tss_const_iterator_type<mse::TRegisteredConstPointer<_MA>> base_class;
				MSE_USING(reg_ss_const_iterator_type, base_class);
				reg_ss_const_iterator_type(const base_class& src) : base_class(src) {}
				reg_ss_const_iterator_type(const reg_ss_iterator_type& src) {
					(*this).m_owner_cptr = src.m_owner_ptr;
					(*this).m_index = src.m_index;
				}
			};

			class xscope_const_iterator;
			class xscope_iterator;

			class const_iterator : public _MA::random_access_const_iterator_base {
			public:
				typedef typename reg_ss_const_iterator_type::iterator_category iterator_category;
				typedef typename reg_ss_const_iterator_type::value_type value_type;
				typedef typename reg_ss_const_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename reg_ss_const_iterator_type::pointer pointer;
				typedef typename reg_ss_const_iterator_type::reference reference;

				const_iterator() {}
				const_iterator(const const_iterator& src_cref) = default;
				~const_iterator() {}
				const reg_ss_const_iterator_type& nii_array_reg_ss_const_iterator_type() const {
					//if (!m_nii_array_regcptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::const_iterator")); }
					return m_reg_ss_const_iterator;
				}
				reg_ss_const_iterator_type& nii_array_reg_ss_const_iterator_type() {
					//if (!m_nii_array_regcptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::const_iterator")); }
					return m_reg_ss_const_iterator;
				}
				const reg_ss_const_iterator_type& mvssci() const { return nii_array_reg_ss_const_iterator_type(); }
				reg_ss_const_iterator_type& mvssci() { return nii_array_reg_ss_const_iterator_type(); }

				void reset() { nii_array_reg_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return nii_array_reg_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return nii_array_reg_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return nii_array_reg_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return nii_array_reg_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return nii_array_reg_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return nii_array_reg_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { nii_array_reg_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { nii_array_reg_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { nii_array_reg_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { nii_array_reg_ss_const_iterator_type().set_to_previous(); }
				const_iterator& operator ++() { nii_array_reg_ss_const_iterator_type().operator ++(); return (*this); }
				const_iterator operator++(int) { const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				const_iterator& operator --() { nii_array_reg_ss_const_iterator_type().operator --(); return (*this); }
				const_iterator operator--(int) { const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { nii_array_reg_ss_const_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { nii_array_reg_ss_const_iterator_type().regress(n); }
				const_iterator& operator +=(difference_type n) { nii_array_reg_ss_const_iterator_type().operator +=(n); return (*this); }
				const_iterator& operator -=(difference_type n) { nii_array_reg_ss_const_iterator_type().operator -=(n); return (*this); }
				const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const const_iterator& _Right_cref) const { return nii_array_reg_ss_const_iterator_type() - (_Right_cref.nii_array_reg_ss_const_iterator_type()); }
				typename _MA::const_reference operator*() const { return nii_array_reg_ss_const_iterator_type().operator*(); }
				typename _MA::const_reference item() const { return operator*(); }
				typename _MA::const_reference previous_item() const { return nii_array_reg_ss_const_iterator_type().previous_item(); }
				typename _MA::const_pointer operator->() const { return nii_array_reg_ss_const_iterator_type().operator->(); }
				typename _MA::const_reference operator[](typename _MA::difference_type _Off) const { return nii_array_reg_ss_const_iterator_type()[_Off]; }
				bool operator==(const const_iterator& _Right_cref) const { return nii_array_reg_ss_const_iterator_type().operator==(_Right_cref.nii_array_reg_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (nii_array_reg_ss_const_iterator_type() < _Right.nii_array_reg_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (nii_array_reg_ss_const_iterator_type() <= _Right.nii_array_reg_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (nii_array_reg_ss_const_iterator_type() > _Right.nii_array_reg_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (nii_array_reg_ss_const_iterator_type() >= _Right.nii_array_reg_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { nii_array_reg_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.nii_array_reg_ss_const_iterator_type()); }
				msear_size_t position() const { return nii_array_reg_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(nii_array_reg_ss_const_iterator_type().target_container_ptr()) {
					return nii_array_reg_ss_const_iterator_type().target_container_ptr();
				}
			private:
				const_iterator(mse::TRegisteredConstPointer<_MA> nii_array_regcptr) {
					if (nii_array_regcptr) {
						m_reg_ss_const_iterator = _MA::template ss_cbegin<mse::TRegisteredConstPointer<_MA>>(nii_array_regcptr);
					}
				}
				reg_ss_const_iterator_type m_reg_ss_const_iterator;

				friend class /*_Myt*/array<_Ty, _Size>;
				friend class iterator;
				friend class xscope_const_iterator;
			};
			class iterator : public _MA::random_access_iterator_base {
			public:
				typedef typename reg_ss_iterator_type::iterator_category iterator_category;
				typedef typename reg_ss_iterator_type::value_type value_type;
				typedef typename reg_ss_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename reg_ss_iterator_type::pointer pointer;
				typedef typename reg_ss_iterator_type::reference reference;

				iterator() {}
				iterator(const iterator& src_cref) = default;
				~iterator() {}
				const reg_ss_iterator_type& nii_array_reg_ss_iterator_type() const {
					//if (!m_nii_array_regptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::iterator")); }
					return m_reg_ss_iterator;
				}
				reg_ss_iterator_type& nii_array_reg_ss_iterator_type() {
					//if (!m_nii_array_regptr) { MSE_THROW(mstdarray_range_error("attempt to use an invalid iterator - mse::mstd::array<>::iterator")); }
					return m_reg_ss_iterator;
				}
				const reg_ss_iterator_type& mvssi() const { return nii_array_reg_ss_iterator_type(); }
				reg_ss_iterator_type& mvssi() { return nii_array_reg_ss_iterator_type(); }
				operator const_iterator() const {
					auto nii_array_regptr = m_reg_ss_iterator.target_container_ptr();
					const_iterator retval(nii_array_regptr);
					if (nii_array_regptr) {
						retval.nii_array_reg_ss_const_iterator_type().set_to_beginning();
						retval.nii_array_reg_ss_const_iterator_type().advance(msear_int(nii_array_reg_ss_iterator_type().position()));
					}
					return retval;
				}

				void reset() { nii_array_reg_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return nii_array_reg_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return nii_array_reg_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return nii_array_reg_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return nii_array_reg_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return nii_array_reg_ss_iterator_type().has_next(); }
				bool has_previous() const { return nii_array_reg_ss_iterator_type().has_previous(); }
				void set_to_beginning() { nii_array_reg_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { nii_array_reg_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { nii_array_reg_ss_iterator_type().set_to_next(); }
				void set_to_previous() { nii_array_reg_ss_iterator_type().set_to_previous(); }
				iterator& operator ++() { nii_array_reg_ss_iterator_type().operator ++(); return (*this); }
				iterator operator++(int) { iterator _Tmp = *this; ++*this; return (_Tmp); }
				iterator& operator --() { nii_array_reg_ss_iterator_type().operator --(); return (*this); }
				iterator operator--(int) { iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { nii_array_reg_ss_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { nii_array_reg_ss_iterator_type().regress(n); }
				iterator& operator +=(difference_type n) { nii_array_reg_ss_iterator_type().operator +=(n); return (*this); }
				iterator& operator -=(difference_type n) { nii_array_reg_ss_iterator_type().operator -=(n); return (*this); }
				iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const iterator& _Right_cref) const { return nii_array_reg_ss_iterator_type() - (_Right_cref.nii_array_reg_ss_iterator_type()); }
				typename _MA::reference operator*() const { return nii_array_reg_ss_iterator_type().operator*(); }
				typename _MA::reference item() const { return operator*(); }
				typename _MA::reference previous_item() const { return nii_array_reg_ss_iterator_type().previous_item(); }
				typename _MA::pointer operator->() const { return nii_array_reg_ss_iterator_type().operator->(); }
				typename _MA::reference operator[](typename _MA::difference_type _Off) const { return nii_array_reg_ss_iterator_type()[_Off]; }
				bool operator==(const iterator& _Right_cref) const { return nii_array_reg_ss_iterator_type().operator==(_Right_cref.nii_array_reg_ss_iterator_type()); }
				bool operator!=(const iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const iterator& _Right) const { return (nii_array_reg_ss_iterator_type() < _Right.nii_array_reg_ss_iterator_type()); }
				bool operator<=(const iterator& _Right) const { return (nii_array_reg_ss_iterator_type() <= _Right.nii_array_reg_ss_iterator_type()); }
				bool operator>(const iterator& _Right) const { return (nii_array_reg_ss_iterator_type() > _Right.nii_array_reg_ss_iterator_type()); }
				bool operator>=(const iterator& _Right) const { return (nii_array_reg_ss_iterator_type() >= _Right.nii_array_reg_ss_iterator_type()); }
				void set_to_item_pointer(const iterator& _Right_cref) { nii_array_reg_ss_iterator_type().set_to_item_pointer(_Right_cref.nii_array_reg_ss_iterator_type()); }
				msear_size_t position() const { return nii_array_reg_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(nii_array_reg_ss_iterator_type().target_container_ptr()) {
					return nii_array_reg_ss_iterator_type().target_container_ptr();
				}
			private:
				reg_ss_iterator_type m_reg_ss_iterator;

				friend class /*_Myt*/array<_Ty, _Size>;
				friend class xscope_const_iterator;
				friend class xscope_iterator;
			};

			iterator begin()
			{	// return iterator for beginning of mutable sequence
				iterator retval; //retval.m_nii_array_regptr = &(this->m_nii_array);
				(retval.m_reg_ss_iterator) = m_nii_array.template ss_begin<mse::TRegisteredPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}

			const_iterator begin() const
			{	// return iterator for beginning of nonmutable sequence
				const_iterator retval; //retval.m_nii_array_regcptr = &(this->m_nii_array);
				(retval.m_reg_ss_const_iterator) = m_nii_array.template ss_cbegin<mse::TRegisteredConstPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}

			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; //retval.m_nii_array_regptr = &(this->m_nii_array);
				(retval.m_reg_ss_iterator) = m_nii_array.template ss_end<mse::TRegisteredPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; //retval.m_nii_array_regcptr = &(this->m_nii_array);
				(retval.m_reg_ss_const_iterator) = m_nii_array.template ss_cend<mse::TRegisteredConstPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; //retval.m_nii_array_regcptr = &(this->m_nii_array);
				(retval.m_reg_ss_const_iterator) = m_nii_array.template ss_cbegin<mse::TRegisteredConstPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; //retval.m_nii_array_regcptr = &(this->m_nii_array);
				(retval.m_reg_ss_const_iterator) = m_nii_array.template ss_cend<mse::TRegisteredConstPointer<_MA>>(&(this->m_nii_array));
				return retval;
			}

			typedef std::reverse_iterator<iterator> reverse_iterator;
			typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

			reverse_iterator rbegin() {	// return iterator for beginning of reversed mutable sequence
				return (reverse_iterator(end()));
			}
			const_reverse_iterator rbegin() const {	// return iterator for beginning of reversed nonmutable sequence
				return (const_reverse_iterator(end()));
			}
			reverse_iterator rend() {	// return iterator for end of reversed mutable sequence
				return (reverse_iterator(begin()));
			}
			const_reverse_iterator rend() const {	// return iterator for end of reversed nonmutable sequence
				return (const_reverse_iterator(begin()));
			}
			const_reverse_iterator crbegin() const {	// return iterator for beginning of reversed nonmutable sequence
				return (rbegin());
			}
			const_reverse_iterator crend() const {	// return iterator for end of reversed nonmutable sequence
				return (rend());
			}


			bool operator==(const _Myt& _Right) const {	// test for array equality
				return (_Right.m_nii_array == m_nii_array);
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for arrays
				return (m_nii_array < _Right.m_nii_array);
			}

			/* These static functions are just used to obtain a (base class) reference to an
			object of a (possibly) derived class. */
			static _MA& _MA_ref(_MA& obj) { return obj; }
			static const _MA& _MA_cref(const _MA& obj) { return obj; }

			class xscope_const_iterator : public _MA::random_access_iterator_base, public XScopeTagBase {
			public:
				typedef typename _MA::xscope_ss_const_iterator_type::iterator_category iterator_category;
				typedef typename _MA::xscope_ss_const_iterator_type::value_type value_type;
				typedef typename _MA::xscope_ss_const_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::xscope_ss_const_iterator_type::pointer pointer;
				typedef typename _MA::xscope_ss_const_iterator_type::reference reference;

				xscope_const_iterator(const mse::TXScopeFixedConstPointer<array>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr)) {}
				xscope_const_iterator(const mse::TXScopeFixedPointer<array>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr)) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedConstPointer<const array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr.lease_pointer())) {}
				template <class _TLeasePointerType>
				xscope_const_iterator(const mse::TXScopeWeakFixedPointer<const array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscopeweak(_MA_cref((*owner_ptr).m_nii_array), owner_ptr.lease_pointer())) {}

				xscope_const_iterator(const xscope_const_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_const_iterator) {}
				xscope_const_iterator(const xscope_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_const_iterator() {}
				const typename _MA::xscope_ss_const_iterator_type& nii_array_xscope_ss_const_iterator_type() const {
					return m_xscope_ss_const_iterator;
				}
				typename _MA::xscope_ss_const_iterator_type& nii_array_xscope_ss_const_iterator_type() {
					return m_xscope_ss_const_iterator;
				}
				const typename _MA::xscope_ss_const_iterator_type& mvssci() const { return nii_array_xscope_ss_const_iterator_type(); }
				typename _MA::xscope_ss_const_iterator_type& mvssci() { return nii_array_xscope_ss_const_iterator_type(); }

				void reset() { nii_array_xscope_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return nii_array_xscope_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return nii_array_xscope_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return nii_array_xscope_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return nii_array_xscope_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return nii_array_xscope_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return nii_array_xscope_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { nii_array_xscope_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { nii_array_xscope_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { nii_array_xscope_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { nii_array_xscope_ss_const_iterator_type().set_to_previous(); }
				xscope_const_iterator& operator ++() { nii_array_xscope_ss_const_iterator_type().operator ++(); return (*this); }
				xscope_const_iterator operator++(int) { xscope_const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_const_iterator& operator --() { nii_array_xscope_ss_const_iterator_type().operator --(); return (*this); }
				xscope_const_iterator operator--(int) { xscope_const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { nii_array_xscope_ss_const_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { nii_array_xscope_ss_const_iterator_type().regress(n); }
				xscope_const_iterator& operator +=(difference_type n) { nii_array_xscope_ss_const_iterator_type().operator +=(n); return (*this); }
				xscope_const_iterator& operator -=(difference_type n) { nii_array_xscope_ss_const_iterator_type().operator -=(n); return (*this); }
				xscope_const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const xscope_const_iterator& _Right_cref) const { return nii_array_xscope_ss_const_iterator_type() - (_Right_cref.nii_array_xscope_ss_const_iterator_type()); }
				typename _MA::const_reference operator*() const { return nii_array_xscope_ss_const_iterator_type().operator*(); }
				typename _MA::const_reference item() const { return operator*(); }
				typename _MA::const_reference previous_item() const { return nii_array_xscope_ss_const_iterator_type().previous_item(); }
				typename _MA::const_pointer operator->() const { return nii_array_xscope_ss_const_iterator_type().operator->(); }
				typename _MA::const_reference operator[](typename _MA::difference_type _Off) const { return nii_array_xscope_ss_const_iterator_type()[_Off]; }
				xscope_const_iterator& operator=(const xscope_const_iterator& _Right_cref) {
					nii_array_xscope_ss_const_iterator_type().operator=(_Right_cref.nii_array_xscope_ss_const_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const xscope_iterator& _Right_cref) {
					nii_array_xscope_ss_const_iterator_type().operator=(_Right_cref.nii_array_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::const_iterator& _Right_cref) {
					//nii_array_xscope_ss_const_iterator_type().operator=(_Right_cref.nii_array_reg_ss_const_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) { 
						MSE_THROW(mstdarray_range_error("invalid assignment - mse::mstd::array<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//nii_array_xscope_ss_const_iterator_type().operator=(_Right_cref.nii_array_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdarray_range_error("invalid assignment - mse::mstd::array<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_const_iterator& _Right_cref) const { return nii_array_xscope_ss_const_iterator_type().operator==(_Right_cref.nii_array_xscope_ss_const_iterator_type()); }
				bool operator!=(const xscope_const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_const_iterator& _Right) const { return (nii_array_xscope_ss_const_iterator_type() < _Right.nii_array_xscope_ss_const_iterator_type()); }
				bool operator<=(const xscope_const_iterator& _Right) const { return (nii_array_xscope_ss_const_iterator_type() <= _Right.nii_array_xscope_ss_const_iterator_type()); }
				bool operator>(const xscope_const_iterator& _Right) const { return (nii_array_xscope_ss_const_iterator_type() > _Right.nii_array_xscope_ss_const_iterator_type()); }
				bool operator>=(const xscope_const_iterator& _Right) const { return (nii_array_xscope_ss_const_iterator_type() >= _Right.nii_array_xscope_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const xscope_const_iterator& _Right_cref) { nii_array_xscope_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.nii_array_xscope_ss_const_iterator_type()); }
				msear_size_t position() const { return nii_array_xscope_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(nii_array_xscope_ss_const_iterator_type().target_container_ptr()) {
					return nii_array_xscope_ss_const_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
			private:
				typename _MA::xscope_ss_const_iterator_type m_xscope_ss_const_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
			};
			class xscope_iterator : public _MA::random_access_iterator_base, public XScopeTagBase {
			public:
				typedef typename _MA::xscope_ss_iterator_type::iterator_category iterator_category;
				typedef typename _MA::xscope_ss_iterator_type::value_type value_type;
				typedef typename _MA::xscope_ss_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::xscope_ss_iterator_type::pointer pointer;
				typedef typename _MA::xscope_ss_iterator_type::reference reference;

				xscope_iterator(const mse::TXScopeFixedPointer<array>& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscopeweak(_MA_ref((*owner_ptr).m_nii_array), owner_ptr)) {}
				template <class _TLeasePointerType>
				xscope_iterator(const mse::TXScopeWeakFixedPointer<array, _TLeasePointerType>& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscopeweak(_MA_ref((*owner_ptr).m_nii_array), owner_ptr.lease_pointer())) {}

				xscope_iterator(const xscope_iterator& src_cref) : m_xscope_ss_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_iterator() {}
				const typename _MA::xscope_ss_iterator_type& nii_array_xscope_ss_iterator_type() const {
					return m_xscope_ss_iterator;
				}
				typename _MA::xscope_ss_iterator_type& nii_array_xscope_ss_iterator_type() {
					return m_xscope_ss_iterator;
				}
				const typename _MA::xscope_ss_iterator_type& mvssi() const { return nii_array_xscope_ss_iterator_type(); }
				typename _MA::xscope_ss_iterator_type& mvssi() { return nii_array_xscope_ss_iterator_type(); }

				void reset() { nii_array_xscope_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return nii_array_xscope_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return nii_array_xscope_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return nii_array_xscope_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return nii_array_xscope_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return nii_array_xscope_ss_iterator_type().has_next(); }
				bool has_previous() const { return nii_array_xscope_ss_iterator_type().has_previous(); }
				void set_to_beginning() { nii_array_xscope_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { nii_array_xscope_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { nii_array_xscope_ss_iterator_type().set_to_next(); }
				void set_to_previous() { nii_array_xscope_ss_iterator_type().set_to_previous(); }
				xscope_iterator& operator ++() { nii_array_xscope_ss_iterator_type().operator ++(); return (*this); }
				xscope_iterator operator++(int) { xscope_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_iterator& operator --() { nii_array_xscope_ss_iterator_type().operator --(); return (*this); }
				xscope_iterator operator--(int) { xscope_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MA::difference_type n) { nii_array_xscope_ss_iterator_type().advance(n); }
				void regress(typename _MA::difference_type n) { nii_array_xscope_ss_iterator_type().regress(n); }
				xscope_iterator& operator +=(difference_type n) { nii_array_xscope_ss_iterator_type().operator +=(n); return (*this); }
				xscope_iterator& operator -=(difference_type n) { nii_array_xscope_ss_iterator_type().operator -=(n); return (*this); }
				xscope_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MA::difference_type operator-(const xscope_iterator& _Right_cref) const { return nii_array_xscope_ss_iterator_type() - (_Right_cref.nii_array_xscope_ss_iterator_type()); }
				typename _MA::reference operator*() const { return nii_array_xscope_ss_iterator_type().operator*(); }
				typename _MA::reference item() const { return operator*(); }
				typename _MA::reference previous_item() const { return nii_array_xscope_ss_iterator_type().previous_item(); }
				typename _MA::pointer operator->() const { return nii_array_xscope_ss_iterator_type().operator->(); }
				typename _MA::reference operator[](typename _MA::difference_type _Off) const { return nii_array_xscope_ss_iterator_type()[_Off]; }
				xscope_iterator& operator=(const xscope_iterator& _Right_cref) {
					nii_array_xscope_ss_iterator_type().operator=(_Right_cref.nii_array_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//nii_array_xscope_ss_iterator_type().operator=(_Right_cref.nii_array_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdarray_range_error("invalid assignment - mse::mstd::array<>::xscope_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_iterator& _Right_cref) const { return nii_array_xscope_ss_iterator_type().operator==(_Right_cref.nii_array_xscope_ss_iterator_type()); }
				bool operator!=(const xscope_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_iterator& _Right) const { return (nii_array_xscope_ss_iterator_type() < _Right.nii_array_xscope_ss_iterator_type()); }
				bool operator<=(const xscope_iterator& _Right) const { return (nii_array_xscope_ss_iterator_type() <= _Right.nii_array_xscope_ss_iterator_type()); }
				bool operator>(const xscope_iterator& _Right) const { return (nii_array_xscope_ss_iterator_type() > _Right.nii_array_xscope_ss_iterator_type()); }
				bool operator>=(const xscope_iterator& _Right) const { return (nii_array_xscope_ss_iterator_type() >= _Right.nii_array_xscope_ss_iterator_type()); }
				void set_to_item_pointer(const xscope_iterator& _Right_cref) { nii_array_xscope_ss_iterator_type().set_to_item_pointer(_Right_cref.nii_array_xscope_ss_iterator_type()); }
				msear_size_t position() const { return nii_array_xscope_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(nii_array_xscope_ss_iterator_type().target_container_ptr()) {
					return nii_array_xscope_ss_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
			private:
				typename _MA::xscope_ss_iterator_type m_xscope_ss_iterator;
				friend class /*_Myt*/array<_Ty, _Size>;
				friend class xscope_const_iterator;
			};

		private:
			mse::TRegisteredObj<_MA> m_nii_array;

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

#endif /*MSE_MSTDARRAY_DISABLED*/
	}
}

#ifndef MSE_MSTDARRAY_DISABLED
namespace std {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmismatched-tags"
#endif /*__clang__*/

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

#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

	// TUPLE INTERFACE TO array
	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty& get(mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_nii_array));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
		_CONST_FUN const _Ty& get(const mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (std::get<_Idx>(_Arr.m_nii_array));
	}

	template<size_t _Idx, class _Ty, size_t _Size>
	_CONST_FUN _Ty&& get(mse::mstd::array<_Ty, _Size>&& _Arr) _NOEXCEPT
	{	// return element at _Idx in array _Arr
		static_assert(_Idx < _Size, "array index out of bounds");
		return (_STD move(std::get<_Idx>(_Arr.m_nii_array)));
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

#ifdef __clang__
#pragma clang diagnostic pop
#else /*__clang__*/
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /*__GNUC__*/
#endif /*__clang__*/

#undef MSE_THROW

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif /*ndef MSEMSTDARRAY_H*/
