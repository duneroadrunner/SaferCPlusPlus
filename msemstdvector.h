
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSTDVECTOR_H
#define MSEMSTDVECTOR_H

#include "msemsevector.h"
#include "msemstdarray.h"

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDVECTOR_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

#ifdef MSE_CUSTOM_THROW_DEFINITION
#include <iostream>
#define MSE_THROW(x) MSE_CUSTOM_THROW_DEFINITION(x)
#else // MSE_CUSTOM_THROW_DEFINITION
#define MSE_THROW(x) throw(x)
#endif // MSE_CUSTOM_THROW_DEFINITION

namespace mse {

	namespace mstd {

#ifdef MSE_MSTDVECTOR_DISABLED
		template<class _Ty, class _A = std::allocator<_Ty> > using vector = std::vector<_Ty, _A>;

		template<class _Ty, class _A = std::allocator<_Ty> >
		class xscope_structure_change_lock_guard : public XScopeTagBase {
		public:
			xscope_structure_change_lock_guard(const mse::TXScopeFixedPointer<vector<_Ty, _A> >& owner_ptr)
				: m_owner_ptr(owner_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			xscope_structure_change_lock_guard(const mse::TXScopeItemFixedPointer<vector<_Ty, _A> >& owner_ptr)
				: m_owner_ptr(owner_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

			auto xscope_ptr_to_element(size_t _P) const {
				return mse::us::unsafe_make_xscope_pointer_to((*m_owner_ptr).at(_P));
			}
			auto xscope_ptr_to_element(const xscope_array_iterator<vector<_Ty, _A> >& iter) const {
				return xscope_ptr_to_element(iter - (*m_owner_ptr).begin());
			}
			auto target_container_ptr() const {
				return m_owner_ptr;
			}
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		private:
			mse::TXScopeItemFixedPointer<vector<_Ty, _A> > m_owner_ptr;
		};
		template<class _Ty, class _A = std::allocator<_Ty> >
		class xscope_const_structure_change_lock_guard : public XScopeTagBase {
		public:
			xscope_const_structure_change_lock_guard(const mse::TXScopeFixedConstPointer<vector<_Ty, _A> >& owner_ptr)
				: m_owner_ptr(owner_ptr) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
			xscope_const_structure_change_lock_guard(const mse::TXScopeItemFixedConstPointer<vector<_Ty, _A> >& owner_ptr)
				: m_owner_ptr(owner_ptr) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

			auto xscope_ptr_to_element(size_t _P) const {
				return mse::us::unsafe_make_xscope_const_pointer_to((*m_owner_ptr).at(_P));
			}
			auto xscope_ptr_to_element(const xscope_array_const_iterator<vector<_Ty, _A> >& citer) const {
				return xscope_ptr_to_element(citer - (*m_owner_ptr).cbegin());
			}
			auto target_container_ptr() const {
				return m_owner_ptr;
			}
			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
		private:
			mse::TXScopeItemFixedConstPointer<vector<_Ty, _A> > m_owner_ptr;
		};

		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedPointer<vector<_Ty, _A> >& owner_ptr) {
			return xscope_structure_change_lock_guard<_Ty, _A>(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedPointer<vector<_Ty, _A> >& owner_ptr) {
			return xscope_structure_change_lock_guard<_Ty, _A>(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedConstPointer<vector<_Ty, _A> >& owner_ptr) {
			return xscope_const_structure_change_lock_guard<_Ty, _A>(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedConstPointer<vector<_Ty, _A> >& owner_ptr) {
			return xscope_const_structure_change_lock_guard<_Ty, _A>(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

#else /*MSE_MSTDVECTOR_DISABLED*/

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

		class mstdvector_range_error : public std::range_error {
		public:
			using std::range_error::range_error;
		};

		template<class _Ty, class _A = std::allocator<_Ty> >
		class vector {
		public:
			typedef vector _Myt;
			typedef mse::us::msevector<_Ty, _A> _MV;

			typedef typename _MV::allocator_type allocator_type;
			typedef typename _MV::value_type value_type;
			typedef typename _MV::size_type size_type;
			typedef typename _MV::difference_type difference_type;
			typedef typename _MV::pointer pointer;
			typedef typename _MV::const_pointer const_pointer;
			typedef typename _MV::reference reference;
			typedef typename _MV::const_reference const_reference;

			operator mse::nii_vector<_Ty, _A>() const { return msevector(); }
			operator std::vector<_Ty, _A>() const { return msevector(); }

			explicit vector(const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_Al)) {}
			explicit vector(size_type _N) : m_shptr(std::make_shared<_MV>(_N)) {}
			explicit vector(size_type _N, const _Ty& _V, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_N, _V, _Al)) {}
			vector(_Myt&& _X) : m_shptr(std::make_shared<_MV>()) {
				static_assert(typename std::is_rvalue_reference<decltype(_X)>::type(), "");
				/* It would be more efficient to just move _X.m_shptr into m_shptr, but that would leave _X in what we
				would consider an invalid state. */
				msevector() = std::move(_X.msevector());
			}
			vector(const _Myt& _X) : m_shptr(std::make_shared<_MV>(_X.msevector())) {}
			vector(_MV&& _X) : m_shptr(std::make_shared<_MV>(std::forward<decltype(_X)>(_X))) {}
			vector(const _MV& _X) : m_shptr(std::make_shared<_MV>(_X)) {}
			vector(std::vector<_Ty>&& _X) : m_shptr(std::make_shared<_MV>(std::forward<decltype(_X)>(_X))) {}
			vector(const std::vector<_Ty>& _X) : m_shptr(std::make_shared<_MV>(_X)) {}
			typedef typename _MV::const_iterator _It;
			vector(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_F, _L, _Al)) {}
			vector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_F, _L, _Al)) {}
			template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			vector(_Iter _First, _Iter _Last) : m_shptr(std::make_shared<_MV>(_First, _Last)) {}
			template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			vector(_Iter _First, _Iter _Last, const _A& _Al) : m_shptr(std::make_shared<_MV>(_First, _Last, _Al)) {}

			_Myt& operator=(_MV&& _X) { msevector() = (std::forward<decltype(_X)>(_X)); return (*this); }
			_Myt& operator=(const _MV& _X) { msevector() = (_X); return (*this); }
			_Myt& operator=(_Myt&& _X) {
				static_assert(typename std::is_rvalue_reference<decltype(_X)>::type(), "");
				/* It would be more efficient to just move _X.m_shptr into m_shptr, but that would leave _X in what we
				would consider an invalid state. */
				msevector() = std::move(_X.msevector());
				return (*this);
			}
			_Myt& operator=(const _Myt& _X) { msevector() = (_X.msevector()); return (*this); }
			void reserve(size_type _Count) { m_shptr->reserve(_Count); }
			void resize(size_type _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
			typename _MV::const_reference operator[](size_type _P) const { return m_shptr->operator[](_P); }
			typename _MV::reference operator[](size_type _P) { return m_shptr->operator[](_P); }
			void push_back(_Ty&& _X) { m_shptr->push_back(std::forward<decltype(_X)>(_X)); }
			void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
			void pop_back() { m_shptr->pop_back(); }
			void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
			void assign(size_type _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
			template<class ..._Valty>
			void emplace_back(_Valty&& ..._Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)...); }
			void clear() { m_shptr->clear(); }
			void swap(_MV& _X) { m_shptr->swap(_X); }
			void swap(_Myt& _X) { m_shptr->swap(_X.msevector()); }
			void swap(mse::nii_vector<_Ty, _A>& _X) { m_shptr->swap(_X); }
			void swap(std::vector<_Ty, _A>& _X) { m_shptr->swap(_X); }

			vector(_XSTD initializer_list<typename _MV::value_type> _Ilist, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_Ilist, _Al)) {}
			_Myt& operator=(_XSTD initializer_list<typename _MV::value_type> _Ilist) { msevector() = (_Ilist); return (*this); }
			void assign(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->assign(_Ilist); }

