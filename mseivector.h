
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEIVECTOR_H
#define MSEIVECTOR_H

#include "msemsevector.h"

namespace mse {

	template<class _Ty, class _A = std::allocator<_Ty> >
	class ivector {
	public:
		typedef ivector<_Ty, _A> _Myt;
		typedef msevector<_Ty, _A> _MV;

		msevector<_Ty, _A>& msevector() const { return (*m_shptr); }
		operator _MV() { return msevector(); }

		explicit ivector(const _A& _Al = _A()) : m_shptr(new _MV(_Al)) {}
		explicit ivector(size_t _N, const _Ty& _V = _Ty(), const _A& _Al = _A()) : m_shptr(new _MV(_N, _V, _Al)) {}
		ivector(_MV&& _X) : m_shptr(new _MV(std::move(_X))) {}
		ivector(const _MV& _X) : m_shptr(new _MV(_X)) {}
		ivector(_Myt&& _X) : m_shptr(new _MV(std::move(_X.msevector()))) {}
		ivector(const _Myt& _X) : m_shptr(new _MV(_X.msevector())) {}
		typedef typename _MV::const_iterator _It;
		ivector(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(new _MV(_F, _L, _Al)) {}
		template<class _Iter
#ifndef MSVC2010_COMPATIBILE
			, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
#endif /*MSVC2010_COMPATIBILE*/
		>
		ivector(_Iter _First, _Iter _Last) : m_shptr(new _MV(_First, _Last)) {}

		_Myt& operator=(_MV&& _X) { m_shptr->operator=(std::move(_X)); return (*this); }
		_Myt& operator=(const _MV& _X) { m_shptr->operator=(_X); return (*this); }
		_Myt& operator=(_Myt&& _X) { m_shptr->operator=(std::move(_X.msevector())); return (*this); }
		_Myt& operator=(const _Myt& _X) { m_shptr->operator=(_X.msevector()); return (*this); }
		void reserve(size_t _Count) { m_shptr->reserve(_Count); }
		void resize(size_t _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
		typename _MV::const_reference operator[](size_t _P) const { return m_shptr->operator[](_P); }
		typename _MV::reference operator[](size_t _P) { return m_shptr->operator[](_P); }
		void push_back(_Ty&& _X) { m_shptr->push_back(std::move(_X)); }
		void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
		void pop_back() { m_shptr->pop_back(); }
		void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
		void assign(size_t _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
		typename _MV::iterator insert(typename _MV::iterator _P, _Ty&& _X) { return m_shptr->insert(_P, std::move(_X)); }
		typename _MV::iterator insert(typename _MV::iterator _P, const _Ty& _X = _Ty()) { return m_shptr->insert(_P, _X); }
		typename _MV::iterator insert(typename _MV::iterator _P, size_t _M, const _Ty& _X) { return m_shptr->insert(_P, _M, _X); }
		template<class _Iter>
		typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename _MV::iterator>::type
			insert(typename _MV::const_iterator _Where, _Iter _First, _Iter _Last) { return m_shptr->insert(_Where, _First, _Last); }
		template<class
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
			_Valty>
		void emplace_back(_Valty&&
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
		_Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
		); }
		template<class
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
			_Valty>
		typename _MV::iterator emplace(typename _MV::const_iterator _Where, _Valty&&
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
		_Val) { return m_shptr->emplace(_Where, std::forward<_Valty>(_Val)
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
		); }
		typename _MV::iterator erase(typename _MV::iterator _P) { return m_shptr->emplace(_P); }
		typename _MV::iterator erase(typename _MV::iterator _F, typename _MV::iterator _L) { return m_shptr->emplace(_F, _L); }
		void clear() { m_shptr->clear(); }
		void swap(_MV& _X) { m_shptr->swap(_X); }
		void swap(_Myt& _X) { m_shptr->swap(_X.msevector()); }

#ifndef MSVC2010_COMPATIBILE
		ivector(_XSTD initializer_list<typename _MV::value_type> _Ilist, const _A& _Al = _A()) : m_shptr(new _MV(_Ilist, _Al)) {}
		_Myt& operator=(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->operator=(_Ilist); return (*this); }
		void assign(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->assign(_Ilist); }
		typename _MV::iterator insert(typename _MV::const_iterator _Where, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return m_shptr->insert(_Where, _Ilist); }
#endif /*MSVC2010_COMPATIBILE*/

		size_t capacity() const _NOEXCEPT{ return m_shptr->capacity(); }
		void shrink_to_fit() { m_shptr->shrink_to_fit(); }
		size_t size() const _NOEXCEPT{ return m_shptr->size(); }
		size_t max_size() const _NOEXCEPT{ return m_shptr->max_size(); }
		bool empty() const _NOEXCEPT{ return m_shptr->empty(); }
		_A get_allocator() const _NOEXCEPT{ return m_shptr->get_allocator(); }
		typename _MV::const_reference at(size_t _Pos) const { return m_shptr->at(_Pos); }
		typename _MV::reference at(size_t _Pos) { return m_shptr->at(_Pos); }
		typename _MV::reference front() { return m_shptr->front(); }
		typename _MV::const_reference front() const { return m_shptr->front(); }
		typename _MV::reference back() { return m_shptr->back(); }
		typename _MV::const_reference back() const { return m_shptr->back(); }

		class cipointer {
		public:
			typedef typename _MV::mm_const_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_const_iterator_type::value_type value_type;
			typedef typename _MV::mm_const_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_const_iterator_type::pointer pointer;
			typedef typename _MV::mm_const_iterator_type::reference reference;

			cipointer(const _Myt& owner_cref) : m_msevector_shptr(owner_cref.m_shptr), m_cipointer_shptr(new typename _MV::cipointer(*(owner_cref.m_shptr))) {}
			cipointer(const cipointer& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_cipointer_shptr(new typename _MV::cipointer(*(src_cref.m_cipointer_shptr))) {
				(*this) = src_cref;
			}
			~cipointer() {}
			typename _MV::cipointer& msevector_cipointer() const { return (*m_cipointer_shptr); }
			typename _MV::cipointer& mvcip() const { return msevector_cipointer(); }

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
			cipointer& operator +=(int n) { msevector_cipointer().operator +=(n); return (*this); }
			cipointer& operator -=(int n) { msevector_cipointer().operator -=(n); return (*this); }
			cipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			cipointer operator-(int n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const cipointer& _Right_cref) const { return msevector_cipointer() - (_Right_cref.msevector_cipointer()); }
			typename _MV::const_reference operator*() const { return msevector_cipointer().operator*(); }
			typename _MV::const_reference item() const { return operator*(); }
			typename _MV::const_reference previous_item() const { return msevector_cipointer().previous_item(); }
			typename _MV::const_pointer operator->() const { return msevector_cipointer().operator->(); }
			typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
			cipointer& operator=(const cipointer& _Right_cref) { msevector_cipointer().operator=(_Right_cref.msevector_cipointer()); return (*this); }
			bool operator==(const cipointer& _Right_cref) const { return msevector_cipointer().operator==(_Right_cref.msevector_cipointer()); }
			bool operator!=(const cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const cipointer& _Right) const { return (msevector_cipointer() < _Right.msevector_cipointer()); }
			bool operator<=(const cipointer& _Right) const { return (msevector_cipointer() <= _Right.msevector_cipointer()); }
			bool operator>(const cipointer& _Right) const { return (msevector_cipointer() > _Right.msevector_cipointer()); }
			bool operator>=(const cipointer& _Right) const { return (msevector_cipointer() >= _Right.msevector_cipointer()); }
			void set_to_const_item_pointer(const cipointer& _Right_cref) { msevector_cipointer().set_to_const_item_pointer(_Right_cref.msevector_cipointer()); }
			msev_size_t position() const { return msevector_cipointer().position(); }
		private:
			cipointer(const std::shared_ptr<_MV>& msevector_shptr) : m_msevector_shptr(msevector_shptr), m_cipointer_shptr(new typename _MV::cipointer(*(msevector_shptr))) {}
			std::shared_ptr<_MV> m_msevector_shptr;
			/* m_cipointer_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
			std::shared_ptr<typename _MV::cipointer> m_cipointer_shptr;
			friend class /*_Myt*/ivector<_Ty, _A>;
			friend class ipointer;
		};
		class ipointer {
		public:
			typedef typename _MV::mm_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_iterator_type::value_type value_type;
			typedef typename _MV::mm_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_iterator_type::pointer pointer;
			typedef typename _MV::mm_iterator_type::reference reference;

			ipointer(_Myt& owner_ref) : m_msevector_shptr(owner_ref.m_shptr), m_ipointer_shptr(new typename _MV::ipointer(*(owner_ref.m_shptr))) {}
			ipointer(const ipointer& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_ipointer_shptr(new typename _MV::ipointer(*(src_cref.m_ipointer_shptr))) {
				(*this) = src_cref;
			}
			~ipointer() {}
			typename _MV::ipointer& msevector_ipointer() const { return (*m_ipointer_shptr); }
			typename _MV::ipointer& mvip() const { return msevector_ipointer(); }
			//const mm_iterator_handle_type& handle() const { return (*m_handle_shptr); }
			operator cipointer() const {
				cipointer retval(m_msevector_shptr);
				retval.set_to_beginning();
				retval.advance(mse::CInt(msevector_ipointer().position()));
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
			ipointer& operator +=(int n) { msevector_ipointer().operator +=(n); return (*this); }
			ipointer& operator -=(int n) { msevector_ipointer().operator -=(n); return (*this); }
			ipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			ipointer operator-(int n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const ipointer& _Right_cref) const { return msevector_ipointer() - (_Right_cref.msevector_ipointer()); }
			typename _MV::reference operator*() const { return msevector_ipointer().operator*(); }
			typename _MV::reference item() const { return operator*(); }
			typename _MV::reference previous_item() const { return msevector_ipointer().previous_item(); }
			typename _MV::pointer operator->() const { return msevector_ipointer().operator->(); }
			typename _MV::reference operator[](typename _MV::difference_type _Off) { return (*(*this + _Off)); }
			ipointer& operator=(const ipointer& _Right_cref) { msevector_ipointer().operator=(_Right_cref.msevector_ipointer()); return (*this); }
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
			/* m_ipointer_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
			std::shared_ptr<typename _MV::ipointer> m_ipointer_shptr;
			friend class /*_Myt*/ivector<_Ty, _A>;
		};

		/* begin() and end() are provided so that ivector can be used with stl algorithms. When using ipointers directly,
		set_to_beginning() and set_to_end_marker() member functions are preferred. */
		ipointer begin() {	// return ipointer for beginning of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer_shptr->set_to_beginning();
			return retval;
		}
		cipointer begin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}
		ipointer end() {	// return ipointer for end of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer_shptr->set_to_end_marker();
			return retval;
		}
		cipointer end() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_end_marker();
			return retval;
		}
		cipointer cbegin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}
		cipointer cend() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}

		ivector(const cipointer &start, const cipointer &end, const _A& _Al = _A())
			: m_shptr(new _MV(start.msevector_cipointer(), end.msevector_cipointer(), _Al)) {}
		void assign(const cipointer &start, const cipointer &end) {
			m_shptr->assign(start.msevector_cipointer(), end.msevector_cipointer());
		}
		void assign_inclusive(const cipointer &first, const cipointer &last) {
			m_shptr->assign_inclusive(first.msevector_cipointer(), last.msevector_cipointer());
		}
		ipointer insert_before(const ipointer &pos, size_t _M, const _Ty& _X) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), _M, _X);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(const ipointer &pos, _Ty&& _X) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), std::move(_X));
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(const ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
		ipointer insert_before(const ipointer &pos, const cipointer &start, const cipointer &end) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), start.msevector_cipointer(), end.msevector_cipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before_inclusive(const ipointer &pos, const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return insert_before_inclusive(pos, first, end);
		}
#ifndef MSVC2010_COMPATIBILE
		ipointer insert_before(const ipointer &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), _Ilist);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
#endif /*MSVC2010_COMPATIBILE*/
		void insert_before(msev_size_t pos, _Ty&& _X) {
			m_shptr->insert_before(pos, std::move(_X));
		}
		void insert_before(msev_size_t pos, const _Ty& _X = _Ty()) {
			m_shptr->insert_before(pos, _X);
		}
		void insert_before(msev_size_t pos, size_t _M, const _Ty& _X) {
			m_shptr->insert_before(pos, _M, _X);
		}
#ifndef MSVC2010_COMPATIBILE
		void insert_before(msev_size_t pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
			m_shptr->insert_before(pos, _Ilist);
		}
#endif /*MSVC2010_COMPATIBILE*/
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
