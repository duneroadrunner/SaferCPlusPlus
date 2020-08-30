
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSELEGACYHELPERS_H_
#define MSELEGACYHELPERS_H_

#if defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_MSTDARRAY_DISABLED)
#define MSE_LEGACYHELPERS_DISABLED
#endif /*defined(MSE_SAFER_SUBSTITUTES_DISABLED) || defined(MSE_MSTDARRAY_DISABLED)*/

#ifndef MSE_LEGACYHELPERS_DISABLED
#include "msepoly.h"
#include <cstring>
#else // !MSE_LEGACYHELPERS_DISABLED
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif // !MSE_LEGACYHELPERS_DISABLED

#ifdef _MSC_VER
#pragma warning( push )  
#pragma warning( disable : 4100 4456 4189 4127 )
#endif /*_MSC_VER*/

#ifdef MSE_LEGACYHELPERS_DISABLED

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) 
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) 
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) [size]
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) element_type *

#define MSE_LH_ALLOC(element_type, ptr, num_bytes) ptr = (element_type *)malloc(num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) (element_type *)realloc(ptr, num_bytes)
#define MSE_LH_FREE(ptr) free(ptr)
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) (iterator_type)malloc(num_bytes)

#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) element_type *

#define MSE_LH_FREAD(ptr, size, count, stream) fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) fwrite(ptr, size, count, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) memcpy(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) memset(ptr, value, num_bytes)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) object_type
#define MSE_LH_POINTER_TYPE(element_type) element_type *
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) element_type *

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE

#define MSE_LH_IF_ENABLED(x)
#define MSE_LH_IF_DISABLED(x) x

#else /*MSE_LEGACYHELPERS_DISABLED*/

#define MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) mse::lh::TNativeArrayReplacement< 
#define MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) , size >
#define MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size) 
#define MSE_LH_FIXED_ARRAY_DECLARATION(element_type, size, name) MSE_LH_FIXED_ARRAY_TYPE_PREFIX(size) element_type MSE_LH_FIXED_ARRAY_TYPE_SUFFIX(size) name MSE_LH_FIXED_ARRAY_TYPE_POST_NAME_SUFFIX(size)
#define MSE_LH_DYNAMIC_ARRAY_ITERATOR_TYPE(element_type) mse::lh::TStrongVectorIterator< element_type >

#define MSE_LH_ALLOC(element_type, ptr, num_bytes) mse::lh::allocate(ptr, num_bytes)
#define MSE_LH_REALLOC(element_type, ptr, num_bytes) mse::lh::reallocate(ptr, num_bytes)
#define MSE_LH_FREE(ptr) mse::lh::CAllocF<typename std::remove_reference<decltype(ptr)>::type>::free(ptr)
#define MSE_LH_ALLOC_DYN_ARRAY1(iterator_type, num_bytes) mse::lh::allocate_dyn_array1<iterator_type>(num_bytes)

#define MSE_LH_ARRAY_ITERATOR_TYPE(element_type) mse::TNullableAnyRandomAccessIterator< element_type >

#define MSE_LH_FREAD(ptr, size, count, stream) mse::lh::CFileF< mse::TNullableAnyRandomAccessIterator<typename std::remove_reference<decltype((ptr)[0])>::type> >::fread(ptr, size, count, stream)
#define MSE_LH_FWRITE(ptr, size, count, stream) mse::lh::CFileF< mse::TNullableAnyRandomAccessIterator<typename std::remove_reference<decltype((ptr)[0])>::type> >::fwrite(ptr, size, count, stream)

#define MSE_LH_TYPED_MEMCPY(element_type, destination, source, num_bytes) mse::lh::CMemF< mse::TNullableAnyRandomAccessIterator<element_type> >::memcpy(destination, source, num_bytes)
#define MSE_LH_TYPED_MEMSET(element_type, ptr, value, num_bytes) mse::lh::CMemF< mse::TNullableAnyRandomAccessIterator<element_type> >::memset(ptr, value, num_bytes)
#define MSE_LH_MEMCPY(destination, source, num_bytes) mse::lh::CMemF< mse::TNullableAnyRandomAccessIterator<typename std::remove_reference<decltype((destination)[0])>::type> >::memcpy(destination, source, num_bytes)
#define MSE_LH_MEMSET(ptr, value, num_bytes) mse::lh::CMemF< mse::TNullableAnyRandomAccessIterator<typename std::remove_reference<decltype((ptr)[0])>::type> >::memset(ptr, value, num_bytes)