			size_type capacity() const _NOEXCEPT { return m_shptr->capacity(); }
			void shrink_to_fit() { m_shptr->shrink_to_fit(); }
			size_type size() const _NOEXCEPT { return m_shptr->size(); }
			size_type max_size() const _NOEXCEPT { return m_shptr->max_size(); }
			bool empty() const _NOEXCEPT { return m_shptr->empty(); }
			_A get_allocator() const _NOEXCEPT { return m_shptr->get_allocator(); }
			typename _MV::const_reference at(size_type _Pos) const { return m_shptr->at(_Pos); }
			typename _MV::reference at(size_type _Pos) { return m_shptr->at(_Pos); }
			typename _MV::reference front() { return m_shptr->front(); }
			typename _MV::const_reference front() const { return m_shptr->front(); }
			typename _MV::reference back() { return m_shptr->back(); }
			typename _MV::const_reference back() const { return m_shptr->back(); }

			/* Try to avoid using these whenever possible. */
			value_type *data() _NOEXCEPT {
				return m_shptr->data();
			}
			const value_type *data() const _NOEXCEPT {
				return m_shptr->data();
			}

			class xscope_const_iterator;
			class xscope_iterator;

			class const_iterator : public _MV::random_access_const_iterator_base {
			public:
				typedef typename _MV::random_access_const_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				const_iterator() {}
				const_iterator(const const_iterator& src_cref) : m_msevector_cshptr(src_cref.m_msevector_cshptr) {
					(*this) = src_cref;
				}
				~const_iterator() {}
				const typename _MV::ss_const_iterator_type& msevector_ss_const_iterator_type() const { return m_ss_const_iterator; }
				typename _MV::ss_const_iterator_type& msevector_ss_const_iterator_type() { return m_ss_const_iterator; }
				const typename _MV::ss_const_iterator_type& mvssci() const { return msevector_ss_const_iterator_type(); }
				typename _MV::ss_const_iterator_type& mvssci() { return msevector_ss_const_iterator_type(); }

