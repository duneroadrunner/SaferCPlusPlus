
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSTDARRAY_H
#define MSEMSTDARRAY_H

#include "msemsearray.h"
#ifndef MSE_MSTDARRAY_DISABLED
#include "mseregistered.h"
#endif /*MSE_MSTDARRAY_DISABLED*/

#ifdef MSE_SAFER_SUBSTITUTES_DISABLED
#define MSE_MSTDARRAY_DISABLED
#endif /*MSE_SAFER_SUBSTITUTES_DISABLED*/

namespace mse {

	namespace mstd {

#ifdef MSE_MSTDARRAY_DISABLED
		template<class _Ty, size_t _A > using array = std::array<_Ty, _A>;

#else /*MSE_MSTDARRAY_DISABLED*/

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

#ifndef _CONST_FUN
#define _CONST_FUN 
#endif /*_CONST_FUN*/

		template<class _Ty, size_t _A >
		class array {
		public:
			typedef mse::mstd::array<_Ty, _A> _Myt;
			typedef mse::msearray<_Ty, _A> _MA;

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
			array(_XSTD initializer_list<typename _MA::base_class::value_type> _Ilist) : m_msearray(_Ilist) {}

			_Myt& operator=(_MA&& _X) { m_msearray.operator=(std::move(_X)); return (*this); }
			_Myt& operator=(const _MA& _X) { m_msearray.operator=(_X); return (*this); }
			_Myt& operator=(_Myt&& _X) { m_msearray.operator=(std::move(_X.msearray())); return (*this); }
			_Myt& operator=(const _Myt& _X) { m_msearray.operator=(_X.msearray()); return (*this); }
			typename _MA::const_reference operator[](size_t _P) const { return m_msearray.operator[](_P); }
			typename _MA::reference operator[](size_t _P) { return m_msearray.operator[](_P); }
			void fill(const _Ty& _Value) { m_msearray.fill(_Value); }
			void swap(_MA& _X) { m_msearray.swap(_X); }
			void swap(_Myt& _X) { m_msearray.swap(_X.msearray()); }

			size_t size() const _NOEXCEPT { return m_msearray.size(); }
			size_t max_size() const _NOEXCEPT { return m_msearray.max_size(); }
			bool empty() const _NOEXCEPT { return m_msearray.empty(); }
			typename _MA::const_reference at(size_t _Pos) const { return m_msearray.at(_Pos); }
			typename _MA::reference at(size_t _Pos) { return m_msearray.at(_Pos); }
			typename _MA::reference front() { return m_msearray.front(); }
			typename _MA::const_reference front() const { return m_msearray.front(); }
			typename _MA::reference back() { return m_msearray.back(); }
			typename _MA::const_reference back() const { return m_msearray.back(); }


			class const_iterator {
			public:
				typedef typename _MA::ss_const_iterator_type::iterator_category iterator_category;
				typedef typename _MA::ss_const_iterator_type::value_type value_type;
				typedef typename _MA::ss_const_iterator_type::difference_type difference_type;
				typedef typename _MA::difference_type distance_type;	// retained
				typedef typename _MA::ss_const_iterator_type::pointer pointer;
				typedef typename _MA::ss_const_iterator_type::reference reference;