#define MSE_LH_ADDRESSABLE_TYPE(object_type) mse::TRegisteredObj< object_type >
#define MSE_LH_POINTER_TYPE(element_type) mse::TNullableAnyPointer< element_type >
#define MSE_LH_ALLOC_POINTER_TYPE(element_type) mse::TRefCountingPointer< element_type >

#define MSE_LH_SUPPRESS_CHECK_IN_XSCOPE MSE_SUPPRESS_CHECK_IN_XSCOPE
#define MSE_LH_SUPPRESS_CHECK_IN_DECLSCOPE MSE_SUPPRESS_CHECK_IN_DECLSCOPE

#define MSE_LH_IF_ENABLED(x) x
#define MSE_LH_IF_DISABLED(x)

#endif /*MSE_LEGACYHELPERS_DISABLED*/

namespace mse {
	namespace lh {

		/* This data type was motivated by the need for a direct substitute for native pointers targeting dynamically
		allocated (native) arrays, which can kind of play a dual role as a reference to the array object and/or as an
		iterator. */
		template <typename _Ty>
		class TStrongVectorIterator : public mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> {
		public:
			typedef mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TStrongVectorIterator() = default;
			TStrongVectorIterator(const std::nullptr_t& src) : TStrongVectorIterator() {}
			TStrongVectorIterator(const TStrongVectorIterator& src) = default;
			TStrongVectorIterator(TStrongVectorIterator&& src) = default;
			TStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_Ilist), 0) {}
			TStrongVectorIterator(const base_class& src) : base_class(src) {}
			TStrongVectorIterator(const mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>>& src) : base_class(src) {}
			explicit TStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N), 0) {}
			explicit TStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N, _V), 0) {}
			/*
			template <class... Args>
			TStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			TStrongVectorIterator& operator=(const std::nullptr_t& _Right_cref) {
				return operator=(TStrongVectorIterator());
			}
			TStrongVectorIterator& operator=(const TStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TStrongVectorIterator make(Args&&... args) {
				return TStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <class X, class... Args>
		TStrongVectorIterator<X> make_strong_vector_iterator(Args&&... args) {
			return TStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		template <typename _Ty>
		class TXScopeStrongVectorIterator : public mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> {
		public:
			typedef mse::TXScopeRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>> base_class;
			MSE_INHERITED_RANDOM_ACCESS_ITERATOR_MEMBER_TYPE_DECLARATIONS(base_class);

			TXScopeStrongVectorIterator() = default;
			TXScopeStrongVectorIterator(const std::nullptr_t& src) : TXScopeStrongVectorIterator() {}
			TXScopeStrongVectorIterator(const TXScopeStrongVectorIterator& src) = default;
			TXScopeStrongVectorIterator(TXScopeStrongVectorIterator&& src) = default;
			TXScopeStrongVectorIterator(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_Ilist), 0) {}
			TXScopeStrongVectorIterator(const base_class& src) : base_class(src) {}
			TXScopeStrongVectorIterator(const mse::TRAIterator<mse::TRefCountingPointer<mse::stnii_vector<_Ty>>>& src) : base_class(src) {}
			explicit TXScopeStrongVectorIterator(size_type _N) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N), 0) {}
			explicit TXScopeStrongVectorIterator(size_type _N, const _Ty& _V) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(_N, _V), 0) {}
			/*
			template <class... Args>
			TXScopeStrongVectorIterator(Args&&... args) : base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(std::forward<Args>(args)...), 0) {}
			*/

			size_type size() const {
				if (vector_refcptr()) {
					return (*vector_refcptr()).size();
				}
				else {
					return 0;
				}
			}
			void resize(size_type _N, const _Ty& _X = _Ty()) {
				if (!vector_refcptr()) {
					base_class::operator=(base_class(mse::make_refcounting<mse::stnii_vector<_Ty>>(), 0));
				}

				//auto old_size = size();

				(*vector_refcptr()).resize(_N, _X);
				(*vector_refcptr()).shrink_to_fit();

				/*
				if (true || (0 == old_size)) {
					(*this).set_to_beginning();
				}
				*/
			}

			TXScopeStrongVectorIterator& operator=(const std::nullptr_t& _Right_cref) {
				return operator=(TXScopeStrongVectorIterator());
			}
			TXScopeStrongVectorIterator& operator=(const TXScopeStrongVectorIterator& _Right_cref) {
				base_class::operator=(_Right_cref);
				return(*this);
			}

			explicit operator bool() const {
				return ((*this).size() != 0);
			}

			template <class... Args>
			static TXScopeStrongVectorIterator make(Args&&... args) {
				return TXScopeStrongVectorIterator(std::forward<Args>(args)...);
			}

		private:
			auto vector_refcptr() { return (*this).target_container_ptr(); }
			auto vector_refcptr() const { return (*this).target_container_ptr(); }
		};

		template <class X, class... Args>
		TXScopeStrongVectorIterator<X> make_xscope_strong_vector_iterator(Args&&... args) {
			return TXScopeStrongVectorIterator<X>::make(std::forward<Args>(args)...);
		}

		/* TXScopeStrongVectorIterator<> does not directly convert to mse::rsv::TFParam<mse::TXScopeCSSSXSTERandomAccessIterator<> >.
		But the following function can be used to obtain a "locking" scope iterator that does. */
		template <class X>
		auto make_xscope_locking_vector_iterator(const TXScopeStrongVectorIterator<X>& xs_strong_iter) {
			auto retval = mse::make_xscope_begin_iterator(mse::us::unsafe_make_xscope_pointer_to(*xs_strong_iter.target_container_ptr()));
			retval += xs_strong_iter.position();
			return retval;
		}
		template <class X>
		auto make_xscope_locking_vector_iterator(TXScopeStrongVectorIterator<X>&& xs_strong_iter) = delete;

		/* deprecated aliases */
		template <typename _Ty>
		using TIPointerWithBundledVector = TStrongVectorIterator<_Ty>;
		template <class X, class... Args>
		TIPointerWithBundledVector<X> make_ipointer_with_bundled_vector(Args&&... args) {
			return make_string_vector_iterator(std::forward<Args>(args)...);
		}


		template <typename _Ty, size_t _Size>
		class TNativeArrayReplacement : public mse::mstd::array<_Ty, _Size> {
		public:
			typedef mse::mstd::array<_Ty, _Size> base_class;
			using base_class::base_class;

			operator mse::TNullableAnyRandomAccessIterator<_Ty>() {
				return base_class::begin();
			}
			operator mse::TAnyRandomAccessIterator<_Ty>() {
				return base_class::begin();
			}
			operator mse::TAnyRandomAccessConstIterator<_Ty>() const {
				return base_class::cbegin();
			}
			operator typename mse::mstd::array<_Ty, _Size>::iterator() {
				return base_class::begin();
			}
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value), void>::type>
			operator typename mse::mstd::array<_Ty, _Size>::const_iterator() {
				return base_class::cbegin();
			}
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_const<_Ty2>::value), void>::type>
			operator mse::TNullableAnyRandomAccessIterator<const _Ty>() {
				return base_class::begin();
			}
			typename base_class::iterator operator+(typename base_class::difference_type n) { return base_class::begin() + n; }
			typename base_class::iterator operator-(typename base_class::difference_type n) { return base_class::begin() - n; }
			typename base_class::difference_type operator-(const typename base_class::iterator& _Right_cref) const { return base_class::begin() - _Right_cref; }
			typename base_class::const_iterator operator+(typename base_class::difference_type n) const { return base_class::cbegin() + n; }
			typename base_class::const_iterator operator-(typename base_class::difference_type n) const { return base_class::cbegin() - n; }
			template<class _Ty2 = _Ty, class = typename std::enable_if<(std::is_same<_Ty2, _Ty>::value)
				&& (!std::is_same<typename mse::mstd::array<_Ty2, _Size>::const_iterator, typename mse::mstd::array<_Ty2, _Size>::iterator>::value), void>::type>
			typename base_class::difference_type operator-(const typename base_class::const_iterator& _Right_cref) const { return base_class::cbegin() - _Right_cref; }

