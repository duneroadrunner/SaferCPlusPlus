
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSTDSTRING_H
#define MSEMSTDSTRING_H

#include "msemsestring.h"

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDSTRING_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	namespace mstd {

#ifdef MSE_MSTDSTRING_DISABLED
		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> > using basic_string = std::basic_string<_Ty, _Traits, _A>;

#else /*MSE_MSTDSTRING_DISABLED*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

		class mstdbasic_string_range_error : public std::range_error {
		public:
			using std::range_error::range_error;
		};

		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
		class basic_string;

		namespace impl {
			namespace bs {
				template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>>
				std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>&& _Istr, basic_string<_Ty, _Traits, _A>& _Str);
				template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>>
				std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>& _Istr, basic_string<_Ty, _Traits, _A>& _Str);
				template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>>
				std::basic_ostream<_Ty, _Traits>& out_to_stream(std::basic_ostream<_Ty, _Traits>& _Ostr, const basic_string<_Ty, _Traits, _A>& _Str);
			}
		}

		template<class _Ty, class _Traits/* = std::char_traits<_Ty>*/, class _A/* = std::allocator<_Ty> */>
		class basic_string {
		public:
			typedef basic_string _Myt;
			typedef mse::us::msebasic_string<_Ty, _Traits, _A> _MBS;

			typedef typename _MBS::allocator_type allocator_type;
			typedef typename _MBS::value_type value_type;
			typedef typename _MBS::size_type size_type;
			typedef typename _MBS::difference_type difference_type;
			typedef typename _MBS::pointer pointer;
			typedef typename _MBS::const_pointer const_pointer;
			typedef typename _MBS::reference reference;
			typedef typename _MBS::const_reference const_reference;
			static const size_t npos = size_t(-1);

			operator mse::nii_basic_string<_Ty, _Traits, _A>() const { return msebasic_string(); }
			operator std::basic_string<_Ty, _Traits, _A>() const { return msebasic_string(); }

			explicit basic_string(const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_Al)) {}
			explicit basic_string(size_type _N) : m_shptr(std::make_shared<_MBS>(_N)) {}
			explicit basic_string(size_type _N, const _Ty& _V, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_N, _V, _Al)) {}
			basic_string(_Myt&& _X) : m_shptr(std::make_shared<_MBS>()) {
				static_assert(typename std::is_rvalue_reference<decltype(_X)>::type(), "");
				/* It would be more efficient to just move _X.m_shptr into m_shptr, but that would leave _X in what we
				would consider an invalid state. */
				msebasic_string() = std::move(_X.msebasic_string());
			}
			basic_string(const _Myt& _X) : m_shptr(std::make_shared<_MBS>(_X.msebasic_string())) {}
			basic_string(_MBS&& _X) : m_shptr(std::make_shared<_MBS>(std::forward<decltype(_X)>(_X))) {}
			basic_string(const _MBS& _X) : m_shptr(std::make_shared<_MBS>(_X)) {}
			basic_string(mse::nii_basic_string<_Ty, _Traits>&& _X) : m_shptr(std::make_shared<_MBS>(std::forward<decltype(_X)>(_X))) {}
			basic_string(const mse::nii_basic_string<_Ty, _Traits>& _X) : m_shptr(std::make_shared<_MBS>(_X)) {}
			basic_string(std::basic_string<_Ty, _Traits>&& _X) : m_shptr(std::make_shared<_MBS>(std::forward<decltype(_X)>(_X))) {}
			basic_string(const std::basic_string<_Ty, _Traits>& _X) : m_shptr(std::make_shared<_MBS>(_X)) {}
			typedef typename _MBS::const_iterator _It;
			basic_string(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_F, _L, _Al)) {}
			basic_string(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_F, _L, _Al)) {}
			template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			basic_string(_Iter _First, _Iter _Last) : m_shptr(std::make_shared<_MBS>(_First, _Last)) {}
			template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			basic_string(_Iter _First, _Iter _Last, const _A& _Al) : m_shptr(std::make_shared<_MBS>(_First, _Last, _Al)) {}
			basic_string(const _Ty* const _Ptr) : m_shptr(std::make_shared<_MBS>(_Ptr)) {}
			basic_string(const _Ty* const _Ptr, const size_t _Count) : m_shptr(std::make_shared<_MBS>(_Ptr, _Count)) {}
			basic_string(const _Myt& _X, const size_type _Roff, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_X.msebasic_string(), _Roff, npos, _Al)) {}
			basic_string(const _Myt& _X, const size_type _Roff, const size_type _Count, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_X.msebasic_string(), _Roff, _Count, _Al)) {}
			/* construct from mse::string_view and "string sections". */
			template<typename _TStringSection, class = typename std::enable_if<(std::is_base_of<StringSectionTagBase, _TStringSection>::value), void>::type>
			basic_string(const _TStringSection& _X) : m_shptr(std::make_shared<_MBS>(_X)) {}

			_Myt& operator=(_MBS&& _X) { msebasic_string() = (std::forward<decltype(_X)>(_X)); return (*this); }
			_Myt& operator=(const _MBS& _X) { msebasic_string() = (_X); return (*this); }
			_Myt& operator=(_Myt&& _X) {
				static_assert(typename std::is_rvalue_reference<decltype(_X)>::type(), "");
				/* It would be more efficient to just move _X.m_shptr into m_shptr, but that would leave _X in what we
				would consider an invalid state. */
				msebasic_string() = std::move(_X.msebasic_string());
				return (*this);
			}
			_Myt& operator=(const _Myt& _X) { msebasic_string() = (_X.msebasic_string()); return (*this); }
			void reserve(size_type _Count) { m_shptr->reserve(_Count); }
			void resize(size_type _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
			typename _MBS::const_reference operator[](size_type _P) const { return m_shptr->operator[](_P); }
			typename _MBS::reference operator[](size_type _P) { return m_shptr->operator[](_P); }
			void push_back(_Ty&& _X) { m_shptr->push_back(std::forward<decltype(_X)>(_X)); }
			void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
			void pop_back() { m_shptr->pop_back(); }
			void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
			template<class _Iter>
			void assign(const _Iter& _First, const _Iter& _Last) { m_shptr->assign(_First, _Last); }
			void assign(size_type _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
			template<typename _TStringSection, class = typename std::enable_if<(std::is_base_of<StringSectionTagBase, _TStringSection>::value), void>::type>
			void assign(const _TStringSection& _X) { m_shptr->assign(_X); }
			template<class ..._Valty>
			void emplace_back(_Valty&& ..._Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)...); }
			void clear() { m_shptr->clear(); }
			void swap(_MBS& _X) { m_shptr->swap(_X); }
			void swap(_Myt& _X) { m_shptr->swap(_X.msebasic_string()); }
			void swap(mse::nii_basic_string<_Ty, _Traits, _A>& _X) { m_shptr->swap(_X); }
			void swap(std::basic_string<_Ty, _Traits, _A>& _X) { m_shptr->swap(_X); }

			basic_string(_XSTD initializer_list<typename _MBS::value_type> _Ilist, const _A& _Al = _A()) : m_shptr(std::make_shared<_MBS>(_Ilist, _Al)) {}
			_Myt& operator=(_XSTD initializer_list<typename _MBS::value_type> _Ilist) { msebasic_string() = (_Ilist); return (*this); }
			void assign(_XSTD initializer_list<typename _MBS::value_type> _Ilist) { m_shptr->assign(_Ilist); }

			size_type capacity() const _NOEXCEPT { return m_shptr->capacity(); }
			void shrink_to_fit() { m_shptr->shrink_to_fit(); }
			size_type length() const _NOEXCEPT { return m_shptr->length(); }
			size_type size() const _NOEXCEPT { return m_shptr->size(); }
			size_type max_size() const _NOEXCEPT { return m_shptr->max_size(); }
			bool empty() const _NOEXCEPT { return m_shptr->empty(); }
			_A get_allocator() const _NOEXCEPT { return m_shptr->get_allocator(); }
			typename _MBS::const_reference at(size_type _Pos) const { return m_shptr->at(_Pos); }
			typename _MBS::reference at(size_type _Pos) { return m_shptr->at(_Pos); }
			typename _MBS::reference front() { return m_shptr->front(); }
			typename _MBS::const_reference front() const { return m_shptr->front(); }
			typename _MBS::reference back() { return m_shptr->back(); }
			typename _MBS::const_reference back() const { return m_shptr->back(); }

			/* Try to avoid using these whenever possible. */
			value_type *data() _NOEXCEPT {
				return m_shptr->data();
			}
			const value_type *data() const _NOEXCEPT {
				return m_shptr->data();
			}

			class xscope_const_iterator;
			class xscope_iterator;

			class const_iterator : public _MBS::random_access_const_iterator_base {
			public:
				typedef typename _MBS::random_access_const_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				const_iterator() {}
				const_iterator(const const_iterator& src_cref) : m_msebasic_string_cshptr(src_cref.m_msebasic_string_cshptr) {
					(*this) = src_cref;
				}
				~const_iterator() {}
				const typename _MBS::ss_const_iterator_type& msebasic_string_ss_const_iterator_type() const { return m_ss_const_iterator; }
				typename _MBS::ss_const_iterator_type& msebasic_string_ss_const_iterator_type() { return m_ss_const_iterator; }
				const typename _MBS::ss_const_iterator_type& mvssci() const { return msebasic_string_ss_const_iterator_type(); }
				typename _MBS::ss_const_iterator_type& mvssci() { return msebasic_string_ss_const_iterator_type(); }

				void reset() { msebasic_string_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msebasic_string_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msebasic_string_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msebasic_string_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msebasic_string_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msebasic_string_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msebasic_string_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msebasic_string_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msebasic_string_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msebasic_string_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msebasic_string_ss_const_iterator_type().set_to_previous(); }
				const_iterator& operator ++() { msebasic_string_ss_const_iterator_type().operator ++(); return (*this); }
				const_iterator operator++(int) { const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				const_iterator& operator --() { msebasic_string_ss_const_iterator_type().operator --(); return (*this); }
				const_iterator operator--(int) { const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MBS::difference_type n) { msebasic_string_ss_const_iterator_type().advance(n); }
				void regress(typename _MBS::difference_type n) { msebasic_string_ss_const_iterator_type().regress(n); }
				const_iterator& operator +=(difference_type n) { msebasic_string_ss_const_iterator_type().operator +=(n); return (*this); }
				const_iterator& operator -=(difference_type n) { msebasic_string_ss_const_iterator_type().operator -=(n); return (*this); }
				const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MBS::difference_type operator-(const const_iterator& _Right_cref) const { return msebasic_string_ss_const_iterator_type() - (_Right_cref.msebasic_string_ss_const_iterator_type()); }
				typename _MBS::const_reference operator*() const { return msebasic_string_ss_const_iterator_type().operator*(); }
				typename _MBS::const_reference item() const { return operator*(); }
				typename _MBS::const_reference previous_item() const { return msebasic_string_ss_const_iterator_type().previous_item(); }
				typename _MBS::const_pointer operator->() const { return msebasic_string_ss_const_iterator_type().operator->(); }
				typename _MBS::const_reference operator[](typename _MBS::difference_type _Off) const { return (*(*this + _Off)); }
				bool operator==(const const_iterator& _Right_cref) const { return msebasic_string_ss_const_iterator_type().operator==(_Right_cref.msebasic_string_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (msebasic_string_ss_const_iterator_type() < _Right.msebasic_string_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (msebasic_string_ss_const_iterator_type() <= _Right.msebasic_string_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (msebasic_string_ss_const_iterator_type() > _Right.msebasic_string_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (msebasic_string_ss_const_iterator_type() >= _Right.msebasic_string_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { msebasic_string_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msebasic_string_ss_const_iterator_type()); }
				msev_size_t position() const { return msebasic_string_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msebasic_string_ss_const_iterator_type().target_container_ptr()) {
					return msebasic_string_ss_const_iterator_type().target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				const_iterator(std::shared_ptr<_MBS> msebasic_string_shptr) : m_msebasic_string_cshptr(msebasic_string_shptr) {
					m_ss_const_iterator = msebasic_string_shptr->ss_cbegin();
				}
				std::shared_ptr<const _MBS> m_msebasic_string_cshptr;
				/* m_ss_const_iterator needs to be declared after m_msebasic_string_cshptr so that its destructor will be called first. */
				typename _MBS::ss_const_iterator_type m_ss_const_iterator;
				friend class /*_Myt*/basic_string<_Ty, _Traits, _A>;
				friend class iterator;
			};
			class iterator : public _MBS::random_access_iterator_base {
			public:
				typedef typename _MBS::random_access_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				iterator() {}
				iterator(const iterator& src_cref) : m_msebasic_string_shptr(src_cref.m_msebasic_string_shptr) {
					(*this) = src_cref;
				}
				~iterator() {}
				const typename _MBS::ss_iterator_type& msebasic_string_ss_iterator_type() const { return m_ss_iterator; }
				typename _MBS::ss_iterator_type& msebasic_string_ss_iterator_type() { return m_ss_iterator; }
				const typename _MBS::ss_iterator_type& mvssi() const { return msebasic_string_ss_iterator_type(); }
				typename _MBS::ss_iterator_type& mvssi() { return msebasic_string_ss_iterator_type(); }
				operator const_iterator() const {
					const_iterator retval(m_msebasic_string_shptr);
					if (m_msebasic_string_shptr) {
						retval.msebasic_string_ss_const_iterator_type().set_to_beginning();
						retval.msebasic_string_ss_const_iterator_type().advance(msev_int(msebasic_string_ss_iterator_type().position()));
					}
					return retval;
				}

				void reset() { msebasic_string_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msebasic_string_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msebasic_string_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msebasic_string_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msebasic_string_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msebasic_string_ss_iterator_type().has_next(); }
				bool has_previous() const { return msebasic_string_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msebasic_string_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msebasic_string_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msebasic_string_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msebasic_string_ss_iterator_type().set_to_previous(); }
				iterator& operator ++() { msebasic_string_ss_iterator_type().operator ++(); return (*this); }
				iterator operator++(int) { iterator _Tmp = *this; ++*this; return (_Tmp); }
				iterator& operator --() { msebasic_string_ss_iterator_type().operator --(); return (*this); }
				iterator operator--(int) { iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MBS::difference_type n) { msebasic_string_ss_iterator_type().advance(n); }
				void regress(typename _MBS::difference_type n) { msebasic_string_ss_iterator_type().regress(n); }
				iterator& operator +=(difference_type n) { msebasic_string_ss_iterator_type().operator +=(n); return (*this); }
				iterator& operator -=(difference_type n) { msebasic_string_ss_iterator_type().operator -=(n); return (*this); }
				iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MBS::difference_type operator-(const iterator& _Right_cref) const { return msebasic_string_ss_iterator_type() - (_Right_cref.msebasic_string_ss_iterator_type()); }
				typename _MBS::reference operator*() const { return msebasic_string_ss_iterator_type().operator*(); }
				typename _MBS::reference item() const { return operator*(); }
				typename _MBS::reference previous_item() const { return msebasic_string_ss_iterator_type().previous_item(); }
				typename _MBS::pointer operator->() const { return msebasic_string_ss_iterator_type().operator->(); }
				typename _MBS::reference operator[](typename _MBS::difference_type _Off) const { return (*(*this + _Off)); }
				bool operator==(const iterator& _Right_cref) const { return msebasic_string_ss_iterator_type().operator==(_Right_cref.msebasic_string_ss_iterator_type()); }
				bool operator!=(const iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const iterator& _Right) const { return (msebasic_string_ss_iterator_type() < _Right.msebasic_string_ss_iterator_type()); }
				bool operator<=(const iterator& _Right) const { return (msebasic_string_ss_iterator_type() <= _Right.msebasic_string_ss_iterator_type()); }
				bool operator>(const iterator& _Right) const { return (msebasic_string_ss_iterator_type() > _Right.msebasic_string_ss_iterator_type()); }
				bool operator>=(const iterator& _Right) const { return (msebasic_string_ss_iterator_type() >= _Right.msebasic_string_ss_iterator_type()); }
				void set_to_item_pointer(const iterator& _Right_cref) { msebasic_string_ss_iterator_type().set_to_item_pointer(_Right_cref.msebasic_string_ss_iterator_type()); }
				msev_size_t position() const { return msebasic_string_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msebasic_string_ss_iterator_type().target_container_ptr()) {
					return msebasic_string_ss_iterator_type().target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				std::shared_ptr<_MBS> m_msebasic_string_shptr;
				/* m_ss_iterator needs to be declared after m_msebasic_string_shptr so that its destructor will be called first. */
				typename _MBS::ss_iterator_type m_ss_iterator;
				friend class /*_Myt*/basic_string<_Ty, _Traits, _A>;
			};

			iterator begin() {	// return iterator for beginning of mutable sequence
				iterator retval; retval.m_msebasic_string_shptr = this->m_shptr;
				(retval.m_ss_iterator) = m_shptr->ss_begin();
				return retval;
			}
			const_iterator begin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msebasic_string_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_begin();
				return retval;
			}
			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; retval.m_msebasic_string_shptr = this->m_shptr;
				(retval.m_ss_iterator) = m_shptr->ss_end();
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msebasic_string_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_end();
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msebasic_string_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_cbegin();
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msebasic_string_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_cend();
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


			basic_string(const const_iterator &start, const const_iterator &end, const _A& _Al = _A())
				: m_shptr(std::make_shared<_MBS>(start.msebasic_string_ss_const_iterator_type(), end.msebasic_string_ss_const_iterator_type(), _Al)) {}
			void assign(const const_iterator &start, const const_iterator &end) {
				m_shptr->assign(start.msebasic_string_ss_const_iterator_type(), end.msebasic_string_ss_const_iterator_type());
			}
			void assign_inclusive(const const_iterator &first, const const_iterator &last) {
				m_shptr->assign_inclusive(first.msebasic_string_ss_const_iterator_type(), last.msebasic_string_ss_const_iterator_type());
			}
			iterator insert_before(const const_iterator &pos, size_type _M, const _Ty& _X) {
				auto res = m_shptr->insert_before(pos.msebasic_string_ss_const_iterator_type(), _M, _X);
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(const const_iterator &pos, _Ty&& _X) {
				auto res = m_shptr->insert_before(pos.msebasic_string_ss_const_iterator_type(), std::forward<decltype(_X)>(_X));
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(const const_iterator &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
			iterator insert_before(const const_iterator &pos, const _Iter &start, const _Iter &end) {
				auto res = m_shptr->insert_before(pos.msebasic_string_ss_const_iterator_type(), start, end);
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
			iterator insert_before_inclusive(const const_iterator &pos, const _Iter &first, const _Iter &last) {
				auto end = last; end++;
				return insert_before(pos, first, end);
			}
			iterator insert_before(const const_iterator &pos, _XSTD initializer_list<typename _MBS::value_type> _Ilist) {	// insert initializer_list
				auto res = m_shptr->insert_before(pos.msebasic_string_ss_const_iterator_type(), _Ilist);
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			template<typename _TStringSection, class = typename std::enable_if<(std::is_base_of<StringSectionTagBase, _TStringSection>::value), void>::type>
			iterator insert_before(const const_iterator &pos, const _TStringSection& _X) {	// insert initializer_list
				auto res = m_shptr->insert_before(pos.msebasic_string_ss_const_iterator_type(), _X);
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			basic_string& insert_before(msev_size_t pos, const _Ty& _X = _Ty()) {
				m_shptr->insert_before(pos, _X);
				return *this;
			}
			basic_string& insert_before(msev_size_t pos, size_type _M, const _Ty& _X) {
				m_shptr->insert_before(pos, _M, _X);
				return *this;
			}
			basic_string& insert_before(msev_size_t pos, _XSTD initializer_list<typename _MBS::value_type> _Ilist) {	// insert initializer_list
				m_shptr->insert_before(pos, _Ilist);
				return *this;
			}
			template<typename _TStringSection, class = typename std::enable_if<(std::is_base_of<StringSectionTagBase, _TStringSection>::value), void>::type>
			basic_string& insert_before(msev_size_t pos, const _TStringSection& _X) {
				m_shptr->insert_before(pos, _X);
				return *this;
			}
			/* These insert() functions are just aliases for their corresponding insert_before() functions. */
			iterator insert(const const_iterator &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
			iterator insert(const const_iterator &pos, _Ty&& _X) { return insert_before(pos, std::forward<decltype(_X)>(_X)); }
			iterator insert(const const_iterator &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				iterator insert(const const_iterator &pos, const _Iter &start, const _Iter &end) { return insert_before(pos, start, end); }
			iterator insert(const const_iterator &pos, const _Ty* start, const _Ty* end) { return insert_before(pos, start, end); }
			iterator insert(const const_iterator &pos, _XSTD initializer_list<typename _MBS::value_type> _Ilist) { return insert_before(pos, _Ilist); }
			template<typename _TStringSection, class = typename std::enable_if<(std::is_base_of<StringSectionTagBase, _TStringSection>::value), void>::type>
			iterator insert(const const_iterator &pos, const _TStringSection& _X) { return insert_before(pos, _X); }
			template<class ..._Valty>
			iterator emplace(const const_iterator &pos, _Valty&& ..._Val) {
				auto res = m_shptr->emplace(pos.msebasic_string_ss_const_iterator_type(), std::forward<_Valty>(_Val)...);
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			iterator erase(const const_iterator &pos) {
				auto res = m_shptr->erase(pos.msebasic_string_ss_const_iterator_type());
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			iterator erase(const const_iterator &start, const const_iterator &end) {
				auto res = m_shptr->erase(start.msebasic_string_ss_const_iterator_type(), end.msebasic_string_ss_const_iterator_type());
				iterator retval = begin(); retval.msebasic_string_ss_iterator_type() = res;
				return retval;
			}
			iterator erase_inclusive(const const_iterator &first, const const_iterator &last) {
				auto end = last; end.set_to_next();
				return erase_inclusive(first, end);
			}
			bool operator==(const _Myt& _Right) const {	// test for basic_string equality
				return ((*(_Right.m_shptr)) == (*m_shptr));
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for basic_strings
				return ((*m_shptr) < (*(_Right.m_shptr)));
			}

			basic_string& append(std::initializer_list<_Ty> _Ilist) {
				msebasic_string().append(_Ilist);
				return (*this);
			}
			basic_string& append(const basic_string& _Right) {
				msebasic_string().append(_Right);
				return (*this);
			}
			basic_string& append(const basic_string& _Right, const size_type _Roff, size_type _Count = npos) {
				msebasic_string().append(_Right, _Roff, _Count);
				return (*this);
			}
#if 0//_HAS_CXX17
			basic_string& append(const basic_string_view<_Ty, _Traits> _Right)
			{	// append _Right
				return (append(_Right.data(), _Convert_size<size_type>(_Right.size())));
			}
			template<class _StringViewIsh,
				class = _Is_string_view_ish<_StringViewIsh>>
				basic_string& append(const _StringViewIsh& _Right, const size_type _Roff, const size_type _Count = npos)
			{	// append _Right [_Roff, _Roff + _Count)
				basic_string_view<_Ty, _Traits> _As_view = _Right;
				return (append(_As_view.substr(_Roff, _Count)));
			}
#endif /* _HAS_CXX17 */
			basic_string& append(const _Ty * const _Ptr, const size_type _Count) {
				msebasic_string().append(_Ptr, _Count);
				return (*this);
			}
			basic_string& append(const _Ty * const _Ptr) {
				msebasic_string().append(_Ptr);
				return (*this);
			}
			basic_string& append(const size_type _Count, const _Ty _Ch) {
				msebasic_string().append(_Count, _Ch);
				return (*this);
			}
			template<class _Iter, class = typename std::enable_if<mse::_mse_Is_iterator<_Iter>::value, void>::type>
			basic_string& append(const _Iter _First, const _Iter _Last) {
				msebasic_string().append(_First, _Last);
				return (*this);
			}

			basic_string& operator+=(std::initializer_list<_Ty> _Ilist) {
				msebasic_string().append(_Ilist);
				return (*this);
			}
			basic_string& operator+=(const basic_string& _Right) {
				msebasic_string().append(_Right.msebasic_string());
				return (*this);
			}
#if 0//_HAS_CXX17
			basic_string& operator+=(const basic_string_view<_Ty, _Traits> _Right)
			{	// append _Right
				return (append(_Right.data(), _Convert_size<size_type>(_Right.size())));
			}
#endif /* _HAS_CXX17 */
			/*
			basic_string& operator+=(_In_z_ const _Ty * const _Ptr) {	// append [_Ptr, <null>)
			return (append(_Ptr, _Convert_size<size_type>(_Traits::length(_Ptr))));
			}
			*/
			basic_string& operator+=(const _Ty _Ch) {
				msebasic_string() += _Ch;
				return (*this);
			}

			basic_string& replace(const size_type _Off, const size_type _N0, const basic_string& _Right) {
				msebasic_string().replace(_Off, _N0, _Right.msebasic_string());
				return (*this);
			}

			basic_string& replace(const size_type _Off, size_type _N0,
				const basic_string& _Right, const size_type _Roff, size_type _Count = npos) {
				msebasic_string().replace(_Right.msebasic_string(), _Roff, _Count);
				return (*this);
			}

#if 0//_HAS_CXX17
			basic_string& replace(const size_type _Off, const size_type _N0, const basic_string_view<_Ty, _Traits> _Right)
			{	// replace [_Off, _Off + _N0) with _Right
				msebasic_string().replace(_Off, _N0, _Right);
				return (*this);
			}

			template<class _StringViewIsh,
				class = _Is_string_view_ish<_StringViewIsh>>
				basic_string& replace(const size_type _Off, const size_type _N0,
					const _StringViewIsh& _Right, const size_type _Roff, const size_type _Count = npos)
			{	// replace [_Off, _Off + _N0) with _Right [_Roff, _Roff + _Count)
				basic_string_view<_Ty, _Traits> _As_view = _Right;
				return (replace(_Off, _N0, _As_view.substr(_Roff, _Count)));
			}
#endif /* _HAS_CXX17 */

			basic_string& replace(const size_type _Off, size_type _N0, const _Ty * const _Ptr, const size_type _Count) {
				msebasic_string().replace(_Off, _N0, _Ptr, _Count);
				return (*this);
			}

			basic_string& replace(const size_type _Off, const size_type _N0, const _Ty * const _Ptr) {
				return (replace(_Off, _N0, _Ptr, size_type(_Traits::length(_Ptr))));
			}

			basic_string& replace(const size_type _Off, size_type _N0, const size_type _Count, const _Ty _Ch) {
				msebasic_string().replace(_Off, _N0, _Count, _Ch);
				return (*this);
			}

			basic_string& replace(const const_iterator _First, const const_iterator _Last, const basic_string& _Right) {
				return (replace(_First - begin(), _Last - _First, _Right));
			}

#if 0//_HAS_CXX17
			basic_string& replace(const const_iterator _First, const const_iterator _Last,
				const basic_string_view<_Ty, _Traits> _Right) {
				return (replace(_First - begin(), _Last - _First, _Right));
			}
#endif /* _HAS_CXX17 */

			basic_string& replace(const const_iterator _First, const const_iterator _Last,
				const _Ty * const _Ptr, const size_type _Count) {
				return (replace(_First - begin(), _Last - _First, _Ptr, _Count));
			}

			basic_string& replace(const const_iterator _First, const const_iterator _Last, const _Ty * const _Ptr) {
				return (replace(_First - begin(), _Last - _First, _Ptr));
			}

			basic_string& replace(const const_iterator _First, const const_iterator _Last, const size_type _Count, const _Ty _Ch) {
				return (replace(_First - begin(), _Last - _First, _Count, _Ch));
			}

			template<class _Iter, class = typename std::enable_if<mse::_mse_Is_iterator<_Iter>::value>::type>
			basic_string& replace(const const_iterator _First, const const_iterator _Last,
				const _Iter _First2, const _Iter _Last2) {
				const basic_string _Right(_First2, _Last2, get_allocator());
				replace(_First, _Last, _Right);
				return (*this);
			}

#if 0//_HAS_CXX17
			int compare(const basic_string_view<_Ty, _Traits> _Right) const _NOEXCEPT {
				auto& _My_data = this->_Get_data();
				return (_Traits_compare<_Traits>(_My_data._Myptr(), _My_data._Mysize,
					_Right.data(), _Right.size()));
			}

			int compare(const size_type _Off, const size_type _N0,
				const basic_string_view<_Ty, _Traits> _Right) const
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
				basic_string_view<_Ty, _Traits> _As_view = _Right;
				auto& _My_data = this->_Get_data();
				_My_data._Check_offset(_Off);
				const auto _With_substr = _As_view.substr(_Roff, _Count);
				return (_Traits_compare<_Traits>(_My_data._Myptr() + _Off, _My_data._Clamp_suffix_size(_Off, _N0),
					_With_substr.data(), _With_substr.size()));
			}
#endif /* _HAS_CXX17 */

			int compare(const basic_string& _Right) const _NOEXCEPT {
				return msebasic_string().compare(_Right.msebasic_string());
			}

			int compare(size_type _Off, size_type _N0, const basic_string& _Right) const {
				return msebasic_string().compare(_Off, _N0, _Right.msebasic_string());
			}

			int compare(const size_type _Off, const size_type _N0, const basic_string& _Right,
				const size_type _Roff, const size_type _Count = npos) const {
				return msebasic_string().compare(_Off, _N0, _Right.msebasic_string(), _Roff, _Count);
			}

			int compare(const _Ty * const _Ptr) const _NOEXCEPT {
				return msebasic_string().compare(_Ptr);
			}

			int compare(const size_type _Off, const size_type _N0, const _Ty * const _Ptr) const {
				return msebasic_string().compare(_Off, _N0, _Ptr);
			}

			int compare(const size_type _Off, const size_type _N0, const _Ty * const _Ptr,
				const size_type _Count) const {
				return msebasic_string().compare(_Off, _N0, _Ptr, _Count);
			}


#if 0//_HAS_CXX17
			size_type find(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const _NOEXCEPT
			{	// look for _Right beginning at or after _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(
					_Traits_find<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off, _Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type find(const basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find(_Right.msebasic_string(), _Off);
			}

			size_type find(const _Ty * const _Ptr, const size_type _Off, const size_type _Count) const _NOEXCEPT {
				return msebasic_string().find(_Ptr, _Off, _Count);
			}

			size_type find(const _Ty * const _Ptr, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find(_Ptr, _Off);
			}

			size_type find(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find(_Ch, _Off);
			}

#if 0//_HAS_CXX17
			size_type rfind(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const _NOEXCEPT
			{	// look for _Right beginning before _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(
					_Traits_rfind<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off, _Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type rfind(const basic_string& _Right, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().rfind(_Right.msebasic_string(), _Off);
			}

			size_type rfind(const _Ty * const _Ptr, const size_type _Off, const size_type _Count) const _NOEXCEPT {
				return msebasic_string().rfind(_Ptr, _Off, _Count);
			}

			size_type rfind(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().rfind(_Ptr, _Off);
			}

			size_type rfind(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().rfind(_Ch, _Off);
			}

#if 0//_HAS_CXX17
			size_type find_first_of(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const _NOEXCEPT
			{	// look for one of _Right at or after _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(_Traits_find_first_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
					_Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type find_first_of(const basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_of(_Right.msebasic_string(), _Off);
			}

			size_type find_first_of(const _Ty * const _Ptr, const size_type _Off,
				const size_type _Count) const _NOEXCEPT {
				return msebasic_string().find_first_of(_Ptr, _Off, _Count);
			}

			size_type find_first_of(const _Ty * const _Ptr, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_of(_Ptr, _Off);
			}

			size_type find_first_of(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_of(_Ch, _Off);
			}

#if 0//_HAS_CXX17
			size_type find_last_of(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const _NOEXCEPT
			{	// look for one of _Right before _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(_Traits_find_last_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
					_Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type find_last_of(const basic_string& _Right, size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_of(_Right.msebasic_string(), _Off);
			}

			size_type find_last_of(const _Ty * const _Ptr, const size_type _Off,
				const size_type _Count) const _NOEXCEPT {
				return msebasic_string().find_last_of(_Ptr, _Off, _Count);
			}

			size_type find_last_of(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_of(_Ptr, _Off);
			}

			size_type find_last_of(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_of(_Ch, _Off);
			}

#if 0//_HAS_CXX17
			size_type find_first_not_of(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = 0) const
				_NOEXCEPT
			{	// look for none of _Right at or after _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(_Traits_find_first_not_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
					_Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type find_first_not_of(const basic_string& _Right, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_not_of(_Right.msebasic_string(), _Off);
			}

			size_type find_first_not_of(const _Ty * const _Ptr, const size_type _Off,
				const size_type _Count) const _NOEXCEPT {
				return msebasic_string().find_first_not_of(_Ptr, _Off, _Count);
			}

			size_type find_first_not_of(const _Ty * const _Ptr, size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_not_of(_Ptr, _Off);
			}

			size_type find_first_not_of(const _Ty _Ch, const size_type _Off = 0) const _NOEXCEPT {
				return msebasic_string().find_first_not_of(_Ch, _Off);
			}

#if 0//_HAS_CXX17
			size_type find_last_not_of(const basic_string_view<_Ty, _Traits> _Right, const size_type _Off = npos) const
				_NOEXCEPT
			{	// look for none of _Right before _Off
				auto& _My_data = this->_Get_data();
				return (static_cast<size_type>(_Traits_find_last_not_of<_Traits>(_My_data._Myptr(), _My_data._Mysize, _Off,
					_Right.data(), _Right.size())));
			}
#endif /* _HAS_CXX17 */

			size_type find_last_not_of(const basic_string& _Right, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_not_of(_Right.msebasic_string(), _Off);
			}

			size_type find_last_not_of(const _Ty * const _Ptr, const size_type _Off,
				const size_type _Count) const _NOEXCEPT {
				return msebasic_string().find_last_not_of(_Ptr, _Off, _Count);
			}

			size_type find_last_not_of(const _Ty * const _Ptr, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_not_of(_Ptr, _Off);
			}

			size_type find_last_not_of(const _Ty _Ch, const size_type _Off = npos) const _NOEXCEPT {
				return msebasic_string().find_last_not_of(_Ch, _Off);
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
					(*target_iter) = msebasic_string()[i];
					++target_iter;
				}
				return n;
			}

			basic_string substr(const size_type _Off = 0, const size_type _Count = npos) const {
				//return (basic_string(*this, _Off, _Count, get_allocator()));
				return (basic_string(msebasic_string().substr(_Off, _Count)));
			}

			static basic_string& _Myt_ref(basic_string& bs_ref) {
				return bs_ref;
			}

			template<typename _TThisPointer>
			static std::basic_istream<_Ty, _Traits>& getline(std::basic_istream<_Ty, _Traits>&& _Istr, _TThisPointer this_ptr, const _Ty _Delim) {
				return std::getline(std::forward<decltype(_Istr)>(_Istr), _Myt_ref(*this_ptr).msebasic_string(), _Delim);
			}
			template<typename _TThisPointer>
			static std::basic_istream<_Ty, _Traits>& getline(std::basic_istream<_Ty, _Traits>&& _Istr, _TThisPointer this_ptr) {
				return std::getline(std::forward<decltype(_Istr)>(_Istr), _Myt_ref(*this_ptr).msebasic_string());
			}
			template<typename _TThisPointer>
			static std::basic_istream<_Ty, _Traits>& getline(std::basic_istream<_Ty, _Traits>& _Istr, _TThisPointer this_ptr, const _Ty _Delim) {
				return std::getline(_Istr, _Myt_ref(*this_ptr).msebasic_string(), _Delim);
			}
			template<typename _TThisPointer>
			static std::basic_istream<_Ty, _Traits>& getline(std::basic_istream<_Ty, _Traits>& _Istr, _TThisPointer this_ptr) {
				return std::getline(_Istr, _Myt_ref(*this_ptr).msebasic_string());
			}


			class xscope_const_iterator : public _MBS::random_access_const_iterator_base, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				typedef typename _MBS::random_access_const_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedConstPointer<basic_string> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<basic_string> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedConstPointer<basic_string> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<basic_string> >::value
					, void>::type>
					xscope_const_iterator(const _TXScopePointer& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscope_const_pointer_to_member(*((*owner_ptr).m_shptr), owner_ptr)) {}

				xscope_const_iterator(const xscope_const_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_const_iterator) {}
				xscope_const_iterator(const xscope_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_const_iterator() {}
				const typename _MBS::xscope_ss_const_iterator_type& msebasic_string_xscope_ss_const_iterator_type() const {
					return m_xscope_ss_const_iterator;
				}
				typename _MBS::xscope_ss_const_iterator_type& msebasic_string_xscope_ss_const_iterator_type() {
					return m_xscope_ss_const_iterator;
				}
				const typename _MBS::xscope_ss_const_iterator_type& mvssci() const { return msebasic_string_xscope_ss_const_iterator_type(); }
				typename _MBS::xscope_ss_const_iterator_type& mvssci() { return msebasic_string_xscope_ss_const_iterator_type(); }

				void reset() { msebasic_string_xscope_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msebasic_string_xscope_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msebasic_string_xscope_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msebasic_string_xscope_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msebasic_string_xscope_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msebasic_string_xscope_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msebasic_string_xscope_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msebasic_string_xscope_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msebasic_string_xscope_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msebasic_string_xscope_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msebasic_string_xscope_ss_const_iterator_type().set_to_previous(); }
				xscope_const_iterator& operator ++() { msebasic_string_xscope_ss_const_iterator_type().operator ++(); return (*this); }
				xscope_const_iterator operator++(int) { xscope_const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_const_iterator& operator --() { msebasic_string_xscope_ss_const_iterator_type().operator --(); return (*this); }
				xscope_const_iterator operator--(int) { xscope_const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MBS::difference_type n) { msebasic_string_xscope_ss_const_iterator_type().advance(n); }
				void regress(typename _MBS::difference_type n) { msebasic_string_xscope_ss_const_iterator_type().regress(n); }
				xscope_const_iterator& operator +=(difference_type n) { msebasic_string_xscope_ss_const_iterator_type().operator +=(n); return (*this); }
				xscope_const_iterator& operator -=(difference_type n) { msebasic_string_xscope_ss_const_iterator_type().operator -=(n); return (*this); }
				xscope_const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MBS::difference_type operator-(const xscope_const_iterator& _Right_cref) const { return msebasic_string_xscope_ss_const_iterator_type() - (_Right_cref.msebasic_string_xscope_ss_const_iterator_type()); }
				typename _MBS::const_reference operator*() const { return msebasic_string_xscope_ss_const_iterator_type().operator*(); }
				typename _MBS::const_reference item() const { return operator*(); }
				typename _MBS::const_reference previous_item() const { return msebasic_string_xscope_ss_const_iterator_type().previous_item(); }
				typename _MBS::const_pointer operator->() const { return msebasic_string_xscope_ss_const_iterator_type().operator->(); }
				typename _MBS::const_reference operator[](typename _MBS::difference_type _Off) const { return msebasic_string_xscope_ss_const_iterator_type()[_Off]; }
				xscope_const_iterator& operator=(const xscope_const_iterator& _Right_cref) {
					msebasic_string_xscope_ss_const_iterator_type().operator=(_Right_cref.msebasic_string_xscope_ss_const_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const xscope_iterator& _Right_cref) {
					msebasic_string_xscope_ss_const_iterator_type().operator=(_Right_cref.msebasic_string_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::const_iterator& _Right_cref) {
					//msebasic_string_xscope_ss_const_iterator_type().operator=(_Right_cref.msebasic_string_reg_ss_const_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdbasic_string_range_error("invalid assignment - mse::mstd::basic_string<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//msebasic_string_xscope_ss_const_iterator_type().operator=(_Right_cref.msebasic_string_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdbasic_string_range_error("invalid assignment - mse::mstd::basic_string<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_const_iterator& _Right_cref) const { return msebasic_string_xscope_ss_const_iterator_type().operator==(_Right_cref.msebasic_string_xscope_ss_const_iterator_type()); }
				bool operator!=(const xscope_const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_const_iterator& _Right) const { return (msebasic_string_xscope_ss_const_iterator_type() < _Right.msebasic_string_xscope_ss_const_iterator_type()); }
				bool operator<=(const xscope_const_iterator& _Right) const { return (msebasic_string_xscope_ss_const_iterator_type() <= _Right.msebasic_string_xscope_ss_const_iterator_type()); }
				bool operator>(const xscope_const_iterator& _Right) const { return (msebasic_string_xscope_ss_const_iterator_type() > _Right.msebasic_string_xscope_ss_const_iterator_type()); }
				bool operator>=(const xscope_const_iterator& _Right) const { return (msebasic_string_xscope_ss_const_iterator_type() >= _Right.msebasic_string_xscope_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const xscope_const_iterator& _Right_cref) { msebasic_string_xscope_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msebasic_string_xscope_ss_const_iterator_type()); }
				msear_size_t position() const { return msebasic_string_xscope_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msebasic_string_xscope_ss_const_iterator_type().target_container_ptr()) {
					return msebasic_string_xscope_ss_const_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename _MBS::xscope_ss_const_iterator_type m_xscope_ss_const_iterator;
				friend class /*_Myt*/basic_string<_Ty, _Traits>;
			};
			class xscope_iterator : public _MBS::random_access_iterator_base, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				typedef typename _MBS::random_access_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<basic_string> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<basic_string> >::value
					, void>::type>
					xscope_iterator(const _TXScopePointer& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscope_pointer_to_member(*((*owner_ptr).m_shptr), owner_ptr)) {}

				xscope_iterator(const xscope_iterator& src_cref) : m_xscope_ss_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_iterator() {}
				const typename _MBS::xscope_ss_iterator_type& msebasic_string_xscope_ss_iterator_type() const {
					return m_xscope_ss_iterator;
				}
				typename _MBS::xscope_ss_iterator_type& msebasic_string_xscope_ss_iterator_type() {
					return m_xscope_ss_iterator;
				}
				const typename _MBS::xscope_ss_iterator_type& mvssi() const { return msebasic_string_xscope_ss_iterator_type(); }
				typename _MBS::xscope_ss_iterator_type& mvssi() { return msebasic_string_xscope_ss_iterator_type(); }

				void reset() { msebasic_string_xscope_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msebasic_string_xscope_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msebasic_string_xscope_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msebasic_string_xscope_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msebasic_string_xscope_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msebasic_string_xscope_ss_iterator_type().has_next(); }
				bool has_previous() const { return msebasic_string_xscope_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msebasic_string_xscope_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msebasic_string_xscope_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msebasic_string_xscope_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msebasic_string_xscope_ss_iterator_type().set_to_previous(); }
				xscope_iterator& operator ++() { msebasic_string_xscope_ss_iterator_type().operator ++(); return (*this); }
				xscope_iterator operator++(int) { xscope_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_iterator& operator --() { msebasic_string_xscope_ss_iterator_type().operator --(); return (*this); }
				xscope_iterator operator--(int) { xscope_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MBS::difference_type n) { msebasic_string_xscope_ss_iterator_type().advance(n); }
				void regress(typename _MBS::difference_type n) { msebasic_string_xscope_ss_iterator_type().regress(n); }
				xscope_iterator& operator +=(difference_type n) { msebasic_string_xscope_ss_iterator_type().operator +=(n); return (*this); }
				xscope_iterator& operator -=(difference_type n) { msebasic_string_xscope_ss_iterator_type().operator -=(n); return (*this); }
				xscope_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MBS::difference_type operator-(const xscope_iterator& _Right_cref) const { return msebasic_string_xscope_ss_iterator_type() - (_Right_cref.msebasic_string_xscope_ss_iterator_type()); }
				typename _MBS::reference operator*() const { return msebasic_string_xscope_ss_iterator_type().operator*(); }
				typename _MBS::reference item() const { return operator*(); }
				typename _MBS::reference previous_item() const { return msebasic_string_xscope_ss_iterator_type().previous_item(); }
				typename _MBS::pointer operator->() const { return msebasic_string_xscope_ss_iterator_type().operator->(); }
				typename _MBS::reference operator[](typename _MBS::difference_type _Off) const { return msebasic_string_xscope_ss_iterator_type()[_Off]; }
				xscope_iterator& operator=(const xscope_iterator& _Right_cref) {
					msebasic_string_xscope_ss_iterator_type().operator=(_Right_cref.msebasic_string_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//msebasic_string_xscope_ss_iterator_type().operator=(_Right_cref.msebasic_string_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdbasic_string_range_error("invalid assignment - mse::mstd::basic_string<>::xscope_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_iterator& _Right_cref) const { return msebasic_string_xscope_ss_iterator_type().operator==(_Right_cref.msebasic_string_xscope_ss_iterator_type()); }
				bool operator!=(const xscope_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_iterator& _Right) const { return (msebasic_string_xscope_ss_iterator_type() < _Right.msebasic_string_xscope_ss_iterator_type()); }
				bool operator<=(const xscope_iterator& _Right) const { return (msebasic_string_xscope_ss_iterator_type() <= _Right.msebasic_string_xscope_ss_iterator_type()); }
				bool operator>(const xscope_iterator& _Right) const { return (msebasic_string_xscope_ss_iterator_type() > _Right.msebasic_string_xscope_ss_iterator_type()); }
				bool operator>=(const xscope_iterator& _Right) const { return (msebasic_string_xscope_ss_iterator_type() >= _Right.msebasic_string_xscope_ss_iterator_type()); }
				void set_to_item_pointer(const xscope_iterator& _Right_cref) { msebasic_string_xscope_ss_iterator_type().set_to_item_pointer(_Right_cref.msebasic_string_xscope_ss_iterator_type()); }
				msear_size_t position() const { return msebasic_string_xscope_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msebasic_string_xscope_ss_iterator_type().target_container_ptr()) {
					return msebasic_string_xscope_ss_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename _MBS::xscope_ss_iterator_type m_xscope_ss_iterator;
				friend class /*_Myt*/basic_string<_Ty, _Traits>;
				friend class xscope_const_iterator;
			};

			/* For each (scope) basic_string instance, only one instance of xscope_structure_change_lock_guard may exist at any one
			time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the basic_string do not become invalid by preventing any operation that might resize the basic_string
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			class xscope_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_structure_change_lock_guard(const mse::TXScopeFixedPointer<basic_string>& owner_ptr)
					: m_MBS_xscope_structure_change_lock_guard(mse::us::unsafe_make_xscope_pointer_to(*((*owner_ptr).m_shptr))) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_structure_change_lock_guard(const mse::TXScopeItemFixedPointer<basic_string>& owner_ptr)
					: m_MBS_xscope_structure_change_lock_guard(mse::us::unsafe_make_xscope_pointer_to(*((*owner_ptr).m_shptr))) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return m_MBS_xscope_structure_change_lock_guard.xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const xscope_iterator& iter) const {
					assert(std::addressof(*(iter.target_container_ptr())) == std::addressof(*target_container_ptr()));
					return xscope_ptr_to_element(iter.position());
				}
				auto target_container_ptr() const {
					return m_MBS_xscope_structure_change_lock_guard.target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename mse::us::msebasic_string<_Ty, _Traits>::xscope_structure_change_lock_guard m_MBS_xscope_structure_change_lock_guard;
			};
			class xscope_const_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_const_structure_change_lock_guard(const mse::TXScopeFixedConstPointer<basic_string>& owner_ptr)
					: m_MBS_xscope_const_structure_change_lock_guard(mse::us::unsafe_make_xscope_const_pointer_to(*((*owner_ptr).m_shptr))) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_const_structure_change_lock_guard(const mse::TXScopeItemFixedConstPointer<basic_string>& owner_ptr)
					: m_MBS_xscope_const_structure_change_lock_guard(mse::us::unsafe_make_xscope_const_pointer_to(*((*owner_ptr).m_shptr))) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return m_MBS_xscope_const_structure_change_lock_guard.xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const xscope_const_iterator& citer) const {
					assert(std::addressof(*(citer.target_container_ptr())) == std::addressof(*target_container_ptr()));
					return xscope_ptr_to_element(citer.position());
				}
				auto target_container_ptr() const {
					return m_MBS_xscope_const_structure_change_lock_guard.target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename mse::us::msebasic_string<_Ty, _Traits>::xscope_const_structure_change_lock_guard m_MBS_xscope_const_structure_change_lock_guard;
			};

			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

		private:
			static std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>&& _Istr, basic_string& _Str) {
				return std::forward<decltype(_Istr)>(_Istr) >> _Str.msebasic_string();
			}
			static std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>& _Istr, basic_string& _Str) {
				return _Istr >> _Str.msebasic_string();
			}
			static std::basic_ostream<_Ty, _Traits>& out_to_stream(std::basic_ostream<_Ty, _Traits>& _Ostr, const basic_string& _Str) {
				return _Ostr << _Str.msebasic_string();
			}

			const _MBS& msebasic_string() const { return (*m_shptr); }
			_MBS& msebasic_string() { return (*m_shptr); }

			std::shared_ptr<_MBS> m_shptr;

			friend struct std::hash<basic_string>;
			template<class _Ty2, class _Traits2/* = std::char_traits<_Ty2>*/, class _A2/* = std::allocator<_Ty2> */>
			friend std::basic_istream<_Ty2, _Traits2>& impl::bs::in_from_stream(std::basic_istream<_Ty2, _Traits2>&& _Istr, basic_string<_Ty2, _Traits2, _A2>& _Str);
			template<class _Ty2, class _Traits2/* = std::char_traits<_Ty2>*/, class _A2/* = std::allocator<_Ty2> */>
			friend std::basic_istream<_Ty2, _Traits2>& impl::bs::in_from_stream(std::basic_istream<_Ty2, _Traits2>& _Istr, basic_string<_Ty2, _Traits2, _A2>& _Str);
			template<class _Ty2, class _Traits2/* = std::char_traits<_Ty2>*/, class _A2/* = std::allocator<_Ty2> */>
			friend std::basic_ostream<_Ty2, _Traits2>& impl::bs::out_to_stream(std::basic_ostream<_Ty2, _Traits2>& _Ostr, const basic_string<_Ty2, _Traits2, _A2>& _Str);
		};

		namespace impl {
			namespace bs {
				template<class _Ty, class _Traits/* = std::char_traits<_Ty>*/, class _A/* = std::allocator<_Ty> */>
				std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>&& _Istr, basic_string<_Ty, _Traits, _A>& _Str) {
					return _Str.in_from_stream(std::forward<decltype(_Istr)>(_Istr), _Str);
				}
				template<class _Ty, class _Traits/* = std::char_traits<_Ty>*/, class _A/* = std::allocator<_Ty> */>
				std::basic_istream<_Ty, _Traits>& in_from_stream(std::basic_istream<_Ty, _Traits>& _Istr, basic_string<_Ty, _Traits, _A>& _Str) {
					return _Str.in_from_stream(_Istr, _Str);
				}
				template<class _Ty, class _Traits/* = std::char_traits<_Ty>*/, class _A/* = std::allocator<_Ty> */>
				std::basic_ostream<_Ty, _Traits>& out_to_stream(std::basic_ostream<_Ty, _Traits>& _Ostr, const basic_string<_Ty, _Traits, _A>& _Str) {
					return _Str.out_to_stream(_Ostr, _Str);
				}
			}
		}

		template<class _Ty, class _Traits>
		std::basic_istream<_Ty, _Traits>& operator>>(std::basic_istream<_Ty, _Traits>&& _Istr, basic_string<_Ty, _Traits>& _Str) {
			return impl::bs::in_from_stream(std::forward<decltype(_Istr)>(_Istr), _Str);
		}
		template<class _Ty, class _Traits>
		std::basic_istream<_Ty, _Traits>& operator>>(std::basic_istream<_Ty, _Traits>& _Istr, basic_string<_Ty, _Traits>& _Str) {
			return impl::bs::in_from_stream(_Istr, _Str);
		}
		template<class _Ty, class _Traits>
		std::basic_ostream<_Ty, _Traits>& operator<<(std::basic_ostream<_Ty, _Traits>& _Ostr, const basic_string<_Ty, _Traits>& _Str) {
			return impl::bs::out_to_stream(_Ostr, _Str);
		}

		template<class _Ty, class _Traits, class _Alloc> inline bool operator!=(const basic_string<_Ty, _Traits, _Alloc>& _Left, const basic_string<_Ty, _Traits, _Alloc>& _Right) {	// test for basic_string inequality
			return (!(_Left == _Right));
		}
		template<class _Ty, class _Traits, class _Alloc> inline bool operator>(const basic_string<_Ty, _Traits, _Alloc>& _Left, const basic_string<_Ty, _Traits, _Alloc>& _Right) {	// test if _Left > _Right for basic_strings
			return (_Right < _Left);
		}
		template<class _Ty, class _Traits, class _Alloc> inline bool operator<=(const basic_string<_Ty, _Traits, _Alloc>& _Left, const basic_string<_Ty, _Traits, _Alloc>& _Right) {	// test if _Left <= _Right for basic_strings
			return (!(_Right < _Left));
		}
		template<class _Ty, class _Traits, class _Alloc> inline bool operator>=(const basic_string<_Ty, _Traits, _Alloc>& _Left, const basic_string<_Ty, _Traits, _Alloc>& _Right) {	// test if _Left >= _Right for basic_strings
			return (!(_Left < _Right));
		}

		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const basic_string<_Elem, _Traits, _Alloc>& _Left,
			const basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return string + string
			basic_string<_Elem, _Traits, _Alloc> _Ans;
			_Ans.reserve(_Left.size() + _Right.size());
			_Ans += _Left;
			_Ans += _Right;
			return (_Ans);
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem * const _Left,
			const basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return NTCTS + string
			using _String_type = basic_string<_Elem, _Traits, _Alloc>;
			using _Size_type = typename _String_type::size_type;
			_String_type _Ans;
			_Ans.reserve(_Size_type(_Traits::length(_Left) + _Right.size()));
			_Ans += _Left;
			_Ans += _Right;
			return (_Ans);
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem _Left,
			const basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return character + string
			basic_string<_Elem, _Traits, _Alloc> _Ans;
			_Ans.reserve(1 + _Right.size());
			_Ans += _Left;
			_Ans += _Right;
			return (_Ans);
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const basic_string<_Elem, _Traits, _Alloc>& _Left,
			const _Elem * const _Right) {	// return string + NTCTS
			using _String_type = basic_string<_Elem, _Traits, _Alloc>;
			using _Size_type = typename _String_type::size_type;
			_String_type _Ans;
			_Ans.reserve(_Size_type(_Left.size() + _Traits::length(_Right)));
			_Ans += _Left;
			_Ans += _Right;
			return (_Ans);
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const basic_string<_Elem, _Traits, _Alloc>& _Left,
			const _Elem _Right) {	// return string + character
			basic_string<_Elem, _Traits, _Alloc> _Ans;
			_Ans.reserve(_Left.size() + 1);
			_Ans += _Left;
			_Ans += _Right;
			return (_Ans);
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const basic_string<_Elem, _Traits, _Alloc>& _Left,
			basic_string<_Elem, _Traits, _Alloc>&& _Right) {	// return string + string
			return (_STD move(_Right.insert(0, _Left)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const basic_string<_Elem, _Traits, _Alloc>& _Right) {	// return string + string
			return (_STD move(_Left.append(_Right)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(basic_string<_Elem, _Traits, _Alloc>&& _Left,
			basic_string<_Elem, _Traits, _Alloc>&& _Right) {	// return string + string
			if (_Right.size() <= _Left.capacity() - _Left.size()
				|| _Right.capacity() - _Right.size() < _Left.size())
				return (_STD move(_Left.append(_Right)));
			else
				return (_STD move(_Right.insert(0, _Left)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem * const _Left,
			basic_string<_Elem, _Traits, _Alloc>&& _Right) {
			return (_STD move(_Right.insert(0, _Left)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(const _Elem _Left,
			basic_string<_Elem, _Traits, _Alloc>&& _Right) {
			using size_type = typename std::allocator_traits<_Alloc>::size_type;
			return (_STD move(_Right.insert(static_cast<size_type>(0), static_cast<size_type>(1), _Left)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const _Elem * const _Right) {
			return (_STD move(_Left.append(_Right)));
		}
		template<class _Elem, class _Traits, class _Alloc>
		inline basic_string<_Elem, _Traits, _Alloc> operator+(basic_string<_Elem, _Traits, _Alloc>&& _Left,
			const _Elem _Right) {	// return string + character
			_Left.push_back(_Right);
			return (_STD move(_Left));
		}

		/* For each (scope) basic_string instance, only one instance of xscope_structure_change_lock_guard may exist at any one
		time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
		individual elements in the basic_string do not become invalid by preventing any operation that might resize the basic_string
		or increase its capacity. Any attempt to execute such an operation would result in an exception. */
		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
		auto make_xscope_basic_string_size_change_lock_guard(const mse::TXScopeFixedPointer<basic_string<_Ty, _Traits, _A> >& owner_ptr) {
			return typename basic_string<_Ty, _Traits, _A>::xscope_structure_change_lock_guard(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
		auto make_xscope_basic_string_size_change_lock_guard(const mse::TXScopeItemFixedPointer<basic_string<_Ty, _Traits, _A> >& owner_ptr) {
			return typename basic_string<_Ty, _Traits, _A>::xscope_structure_change_lock_guard(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
		auto make_xscope_basic_string_size_change_lock_guard(const mse::TXScopeFixedConstPointer<basic_string<_Ty, _Traits, _A> >& owner_ptr) {
			return basic_string<_Ty, _Traits, _A>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
		auto make_xscope_basic_string_size_change_lock_guard(const mse::TXScopeItemFixedConstPointer<basic_string<_Ty, _Traits, _A> >& owner_ptr) {
			return basic_string<_Ty, _Traits, _A>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	}
}

namespace std {

	template<class _Elem, class _Traits, class _Alloc>
	struct hash<mse::mstd::basic_string<_Elem, _Traits, _Alloc> > {	// hash functor for mse::mstd::basic_string
		typedef typename mse::mstd::basic_string<_Elem, _Traits, _Alloc>::_MBS base_string_t;
		using argument_type = mse::mstd::basic_string<_Elem, _Traits, _Alloc>;
		using result_type = size_t;

		size_t operator()(const mse::mstd::basic_string<_Elem, _Traits, _Alloc>& _Keyval) const _NOEXCEPT {
			auto retval = m_bs_hash(_Keyval.msebasic_string());
			return retval;
		}

		hash<base_string_t> m_bs_hash;
	};

	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>&& _Istr,
		mse::mstd::basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim) {	// get characters into string, discard delimiter
		return _Str.getline(std::forward<decltype(_Istr)>(_Istr), &_Str, _Delim);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>&& _Istr,
		mse::mstd::basic_string<_Elem, _Traits, _Alloc>& _Str) {	// get characters into string, discard newline
		return _Str.getline(std::forward<decltype(_Istr)>(_Istr), &_Str);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>& _Istr,
		mse::mstd::basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim) {	// get characters into string, discard delimiter
		return _Str.getline(_Istr, &_Str, _Delim);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>& getline(basic_istream<_Elem, _Traits>& _Istr,
		mse::mstd::basic_string<_Elem, _Traits, _Alloc>& _Str) {	// get characters into string, discard newline
		return _Str.getline(_Istr, &_Str);
	}


	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty> >
	void swap(mse::mstd::basic_string<_Ty, _Traits, _A>& _Left, mse::mstd::basic_string<_Ty, _Traits, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap basic_strings
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::basic_string<_Ty, _Traits, _A>& _Left, mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap basic_strings
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::basic_string<_Ty, _Traits, _A>& _Left, mse::us::msebasic_string<_Ty, _Traits, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap basic_strings
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::basic_string<_Ty, _Traits, _A>& _Left, std::basic_string<_Ty, _Traits, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap basic_strings
		return (_Left.swap(_Right));
	}

	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_basic_string<_Ty, _Traits, _A, _TStateMutex>& _Left, mse::mstd::basic_string<_Ty, _Traits, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap basic_strings
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::us::msebasic_string<_Ty, _Traits, _A, _TStateMutex>& _Left, mse::mstd::basic_string<_Ty, _Traits, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap basic_strings
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _Traits = std::char_traits<_Ty>, class _A = std::allocator<_Ty>/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(std::basic_string<_Ty, _Traits, _A>& _Left, mse::mstd::basic_string<_Ty, _Traits, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap basic_strings
		return (_Right.swap(_Left));
	}
}

namespace mse {
	namespace mstd {

#endif /*MSE_MSTDSTRING_DISABLED*/

		using string = basic_string<char>;
		using wstring = basic_string<wchar_t>;
		using u16string = basic_string<char16_t>;
		using u32string = basic_string<char32_t>;

		inline namespace literals {
			inline namespace string_literals {
				inline mse::mstd::basic_string<char> operator "" _mstds(const char *__str, size_t __len) _NOEXCEPT {
					return mse::mstd::basic_string<char>(__str, __len);
				}
				inline mse::mstd::basic_string<wchar_t> operator "" _mstds(const wchar_t *__str, size_t __len) _NOEXCEPT {
					return mse::mstd::basic_string<wchar_t>(__str, __len);
				}
				inline mse::mstd::basic_string<char16_t> operator "" _mstds(const char16_t *__str, size_t __len) _NOEXCEPT {
					return mse::mstd::basic_string<char16_t>(__str, __len);
				}
				inline mse::mstd::basic_string<char32_t> operator "" _mstds(const char32_t *__str, size_t __len) _NOEXCEPT {
					return mse::mstd::basic_string<char32_t>(__str, __len);
				}
			}
		}
	}
}

#endif /*ndef MSEMSTDSTRING_H*/