				const_iterator() {}
				const_iterator(const const_iterator& src_cref) : m_msearray_regptr(src_cref.m_msearray_regptr) {
					(*this) = src_cref;
				}
				~const_iterator() {}
				const typename _MA::ss_const_iterator_type& msearray_ss_const_iterator_type() const {
					if (!m_msearray_regptr) { auto z = (*m_msearray_regptr);/*This will throw.*/ }
					return m_ss_const_iterator;
				}
				typename _MA::ss_const_iterator_type& msearray_ss_const_iterator_type() {
					if (!m_msearray_regptr) { auto z = (*m_msearray_regptr);/*This will throw.*/ }
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
				typename _MA::const_reference operator[](typename _MA::difference_type _Off) const { return (*(*this + _Off)); }
				bool operator==(const const_iterator& _Right_cref) const { return msearray_ss_const_iterator_type().operator==(_Right_cref.msearray_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() < _Right.msearray_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() <= _Right.msearray_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() > _Right.msearray_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (msearray_ss_const_iterator_type() >= _Right.msearray_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { msearray_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msearray_ss_const_iterator_type()); }
				msear_size_t position() const { return msearray_ss_const_iterator_type().position(); }
			private:
				const_iterator(std::shared_ptr<_MA> msearray_shptr) : m_msearray_regptr(msearray_shptr) {
					m_ss_const_iterator = msearray_shptr->ss_cbegin();
				}
				mse::TRegisteredPointer<_MA> m_msearray_regptr = nullptr;
				/* m_ss_const_iterator needs to be declared after m_msearray_regptr so that it's destructor will be called first. */
				typename _MA::ss_const_iterator_type m_ss_const_iterator;
				friend class /*_Myt*/array<_Ty, _A>;
				friend class iterator;
			};
			class iterator {
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
					if (!m_msearray_regptr) { auto z = (*m_msearray_regptr);/*This will throw.*/ }
					return m_ss_iterator;
				}
				typename _MA::ss_iterator_type& msearray_ss_iterator_type() {
					if (!m_msearray_regptr) { auto z = (*m_msearray_regptr);/*This will throw.*/ }
					return m_ss_iterator;
				}
				const typename _MA::ss_iterator_type& mvssi() const { return msearray_ss_iterator_type(); }
				typename _MA::ss_iterator_type& mvssi() { return msearray_ss_iterator_type(); }
				operator const_iterator() const {
					const_iterator retval(m_msearray_regptr);
					retval.msearray_ss_const_iterator_type().set_to_beginning();
					retval.msearray_ss_const_iterator_type().advance(msear_int(msearray_ss_iterator_type().position()));
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
				typename _MA::reference operator[](typename _MA::difference_type _Off) const { return (*(*this + _Off)); }
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
				friend class /*_Myt*/array<_Ty, _A>;
			};

			iterator begin()
			{	// return iterator for beginning of mutable sequence
				iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_iterator) = m_msearray.ss_begin();
				return retval;
			}

			const_iterator begin() const
			{	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_begin();
				return retval;
			}

			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_iterator) = m_msearray.ss_end();
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_end();
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_cbegin();
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msearray_regptr = &(this->m_msearray);
				(retval.m_ss_const_iterator) = m_msearray.ss_cend();
				return retval;
			}


			bool operator==(const _Myt& _Right) const {	// test for array equality
				return (_Right.m_msearray == m_msearray);
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for arrays
				return (m_msearray < _Right.m_msearray);
			}

		private:
			mse::TRegisteredObj<_MA> m_msearray;

			template<size_t _Idx, class _Ty, size_t _Size>
			friend _CONST_FUN _Ty& std::get(mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT;
			template<size_t _Idx, class _Ty, size_t _Size>
			friend _CONST_FUN const _Ty& get(const mse::mstd::array<_Ty, _Size>& _Arr) _NOEXCEPT;
			template<size_t _Idx, class _Ty, size_t _Size>
			friend _CONST_FUN _Ty&& get(mse::mstd::array<_Ty, _Size>&& _Arr) _NOEXCEPT;
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

		template<class _Ty, size_t _Size>
		struct std::tuple_size<mse::mstd::array<_Ty, _Size> >
			: integral_constant<size_t, _Size>
		{	// struct to determine number of elements in array
		};

		template<size_t _Idx, class _Ty, size_t _Size>
		struct std::tuple_element<_Idx, mse::mstd::array<_Ty, _Size> >
		{	// struct to determine type of element _Idx in array
			static_assert(_Idx < _Size, "array index out of bounds");

			typedef _Ty type;
		};
#endif /*MSE_MSTDARRAY_DISABLED*/
	}
}

namespace std {
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
#endif // MSE_SELF_TESTS
			}
		};
	}
}
#endif /*ndef MSEMSTDARRAY_H*/