#ifdef MSE_LEGACYHELPERS_DISABLED
			TNativeArrayReplacement(_XSTD initializer_list<_Ty> _Ilist) : base_class(mse::nii_array<_Ty, _Size>(_Ilist)) {}
#endif // MSE_LEGACYHELPERS_DISABLED

		};

		template<class _Ty>
		class CAllocF {
		public:
			static void free(_Ty& ptr);
			static void allocate(_Ty& ptr, size_t num_bytes);
			static void reallocate(_Ty& ptr, size_t num_bytes);
		};
		template<class _Ty>
		class CAllocF<_Ty*> {
		public:
			static void free(_Ty* ptr) {
				::free(ptr);
			}
			static void allocate(_Ty*& ptr, size_t num_bytes) {
				ptr = ::malloc(num_bytes);
			}
			static void reallocate(_Ty*& ptr, size_t num_bytes) {
				ptr = ::realloc(ptr, num_bytes);
			}
		};
		template<class _Ty>
		class CAllocF<mse::lh::TStrongVectorIterator<_Ty>> {
		public:
			static void free(mse::lh::TStrongVectorIterator<_Ty>& ptr) {
				ptr = mse::lh::TStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			static void reallocate(mse::lh::TStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
		};
		template<class _Ty>
		class CAllocF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
		public:
			static void free(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr) {
				ptr = mse::lh::TStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			//static void reallocate(mse::TNullableAnyRandomAccessIterator<_Ty>& ptr, size_t num_bytes);
		};
		template<class _Ty>
		class CAllocF<mse::lh::TXScopeStrongVectorIterator<_Ty>> {
		public:
			static void free(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr) {
				ptr = mse::lh::TXScopeStrongVectorIterator<_Ty>();
			}
			static void allocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				mse::lh::TXScopeStrongVectorIterator<_Ty> tmp(num_bytes / sizeof(_Ty));
				ptr = tmp;
			}
			static void reallocate(mse::lh::TXScopeStrongVectorIterator<_Ty>& ptr, size_t num_bytes) {
				ptr.resize(num_bytes / sizeof(_Ty));
			}
		};
		template<class _Ty>
		class CAllocF<mse::TRefCountingPointer<_Ty>> {
		public:
			static void free(mse::TRefCountingPointer<_Ty>& ptr) {
				ptr = nullptr;
			}
			static void allocate(mse::TRefCountingPointer<_Ty>& ptr, size_t num_bytes) {
				ptr = mse::TRefCountingPointer<_Ty>();
			}
			static void reallocate(mse::TRefCountingPointer<_Ty>& ptr, size_t num_bytes) {
				if (0 == num_bytes) {
					ptr = nullptr;
				}
				else {
					assert(false);
				}
			}
		};
		template<class _Ty>
		class CAllocF<mse::TNullableAnyPointer<_Ty>> {
		public:
			static void free(mse::TNullableAnyPointer<_Ty>& ptr) {
				ptr = nullptr;
			}
			static void allocate(mse::TNullableAnyPointer<_Ty>& ptr, size_t num_bytes) {
				if (0 == num_bytes) {
					ptr = nullptr;
				}
				else if (sizeof(_Ty) == num_bytes) {
					ptr = mse::make_refcounting<_Ty>();
				}
				else {
					assert(false);
					ptr = mse::make_refcounting<_Ty>();
					//MSE_THROW(std::bad_alloc("the given allocation size is not supported for this pointer type - CAllocF<mse::TNullableAnyPointer<_Ty>>::allocate()"));
				}
			}
			//static void reallocate(mse::TNullableAnyPointer<_Ty>& ptr, size_t num_bytes);
		};

		template<class _TDynArrayIter>
		_TDynArrayIter& allocate(_TDynArrayIter& ptr, size_t num_bytes) {
			CAllocF<_TDynArrayIter>::allocate(ptr, num_bytes);
			return ptr;
		}
		template<class _TDynArrayIter>
		auto reallocate(const _TDynArrayIter& ptr2, size_t num_bytes) {
			auto ptr = ptr2;
			CAllocF<_TDynArrayIter>::reallocate(ptr, num_bytes);
			return ptr;
		}
		template<class _TDynArrayIter>
		_TDynArrayIter allocate_dyn_array1(size_t num_bytes) {
			_TDynArrayIter ptr;
			CAllocF<_TDynArrayIter>::allocate(ptr, num_bytes);
			return ptr;
		}

		template<class _Ty>
		class CFileF {
		public:
			static size_t fread(_Ty ptr, size_t size, size_t count, FILE * stream);
			static size_t fwrite(_Ty ptr, size_t size, size_t count, FILE * stream);
		};
		template<class _Ty>
		class CFileF<_Ty*> {
		public:
			static size_t fread(_Ty* ptr, size_t size, size_t count, FILE * stream) {
				return ::fread(ptr, size, count, stream);
			}
			static size_t fwrite(_Ty* ptr, size_t size, size_t count, FILE * stream) {
				return ::fwrite(ptr, size, count, stream);
			}
		};
		template<class _Ty>
		class CFileF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
		public:
			static size_t fread(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, size_t size, size_t count, FILE * stream) {
				static std::vector<unsigned char> v;
				v.resize(size * count);
				auto num_bytes_read = ::fread(v.data(), size, count, stream);
				auto num_items_read = num_bytes_read / sizeof(_Ty);
				size_t uc_index = 0;
				size_t Ty_index = 0;
				for (; Ty_index < num_items_read; uc_index += sizeof(_Ty), Ty_index += 1) {
					unsigned char* uc_ptr = &(v[uc_index]);
					_Ty* Ty_ptr = reinterpret_cast<_Ty*>(uc_ptr);
					ptr[Ty_index] = (*Ty_ptr);
				}
				v.resize(0);
				return num_bytes_read;
			}
			static size_t fwrite(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, size_t size, size_t count, FILE * stream) {
				typedef typename std::remove_const<_Ty>::type non_const_Ty;
				static std::vector<unsigned char> v;
				v.resize(size * count);
				auto num_items_to_write = size * count / sizeof(_Ty);
				size_t uc_index = 0;
				size_t Ty_index = 0;
				for (; Ty_index < num_items_to_write; uc_index += sizeof(_Ty), Ty_index += 1) {
					unsigned char* uc_ptr = &(v[uc_index]);
					non_const_Ty* Ty_ptr = reinterpret_cast<non_const_Ty*>(uc_ptr);
					(*Ty_ptr) = ptr[Ty_index];
				}
				auto res = ::fwrite(v.data(), size, count, stream);
				v.resize(0);
				return res;
			}
		};

		template<class _Ty>
		class CMemF {
		public:
			static void memcpy(_Ty destination, _Ty source, size_t num);
			static void memset(_Ty ptr, int value, size_t num);
		};
		template<class _Ty>
		class CMemF<_Ty*> {
		public:
			static void memcpy(_Ty* destination, _Ty* source, size_t num) {
				::memcpy(destination, source, num);
			}
			static void memset(_Ty ptr, int value, size_t num) {
				::memset(ptr, value, num);
			}
		};
		template<class _Ty>
		class CMemF<mse::TNullableAnyRandomAccessIterator<_Ty>> {
		public:
			static void memcpy(mse::TNullableAnyRandomAccessIterator<_Ty> destination, mse::TNullableAnyRandomAccessIterator<_Ty> source, size_t num_bytes) {
				auto num_items = num_bytes / sizeof(_Ty);
				for (size_t i = 0; i < num_items; i += 1) {
					destination[i] = source[i];
				}
			}
			static void memset(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, int value, size_t num_bytes) {
				auto Ty_value = _Ty(value);
				auto num_items = num_bytes / sizeof(_Ty);
				for (size_t i = 0; i < num_items; i += 1) {
					ptr[i] = Ty_value;
				}
			}
		};
		template<class _Ty>
		class CMemF<mse::lh::TIPointerWithBundledVector<_Ty>> {
		public:
			static void memcpy(mse::TNullableAnyRandomAccessIterator<_Ty> destination, mse::TNullableAnyRandomAccessIterator<_Ty> source, size_t num_bytes) {
				CMemF< mse::TNullableAnyRandomAccessIterator<_Ty> >::memcpy(destination, source, num_bytes);
			}
			static void memset(mse::TNullableAnyRandomAccessIterator<_Ty> ptr, int value, size_t num_bytes) {
				CMemF< mse::TNullableAnyRandomAccessIterator<_Ty> >::memset(ptr, value, num_bytes);
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )  
#endif /*_MSC_VER*/

#endif // MSELEGACYHELPERS_H_
