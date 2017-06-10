
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEIVECTOR_H
#define MSEIVECTOR_H

#include "msemsevector.h"

namespace mse {

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

	template<class _Ty, class _A = std::allocator<_Ty> >
	class ivector {
	public:
		typedef mse::ivector<_Ty, _A> _Myt;
		typedef mse::msevector<_Ty, _A> _MV;

		typedef typename _MV::allocator_type allocator_type;
		typedef typename _MV::value_type value_type;
		typedef typename _MV::size_type size_type;
		typedef typename _MV::difference_type difference_type;
		typedef typename _MV::pointer pointer;
		typedef typename _MV::const_pointer const_pointer;
		typedef typename _MV::reference reference;
		typedef typename _MV::const_reference const_reference;

		const _MV& msevector() const { return (*m_shptr); }
		_MV& msevector() { return (*m_shptr); }
		operator const _MV() const { return msevector(); }
		operator _MV() { return msevector(); }

		explicit ivector(const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_Al)) {}
		explicit ivector(size_type _N) : m_shptr(std::make_shared<_MV>(_N)) {}
		explicit ivector(size_type _N, const _Ty& _V, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_N, _V, _Al)) {}
		ivector(_MV&& _X) : m_shptr(std::make_shared<_MV>(std::move(_X))) {}
		ivector(const _MV& _X) : m_shptr(std::make_shared<_MV>(_X)) {}
		ivector(_Myt&& _X) : m_shptr(std::make_shared<_MV>(std::move(_X.msevector()))) {}
		ivector(const _Myt& _X) : m_shptr(std::make_shared<_MV>(_X.msevector())) {}
		typedef typename _MV::const_iterator _It;
		ivector(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_F, _L, _Al)) {}
		ivector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_F, _L, _Al)) {}
		template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			ivector(_Iter _First, _Iter _Last) : m_shptr(std::make_shared<_MV>(_First, _Last)) {}
		template<class _Iter, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type>
			ivector(_Iter _First, _Iter _Last, const _A& _Al) : m_shptr(std::make_shared<_MV>(_First, _Last, _Al)) {}

		_Myt& operator=(_MV&& _X) { m_shptr->operator=(std::move(_X)); return (*this); }
		_Myt& operator=(const _MV& _X) { m_shptr->operator=(_X); return (*this); }
		_Myt& operator=(_Myt&& _X) { m_shptr->operator=(std::move(_X.msevector())); return (*this); }
		_Myt& operator=(const _Myt& _X) { m_shptr->operator=(_X.msevector()); return (*this); }
		void reserve(size_type _Count) { m_shptr->reserve(_Count); }
		void resize(size_type _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
		typename _MV::const_reference operator[](size_type _P) const { return m_shptr->operator[](_P); }
		typename _MV::reference operator[](size_type _P) { return m_shptr->operator[](_P); }
		void push_back(_Ty&& _X) { m_shptr->push_back(std::move(_X)); }
		void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
		void pop_back() { m_shptr->pop_back(); }
		void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
		void assign(size_type _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
		template<class ..._Valty>
		void emplace_back(_Valty&& ..._Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)...); }
		void clear() { m_shptr->clear(); }
		void swap(_MV& _X) { m_shptr->swap(_X); }
		void swap(_Myt& _X) { m_shptr->swap(_X.msevector()); }

		ivector(_XSTD initializer_list<typename _MV::value_type> _Ilist, const _A& _Al = _A()) : m_shptr(std::make_shared<_MV>(_Ilist, _Al)) {}
		_Myt& operator=(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->operator=(_Ilist); return (*this); }
		void assign(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->assign(_Ilist); }
		typename _MV::iterator insert(typename _MV::const_iterator _Where, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return m_shptr->insert(_Where, _Ilist); }

		size_type capacity() const _NOEXCEPT{ return m_shptr->capacity(); }
		void shrink_to_fit() { m_shptr->shrink_to_fit(); }
		size_type size() const _NOEXCEPT{ return m_shptr->size(); }
		size_type max_size() const _NOEXCEPT{ return m_shptr->max_size(); }
		bool empty() const _NOEXCEPT{ return m_shptr->empty(); }
		_A get_allocator() const _NOEXCEPT{ return m_shptr->get_allocator(); }
		typename _MV::const_reference at(size_type _Pos) const { return m_shptr->at(_Pos); }
		typename _MV::reference at(size_type _Pos) { return m_shptr->at(_Pos); }
		typename _MV::reference front() { return m_shptr->front(); }
		typename _MV::const_reference front() const { return m_shptr->front(); }
		typename _MV::reference back() { return m_shptr->back(); }
		typename _MV::const_reference back() const { return m_shptr->back(); }

		class cipointer : public _MV::random_access_const_iterator_base {
		public:
			typedef typename _MV::mm_const_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_const_iterator_type::value_type value_type;
			typedef typename _MV::mm_const_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_const_iterator_type::pointer pointer;
			typedef typename _MV::mm_const_iterator_type::reference reference;

			cipointer(const _Myt& owner_cref) : m_msevector_cshptr(owner_cref.m_shptr), m_cipointer(*(owner_cref.m_shptr)) {}
			cipointer(const cipointer& src_cref) : m_msevector_cshptr(src_cref.m_msevector_cshptr), m_cipointer(src_cref.m_cipointer) {}
			~cipointer() {}
			const typename _MV::cipointer& msevector_cipointer() const { return m_cipointer; }
			typename _MV::cipointer& msevector_cipointer() { return m_cipointer; }
			const typename _MV::cipointer& mvcip() const { return msevector_cipointer(); }
			typename _MV::cipointer& mvcip() { return msevector_cipointer(); }

			void reset() { msevector_cipointer().reset(); }
			bool points_to_an_item() const { return msevector_cipointer().points_to_an_item(); }
			bool points_to_end_marker() const { return msevector_cipointer().points_to_end_marker(); }
			bool points_to_beginning() const { return msevector_cipointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return msevector_cipointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return msevector_cipointer().has_next(); }
			bool has_previous() const { return msevector_cipointer().has_previous(); }
			void set_to_beginning() { msevector_cipointer().set_to_beginning(); }
			void set_to_end_marker() { msevector_cipointer().set_to_end_marker(); }
			void set_to_next() { msevector_cipointer().set_to_next(); }
			void set_to_previous() { msevector_cipointer().set_to_previous(); }
			cipointer& operator ++() { msevector_cipointer().operator ++(); return (*this); }
			cipointer operator++(int) { cipointer _Tmp = *this; ++*this; return (_Tmp); }
			cipointer& operator --() { msevector_cipointer().operator --(); return (*this); }
			cipointer operator--(int) { cipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(typename _MV::difference_type n) { msevector_cipointer().advance(n); }
			void regress(typename _MV::difference_type n) { msevector_cipointer().regress(n); }
			cipointer& operator +=(typename _MV::difference_type n) { msevector_cipointer().operator +=(n); return (*this); }
			cipointer& operator -=(typename _MV::difference_type n) { msevector_cipointer().operator -=(n); return (*this); }
			cipointer operator+(typename _MV::difference_type n) const { auto retval = (*this); retval += n; return retval; }
			cipointer operator-(typename _MV::difference_type n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const cipointer& _Right_cref) const { return msevector_cipointer() - (_Right_cref.msevector_cipointer()); }
			typename _MV::const_reference operator*() const { return msevector_cipointer().operator*(); }
			typename _MV::const_reference item() const { return operator*(); }
			typename _MV::const_reference previous_item() const { return msevector_cipointer().previous_item(); }
			typename _MV::const_pointer operator->() const { return msevector_cipointer().operator->(); }
			typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
			bool operator==(const cipointer& _Right_cref) const { return msevector_cipointer().operator==(_Right_cref.msevector_cipointer()); }
			bool operator!=(const cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const cipointer& _Right) const { return (msevector_cipointer() < _Right.msevector_cipointer()); }
			bool operator<=(const cipointer& _Right) const { return (msevector_cipointer() <= _Right.msevector_cipointer()); }
			bool operator>(const cipointer& _Right) const { return (msevector_cipointer() > _Right.msevector_cipointer()); }
			bool operator>=(const cipointer& _Right) const { return (msevector_cipointer() >= _Right.msevector_cipointer()); }
			void set_to_const_item_pointer(const cipointer& _Right_cref) { msevector_cipointer().set_to_const_item_pointer(_Right_cref.msevector_cipointer()); }
			msev_size_t position() const { return msevector_cipointer().position(); }
		private:
			cipointer(const std::shared_ptr<_MV>& msevector_shptr) : m_msevector_cshptr(msevector_shptr), m_cipointer(*msevector_shptr) {}
			std::shared_ptr<const _MV> m_msevector_cshptr;
			/* m_cipointer needs to be declared after m_msevector_cshptr so that it's destructor will be called first. */
			typename _MV::cipointer m_cipointer;
			friend class /*_Myt*/ivector<_Ty, _A>;
			friend class ipointer;
		};
		class ipointer : public _MV::random_access_iterator_base {
		public:
			typedef typename _MV::mm_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_iterator_type::value_type value_type;
			typedef typename _MV::mm_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_iterator_type::pointer pointer;
			typedef typename _MV::mm_iterator_type::reference reference;

			ipointer(_Myt& owner_ref) : m_msevector_shptr(owner_ref.m_shptr), m_ipointer(*(owner_ref.m_shptr)) {}
			ipointer(const ipointer& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_ipointer(src_cref.m_ipointer) {}
			~ipointer() {}
			const typename _MV::ipointer& msevector_ipointer() const { return m_ipointer; }
			typename _MV::ipointer& msevector_ipointer() { return m_ipointer; }
			const typename _MV::ipointer& mvip() const { return msevector_ipointer(); }
			typename _MV::ipointer& mvip() { return msevector_ipointer(); }
			operator cipointer() const {
				cipointer retval(m_msevector_shptr);
				assert(m_msevector_shptr);
				retval.set_to_beginning();
				retval.advance(msev_int(msevector_ipointer().position()));
				return retval;
			}

			void reset() { msevector_ipointer().reset(); }
			bool points_to_an_item() const { return msevector_ipointer().points_to_an_item(); }
			bool points_to_end_marker() const { return msevector_ipointer().points_to_end_marker(); }
			bool points_to_beginning() const { return msevector_ipointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return msevector_ipointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return msevector_ipointer().has_next(); }
			bool has_previous() const { return msevector_ipointer().has_previous(); }
			void set_to_beginning() { msevector_ipointer().set_to_beginning(); }
			void set_to_end_marker() { msevector_ipointer().set_to_end_marker(); }
			void set_to_next() { msevector_ipointer().set_to_next(); }
			void set_to_previous() { msevector_ipointer().set_to_previous(); }
			ipointer& operator ++() { msevector_ipointer().operator ++(); return (*this); }
			ipointer operator++(int) { ipointer _Tmp = *this; ++*this; return (_Tmp); }
			ipointer& operator --() { msevector_ipointer().operator --(); return (*this); }
			ipointer operator--(int) { ipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(typename _MV::difference_type n) { msevector_ipointer().advance(n); }
			void regress(typename _MV::difference_type n) { msevector_ipointer().regress(n); }
			ipointer& operator +=(typename _MV::difference_type n) { msevector_ipointer().operator +=(n); return (*this); }
			ipointer& operator -=(typename _MV::difference_type n) { msevector_ipointer().operator -=(n); return (*this); }
			ipointer operator+(typename _MV::difference_type n) const { auto retval = (*this); retval += n; return retval; }
			ipointer operator-(typename _MV::difference_type n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const ipointer& _Right_cref) const { return msevector_ipointer() - (_Right_cref.msevector_ipointer()); }
			typename _MV::reference operator*() const { return msevector_ipointer().operator*(); }
			typename _MV::reference item() const { return operator*(); }
			typename _MV::reference previous_item() const { return msevector_ipointer().previous_item(); }
			typename _MV::pointer operator->() const { return msevector_ipointer().operator->(); }
			typename _MV::reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
			bool operator==(const ipointer& _Right_cref) const { return msevector_ipointer().operator==(_Right_cref.msevector_ipointer()); }
			bool operator!=(const ipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ipointer& _Right) const { return (msevector_ipointer() < _Right.msevector_ipointer()); }
			bool operator<=(const ipointer& _Right) const { return (msevector_ipointer() <= _Right.msevector_ipointer()); }
			bool operator>(const ipointer& _Right) const { return (msevector_ipointer() > _Right.msevector_ipointer()); }
			bool operator>=(const ipointer& _Right) const { return (msevector_ipointer() >= _Right.msevector_ipointer()); }
			void set_to_item_pointer(const ipointer& _Right_cref) { msevector_ipointer().set_to_item_pointer(_Right_cref.msevector_ipointer()); }
			msev_size_t position() const { return msevector_ipointer().position(); }
		private:
			std::shared_ptr<_MV> m_msevector_shptr;
			/* m_ipointer needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
			typename _MV::ipointer m_ipointer;
			friend class /*_Myt*/ivector<_Ty, _A>;
		};

		/* begin() and end() are provided so that ivector can be used with stl algorithms. When using ipointers directly,
		set_to_beginning() and set_to_end_marker() member functions are preferred. */
		ipointer begin() {	// return ipointer for beginning of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer.set_to_beginning();
			return retval;
		}
		cipointer begin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer.set_to_beginning();
			return retval;
		}
		ipointer end() {	// return ipointer for end of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer.set_to_end_marker();
			return retval;
		}
		cipointer end() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer.set_to_end_marker();
			return retval;
		}
		cipointer cbegin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer.set_to_beginning();
			return retval;
		}
		cipointer cend() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer.set_to_beginning();
			return retval;
		}

		ivector(const cipointer &start, const cipointer &end, const _A& _Al = _A())
			: m_shptr(std::make_shared<_MV>(start.msevector_cipointer(), end.msevector_cipointer(), _Al)) {}
		void assign(const cipointer &start, const cipointer &end) {
			m_shptr->assign(start.msevector_cipointer(), end.msevector_cipointer());
		}
		void assign_inclusive(const cipointer &first, const cipointer &last) {
			m_shptr->assign_inclusive(first.msevector_cipointer(), last.msevector_cipointer());
		}
		ipointer insert_before(const cipointer &pos, size_type _M, const _Ty& _X) {
			auto res = m_shptr->insert_before(pos.msevector_cipointer(), _M, _X);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(const cipointer &pos, _Ty&& _X) {
			auto res = m_shptr->insert_before(pos.msevector_cipointer(), std::move(_X));
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(const cipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
		ipointer insert_before(const cipointer &pos, const cipointer &start, const cipointer &end) {
			auto res = m_shptr->insert_before(pos.msevector_cipointer(), start.msevector_cipointer(), end.msevector_cipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before_inclusive(const cipointer &pos, const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return insert_before(pos, first, end);
		}
		ipointer insert_before(const cipointer &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
			auto res = m_shptr->insert_before(pos.msevector_cipointer(), _Ilist);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		void insert_before(msev_size_t pos, _Ty&& _X) {
			m_shptr->insert_before(pos, std::move(_X));
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
		template<class ..._Valty>
		ipointer emplace(const cipointer &pos, _Valty&& ..._Val) {
			auto res = m_shptr->emplace(pos.msevector_cipointer(), std::forward<_Valty>(_Val)...);
			ipointer retval = begin(); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer erase(const ipointer &pos) {
			auto res = m_shptr->erase(pos.msevector_ipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer erase(const ipointer &start, const ipointer &end) {
			auto res = m_shptr->erase(start.msevector_ipointer(), end.msevector_ipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer erase_inclusive(const ipointer &first, const ipointer &last) {
			auto end = last; end.set_to_next();
			return erase_inclusive(first, end);
		}
		bool operator==(const _Myt& _Right) const {	// test for ivector equality
			return ((*(_Right.m_shptr)) == (*m_shptr));
		}
		bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for ivectors
			return ((*m_shptr) < (*(_Right.m_shptr)));
			}

	private:
		std::shared_ptr<_MV> m_shptr;
	};

	template<class _Ty, class _Alloc> inline bool operator!=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test for ivector inequality
			return (!(_Left == _Right));
		}

	template<class _Ty, class _Alloc> inline bool operator>(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left > _Right for ivectors
			return (_Right < _Left);
		}

	template<class _Ty, class _Alloc> inline bool operator<=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left <= _Right for ivectors
			return (!(_Right < _Left));
		}

	template<class _Ty, class _Alloc> inline bool operator>=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left >= _Right for ivectors
			return (!(_Left < _Right));
		}

}
#endif /*ndef MSEIVECTOR_H*/