				void reset() { msevector_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msevector_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_ss_const_iterator_type().set_to_previous(); }
				const_iterator& operator ++() { msevector_ss_const_iterator_type().operator ++(); return (*this); }
				const_iterator operator++(int) { const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				const_iterator& operator --() { msevector_ss_const_iterator_type().operator --(); return (*this); }
				const_iterator operator--(int) { const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_ss_const_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_ss_const_iterator_type().regress(n); }
				const_iterator& operator +=(difference_type n) { msevector_ss_const_iterator_type().operator +=(n); return (*this); }
				const_iterator& operator -=(difference_type n) { msevector_ss_const_iterator_type().operator -=(n); return (*this); }
				const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const const_iterator& _Right_cref) const { return msevector_ss_const_iterator_type() - (_Right_cref.msevector_ss_const_iterator_type()); }
				typename _MV::const_reference operator*() const { return msevector_ss_const_iterator_type().operator*(); }
				typename _MV::const_reference item() const { return operator*(); }
				typename _MV::const_reference previous_item() const { return msevector_ss_const_iterator_type().previous_item(); }
				typename _MV::const_pointer operator->() const { return msevector_ss_const_iterator_type().operator->(); }
				typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
				bool operator==(const const_iterator& _Right_cref) const { return msevector_ss_const_iterator_type().operator==(_Right_cref.msevector_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() < _Right.msevector_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() <= _Right.msevector_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() > _Right.msevector_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() >= _Right.msevector_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { msevector_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msevector_ss_const_iterator_type()); }
				msev_size_t position() const { return msevector_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msevector_ss_const_iterator_type().target_container_ptr()) {
					return msevector_ss_const_iterator_type().target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				const_iterator(std::shared_ptr<_MV> msevector_shptr) : m_msevector_cshptr(msevector_shptr) {
					m_ss_const_iterator = msevector_shptr->ss_cbegin();
				}
				std::shared_ptr<const _MV> m_msevector_cshptr;
				/* m_ss_const_iterator needs to be declared after m_msevector_cshptr so that its destructor will be called first. */
				typename _MV::ss_const_iterator_type m_ss_const_iterator;
				friend class /*_Myt*/vector<_Ty, _A>;
				friend class iterator;
			};
			class iterator : public _MV::random_access_iterator_base {
			public:
				typedef typename _MV::random_access_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				iterator() {}
				iterator(const iterator& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr) {
					(*this) = src_cref;
				}
				~iterator() {}
				const typename _MV::ss_iterator_type& msevector_ss_iterator_type() const { return m_ss_iterator; }
				typename _MV::ss_iterator_type& msevector_ss_iterator_type() { return m_ss_iterator; }
				const typename _MV::ss_iterator_type& mvssi() const { return msevector_ss_iterator_type(); }
				typename _MV::ss_iterator_type& mvssi() { return msevector_ss_iterator_type(); }
				operator const_iterator() const {
					const_iterator retval(m_msevector_shptr);
					if (m_msevector_shptr) {
						retval.msevector_ss_const_iterator_type().set_to_beginning();
						retval.msevector_ss_const_iterator_type().advance(msev_int(msevector_ss_iterator_type().position()));
					}
					return retval;
				}

				void reset() { msevector_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_ss_iterator_type().has_next(); }
				bool has_previous() const { return msevector_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_ss_iterator_type().set_to_previous(); }
				iterator& operator ++() { msevector_ss_iterator_type().operator ++(); return (*this); }
				iterator operator++(int) { iterator _Tmp = *this; ++*this; return (_Tmp); }
				iterator& operator --() { msevector_ss_iterator_type().operator --(); return (*this); }
				iterator operator--(int) { iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_ss_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_ss_iterator_type().regress(n); }
				iterator& operator +=(difference_type n) { msevector_ss_iterator_type().operator +=(n); return (*this); }
				iterator& operator -=(difference_type n) { msevector_ss_iterator_type().operator -=(n); return (*this); }
				iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const iterator& _Right_cref) const { return msevector_ss_iterator_type() - (_Right_cref.msevector_ss_iterator_type()); }
				typename _MV::reference operator*() const { return msevector_ss_iterator_type().operator*(); }
				typename _MV::reference item() const { return operator*(); }
				typename _MV::reference previous_item() const { return msevector_ss_iterator_type().previous_item(); }
				typename _MV::pointer operator->() const { return msevector_ss_iterator_type().operator->(); }
				typename _MV::reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
				bool operator==(const iterator& _Right_cref) const { return msevector_ss_iterator_type().operator==(_Right_cref.msevector_ss_iterator_type()); }
				bool operator!=(const iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const iterator& _Right) const { return (msevector_ss_iterator_type() < _Right.msevector_ss_iterator_type()); }
				bool operator<=(const iterator& _Right) const { return (msevector_ss_iterator_type() <= _Right.msevector_ss_iterator_type()); }
				bool operator>(const iterator& _Right) const { return (msevector_ss_iterator_type() > _Right.msevector_ss_iterator_type()); }
				bool operator>=(const iterator& _Right) const { return (msevector_ss_iterator_type() >= _Right.msevector_ss_iterator_type()); }
				void set_to_item_pointer(const iterator& _Right_cref) { msevector_ss_iterator_type().set_to_item_pointer(_Right_cref.msevector_ss_iterator_type()); }
				msev_size_t position() const { return msevector_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msevector_ss_iterator_type().target_container_ptr()) {
					return msevector_ss_iterator_type().target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				std::shared_ptr<_MV> m_msevector_shptr;
				/* m_ss_iterator needs to be declared after m_msevector_shptr so that its destructor will be called first. */
				typename _MV::ss_iterator_type m_ss_iterator;
				friend class /*_Myt*/vector<_Ty, _A>;
			};

			iterator begin() {	// return iterator for beginning of mutable sequence
				iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(retval.m_ss_iterator) = m_shptr->ss_begin();
				return retval;
			}
			const_iterator begin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msevector_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_begin();
				return retval;
			}
			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(retval.m_ss_iterator) = m_shptr->ss_end();
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msevector_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_end();
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msevector_cshptr = this->m_shptr;
				(retval.m_ss_const_iterator) = m_shptr->ss_cbegin();
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msevector_cshptr = this->m_shptr;
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


			vector(const const_iterator &start, const const_iterator &end, const _A& _Al = _A())
				: m_shptr(std::make_shared<_MV>(start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type(), _Al)) {}
			void assign(const const_iterator &start, const const_iterator &end) {
				m_shptr->assign(start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type());
			}
			void assign_inclusive(const const_iterator &first, const const_iterator &last) {
				m_shptr->assign_inclusive(first.msevector_ss_const_iterator_type(), last.msevector_ss_const_iterator_type());
			}
			iterator insert_before(const const_iterator &pos, size_type _M, const _Ty& _X) {
				auto res = m_shptr->insert_before(pos.msevector_ss_const_iterator_type(), _M, _X);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(const const_iterator &pos, _Ty&& _X) {
				auto res = m_shptr->insert_before(pos.msevector_ss_const_iterator_type(), std::forward<decltype(_X)>(_X));
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(const const_iterator &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				iterator insert_before(const const_iterator &pos, const _Iter &start, const _Iter &end) {
				auto res = m_shptr->insert_before(pos.msevector_ss_const_iterator_type(), start, end);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			template<class _Iter
				//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
				, class = _mse_RequireInputIter<_Iter> >
				iterator insert_before_inclusive(const const_iterator &pos, const _Iter &first, const _Iter &last) {
				auto end = last; end++;
				return insert_before(pos, first, end);
			}
			iterator insert_before(const const_iterator &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
				auto res = m_shptr->insert_before(pos.msevector_ss_const_iterator_type(), _Ilist);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			void insert_before(msev_size_t pos, const _Ty& _X = _Ty()) {
				m_shptr->insert_before(pos, _X);
			}
			void insert_before(msev_size_t pos, size_type _M, const _Ty& _X) {
				m_shptr->insert_before(pos, _M, _X);
			}
			void insert_before(msev_size_t pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
				m_shptr->insert_before(pos, _Ilist);
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
			iterator insert(const const_iterator &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return insert_before(pos, _Ilist); }
			template<class ..._Valty>
			iterator emplace(const const_iterator &pos, _Valty&& ..._Val) {
				auto res = m_shptr->emplace(pos.msevector_ss_const_iterator_type(), std::forward<_Valty>(_Val)...);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator erase(const const_iterator &pos) {
				auto res = m_shptr->erase(pos.msevector_ss_const_iterator_type());
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator erase(const const_iterator &start, const const_iterator &end) {
				auto res = m_shptr->erase(start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type());
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator erase_inclusive(const const_iterator &first, const const_iterator &last) {
				auto end = last; end.set_to_next();
				return erase_inclusive(first, end);
			}
			bool operator==(const _Myt& _Right) const {	// test for vector equality
				return ((*(_Right.m_shptr)) == (*m_shptr));
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for vectors
				return ((*m_shptr) < (*(_Right.m_shptr)));
			}

			class xscope_const_iterator : public _MV::random_access_const_iterator_base, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				typedef typename _MV::random_access_const_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedConstPointer<vector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<vector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedConstPointer<vector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<vector> >::value
					, void>::type>
				xscope_const_iterator(const _TXScopePointer& owner_ptr)
					: m_xscope_ss_const_iterator(mse::make_xscope_const_pointer_to_member(*((*owner_ptr).m_shptr), owner_ptr)) {}

				xscope_const_iterator(const xscope_const_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_const_iterator) {}
				xscope_const_iterator(const xscope_iterator& src_cref) : m_xscope_ss_const_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_const_iterator() {}
				const typename _MV::xscope_ss_const_iterator_type& msevector_xscope_ss_const_iterator_type() const {
					return m_xscope_ss_const_iterator;
				}
				typename _MV::xscope_ss_const_iterator_type& msevector_xscope_ss_const_iterator_type() {
					return m_xscope_ss_const_iterator;
				}
				const typename _MV::xscope_ss_const_iterator_type& mvssci() const { return msevector_xscope_ss_const_iterator_type(); }
				typename _MV::xscope_ss_const_iterator_type& mvssci() { return msevector_xscope_ss_const_iterator_type(); }

				void reset() { msevector_xscope_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_xscope_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_xscope_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_xscope_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_xscope_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_xscope_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msevector_xscope_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_xscope_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_xscope_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_xscope_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_xscope_ss_const_iterator_type().set_to_previous(); }
				xscope_const_iterator& operator ++() { msevector_xscope_ss_const_iterator_type().operator ++(); return (*this); }
				xscope_const_iterator operator++(int) { xscope_const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_const_iterator& operator --() { msevector_xscope_ss_const_iterator_type().operator --(); return (*this); }
				xscope_const_iterator operator--(int) { xscope_const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_xscope_ss_const_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_xscope_ss_const_iterator_type().regress(n); }
				xscope_const_iterator& operator +=(difference_type n) { msevector_xscope_ss_const_iterator_type().operator +=(n); return (*this); }
				xscope_const_iterator& operator -=(difference_type n) { msevector_xscope_ss_const_iterator_type().operator -=(n); return (*this); }
				xscope_const_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_const_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const xscope_const_iterator& _Right_cref) const { return msevector_xscope_ss_const_iterator_type() - (_Right_cref.msevector_xscope_ss_const_iterator_type()); }
				typename _MV::const_reference operator*() const { return msevector_xscope_ss_const_iterator_type().operator*(); }
				typename _MV::const_reference item() const { return operator*(); }
				typename _MV::const_reference previous_item() const { return msevector_xscope_ss_const_iterator_type().previous_item(); }
				typename _MV::const_pointer operator->() const { return msevector_xscope_ss_const_iterator_type().operator->(); }
				typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return msevector_xscope_ss_const_iterator_type()[_Off]; }
				xscope_const_iterator& operator=(const xscope_const_iterator& _Right_cref) {
					msevector_xscope_ss_const_iterator_type().operator=(_Right_cref.msevector_xscope_ss_const_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const xscope_iterator& _Right_cref) {
					msevector_xscope_ss_const_iterator_type().operator=(_Right_cref.msevector_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::const_iterator& _Right_cref) {
					//msevector_xscope_ss_const_iterator_type().operator=(_Right_cref.msevector_reg_ss_const_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdvector_range_error("invalid assignment - mse::mstd::vector<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				xscope_const_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//msevector_xscope_ss_const_iterator_type().operator=(_Right_cref.msevector_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdvector_range_error("invalid assignment - mse::mstd::vector<>::xscope_const_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_const_iterator& _Right_cref) const { return msevector_xscope_ss_const_iterator_type().operator==(_Right_cref.msevector_xscope_ss_const_iterator_type()); }
				bool operator!=(const xscope_const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_const_iterator& _Right) const { return (msevector_xscope_ss_const_iterator_type() < _Right.msevector_xscope_ss_const_iterator_type()); }
				bool operator<=(const xscope_const_iterator& _Right) const { return (msevector_xscope_ss_const_iterator_type() <= _Right.msevector_xscope_ss_const_iterator_type()); }
				bool operator>(const xscope_const_iterator& _Right) const { return (msevector_xscope_ss_const_iterator_type() > _Right.msevector_xscope_ss_const_iterator_type()); }
				bool operator>=(const xscope_const_iterator& _Right) const { return (msevector_xscope_ss_const_iterator_type() >= _Right.msevector_xscope_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const xscope_const_iterator& _Right_cref) { msevector_xscope_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msevector_xscope_ss_const_iterator_type()); }
				msear_size_t position() const { return msevector_xscope_ss_const_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msevector_xscope_ss_const_iterator_type().target_container_ptr()) {
					return msevector_xscope_ss_const_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename _MV::xscope_ss_const_iterator_type m_xscope_ss_const_iterator;
				friend class /*_Myt*/vector<_Ty>;
			};
			class xscope_iterator : public _MV::random_access_iterator_base, public XScopeContainsNonOwningScopeReferenceTagBase {
			public:
				typedef typename _MV::random_access_iterator_base base_class;
				typedef typename base_class::iterator_category iterator_category;
				typedef typename base_class::value_type value_type;
				typedef typename base_class::difference_type difference_type;
				typedef typename base_class::pointer pointer;
				typedef typename base_class::reference reference;
				typedef const pointer const_pointer;
				typedef const reference const_reference;

				template <typename _TXScopePointer, class = typename std::enable_if<
					std::is_convertible<_TXScopePointer, mse::TXScopeItemFixedPointer<vector> >::value
					|| std::is_convertible<_TXScopePointer, mse::TXScopeFixedPointer<vector> >::value
					, void>::type>
				xscope_iterator(const _TXScopePointer& owner_ptr)
					: m_xscope_ss_iterator(mse::make_xscope_pointer_to_member(*((*owner_ptr).m_shptr), owner_ptr)) {}

				xscope_iterator(const xscope_iterator& src_cref) : m_xscope_ss_iterator(src_cref.m_xscope_ss_iterator) {}
				~xscope_iterator() {}
				const typename _MV::xscope_ss_iterator_type& msevector_xscope_ss_iterator_type() const {
					return m_xscope_ss_iterator;
				}
				typename _MV::xscope_ss_iterator_type& msevector_xscope_ss_iterator_type() {
					return m_xscope_ss_iterator;
				}
				const typename _MV::xscope_ss_iterator_type& mvssi() const { return msevector_xscope_ss_iterator_type(); }
				typename _MV::xscope_ss_iterator_type& mvssi() { return msevector_xscope_ss_iterator_type(); }

				void reset() { msevector_xscope_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_xscope_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_xscope_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_xscope_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_xscope_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_xscope_ss_iterator_type().has_next(); }
				bool has_previous() const { return msevector_xscope_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_xscope_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_xscope_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_xscope_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_xscope_ss_iterator_type().set_to_previous(); }
				xscope_iterator& operator ++() { msevector_xscope_ss_iterator_type().operator ++(); return (*this); }
				xscope_iterator operator++(int) { xscope_iterator _Tmp = *this; ++*this; return (_Tmp); }
				xscope_iterator& operator --() { msevector_xscope_ss_iterator_type().operator --(); return (*this); }
				xscope_iterator operator--(int) { xscope_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_xscope_ss_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_xscope_ss_iterator_type().regress(n); }
				xscope_iterator& operator +=(difference_type n) { msevector_xscope_ss_iterator_type().operator +=(n); return (*this); }
				xscope_iterator& operator -=(difference_type n) { msevector_xscope_ss_iterator_type().operator -=(n); return (*this); }
				xscope_iterator operator+(difference_type n) const { auto retval = (*this); retval += n; return retval; }
				xscope_iterator operator-(difference_type n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const xscope_iterator& _Right_cref) const { return msevector_xscope_ss_iterator_type() - (_Right_cref.msevector_xscope_ss_iterator_type()); }
				typename _MV::reference operator*() const { return msevector_xscope_ss_iterator_type().operator*(); }
				typename _MV::reference item() const { return operator*(); }
				typename _MV::reference previous_item() const { return msevector_xscope_ss_iterator_type().previous_item(); }
				typename _MV::pointer operator->() const { return msevector_xscope_ss_iterator_type().operator->(); }
				typename _MV::reference operator[](typename _MV::difference_type _Off) const { return msevector_xscope_ss_iterator_type()[_Off]; }
				xscope_iterator& operator=(const xscope_iterator& _Right_cref) {
					msevector_xscope_ss_iterator_type().operator=(_Right_cref.msevector_xscope_ss_iterator_type());
					return (*this);
				}
				xscope_iterator& operator=(const typename _Myt::iterator& _Right_cref) {
					//msevector_xscope_ss_iterator_type().operator=(_Right_cref.msevector_reg_ss_iterator_type());
					if (!(_Right_cref.target_container_ptr())
						|| (!(std::addressof(*(_Right_cref.target_container_ptr())) == std::addressof(*((*this).target_container_ptr()))))) {
						MSE_THROW(mstdvector_range_error("invalid assignment - mse::mstd::vector<>::xscope_iterator"));
					}
					(*this).set_to_beginning();
					(*this) += _Right_cref.position();
					return (*this);
				}
				bool operator==(const xscope_iterator& _Right_cref) const { return msevector_xscope_ss_iterator_type().operator==(_Right_cref.msevector_xscope_ss_iterator_type()); }
				bool operator!=(const xscope_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const xscope_iterator& _Right) const { return (msevector_xscope_ss_iterator_type() < _Right.msevector_xscope_ss_iterator_type()); }
				bool operator<=(const xscope_iterator& _Right) const { return (msevector_xscope_ss_iterator_type() <= _Right.msevector_xscope_ss_iterator_type()); }
				bool operator>(const xscope_iterator& _Right) const { return (msevector_xscope_ss_iterator_type() > _Right.msevector_xscope_ss_iterator_type()); }
				bool operator>=(const xscope_iterator& _Right) const { return (msevector_xscope_ss_iterator_type() >= _Right.msevector_xscope_ss_iterator_type()); }
				void set_to_item_pointer(const xscope_iterator& _Right_cref) { msevector_xscope_ss_iterator_type().set_to_item_pointer(_Right_cref.msevector_xscope_ss_iterator_type()); }
				msear_size_t position() const { return msevector_xscope_ss_iterator_type().position(); }
				auto target_container_ptr() const -> decltype(msevector_xscope_ss_iterator_type().target_container_ptr()) {
					return msevector_xscope_ss_iterator_type().target_container_ptr();
				}
				void xscope_tag() const {}
				void xscope_iterator_tag() const {}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename _MV::xscope_ss_iterator_type m_xscope_ss_iterator;
				friend class /*_Myt*/vector<_Ty>;
				friend class xscope_const_iterator;
			};

			/* For each (scope) vector instance, only one instance of xscope_structure_change_lock_guard may exist at any one
			time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
			individual elements in the vector do not become invalid by preventing any operation that might resize the vector
			or increase its capacity. Any attempt to execute such an operation would result in an exception. */
			class xscope_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_structure_change_lock_guard(const mse::TXScopeFixedPointer<vector>& owner_ptr)
					: m_MV_xscope_structure_change_lock_guard(mse::us::unsafe_make_xscope_pointer_to(*((*owner_ptr).m_shptr))) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_structure_change_lock_guard(const mse::TXScopeItemFixedPointer<vector>& owner_ptr)
					: m_MV_xscope_structure_change_lock_guard(mse::us::unsafe_make_xscope_pointer_to(*((*owner_ptr).m_shptr))) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return m_MV_xscope_structure_change_lock_guard.xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const xscope_iterator& iter) const {
					assert(std::addressof(*(iter.target_container_ptr())) == std::addressof(*target_container_ptr()));
					return xscope_ptr_to_element(iter.position());
				}
				auto target_container_ptr() const {
					return m_MV_xscope_structure_change_lock_guard.target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename mse::us::msevector<_Ty>::xscope_structure_change_lock_guard m_MV_xscope_structure_change_lock_guard;
			};
			class xscope_const_structure_change_lock_guard : public XScopeTagBase {
			public:
				xscope_const_structure_change_lock_guard(const mse::TXScopeFixedConstPointer<vector>& owner_ptr)
					: m_MV_xscope_const_structure_change_lock_guard(mse::us::unsafe_make_xscope_const_pointer_to(*((*owner_ptr).m_shptr))) {}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
				xscope_const_structure_change_lock_guard(const mse::TXScopeItemFixedConstPointer<vector>& owner_ptr)
					: m_MV_xscope_const_structure_change_lock_guard(mse::us::unsafe_make_xscope_const_pointer_to(*((*owner_ptr).m_shptr))) {}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

				auto xscope_ptr_to_element(size_type _P) const {
					return m_MV_xscope_const_structure_change_lock_guard.xscope_ptr_to_element(_P);
				}
				auto xscope_ptr_to_element(const xscope_const_iterator& citer) const {
					assert(std::addressof(*(citer.target_container_ptr())) == std::addressof(*target_container_ptr()));
					return xscope_ptr_to_element(citer.position());
				}
				auto target_container_ptr() const {
					return m_MV_xscope_const_structure_change_lock_guard.target_container_ptr();
				}
				void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */
			private:
				typename mse::us::msevector<_Ty>::xscope_const_structure_change_lock_guard m_MV_xscope_const_structure_change_lock_guard;
			};

			void not_async_shareable_tag() const {} /* Indication that this type is not eligible to be shared between threads. */

		private:
			const _MV& msevector() const { return (*m_shptr); }
			_MV& msevector() { return (*m_shptr); }

			std::shared_ptr<_MV> m_shptr;
		};

		template<class _Ty, class _Alloc> inline bool operator!=(const vector<_Ty, _Alloc>& _Left, const vector<_Ty, _Alloc>& _Right) {	// test for vector inequality
			return (!(_Left == _Right));
		}
		template<class _Ty, class _Alloc> inline bool operator>(const vector<_Ty, _Alloc>& _Left, const vector<_Ty, _Alloc>& _Right) {	// test if _Left > _Right for vectors
			return (_Right < _Left);
		}
		template<class _Ty, class _Alloc> inline bool operator<=(const vector<_Ty, _Alloc>& _Left, const vector<_Ty, _Alloc>& _Right) {	// test if _Left <= _Right for vectors
			return (!(_Right < _Left));
		}
		template<class _Ty, class _Alloc> inline bool operator>=(const vector<_Ty, _Alloc>& _Left, const vector<_Ty, _Alloc>& _Right) {	// test if _Left >= _Right for vectors
			return (!(_Left < _Right));
		}

		/* For each (scope) vector instance, only one instance of xscope_structure_change_lock_guard may exist at any one
		time. While an instance of xscope_structure_change_lock_guard exists it ensures that direct (scope) pointers to
		individual elements in the vector do not become invalid by preventing any operation that might resize the vector
		or increase its capacity. Any attempt to execute such an operation would result in an exception. */
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedPointer<vector<_Ty, _A> >& owner_ptr) {
			return typename vector<_Ty, _A>::xscope_structure_change_lock_guard(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedPointer<vector<_Ty, _A> >& owner_ptr) {
			return typename vector<_Ty, _A>::xscope_structure_change_lock_guard(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeFixedConstPointer<vector<_Ty, _A> >& owner_ptr) {
			return vector<_Ty, _A>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
#if !defined(MSE_SCOPEPOINTER_DISABLED)
		template<class _Ty, class _A = std::allocator<_Ty> >
		auto make_xscope_vector_size_change_lock_guard(const mse::TXScopeItemFixedConstPointer<vector<_Ty, _A> >& owner_ptr) {
			return vector<_Ty, _A>::xscope_const_structure_change_lock_guard(owner_ptr);
		}
#endif // !defined(MSE_SCOPEPOINTER_DISABLED)

	}
}

namespace std {

	template<class _Ty, class _A = std::allocator<_Ty> >
	void swap(mse::mstd::vector<_Ty, _A>& _Left, mse::mstd::vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap vectors
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::vector<_Ty, _A>& _Left, mse::nii_vector<_Ty, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap vectors
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::vector<_Ty, _A>& _Left, mse::us::msevector<_Ty, _A, _TStateMutex>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap vectors
		return (_Left.swap(_Right));
	}
	template<class _Ty, class _A = std::allocator<_Ty>/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::mstd::vector<_Ty, _A>& _Left, std::vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Left.swap(_Right)))
	{	// swap vectors
		return (_Left.swap(_Right));
	}

	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::nii_vector<_Ty, _A, _TStateMutex>& _Left, mse::mstd::vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap vectors
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _A = std::allocator<_Ty>, class _TStateMutex = mse::default_state_mutex/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(mse::us::msevector<_Ty, _A, _TStateMutex>& _Left, mse::mstd::vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap vectors
		return (_Right.swap(_Left));
	}
	template<class _Ty, class _A = std::allocator<_Ty>/*, class = enable_if_t<_Is_swappable<_Ty>::value>*/>
	void swap(std::vector<_Ty, _A>& _Left, mse::mstd::vector<_Ty, _A>& _Right) _NOEXCEPT_OP(_NOEXCEPT_OP(_Right.swap(_Left)))
	{	// swap vectors
		return (_Right.swap(_Left));
	}
}

namespace mse {
	namespace mstd {

#endif /*MSE_MSTDVECTOR_DISABLED*/
	}
}

#undef MSE_THROW

#endif /*ndef MSEMSTDVECTOR_H*/
